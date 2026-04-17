#pragma once

#include "FBXMesh.h"
#include <d3d12.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class FBXModel final
{
public:
	struct BoneInfo
	{
		DirectX::XMMATRIX OffsetMatrix;
		DirectX::XMMATRIX FinalTransform;
		DirectX::XMMATRIX GlobalTransform;

		BoneInfo()
		{
			OffsetMatrix = DirectX::XMMatrixIdentity();
			FinalTransform = DirectX::XMMatrixIdentity();
			GlobalTransform = DirectX::XMMatrixIdentity();
		}
	};

	struct AnimationInfo
	{
		std::string Name;
		float Duration;
		float TicksPerSecond;
		uint32_t Index;
		std::unordered_map<std::string, const aiNodeAnim*> NodeAnimCache;
	};

	struct cbSkeleton
	{
		DirectX::XMMATRIX BoneTransforms[128];
	};

	struct cbWorld
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};

	struct cbMaterial
	{
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Specular;
		float Shininess;
		float Padding[3];
	};

public:
	FBXModel();
	~FBXModel() noexcept;

	FBXModel(const FBXModel&) = delete;
	FBXModel& operator=(const FBXModel&) = delete;
	FBXModel(FBXModel&&) = delete;
	FBXModel& operator=(FBXModel&&) = delete;

public:
	bool Load(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, const std::string& FilePath);
	void Update(float DeltaTime);
	void Render(ID3D12GraphicsCommandList* pCommandList);
	void Release();

	bool RayCast(const FBXMesh::Ray& Ray, FBXMesh::HitInfo& OutHit) const;

	void SetWorldMatrix(const DirectX::XMMATRIX& World) { m_WorldMatrix = World; }
	void SetViewMatrix(const DirectX::XMMATRIX& View) { m_ViewMatrix = View; }
	void SetProjectionMatrix(const DirectX::XMMATRIX& Projection) { m_ProjectionMatrix = Projection; }

	bool SetAnimationIndex(uint32_t Index);
	inline uint32_t GetAnimationIndex() const noexcept { return m_CurrentAnimation; }
	bool SetAnimationByName(const std::string& AnimationName);
	const std::string& GetCurrentAnimationName() const;
	inline void SetAnimationTime(float Time) { m_AnimationTime = Time; }
	inline float GetCurrentAnimationTime() const noexcept { return m_AnimationTime; }
	inline uint32_t GetAnimationCount() const noexcept { return m_AnimationCount; }
	float GetAnimationDuration(uint32_t Index) const;
	std::string GetAnimationName(uint32_t Index) const;
	std::vector<std::string> GetAnimationNames() const;

	bool GetBoneWorldMatrix(const std::string& BoneName, DirectX::XMMATRIX& OutMatrix) const;
	bool GetBoneWorldMatrixByIndex(uint32_t BoneIndex, DirectX::XMMATRIX& OutMatrix) const;
	bool GetBoneWorldPosition(const std::string& BoneName, DirectX::XMFLOAT3& OutPosition) const;
	int32_t GetBoneIndex(const std::string& BoneName) const;
	std::vector<std::string> GetBoneNames() const;
	inline size_t GetBoneCount() const noexcept { return m_BoneInfos.size(); }

	inline bool IsSkinned() const noexcept { return m_IsBones; }
	inline size_t GetMeshCount() const noexcept { return m_Meshes.size(); }

private:
	bool CreateShaders(ID3D12Device* pDevice);
	bool CreateConstantBuffers(ID3D12Device* pDevice);
	bool CreateInputLayout(ID3D12Device* pDevice);

	FBXMesh::MaterialData LoadMaterial(
		ID3D12Device* pDevice,
		aiMaterial* pMaterial);

	void LoadTexture(
		ID3D12Device* pDevice,
		const std::string& TexturePath,
		FBXMesh::MaterialData& Material);

	void LoadBones(
		aiMesh* pMesh,
		std::vector<FBXMesh::Vertex>& Vertices);

	void ProcessNode(
		ID3D12Device* pDevice,
		ID3D12CommandQueue* pCommandQueue,
		aiNode* pNode,
		const aiScene* pScene,
		const DirectX::XMMATRIX& ParentTransform);

	FBXMesh* ProcessMesh(
		ID3D12Device* pDevice,
		ID3D12CommandQueue* pCommandQueue,
		aiMesh* pMesh,
		const aiScene* pScene,
		const DirectX::XMMATRIX& NodeTransform);

	void BuildAnimationCache();

	void CalculateBoneTransforms(
		float AnimationTime,
		const aiNode* pNode,
		const DirectX::XMMATRIX& ParentTransform);

	const aiNodeAnim* FindNodeAnim(
		uint32_t AnimationIndex,
		const std::string& NodeName);

	DirectX::XMMATRIX ConvertMatrix(const aiMatrix4x4& Matrix);

private:
	std::vector<FBXMesh*> m_Meshes;
	std::map<std::string, uint32_t> m_BoneMapping;
	std::vector<BoneInfo> m_BoneInfos;

	Assimp::Importer m_Importer;
	const aiScene* m_pScene;
	std::string m_Directory;

	std::vector<AnimationInfo> m_AnimationInfos;
	float m_AnimationTime;
	uint32_t m_CurrentAnimation;
	uint32_t m_AnimationCount;
	bool m_IsBones;

	DirectX::XMMATRIX m_WorldMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;
	DirectX::XMMATRIX m_GlobalInverseTransform;

	ID3D12PipelineState* m_pPipelineState;
	ID3D12RootSignature* m_pRootSignature;
	ID3D12Resource* m_pCBWorld;
	ID3D12Resource* m_pCBSkeleton;
	ID3D12Resource* m_pCBMaterial;
	ID3D12DescriptorHeap* m_pSrvHeap;
};
