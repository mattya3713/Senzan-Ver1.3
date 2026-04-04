#pragma once

#include "Graphic/FBX/FBXMesh.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

/**********************************************************
* @author      : 淵脇未来.
* @date        : 2025/02/18.
* @brief       : FBXModel クラス.
*              : キャラクター1体を統合管理する.
**********************************************************/
class FBXModel final
{
public:

	// ボーン情報構造体.
	struct BoneInfo
	{
		DirectX::XMMATRIX OffsetMatrix;			// バインドポーズの逆行列.
		DirectX::XMMATRIX FinalTransform;		// 最終変換行列（シェーダー用）.
		DirectX::XMMATRIX GlobalTransform;		// グローバル変換行列（ワールド座標取得用）.

		BoneInfo()
		{
			OffsetMatrix = DirectX::XMMatrixIdentity();
			FinalTransform = DirectX::XMMatrixIdentity();
			GlobalTransform = DirectX::XMMatrixIdentity();
		}
	};

	// アニメーション情報構造体.
	struct AnimationInfo
	{
		std::string Name;			// アニメーション名.
		float Duration;				// 継続時間（ticks）.
		float TicksPerSecond;		// 1秒あたりのtick数.
		uint32_t Index;				// シーン内のインデックス.

		// ノード名→チャンネルのキャッシュ（検索高速化）.
		std::unordered_map<std::string, const aiNodeAnim*> NodeAnimCache;
	};

	// スケルトン定数バッファ構造体.
	struct cbSkeleton
	{
		DirectX::XMMATRIX BoneTransforms[128];
	};

	// ワールド変換定数バッファ構造体.
	struct cbWorld
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};

	// マテリアル定数バッファ構造体.
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

	// コピー禁止.
	FBXModel(const FBXModel&) = delete;
	FBXModel& operator=(const FBXModel&) = delete;

	// ムーブ禁止（rawポインタ所有のため）.
	FBXModel(FBXModel&&) = delete;
	FBXModel& operator=(FBXModel&&) = delete;

public:

	bool Load(DirectX11* pDX11, const std::string& FilePath);
	void Update(float DeltaTime);
	void Render(DirectX11* pDX11);
	void Release();

	/**********************************************************
	* @brief         : レイキャスト処理でメッシュとの交差判定.
	* @param Ray     : 判定に使用するレイ情報.
	* @param OutHit  : 交差判定の結果情報.
	* @return		 : レイが交差したか否か.
	**********************************************************/
	bool RayCast(const FBXMesh::Ray& Ray, FBXMesh::HitInfo& OutHit) const;

	// 行列設定.
	void SetWorldMatrix(const DirectX::XMMATRIX& World) { m_WorldMatrix = World; }
	void SetViewMatrix(const DirectX::XMMATRIX& View) { m_ViewMatrix = View; }
	void SetProjectionMatrix(const DirectX::XMMATRIX& Projection) { m_ProjectionMatrix = Projection; }

	//-----------------------------------------------------------------------------
	// アニメーション.
	//-----------------------------------------------------------------------------
	
	// アニメーション設定(Index).
	bool SetAnimationIndex(uint32_t Index);
	inline uint32_t GetAnimationIndex() const noexcept { return m_CurrentAnimation; }

	// アニメーション設定(Name).
	bool SetAnimationByName(const std::string& AnimationName);
	const std::string& GetCurrentAnimationName() const;

	// 再生時間の設定.
	inline void SetAnimationTime(float Time) { m_AnimationTime = Time; }
	inline float GetCurrentAnimationTime() const noexcept { return m_AnimationTime; }

	// Debugアニメーション関連.
	inline uint32_t GetAnimationCount() const noexcept { return m_AnimationCount; }	// アニメーション数を取得する.
	float GetAnimationDuration(uint32_t Index) const;	// 指定インデックスのアニメーション継続時間を取得.
	std::string GetAnimationName(uint32_t Index) const;	// 指定インデックスのアニメーション名を取得.
	std::vector<std::string> GetAnimationNames() const;	// 全アニメーション名を取得.

	//-----------------------------------------------------------------------------
	// ボーンl.
	//-----------------------------------------------------------------------------
	
	// ワールド行列取得.
	bool GetBoneWorldMatrix(const std::string& BoneName, DirectX::XMMATRIX& OutMatrix) const;
	bool GetBoneWorldMatrixByIndex(uint32_t BoneIndex, DirectX::XMMATRIX& OutMatrix) const;

	// ワールド座標取得.
	bool GetBoneWorldPosition(const std::string& BoneName, DirectX::XMFLOAT3& OutPosition) const;

	// Debugボーン情報.
	int32_t GetBoneIndex(const std::string& BoneName) const;	// ボーン名からボーンのインデックスを取得.
	std::vector<std::string> GetBoneNames() const;				// すべてのボーン名のリストを取得.
	inline size_t GetBoneCount() const noexcept { return m_BoneInfos.size(); }	// ボーンの総数を取得.

	//-----------------------------------------------------------------------------
	// その他.
	//-----------------------------------------------------------------------------

	inline bool IsSkinned() const noexcept { return m_IsBones; }			// スキンメッシュか否か.
	inline size_t GetMeshCount() const noexcept { return m_Meshes.size(); }	// メッシュパーツの総数取得.

private:

	// シェーダー作成.
	bool CreateShaders(DirectX11* pDX11);

	// 定数バッファ作成.
	bool CreateConstantBuffers(DirectX11* pDX11);

	// 入力レイアウト作成.
	bool CreateInputLayout(DirectX11* pDX11, ID3DBlob* pVertexShaderBlob);

	// マテリアルを読み込む.
	FBXMesh::MaterialData LoadMaterial(
		DirectX11* pDX11,
		aiMaterial* pMaterial);

	// テクスチャを読み込む.
	ID3D11ShaderResourceView* LoadTexture(
		DirectX11* pDX11,
		const std::string& TexturePath);

	// ボーン情報を読み込む.
	void LoadBones(
		aiMesh* pMesh,
		std::vector<FBXMesh::Vertex>& Vertices);

private:
    // ノード階層を再帰的に処理.
    void ProcessNode(
		DirectX11* pDX11, 
		aiNode* pNode,
		const aiScene* pScene,
		const DirectX::XMMATRIX& ParentTransform);

    // メッシュを処理.
    FBXMesh* ProcessMesh(
		DirectX11* pDX11, 
		aiMesh* pMesh, 
		const aiScene* pScene, 
		const DirectX::XMMATRIX& NodeTransform);

	// アニメーション情報をキャッシュ構築.
	void BuildAnimationCache();

    // アニメーション時間でボーン変換を計算.
    void CalculateBoneTransforms(
		float AnimationTime, 
		const aiNode* pNode,
		const DirectX::XMMATRIX& ParentTransform);

    // ノードアニメーションを検索（キャッシュ使用）.
    const aiNodeAnim* FindNodeAnim(
		uint32_t AnimationIndex,
		const std::string& NodeName);

    // Assimpマトリックスを DirectXMath に変換.
    DirectX::XMMATRIX ConvertMatrix(
		const aiMatrix4x4& Matrix);
private:
	std::vector<FBXMesh*> m_Meshes;					// メッシュパーツリスト.
	std::map<std::string, uint32_t> m_BoneMapping;	// ボーン名->インデックスマップ.
	std::vector<BoneInfo> m_BoneInfos;				// ボーン情報配列.

	// Assimp関連.
	Assimp::Importer m_Importer;
	const aiScene* m_pScene;
	std::string m_Directory;

	// アニメーション.
	std::vector<AnimationInfo> m_AnimationInfos;	// アニメーション情報リスト.
	float m_AnimationTime;							// 現在のアニメーション時間.
	uint32_t m_CurrentAnimation;					// 現在のアニメーションインデックス.
	uint32_t m_AnimationCount;						// アニメーション数.
	bool m_IsBones;									// ボーンがあるか.

	// 変換行列.
	DirectX::XMMATRIX m_WorldMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;
	DirectX::XMMATRIX m_GlobalInverseTransform;

	// シェーダー.
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_pInputLayout;

	// 定数バッファ.
	ID3D11Buffer* m_pCBWorld;
	ID3D11Buffer* m_pCBSkeleton;
	ID3D11Buffer* m_pCBMaterial;

	// サンプラーステート.
	ID3D11SamplerState* m_pSamplerState;
};


