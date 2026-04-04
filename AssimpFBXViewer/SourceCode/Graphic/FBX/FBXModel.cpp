#include "Graphic/FBX/FBXModel.h"
#include <algorithm>
#include <iostream>
#include <wincodec.h>
#include <assimp/postprocess.h>

#pragma comment(lib, "windowscodecs.lib")

constexpr const wchar_t* SHADER_PATH = L"Skinning.hlsl";
constexpr int MAX_BONES = 128;

// コンストラクタ.
FBXModel::FBXModel()
    : m_Meshes			()
    , m_BoneMapping		()
    , m_BoneInfos		()
    , m_Importer		()
    , m_pScene			( nullptr )
    , m_Directory		()
    , m_AnimationInfos	()
    , m_AnimationTime	( 0.0f )
    , m_CurrentAnimation( 0 )
    , m_AnimationCount	( 0 )
    , m_IsBones			( false )
    , m_WorldMatrix				( DirectX::XMMatrixIdentity() )
    , m_ViewMatrix				( DirectX::XMMatrixIdentity() )
    , m_ProjectionMatrix		( DirectX::XMMatrixIdentity() )
    , m_GlobalInverseTransform	( DirectX::XMMatrixIdentity() )
    , m_pVertexShader	( nullptr )
    , m_pPixelShader	( nullptr )
    , m_pInputLayout	( nullptr )
    , m_pCBWorld		( nullptr )
    , m_pCBSkeleton		( nullptr )
    , m_pCBMaterial		( nullptr )
    , m_pSamplerState	( nullptr )
{
}

// デストラクタ.
FBXModel::~FBXModel() noexcept
{
	Release();
}

// FBXファイルをロード.
bool FBXModel::Load(DirectX11* pDX11, const std::string& FilePath)
{
	if (!pDX11) { return false; }

	// Assimpフラグ設定.
	unsigned int flags =				
		aiProcess_Triangulate |			// すべてのメッシュのプリミティブ型を三角形に変換.
		aiProcess_GenNormals |			// 法線が存在しない場合、スムーズな法線を生成.
		aiProcess_CalcTangentSpace |	// 接線と従法線を計算(通常マッピング用).
		aiProcess_JoinIdenticalVertices|// 同一頂点を結合し冗長性を排除.
		aiProcess_MakeLeftHanded |		// 左手座標系に変換(Direct3D用).
		aiProcess_FlipWindingOrder |	// ポリゴンのワインディングオーダーを反転.
		aiProcess_FlipUVs |				// UV座標のV成分を反転(D3D互換).
		aiProcess_LimitBoneWeights;		// ボーンの最大影響数を制限(GPUキャパシティ対応).

    // FBXファイル読み込み.
	m_pScene = m_Importer.ReadFile(FilePath, flags);
    if (!m_pScene || m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode)
    {
		std::cerr << "FBXModel::Load - 読み込みに失敗: " << FilePath << std::endl;
        return false;
    }

		  // パスを取得.
			size_t last_slash = FilePath.find_last_of("/\\");
			m_Directory = (last_slash != std::string::npos)
				? FilePath.substr(0, last_slash + 1)
				: "";

	// グローバル逆変換行列を取得.
	m_GlobalInverseTransform = DirectX::XMMatrixInverse(
		nullptr, 
		ConvertMatrix(m_pScene->mRootNode->mTransformation));

	// アニメーション数を取得.
	m_AnimationCount = m_pScene->mNumAnimations;

	// アニメーションキャッシュを構築.
	BuildAnimationCache();

	// ノード階層を処理.
	ProcessNode(pDX11, m_pScene->mRootNode, m_pScene, DirectX::XMMatrixIdentity());

#if _DEBUG
	// デバッグ: アニメーション情報を出力.
	std::cout << "=== アニメーション情報 ===" << std::endl;
	std::cout << "アニメーション数: " << m_AnimationCount << std::endl;
	for (const auto& anim_info : m_AnimationInfos)
	{
		std::cout << "アニメーション[" << anim_info.Index << "]: \"" << anim_info.Name << "\""
			<< " 期間=" << anim_info.Duration
			<< " 秒間ティック数=" << anim_info.TicksPerSecond << std::endl;
	}
	// 全ボーン名をコンソールに出力.
	if (!m_BoneMapping.empty())
	{
		std::cout << "ボーン数 (" << m_BoneMapping.size() << "):" << std::endl;
		for (const auto& kv : m_BoneMapping)
		{
			std::cout << "  " << kv.first << " -> " << kv.second << std::endl;
		}
	}
	std::cout << "===========================" << std::endl;

#endif

	// シェーダー作成.
	if (!CreateShaders(pDX11))
	{
		return false;
	}

	// 定数バッファ作成.
	if (!CreateConstantBuffers(pDX11))
	{
		return false;
	}

	std::cout << "FBXModel: " << m_Meshes.size() << "個のメッシュを読み込みました" << std::endl;

	return true;
}

// アニメーションキャッシュを構築.
void FBXModel::BuildAnimationCache()
{
	m_AnimationInfos.clear();
	m_AnimationInfos.reserve(m_AnimationCount);

	// アニメーションをループして情報をキャッシュ.
	for (int i = 0; i < m_AnimationCount; ++i)
	{
		const aiAnimation* anim = m_pScene->mAnimations[i];
		
		AnimationInfo info = {};
		info.Name = anim->mName.C_Str();
		info.Duration = static_cast<float>(anim->mDuration);
		// 1秒あたりのtick数を設定,0の場合はデフォルト値を使用.
		info.TicksPerSecond = (anim->mTicksPerSecond != 0.0) 
			? static_cast<float>(anim->mTicksPerSecond) : 25.0f;
		info.Index = i;

        // ノードアニメーションをキャッシュ.
        for (int c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim* node_anim = anim->mChannels[c];
            info.NodeAnimCache[node_anim->mNodeName.C_Str()] = node_anim;
        }

		m_AnimationInfos.push_back(std::move(info));
	}
}

// アニメーション更新.
void FBXModel::Update(float DeltaTime)
{
    if (!m_pScene || !m_IsBones) { return; }
    if (m_AnimationCount == 0) { return; }
    if (m_CurrentAnimation >= m_AnimationCount) { return; }

    // 現在のアニメーション情報を取得.
    const auto& anim_info = m_AnimationInfos[m_CurrentAnimation];

    // DeltaTime を ticks に変換して現在時間に加算.
    float time_in_ticks = DeltaTime * anim_info.TicksPerSecond;
    m_AnimationTime += time_in_ticks;

    // ループ処理(アニメーション長を超えたら折り返す).
    if (m_AnimationTime >= anim_info.Duration)
    {
        m_AnimationTime = fmod(m_AnimationTime, anim_info.Duration);
    }

    // ボーン変換を計算.
    CalculateBoneTransforms(m_AnimationTime, m_pScene->mRootNode, DirectX::XMMatrixIdentity());
}


// 描画.
void FBXModel::Render(DirectX11* pDX11)
{
	if (!pDX11) { return; }

    ID3D11DeviceContext* p_context = pDX11->GetContext();
    if (!p_context) { return; }

	if (!m_pVertexShader || !m_pPixelShader || !m_pInputLayout) { return; }

	if (!m_pCBWorld || !m_pCBSkeleton || !m_pCBMaterial) { return; }

	// シェーダー設定.
    p_context->VSSetShader(m_pVertexShader, nullptr, 0);
    p_context->PSSetShader(m_pPixelShader, nullptr, 0);
    p_context->IASetInputLayout(m_pInputLayout);

	// ワールド・ビュー・プロジェクション行列更新.
	cbWorld cb_world =
	{
		DirectX::XMMatrixTranspose(m_WorldMatrix),
		DirectX::XMMatrixTranspose(m_ViewMatrix),
		DirectX::XMMatrixTranspose(m_ProjectionMatrix)
	};
	p_context->UpdateSubresource(m_pCBWorld, 0, nullptr, &cb_world, 0, 0);
	p_context->VSSetConstantBuffers(0, 1, &m_pCBWorld);
	
	// ボーン行列更新.
    cbSkeleton cb_bone = {};
    for (int i = 0; i < MAX_BONES; ++i)
    {
        cb_bone.BoneTransforms[i] = DirectX::XMMatrixIdentity();
    }

	if (m_IsBones) {
        for (size_t i = 0; i < m_BoneInfos.size() && i < static_cast<size_t>(MAX_BONES); ++i)
        {
            cb_bone.BoneTransforms[i] = DirectX::XMMatrixTranspose(m_BoneInfos[i].FinalTransform);
        }
	}

	// ボーン行列の定数バッファを更新.
	p_context->UpdateSubresource(m_pCBSkeleton, 0, nullptr, &cb_bone, 0, 0);
	// 頂点シェーダーのスロット1にボーン定数バッファを設定.
	p_context->VSSetConstantBuffers(1, 1, &m_pCBSkeleton);

	// サンプラーステート設定.
	if (m_pSamplerState)
	{
		p_context->PSSetSamplers(0, 1, &m_pSamplerState);
	}

	// メッシュ数チェック.
	if (m_Meshes.empty())
	{
		std::cerr << "Render: 描画するメッシュがありません" << std::endl;
		return;
	}

	// 各メッシュを描画.
	for (auto* mesh : m_Meshes)
	{
		if (mesh)
		{
			// マテリアル定数バッファ更新.
			const FBXMesh::MaterialData& mat = mesh->GetMaterial();
			cbMaterial cb_material = {};
			cb_material.Diffuse = mat.Diffuse;
			cb_material.Ambient = mat.Ambient;
			cb_material.Specular = mat.Specular;
			cb_material.Shininess = mat.Shininess;
			p_context->UpdateSubresource(m_pCBMaterial, 0, nullptr, &cb_material, 0, 0);
			p_context->PSSetConstantBuffers(0, 1, &m_pCBMaterial);

			mesh->Render(pDX11);
		}
	}
}

// リソース解放.
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

	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}
	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
		m_pPixelShader = nullptr;
	}
	if (m_pVertexShader)
	{
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
	}

	if (m_pCBMaterial)
	{
		m_pCBMaterial->Release();
		m_pCBMaterial = nullptr;
	}
	if (m_pCBSkeleton)
	{
		m_pCBSkeleton->Release();
		m_pCBSkeleton = nullptr;
	}
	if (m_pCBWorld)
	{
		m_pCBWorld->Release();
		m_pCBWorld = nullptr;
	}

	if (m_pSamplerState)
	{
		m_pSamplerState->Release();
		m_pSamplerState = nullptr;
	}

	m_BoneInfos.clear();
	m_BoneMapping.clear();
	m_AnimationInfos.clear();
	m_pScene = nullptr;
}

// 番号で名で設定.
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

// アニメーション名で設定.
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

// アニメーション名一覧を取得.
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

// 指定インデックスのアニメーション名を取得.
std::string FBXModel::GetAnimationName(uint32_t Index) const
{
	if (Index < m_AnimationInfos.size())
	{
		return m_AnimationInfos[Index].Name;
	}
	return "";
}

// アニメーション再生時間を取得.
float FBXModel::GetAnimationDuration(uint32_t Index) const
{
	if (Index < m_AnimationInfos.size())
	{
		return m_AnimationInfos[Index].Duration / m_AnimationInfos[Index].TicksPerSecond;
	}
	return 0.0f;
}

// 現在選択中のアニメーション名を取得.
const std::string& FBXModel::GetCurrentAnimationName() const
{
	if (m_CurrentAnimation < m_AnimationInfos.size())
	{
		return m_AnimationInfos[m_CurrentAnimation].Name;
	}
	static const std::string empty_name;
	return empty_name;
}

// ボーン名からワールド行列を取得.
bool FBXModel::GetBoneWorldMatrix(
	const std::string& BoneName, 
	DirectX::XMMATRIX& OutMatrix) const
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

    // ボーンのグローバル変換 * モデルのワールド行列.
	OutMatrix = m_BoneInfos[bone_index].GlobalTransform * m_WorldMatrix;
    return true;
}

// ボーン名からワールド座標(位置)を取得.
bool FBXModel::GetBoneWorldPosition(
	const std::string& BoneName, 
	DirectX::XMFLOAT3& OutPosition) const
{
	DirectX::XMMATRIX world_mat = {};
    if (!GetBoneWorldMatrix(BoneName, world_mat))
    {
        return false;
    }

    // 行列の平行移動成分を取得.
    DirectX::XMVECTOR v_pos = world_mat.r[3];
    DirectX::XMStoreFloat3(&OutPosition, v_pos);
    return true;
}

// ボーンインデックスからワールド行列を取得.
bool FBXModel::GetBoneWorldMatrixByIndex(
	uint32_t BoneIndex, 
	DirectX::XMMATRIX& OutMatrix) const
{
    if (BoneIndex >= m_BoneInfos.size())
    {
        return false;
    }

    OutMatrix = m_BoneInfos[BoneIndex].GlobalTransform * m_WorldMatrix;
    return true;
}

// ボーン名一覧を取得.
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

// ボーン名からインデックスを取得.
int32_t FBXModel::GetBoneIndex(const std::string& BoneName) const
{
	auto it = m_BoneMapping.find(BoneName);
	if (it != m_BoneMapping.end())
	{
		return static_cast<int32_t>(it->second);
	}
	return -1;
}

// レイキャスト.
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

// ノード階層を再帰的に処理.
void FBXModel::ProcessNode(
	DirectX11* pDX11, 
	aiNode* pNode, 
	const aiScene* pScene, 
	const DirectX::XMMATRIX& ParentTransform)
{
    DirectX::XMMATRIX node_transform = ConvertMatrix(pNode->mTransformation);
    DirectX::XMMATRIX global_transform = node_transform * ParentTransform;

    // このノードのメッシュを処理.
    for (int i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
        FBXMesh* fbx_mesh = ProcessMesh(pDX11, pMesh, pScene, global_transform);
        if (fbx_mesh)
        {
            m_Meshes.push_back(fbx_mesh);
        }
    }

    // 子ノードを処理.
    for (int i = 0; i < pNode->mNumChildren; ++i)
    {
        ProcessNode(pDX11, pNode->mChildren[i], pScene, global_transform);
    }
}

// メッシュを処理.
FBXMesh* FBXModel::ProcessMesh(
	DirectX11* pDX11, 
	aiMesh* pMesh, 
	const aiScene* pScene,
	const DirectX::XMMATRIX& NodeTransform)
{
	if (!pDX11 || !pMesh) { return nullptr; }

	// 頂点データを構築.
	std::vector<FBXMesh::Vertex> vertices;
	vertices.reserve(pMesh->mNumVertices);

	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		FBXMesh::Vertex vertex = {};

		// 位置.
		if (pMesh->HasPositions())
		{
			vertex.Position.x = pMesh->mVertices[i].x;
			vertex.Position.y = pMesh->mVertices[i].y;
			vertex.Position.z = pMesh->mVertices[i].z;
		}

		// 法線.
		if (pMesh->HasNormals())
		{
			vertex.Normal.x = pMesh->mNormals[i].x;
			vertex.Normal.y = pMesh->mNormals[i].y;
			vertex.Normal.z = pMesh->mNormals[i].z;
		}

		// UV座標.
		if (pMesh->HasTextureCoords(0))
		{
			vertex.UV.x = pMesh->mTextureCoords[0][i].x;
			vertex.UV.y = pMesh->mTextureCoords[0][i].y;
		}

		// ボーン情報初期化.
		for (int j = 0; j < 4; ++j)
		{
			vertex.BoneIDs[j] = 0;
			vertex.Weights[j] = 0.0f;
		}

		vertices.push_back(vertex);
	}

	// インデックスデータを構築.
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

	// ボーン情報を読み込む.
	LoadBones(pMesh, vertices);

	// マテリアルを読み込む.
	FBXMesh::MaterialData material;
	if (pMesh->mMaterialIndex < pScene->mNumMaterials)
	{
		material = LoadMaterial(pDX11, pScene->mMaterials[pMesh->mMaterialIndex]);
	}

	// メッシュを作成.
	FBXMesh* fbx_mesh = new FBXMesh();
	if (!fbx_mesh->CreateBuffers(pDX11, vertices, indices, material, true))
	{
		delete fbx_mesh;
		return nullptr;
	}

	return fbx_mesh;
}

// マテリアルを読み込む.
FBXMesh::MaterialData FBXModel::LoadMaterial(
	DirectX11* pDX11, 
	aiMaterial* pMaterial)
{
	FBXMesh::MaterialData material = {};

	if (!pMaterial) { return material; }

	// デフォルト値.
	aiColor3D diffuse(1.0f, 1.0f, 1.0f);
	aiColor3D ambient(0.2f, 0.2f, 0.2f);
	aiColor3D specular(1.0f, 1.0f, 1.0f);
	float shininess = 32.0f;

	// マテリアルに設定があれば取得.
	pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
	pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	pMaterial->Get(AI_MATKEY_SHININESS, shininess);

	material.Diffuse = DirectX::XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, 1.0f);	// 拡散反射色.
	material.Ambient = DirectX::XMFLOAT4(ambient.r, ambient.g, ambient.b, 1.0f);	// 環境色.
	material.Specular = DirectX::XMFLOAT4(specular.r, specular.g, specular.b, 1.0f);// 鏡面反射色.
	material.Shininess = shininess;// 光沢度.

	// テクスチャを読み込む.
	if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString path = {};
		pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		std::string texture_path = m_Directory + path.C_Str();
		material.pTexture = LoadTexture(pDX11, texture_path);
	}

	return material;
}

// テクスチャを読み込む.
ID3D11ShaderResourceView* FBXModel::LoadTexture(DirectX11* pDX11, const std::string& TexturePath)
{
	if (!pDX11 || TexturePath.empty()) { return nullptr; }

    ID3D11Device* p_device = pDX11->GetDevice();
    if (!p_device)  { return nullptr; }

	// UTF-8をワイド文字に変換.
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, TexturePath.c_str(), static_cast<int>(TexturePath.length()), nullptr, 0);
	std::wstring wide_path(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, TexturePath.c_str(), static_cast<int>(TexturePath.length()), &wide_path[0], size_needed);

	// WICイメージングファクトリを作成.
	IWICImagingFactory* wic_factory = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		reinterpret_cast<void**>(&wic_factory));

	if (FAILED(hr) || !wic_factory)
	{
		std::cerr << "WICイメージングファクトリの作成に失敗: " << TexturePath << std::endl;
		return nullptr;
	}

	// ファイルデコーダを作成.
	IWICBitmapDecoder* decoder = nullptr;
	hr = wic_factory->CreateDecoderFromFilename(
		wide_path.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&decoder);

	if (FAILED(hr) || !decoder)
	{
		std::cerr << "デコーダの作成に失敗: " << TexturePath << std::endl;
		wic_factory->Release();
		return nullptr;
	}

	// フレームを取得.
	IWICBitmapFrameDecode* frame = nullptr;
	hr = decoder->GetFrame(0, &frame);

	if (FAILED(hr) || !frame)
	{
		std::cerr << "フレームの取得に失敗: " << TexturePath << std::endl;
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// イメージを32bit RGBAに変換.
	IWICFormatConverter* converter = nullptr;
	hr = wic_factory->CreateFormatConverter(&converter);

	if (FAILED(hr) || !converter)
	{
		std::cerr << "フォーマットコンバーターの作成に失敗: " << TexturePath << std::endl;
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	hr = converter->Initialize(
		frame,
		GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeMedianCut);

	if (FAILED(hr))
	{
		std::cerr << "フォーマットコンバーターの初期化に失敗: " << TexturePath << std::endl;
		converter->Release();
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// イメージサイズを取得.
	UINT width = 0, height = 0;
	hr = converter->GetSize(&width, &height);

	if (FAILED(hr) || width == 0 || height == 0)
	{
		std::cerr << "画像サイズの取得に失敗: " << TexturePath << std::endl;
		converter->Release();
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// ピクセルデータを取得.
	UINT stride = width * 4; // 32-bit RGBA
	UINT image_size = stride * height;
	uint8_t* image_data = new uint8_t[image_size];

	hr = converter->CopyPixels(nullptr, stride, image_size, image_data);

	if (FAILED(hr))
	{
		std::cerr << "ピクセルデータのコピーに失敗: " << TexturePath << std::endl;
		delete[] image_data;
		converter->Release();
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// テクスチャを作成.
	D3D11_TEXTURE2D_DESC tex_desc = {};
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data = {};
	init_data.pSysMem = image_data;
	init_data.SysMemPitch = stride;

	ID3D11Texture2D* texture = nullptr;
    hr = p_device->CreateTexture2D(&tex_desc, &init_data, &texture);

	if (FAILED(hr) || !texture)
	{
		std::cerr << "テクスチャの作成に失敗: " << TexturePath << std::endl;
		delete[] image_data;
		converter->Release();
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// シェーダリソースビューを作成.
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = tex_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* srv = nullptr;
    hr = p_device->CreateShaderResourceView(texture, &srv_desc, &srv);

	if (FAILED(hr))
	{
		std::cerr << "シェーダーリソースビューの作成に失敗: " << TexturePath << std::endl;
		texture->Release();
		delete[] image_data;
		converter->Release();
		frame->Release();
		decoder->Release();
		wic_factory->Release();
		return nullptr;
	}

	// リソースを解放.
	texture->Release();
	delete[] image_data;
	converter->Release();
	frame->Release();
	decoder->Release();
	wic_factory->Release();

	std::cout << "テクスチャを読み込みました: " << TexturePath << std::endl;
	return srv;
}

// ボーン情報を読み込む.
void FBXModel::LoadBones(aiMesh* pMesh, std::vector<FBXMesh::Vertex>& Vertices)
{
	if (!pMesh || pMesh->mNumBones == 0) { return; }

	// メッシュのボーン数に対して処理.
	for (int i = 0; i < pMesh->mNumBones; ++i)
	{
		aiBone* bone = pMesh->mBones[i];
		std::string bone_name = bone->mName.C_Str();

		// ボーンをマッピングに登録(まだ存在しない場合).
		if (m_BoneMapping.find(bone_name) == m_BoneMapping.end())
		{
			uint32_t bone_index = static_cast<uint32_t>(m_BoneInfos.size());
			m_BoneMapping[bone_name] = bone_index;
			m_BoneInfos.push_back(FBXModel::BoneInfo());
			m_BoneInfos[bone_index].OffsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
		}

		uint32_t bone_index = m_BoneMapping[bone_name];

		// ボーンの頂点ウェイトを適用.
		for (unsigned int j = 0; j < bone->mNumWeights; ++j)
		{
			uint32_t vertex_id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;

			if (vertex_id >= Vertices.size())
			{
				continue;
			}

			// 最大4個のボーン影響を記録.
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

// アニメーション時間でボーン変換を計算.
void FBXModel::CalculateBoneTransforms(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform)
{
    if (!pNode || m_CurrentAnimation >= m_AnimationCount) { return; }

    DirectX::XMMATRIX node_transform = ConvertMatrix(pNode->mTransformation);

    // ノードアニメーションを取得.
    const aiNodeAnim* node_anim = FindNodeAnim(m_CurrentAnimation, pNode->mName.C_Str());
    if (node_anim)
    {
        // スケールを取得.
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

        // 回転を取得.
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

        // 位置を取得.
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

    // ボーンの変換を更新.
    auto it = m_BoneMapping.find(pNode->mName.C_Str());
    if (it != m_BoneMapping.end())
    {
		uint32_t bone_index = it->second;
		m_BoneInfos[bone_index].GlobalTransform = global_transform;
		m_BoneInfos[bone_index].FinalTransform = 
			m_BoneInfos[bone_index].OffsetMatrix * global_transform * m_GlobalInverseTransform;
    }

    // 子ノードを処理.
    for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
    {
        CalculateBoneTransforms(AnimationTime, pNode->mChildren[i], global_transform);
    }
}

// ノードアニメーションを検索.
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

// Assimpマトリックスを DirectXMath に変換.
DirectX::XMMATRIX FBXModel::ConvertMatrix(const aiMatrix4x4& Matrix)
{
	return DirectX::XMMATRIX(
		Matrix.a1, Matrix.b1, Matrix.c1, Matrix.d1,
		Matrix.a2, Matrix.b2, Matrix.c2, Matrix.d2,
		Matrix.a3, Matrix.b3, Matrix.c3, Matrix.d3,
		Matrix.a4, Matrix.b4, Matrix.c4, Matrix.d4
	);
}

// シェーダー作成.
bool FBXModel::CreateShaders(DirectX11* pDX11)
{
	if (!pDX11) { return false; }

    ID3D11Device* p_device = pDX11->GetDevice();
    if (!p_device) { return false; }

    ID3DBlob* p_vs_blob = nullptr;
    ID3DBlob* p_ps_blob = nullptr;
    ID3DBlob* p_error_blob = nullptr;

	// シェーダーファイルパスの解決.
	std::wstring shader_path_w = SHADER_PATH;
	std::cout << "Skinning.hlslを検出: ";
	std::wcout << shader_path_w << std::endl;

	// 頂点シェーダーコンパイル.
    HRESULT hr = D3DCompileFromFile(
		shader_path_w.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS_Main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &p_vs_blob,
        &p_error_blob);

	if (FAILED(hr))
	{
		if (p_error_blob)
		{
			std::cerr << "頂点シェーダーのコンパイルエラー: " << (char*)p_error_blob->GetBufferPointer() << std::endl;
			p_error_blob->Release();
		}
		else
		{
			std::cerr << "頂点シェーダーのコンパイルに失敗 (HRESULT: 0x" << std::hex << hr << std::dec << ")" << std::endl;
		}
		return false;
	}

	// ピクセルシェーダーコンパイル.
    hr = D3DCompileFromFile(
		shader_path_w.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS_Main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &p_ps_blob,
        &p_error_blob);

	if (FAILED(hr))
	{
		if (p_error_blob)
		{
			std::cerr << "ピクセルシェーダーのコンパイルエラー: " << (char*)p_error_blob->GetBufferPointer() << std::endl;
			p_error_blob->Release();
		}
		else
		{
			std::cerr << "ピクセルシェーダーのコンパイルに失敗 (HRESULT: 0x" << std::hex << hr << std::dec << ")" << std::endl;
		}
		if (p_vs_blob) p_vs_blob->Release();
		return false;
	}

	// 頂点シェーダー作成.
    hr = p_device->CreateVertexShader(
        p_vs_blob->GetBufferPointer(),
        p_vs_blob->GetBufferSize(),
        nullptr,
        &m_pVertexShader);

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダーの作成に失敗しました。" << std::endl;
		p_vs_blob->Release();
		p_ps_blob->Release();
		return false;
	}

	// ピクセルシェーダー作成.
    hr = p_device->CreatePixelShader(
        p_ps_blob->GetBufferPointer(),
        p_ps_blob->GetBufferSize(),
        nullptr,
        &m_pPixelShader);

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダーの作成に失敗しました。" << std::endl;
		p_vs_blob->Release();
		p_ps_blob->Release();
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
		return false;
	}

	// 入力レイアウト作成.
	if (!CreateInputLayout(pDX11, p_vs_blob))
	{
		std::cerr << "入力レイアウトの作成に失敗しました。" << std::endl;
		p_vs_blob->Release();
		p_ps_blob->Release();
		m_pVertexShader->Release();
		m_pPixelShader->Release();
		m_pVertexShader = nullptr;
		m_pPixelShader = nullptr;
		return false;
	}

    p_vs_blob->Release();
    p_ps_blob->Release();

	return true;
}

// 定数バッファ作成.
bool FBXModel::CreateConstantBuffers(DirectX11* pDX11)
{
	if (!pDX11) { return false; }

    ID3D11Device* p_device = pDX11->GetDevice();
    if (!p_device) { return false; }

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;

	// cbWorld バッファ作成.
	desc.ByteWidth = sizeof(cbWorld);
    HRESULT hr = p_device->CreateBuffer(&desc, nullptr, &m_pCBWorld);
	if (FAILED(hr))
	{
		std::cerr << "cbWorld定数バッファの作成に失敗しました。" << std::endl;
		return false;
	}

	// cbSkeleton バッファ作成.
	desc.ByteWidth = sizeof(cbSkeleton);
    hr = p_device->CreateBuffer(&desc, nullptr, &m_pCBSkeleton);
	if (FAILED(hr))
	{
		std::cerr << "cbSkeleton定数バッファの作成に失敗しました。" << std::endl;
		if (m_pCBWorld)
		{
			m_pCBWorld->Release();
			m_pCBWorld = nullptr;
		}
		return false;
	}

	// cbMaterial バッファ作成.
	desc.ByteWidth = sizeof(cbMaterial);
    hr = p_device->CreateBuffer(&desc, nullptr, &m_pCBMaterial);
	if (FAILED(hr))
	{
		std::cerr << "cbMaterial定数バッファの作成に失敗しました。" << std::endl;
		if (m_pCBWorld)
		{
			m_pCBWorld->Release();
			m_pCBWorld = nullptr;
		}
		if (m_pCBSkeleton)
		{
			m_pCBSkeleton->Release();
			m_pCBSkeleton = nullptr;
		}
		return false;
	}

	return true;
}

// 入力レイアウト作成.
bool FBXModel::CreateInputLayout(DirectX11* pDX11, ID3DBlob* pVertexShaderBlob)
{
	if (!pDX11 || !pVertexShaderBlob)
	{
		return false;
	}

    ID3D11Device* p_device = pDX11->GetDevice();
    if (!p_device)
    {
        return false;
    }

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEIDS",   0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout);

    HRESULT hr = p_device->CreateInputLayout(
        layout,
		num_elements,
		pVertexShaderBlob->GetBufferPointer(),
		pVertexShaderBlob->GetBufferSize(),
        &m_pInputLayout);

	if (FAILED(hr))
	{
		std::cerr << "入力レイアウトの作成に失敗しました。" << std::endl;
		return false;
	}

	return true;
}

