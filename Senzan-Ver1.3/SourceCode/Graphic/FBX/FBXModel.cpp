#include "Graphic/FBX/FBXModel.h"
#include <algorithm>
#include <iostream>
#include <assimp/postprocess.h>

constexpr int MAX_BONES = 128;

FBXModel::FBXModel()
	: m_Meshes()
	, m_BoneMapping()
	, m_BoneInfos()
	, m_Importer()
	, m_pScene(nullptr)
	, m_Directory()
	, m_AnimationInfos()
	, m_AnimationTime(0.0f)
	, m_CurrentAnimation(0)
	, m_AnimationCount(0)
	, m_IsBones(false)
	, m_WorldMatrix(DirectX::XMMatrixIdentity())
	, m_ViewMatrix(DirectX::XMMatrixIdentity())
	, m_ProjectionMatrix(DirectX::XMMatrixIdentity())
	, m_GlobalInverseTransform(DirectX::XMMatrixIdentity())
	, m_pPipelineState(nullptr)
	, m_pRootSignature(nullptr)
	, m_pCBWorld(nullptr)
	, m_pCBSkeleton(nullptr)
	, m_pCBMaterial(nullptr)
	, m_pSrvHeap(nullptr)
{
}

FBXModel::~FBXModel() noexcept
{
	Release();
}

bool FBXModel::Load(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, const std::string& FilePath)
{
	if (!pDevice || !pCommandQueue) { return false; }

	unsigned int flags =
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_LimitBoneWeights;

	m_pScene = m_Importer.ReadFile(FilePath, flags);
	if (!m_pScene || m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode)
	{
		std::string msg = "[FBXModel::Load] ERROR: Assimp failed to load: " + FilePath + "\n";
		OutputDebugStringA(msg.c_str());
		return false;
	}

	std::string msg = "[FBXModel::Load] SUCCESS: Loaded FBX: " + FilePath + "\n";
	OutputDebugStringA(msg.c_str());

	size_t last_slash = FilePath.find_last_of("/\\");
	m_Directory = (last_slash != std::string::npos) ? FilePath.substr(0, last_slash + 1) : "";

	m_GlobalInverseTransform = DirectX::XMMatrixInverse(nullptr, ConvertMatrix(m_pScene->mRootNode->mTransformation));

	m_AnimationCount = m_pScene->mNumAnimations;
	msg = "[FBXModel::Load] Animation count: " + std::to_string(m_AnimationCount) + "\n";
	OutputDebugStringA(msg.c_str());

	BuildAnimationCache();
	ProcessNode(pDevice, pCommandQueue, m_pScene->mRootNode, m_pScene, DirectX::XMMatrixIdentity());

	msg = "[FBXModel::Load] Mesh count: " + std::to_string(m_Meshes.size()) + "\n";
	OutputDebugStringA(msg.c_str());
	
	if (!CreateShaders(pDevice)) { return false; }
	if (!CreateConstantBuffers(pDevice)) { return false; }

	OutputDebugStringA("[FBXModel::Load] COMPLETED\n");
	return true;
}

void FBXModel::Update(float DeltaTime)
{
	if (!m_pScene || !m_IsBones) { return; }
	if (m_AnimationCount == 0) { return; }
	if (m_CurrentAnimation >= m_AnimationCount) { return; }

	const auto& anim_info = m_AnimationInfos[m_CurrentAnimation];
	float time_in_ticks = DeltaTime * anim_info.TicksPerSecond;
	m_AnimationTime += time_in_ticks;

	if (m_AnimationTime >= anim_info.Duration)
	{
		m_AnimationTime = fmod(m_AnimationTime, anim_info.Duration);
	}

	CalculateBoneTransforms(m_AnimationTime, m_pScene->mRootNode, DirectX::XMMatrixIdentity());
}

void FBXModel::Render(ID3D12GraphicsCommandList* pCommandList)
{
	if (!pCommandList) 
	{ 
		OutputDebugStringA("[FBXModel::Render] ERROR: pCommandList is null\n");
		return; 
	}

	if (m_Meshes.empty())
	{
		OutputDebugStringA("[FBXModel::Render] WARNING: No meshes to render\n");
		return;
	}

	// PSO と Root Signature をセット.
	if (m_pPipelineState)
	{
		pCommandList->SetPipelineState(m_pPipelineState);
	}

	if (m_pRootSignature)
	{
		pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	}

	// ===== 定数バッファを更新 =====
	if (m_pCBWorld)
	{
		cbWorld* p_cb_world = nullptr;
		D3D12_RANGE read_range = { 0, 0 };
		if (SUCCEEDED(m_pCBWorld->Map(0, &read_range, (void**)&p_cb_world)))
		{
			p_cb_world->World = DirectX::XMMatrixTranspose(m_WorldMatrix);
			p_cb_world->View = DirectX::XMMatrixTranspose(m_ViewMatrix);
			p_cb_world->Projection = DirectX::XMMatrixTranspose(m_ProjectionMatrix);
			m_pCBWorld->Unmap(0, nullptr);
		}

		// b0: World/View/Projection
		pCommandList->SetGraphicsRootConstantBufferView(0, m_pCBWorld->GetGPUVirtualAddress());
	}

	if (m_pCBSkeleton)
	{
		cbSkeleton* p_cb_skeleton = nullptr;
		D3D12_RANGE read_range = { 0, 0 };
		if (SUCCEEDED(m_pCBSkeleton->Map(0, &read_range, (void**)&p_cb_skeleton)))
		{
			// ボーン行列を転置して設定.
			for (size_t i = 0; i < m_BoneInfos.size() && i < 128; ++i)
			{
				p_cb_skeleton->BoneTransforms[i] = DirectX::XMMatrixTranspose(m_BoneInfos[i].FinalTransform * m_WorldMatrix);
			}
			m_pCBSkeleton->Unmap(0, nullptr);
		}

		// b1: Bone transforms
		pCommandList->SetGraphicsRootConstantBufferView(1, m_pCBSkeleton->GetGPUVirtualAddress());
	}

	std::string msg = "[FBXModel::Render] Rendering " + std::to_string(m_Meshes.size()) + " meshes\n";
	OutputDebugStringA(msg.c_str());

	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		if (m_Meshes[i])
		{
			// メッシュごとにマテリアル定数バッファを更新.
			if (m_pCBMaterial)
			{
				cbMaterial* p_cb_material = nullptr;
				D3D12_RANGE read_range = { 0, 0 };
				if (SUCCEEDED(m_pCBMaterial->Map(0, &read_range, (void**)&p_cb_material)))
				{
					const auto& material = m_Meshes[i]->GetMaterial();
					p_cb_material->Diffuse = material.Diffuse;
					p_cb_material->Ambient = material.Ambient;
					p_cb_material->Specular = material.Specular;
					p_cb_material->Shininess = material.Shininess;
					m_pCBMaterial->Unmap(0, nullptr);
				}

				// b2: Material
				pCommandList->SetGraphicsRootConstantBufferView(2, m_pCBMaterial->GetGPUVirtualAddress());
			}

			if (!m_Meshes[i]->Render(pCommandList))
			{
				msg = "[FBXModel::Render] Mesh[" + std::to_string(i) + "] render failed\n";
				OutputDebugStringA(msg.c_str());
			}
		}
	}
}

void FBXModel::Release()
{
	for (auto* mesh : m_Meshes)
	{
		if (mesh)
		{
			mesh->Release();
			delete mesh;
		}
	}
	m_Meshes.clear();

	if (m_pCBMaterial) { m_pCBMaterial->Release(); m_pCBMaterial = nullptr; }
	if (m_pCBSkeleton) { m_pCBSkeleton->Release(); m_pCBSkeleton = nullptr; }
	if (m_pCBWorld) { m_pCBWorld->Release(); m_pCBWorld = nullptr; }
	if (m_pSrvHeap) { m_pSrvHeap->Release(); m_pSrvHeap = nullptr; }
	if (m_pPipelineState) { m_pPipelineState->Release(); m_pPipelineState = nullptr; }
	if (m_pRootSignature) { m_pRootSignature->Release(); m_pRootSignature = nullptr; }

	m_BoneInfos.clear();
	m_BoneMapping.clear();
	m_AnimationInfos.clear();
	m_pScene = nullptr;
}

bool FBXModel::RayCast(const FBXMesh::Ray& Ray, FBXMesh::HitInfo& OutHit) const
{
	bool hit_found = false;
	float closest_distance = FLT_MAX;
	FBXMesh::HitInfo closest_hit = {};

	for (const auto* mesh : m_Meshes)
	{
		if (mesh)
		{
			FBXMesh::HitInfo mesh_hit = {};
			if (mesh->IsRayCast(Ray, mesh_hit))
			{
				hit_found = true;
				if (mesh_hit.Distance < closest_distance)
				{
					closest_distance = mesh_hit.Distance;
					closest_hit = mesh_hit;
				}
			}
		}
	}

	if (hit_found)
	{
		OutHit = closest_hit;
	}

	return hit_found;
}

bool FBXModel::SetAnimationIndex(uint32_t Index)
{
	if (Index < m_AnimationCount)
	{
		m_CurrentAnimation = Index;
		m_AnimationTime = 0.0f;
		return true;
	}
	return false;
}

bool FBXModel::SetAnimationByName(const std::string& AnimationName)
{
	for (const auto& anim_info : m_AnimationInfos)
	{
		if (anim_info.Name == AnimationName)
		{
			SetAnimationIndex(anim_info.Index);
			return true;
		}
	}
	return false;
}

const std::string& FBXModel::GetCurrentAnimationName() const
{
	if (m_CurrentAnimation < m_AnimationInfos.size())
	{
		return m_AnimationInfos[m_CurrentAnimation].Name;
	}
	static const std::string empty_name;
	return empty_name;
}

float FBXModel::GetAnimationDuration(uint32_t Index) const
{
	if (Index < m_AnimationInfos.size())
	{
		return m_AnimationInfos[Index].Duration / m_AnimationInfos[Index].TicksPerSecond;
	}
	return 0.0f;
}

std::string FBXModel::GetAnimationName(uint32_t Index) const
{
	if (Index < m_AnimationInfos.size())
	{
		return m_AnimationInfos[Index].Name;
	}
	return "";
}

std::vector<std::string> FBXModel::GetAnimationNames() const
{
	std::vector<std::string> names;
	names.reserve(m_AnimationInfos.size());
	for (const auto& info : m_AnimationInfos)
	{
		names.push_back(info.Name);
	}
	return names;
}

bool FBXModel::GetBoneWorldMatrix(const std::string& BoneName, DirectX::XMMATRIX& OutMatrix) const
{
	auto it = m_BoneMapping.find(BoneName);
	if (it == m_BoneMapping.end())
	{
		return false;
	}

	uint32_t bone_index = it->second;
	if (bone_index >= m_BoneInfos.size())
	{
		return false;
	}

	OutMatrix = m_BoneInfos[bone_index].GlobalTransform * m_WorldMatrix;
	return true;
}

bool FBXModel::GetBoneWorldMatrixByIndex(uint32_t BoneIndex, DirectX::XMMATRIX& OutMatrix) const
{
	if (BoneIndex >= m_BoneInfos.size())
	{
		return false;
	}

	OutMatrix = m_BoneInfos[BoneIndex].GlobalTransform * m_WorldMatrix;
	return true;
}

bool FBXModel::GetBoneWorldPosition(const std::string& BoneName, DirectX::XMFLOAT3& OutPosition) const
{
	DirectX::XMMATRIX world_mat = {};
	if (!GetBoneWorldMatrix(BoneName, world_mat))
	{
		return false;
	}

	DirectX::XMVECTOR v_pos = world_mat.r[3];
	DirectX::XMStoreFloat3(&OutPosition, v_pos);
	return true;
}

int32_t FBXModel::GetBoneIndex(const std::string& BoneName) const
{
	auto it = m_BoneMapping.find(BoneName);
	if (it != m_BoneMapping.end())
	{
		return static_cast<int32_t>(it->second);
	}
	return -1;
}

std::vector<std::string> FBXModel::GetBoneNames() const
{
	std::vector<std::string> names;
	names.reserve(m_BoneMapping.size());
	for (const auto& kv : m_BoneMapping)
	{
		names.push_back(kv.first);
	}
	return names;
}

void FBXModel::ProcessNode(
	ID3D12Device* pDevice,
	ID3D12CommandQueue* pCommandQueue,
	aiNode* pNode,
	const aiScene* pScene,
	const DirectX::XMMATRIX& ParentTransform)
{
	DirectX::XMMATRIX node_transform = ConvertMatrix(pNode->mTransformation);
	DirectX::XMMATRIX global_transform = node_transform * ParentTransform;

	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* p_mesh = pScene->mMeshes[pNode->mMeshes[i]];
		FBXMesh* fbx_mesh = ProcessMesh(pDevice, pCommandQueue, p_mesh, pScene, global_transform);
		if (fbx_mesh)
		{
			m_Meshes.push_back(fbx_mesh);
		}
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pDevice, pCommandQueue, pNode->mChildren[i], pScene, global_transform);
	}
}

FBXMesh* FBXModel::ProcessMesh(
	ID3D12Device* pDevice,
	ID3D12CommandQueue* pCommandQueue,
	aiMesh* pMesh,
	const aiScene* pScene,
	const DirectX::XMMATRIX& NodeTransform)
{
	if (!pDevice || !pCommandQueue || !pMesh) { return nullptr; }

	std::vector<FBXMesh::Vertex> vertices;
	vertices.reserve(pMesh->mNumVertices);

	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		FBXMesh::Vertex vertex = {};

		if (pMesh->HasPositions())
		{
			vertex.Position.x = pMesh->mVertices[i].x;
			vertex.Position.y = pMesh->mVertices[i].y;
			vertex.Position.z = pMesh->mVertices[i].z;
		}

		if (pMesh->HasNormals())
		{
			vertex.Normal.x = pMesh->mNormals[i].x;
			vertex.Normal.y = pMesh->mNormals[i].y;
			vertex.Normal.z = pMesh->mNormals[i].z;
		}

		if (pMesh->HasTextureCoords(0))
		{
			vertex.UV.x = pMesh->mTextureCoords[0][i].x;
			vertex.UV.y = pMesh->mTextureCoords[0][i].y;
		}

		for (int j = 0; j < 4; ++j)
		{
			vertex.BoneIDs[j] = 0;
			vertex.Weights[j] = 0.0f;
		}

		vertices.push_back(vertex);
	}

	std::vector<uint32_t> indices;
	indices.reserve(pMesh->mNumFaces * 3);

	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
	{
		aiFace& face = pMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	LoadBones(pMesh, vertices);

	// スタティックメッシュ時は最低1本のウェイトを設定（BoneTransforms[0] を使用）.
	if (pMesh->mNumBones == 0)
	{
		for (auto& vertex : vertices)
		{
			vertex.BoneIDs[0] = 0;
			vertex.Weights[0] = 1.0f;
		}
	}

	FBXMesh::MaterialData material;
	if (pMesh->mMaterialIndex < pScene->mNumMaterials)
	{
		material = LoadMaterial(pDevice, pScene->mMaterials[pMesh->mMaterialIndex]);
	}

	FBXMesh* fbx_mesh = new FBXMesh();
	if (!fbx_mesh->CreateBuffers(pDevice, pCommandQueue, vertices, indices, material, true))
	{
		delete fbx_mesh;
		return nullptr;
	}

	return fbx_mesh;
}

void FBXModel::BuildAnimationCache()
{
	m_AnimationInfos.clear();
	m_AnimationInfos.reserve(m_AnimationCount);

	for (int i = 0; i < static_cast<int>(m_AnimationCount); ++i)
	{
		const aiAnimation* anim = m_pScene->mAnimations[i];

		AnimationInfo info = {};
		info.Name = anim->mName.C_Str();
		info.Duration = static_cast<float>(anim->mDuration);
		info.TicksPerSecond = (anim->mTicksPerSecond != 0.0) ? static_cast<float>(anim->mTicksPerSecond) : 25.0f;
		info.Index = i;

		for (unsigned int c = 0; c < anim->mNumChannels; ++c)
		{
			const aiNodeAnim* node_anim = anim->mChannels[c];
			info.NodeAnimCache[node_anim->mNodeName.C_Str()] = node_anim;
		}

		m_AnimationInfos.push_back(std::move(info));
	}
}

void FBXModel::CalculateBoneTransforms(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform)
{
	if (!pNode || m_CurrentAnimation >= m_AnimationCount) { return; }

	DirectX::XMMATRIX node_transform = ConvertMatrix(pNode->mTransformation);

	const aiNodeAnim* node_anim = FindNodeAnim(m_CurrentAnimation, pNode->mName.C_Str());
	if (node_anim)
	{
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		if (node_anim->mNumScalingKeys > 0)
		{
			uint32_t scale_index = 0;
			for (uint32_t i = 0; i < node_anim->mNumScalingKeys - 1; ++i)
			{
				if (AnimationTime < node_anim->mScalingKeys[i + 1].mTime)
				{
					scale_index = i;
					break;
				}
			}
			const aiVectorKey& scale_key = node_anim->mScalingKeys[scale_index];
			scale.x = scale_key.mValue.x;
			scale.y = scale_key.mValue.y;
			scale.z = scale_key.mValue.z;
		}

		DirectX::XMVECTOR v_rot = DirectX::XMQuaternionIdentity();
		if (node_anim->mNumRotationKeys > 0)
		{
			uint32_t rot_index = 0;
			for (uint32_t i = 0; i < node_anim->mNumRotationKeys - 1; ++i)
			{
				if (AnimationTime < node_anim->mRotationKeys[i + 1].mTime)
				{
					rot_index = i;
					break;
				}
			}
			const aiQuatKey& rot_key = node_anim->mRotationKeys[rot_index];
			v_rot = DirectX::XMVectorSet(rot_key.mValue.x, rot_key.mValue.y, rot_key.mValue.z, rot_key.mValue.w);
		}

		DirectX::XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
		if (node_anim->mNumPositionKeys > 0)
		{
			uint32_t pos_index = 0;
			for (uint32_t i = 0; i < node_anim->mNumPositionKeys - 1; ++i)
			{
				if (AnimationTime < node_anim->mPositionKeys[i + 1].mTime)
				{
					pos_index = i;
					break;
				}
			}
			const aiVectorKey& pos_key = node_anim->mPositionKeys[pos_index];
			pos.x = pos_key.mValue.x;
			pos.y = pos_key.mValue.y;
			pos.z = pos_key.mValue.z;
		}

		DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		DirectX::XMMATRIX rot_matrix = DirectX::XMMatrixRotationQuaternion(v_rot);
		DirectX::XMMATRIX trans_matrix = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		node_transform = scale_matrix * rot_matrix * trans_matrix;
	}

	DirectX::XMMATRIX global_transform = node_transform * ParentTransform;

	auto it = m_BoneMapping.find(pNode->mName.C_Str());
	if (it != m_BoneMapping.end())
	{
		uint32_t bone_index = it->second;
		m_BoneInfos[bone_index].GlobalTransform = global_transform;
		m_BoneInfos[bone_index].FinalTransform =
			m_BoneInfos[bone_index].OffsetMatrix * global_transform * m_GlobalInverseTransform;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		CalculateBoneTransforms(AnimationTime, pNode->mChildren[i], global_transform);
	}
}

const aiNodeAnim* FBXModel::FindNodeAnim(uint32_t AnimationIndex, const std::string& NodeName)
{
	if (AnimationIndex >= m_AnimationInfos.size())
	{
		return nullptr;
	}

	const auto& anim_info = m_AnimationInfos[AnimationIndex];
	auto it = anim_info.NodeAnimCache.find(NodeName);
	if (it != anim_info.NodeAnimCache.end())
	{
		return it->second;
	}
	return nullptr;
}

DirectX::XMMATRIX FBXModel::ConvertMatrix(const aiMatrix4x4& Matrix)
{
	return DirectX::XMMATRIX(
		Matrix.a1, Matrix.b1, Matrix.c1, Matrix.d1,
		Matrix.a2, Matrix.b2, Matrix.c2, Matrix.d2,
		Matrix.a3, Matrix.b3, Matrix.c3, Matrix.d3,
		Matrix.a4, Matrix.b4, Matrix.c4, Matrix.d4);
}

bool FBXModel::CreateShaders(ID3D12Device* pDevice)
{
	return pDevice != nullptr;
}

bool FBXModel::CreateConstantBuffers(ID3D12Device* pDevice)
{
	if (!pDevice)
	{
		return false;
	}

	auto create_cb = [pDevice](size_t size_in_bytes, ID3D12Resource** pp_resource) -> bool
	{
		if (!pp_resource)
		{
			return false;
		}

		const UINT64 aligned_size = (static_cast<UINT64>(size_in_bytes) + 255ull) & ~255ull;

		D3D12_HEAP_PROPERTIES heap_prop = {};
		heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC res_desc = {};
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = aligned_size;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (FAILED(pDevice->CreateCommittedResource(
			&heap_prop,
			D3D12_HEAP_FLAG_NONE,
			&res_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pp_resource))))
		{
			return false;
		}

		return true;
	};

	if (!create_cb(sizeof(cbWorld), &m_pCBWorld))
	{
		OutputDebugStringA("[FBXModel::CreateConstantBuffers] Failed to create cbWorld\n");
		return false;
	}

	if (!create_cb(sizeof(cbSkeleton), &m_pCBSkeleton))
	{
		OutputDebugStringA("[FBXModel::CreateConstantBuffers] Failed to create cbSkeleton\n");
		return false;
	}

	if (!create_cb(sizeof(cbMaterial), &m_pCBMaterial))
	{
		OutputDebugStringA("[FBXModel::CreateConstantBuffers] Failed to create cbMaterial\n");
		return false;
	}

	// 初期値設定（未更新フレームでも不定値を避ける）.
	{
		cbWorld* p_cb_world = nullptr;
		if (SUCCEEDED(m_pCBWorld->Map(0, nullptr, reinterpret_cast<void**>(&p_cb_world))) && p_cb_world)
		{
			p_cb_world->World = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
			p_cb_world->View = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
			p_cb_world->Projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
			m_pCBWorld->Unmap(0, nullptr);
		}
	}

	{
		cbSkeleton* p_cb_skeleton = nullptr;
		if (SUCCEEDED(m_pCBSkeleton->Map(0, nullptr, reinterpret_cast<void**>(&p_cb_skeleton))) && p_cb_skeleton)
		{
			for (int i = 0; i < MAX_BONES; ++i)
			{
				p_cb_skeleton->BoneTransforms[i] = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
			}
			m_pCBSkeleton->Unmap(0, nullptr);
		}
	}

	{
		cbMaterial* p_cb_material = nullptr;
		if (SUCCEEDED(m_pCBMaterial->Map(0, nullptr, reinterpret_cast<void**>(&p_cb_material))) && p_cb_material)
		{
			p_cb_material->Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			p_cb_material->Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			p_cb_material->Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			p_cb_material->Shininess = 32.0f;
			p_cb_material->Padding[0] = 0.0f;
			p_cb_material->Padding[1] = 0.0f;
			p_cb_material->Padding[2] = 0.0f;
			m_pCBMaterial->Unmap(0, nullptr);
		}
	}

	OutputDebugStringA("[FBXModel::CreateConstantBuffers] Constant buffers created\n");
	return true;
}

bool FBXModel::CreateInputLayout(ID3D12Device* pDevice)
{
	return pDevice != nullptr;
}

FBXMesh::MaterialData FBXModel::LoadMaterial(ID3D12Device* pDevice, aiMaterial* pMaterial)
{
	FBXMesh::MaterialData material = {};
	if (!pDevice || !pMaterial) { return material; }

	aiColor3D diffuse(1.0f, 1.0f, 1.0f);
	aiColor3D ambient(0.2f, 0.2f, 0.2f);
	aiColor3D specular(1.0f, 1.0f, 1.0f);
	float shininess = 32.0f;

	pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
	pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	pMaterial->Get(AI_MATKEY_SHININESS, shininess);

	material.Diffuse = DirectX::XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, 1.0f);
	material.Ambient = DirectX::XMFLOAT4(ambient.r, ambient.g, ambient.b, 1.0f);
	material.Specular = DirectX::XMFLOAT4(specular.r, specular.g, specular.b, 1.0f);
	material.Shininess = shininess;

	if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString path = {};
		pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		std::string texture_path = m_Directory + path.C_Str();
		LoadTexture(pDevice, texture_path, material);
	}

	return material;
}

void FBXModel::LoadTexture(ID3D12Device* pDevice, const std::string& TexturePath, FBXMesh::MaterialData& Material)
{
	if (!pDevice || TexturePath.empty()) { return; }
	Material.TexturePath = std::wstring(TexturePath.begin(), TexturePath.end());
}

void FBXModel::LoadBones(aiMesh* pMesh, std::vector<FBXMesh::Vertex>& Vertices)
{
	if (!pMesh || pMesh->mNumBones == 0) { return; }

	for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
	{
		aiBone* bone = pMesh->mBones[i];
		std::string bone_name = bone->mName.C_Str();

		if (m_BoneMapping.find(bone_name) == m_BoneMapping.end())
		{
			uint32_t bone_index = static_cast<uint32_t>(m_BoneInfos.size());
			m_BoneMapping[bone_name] = bone_index;
			m_BoneInfos.push_back(FBXModel::BoneInfo());
			m_BoneInfos[bone_index].OffsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
		}

		uint32_t bone_index = m_BoneMapping[bone_name];

		for (unsigned int j = 0; j < bone->mNumWeights; ++j)
		{
			uint32_t vertex_id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;

			if (vertex_id >= Vertices.size())
			{
				continue;
			}

			for (int k = 0; k < 4; ++k)
			{
				if (Vertices[vertex_id].Weights[k] == 0.0f)
				{
					Vertices[vertex_id].BoneIDs[k] = bone_index;
					Vertices[vertex_id].Weights[k] = weight;
					break;
				}
			}
		}
	}

	m_IsBones = !m_BoneMapping.empty();
}
