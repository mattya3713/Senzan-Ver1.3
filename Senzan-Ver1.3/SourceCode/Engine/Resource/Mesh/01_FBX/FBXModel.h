#pragma once

#include "FBXMesh.h"
#include <d3d12.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

/**********************************************************
* @author      : mattya3713.
* @date        : 2025/02/18.
* @brief       : FBXモデルの管理・描画クラス.
**********************************************************/
class FBXModel final
{
public:
	// ボーン情報.
	struct BoneInfo
	{
		DirectX::XMMATRIX OffsetMatrix;		// オフセット行列.
		DirectX::XMMATRIX FinalTransform;	// 最終変換行列.
		DirectX::XMMATRIX GlobalTransform;	// グローバル変換行列.

		BoneInfo()
		{
			OffsetMatrix = DirectX::XMMatrixIdentity();
			FinalTransform = DirectX::XMMatrixIdentity();
			GlobalTransform = DirectX::XMMatrixIdentity();
		}
	};

	// アニメーション情報.
	struct AnimationInfo
	{
		std::string Name;											// アニメーション名.
		float Duration;											// アニメーション長.
		float TicksPerSecond;									// 1秒あたりのティック数.
		uint32_t Index;											// アニメーションインデックス.
		std::unordered_map<std::string, const aiNodeAnim*> NodeAnimCache; // ノードアニメーションキャッシュ.
	};

	// スケルトン定数バッファ.
	struct cbSkeleton
	{
		DirectX::XMMATRIX BoneTransforms[128];	// ボーン変換行列（最大128個）.
	};

	// ワールド定数バッファ.
	struct cbWorld
	{
		DirectX::XMMATRIX World;		// ワールド行列.
		DirectX::XMMATRIX View;			// ビュー行列.
		DirectX::XMMATRIX Projection;	// プロジェクション行列.
	};

	// マテリアル定数バッファ.
	struct cbMaterial
	{
		DirectX::XMFLOAT4 Diffuse;	// 拡散色.
		DirectX::XMFLOAT4 Ambient;	// 環境光色.
		DirectX::XMFLOAT4 Specular;	// 鏡面色.
		float Shininess;				// 鏡面強度.
		float Padding[3];				// パディング（16byte 境界アライメント用）.
	};

public:
	FBXModel();
	~FBXModel() noexcept;

	FBXModel(const FBXModel&) = delete;
	FBXModel& operator=(const FBXModel&) = delete;
	FBXModel(FBXModel&&) = delete;
	FBXModel& operator=(FBXModel&&) = delete;

public:
	// ライフサイクル.
	bool Load(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, const std::string& FilePath);
	void Update(float DeltaTime);
	void Render(ID3D12GraphicsCommandList* pCommandList);
	void Release();

	// レイキャスト.
	bool RayCast(const FBXMesh::Ray& Ray, FBXMesh::HitInfo& OutHit) const;

	// ワールド・ビュー・プロジェクション行列の設定.
	void SetWorldMatrix(const DirectX::XMMATRIX& World) { m_WorldMatrix = World; }
	void SetViewMatrix(const DirectX::XMMATRIX& View) { m_ViewMatrix = View; }
	void SetProjectionMatrix(const DirectX::XMMATRIX& Projection) { m_ProjectionMatrix = Projection; }

	// アニメーション関連.
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

	// ボーン関連.
	bool GetBoneWorldMatrix(const std::string& BoneName, DirectX::XMMATRIX& OutMatrix) const;
	bool GetBoneWorldMatrixByIndex(uint32_t BoneIndex, DirectX::XMMATRIX& OutMatrix) const;
	bool GetBoneWorldPosition(const std::string& BoneName, DirectX::XMFLOAT3& OutPosition) const;
	int32_t GetBoneIndex(const std::string& BoneName) const;
	std::vector<std::string> GetBoneNames() const;
	inline size_t GetBoneCount() const noexcept { return m_BoneInfos.size(); }

	// メッシュ関連.
	inline bool IsSkinned() const noexcept { return m_IsBones; }
	inline size_t GetMeshCount() const noexcept { return m_Meshes.size(); }

private:
	// シェーダー・レイアウト関連.
	bool CreateShaders(ID3D12Device* pDevice);
	bool CreateConstantBuffers(ID3D12Device* pDevice);
	bool CreateInputLayout(ID3D12Device* pDevice);

	// マテリアル・テクスチャ関連.

	/****************************************
	* @brief マテリアルデータを読み込む.
	* @param pDevice DirectX12 デバイス.
	* @param pMaterial Assimp マテリアル.
	* @return マテリアルデータ.
	****************************************/
	FBXMesh::MaterialData LoadMaterial(
		ID3D12Device* pDevice,
		aiMaterial* pMaterial);

	/****************************************
	* @brief テクスチャを読み込んでマテリアルに設定.
	* @param pDevice DirectX12 デバイス.
	* @param TexturePath テクスチャファイルパス.
	* @param Material テクスチャを設定するマテリアル.
	****************************************/
	void LoadTexture(
		ID3D12Device* pDevice,
		const std::string& TexturePath,
		FBXMesh::MaterialData& Material);

	// ボーン・メッシュ処理関連.

	/****************************************
	* @brief ボーン情報を読み込んで頂点に設定.
	* @param pMesh Assimp メッシュ.
	* @param Vertices ボーン情報を追加する頂点配列.
	****************************************/
	void LoadBones(
		aiMesh* pMesh,
		std::vector<FBXMesh::Vertex>& Vertices);

	/****************************************
	* @brief Assimp ノードを再帰的に処理.
	* @param pDevice DirectX12 デバイス.
	* @param pCommandQueue コマンドキュー.
	* @param pNode 処理するノード.
	* @param pScene Assimp シーン.
	* @param ParentTransform 親ノードの変換行列.
	****************************************/
	void ProcessNode(
		ID3D12Device* pDevice,
		ID3D12CommandQueue* pCommandQueue,
		aiNode* pNode,
		const aiScene* pScene,
		const DirectX::XMMATRIX& ParentTransform);

	/****************************************
	* @brief Assimp メッシュを処理して FBXMesh を作成.
	* @param pDevice DirectX12 デバイス.
	* @param pCommandQueue コマンドキュー.
	* @param pMesh 処理するメッシュ.
	* @param pScene Assimp シーン.
	* @param NodeTransform ノードの変換行列.
	* @return 作成された FBXMesh ポインター.
	****************************************/
	FBXMesh* ProcessMesh(
		ID3D12Device* pDevice,
		ID3D12CommandQueue* pCommandQueue,
		aiMesh* pMesh,
		const aiScene* pScene,
		const DirectX::XMMATRIX& NodeTransform);


	// アニメーション計算関連.

	// アニメーションキャッシュを構築.
	void BuildAnimationCache();

	/****************************************
	* @brief ボーン変換行列を再帰的に計算.
	* @param AnimationTime アニメーション時刻.
	* @param pNode 処理するノード.
	* @param ParentTransform 親ノードの変換行列.
	****************************************/
	void CalculateBoneTransforms(
		float AnimationTime,
		const aiNode* pNode,
		const DirectX::XMMATRIX& ParentTransform);

	/****************************************
	* @brief 指定されたアニメーション内でノードのアニメーションを検索.
	* @param AnimationIndex アニメーションインデックス.
	* @param NodeName ノード名.
	* @return 見つかったノードアニメーションポインター（見つからない場合は nullptr）.
	****************************************/
	const aiNodeAnim* FindNodeAnim(
		uint32_t AnimationIndex,
		const std::string& NodeName);

	/****************************************
	* @brief Assimp 行列を DirectX::XMMATRIX に変換.
	* @param Matrix 変換元の Assimp 行列.
	* @return 変換後の DirectX 行列.
	****************************************/
	DirectX::XMMATRIX ConvertMatrix(const aiMatrix4x4& Matrix);

private:
	// メッシュ・ボーン・アニメーションデータ.
	std::vector<FBXMesh*> m_Meshes;								// メッシュ配列.
	std::map<std::string, uint32_t> m_BoneMapping;			// ボーン名とインデックスのマッピング.
	std::vector<BoneInfo> m_BoneInfos;						// ボーン情報配列.

	// Assimp 関連.
	Assimp::Importer m_Importer;							// Assimp インポーター.
	const aiScene* m_pScene;								// Assimp シーン（データ所有権は Importer が保持）.
	std::string m_Directory;								// FBX ファイルの格納ディレクトリ.

	// アニメーション状態.
	std::vector<AnimationInfo> m_AnimationInfos;			// アニメーション情報配列.
	float m_AnimationTime;									// 現在のアニメーション時刻.
	uint32_t m_CurrentAnimation;							// 現在のアニメーションインデックス.
	uint32_t m_AnimationCount;								// アニメーション数.
	bool m_IsBones;											// ボーンが存在するかフラグ.

	// 行列.
	DirectX::XMMATRIX m_WorldMatrix;						// ワールド行列.
	DirectX::XMMATRIX m_ViewMatrix;							// ビュー行列.
	DirectX::XMMATRIX m_ProjectionMatrix;					// プロジェクション行列.
	DirectX::XMMATRIX m_GlobalInverseTransform;			// グローバル逆変換行列.

	// DirectX12 リソース.
	ID3D12PipelineState* m_pPipelineState;					// パイプラインステート.
	ID3D12RootSignature* m_pRootSignature;					// ルートシグネチャ.
	ID3D12Resource* m_pCBWorld;								// ワールド定数バッファ.
	ID3D12Resource* m_pCBSkeleton;							// スケルトン定数バッファ.
	ID3D12Resource* m_pCBMaterial;							// マテリアル定数バッファ.
	ID3D12DescriptorHeap* m_pSrvHeap;						// SRV ディスクリプタヒープ.
};
