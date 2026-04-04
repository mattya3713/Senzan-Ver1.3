#pragma once

#include "Graphic/DirectX/DirectX11.h"
#include <string>
#include <vector>


// 前方宣言.
class FBXModel;


/**********************************************************
* @author      : 淵脇未来.
* @date        : 2025/02/18.
* @brief       : メッシュパーツ管理クラス.
*              : モデル内の「1パーツ（マテリアル単位のサブメッシュ）」を表す.
**********************************************************/
class FBXMesh final
{
	friend class FBXModel;

public:

	// 頂点構造体.
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;			// 位置.
		DirectX::XMFLOAT3 Normal;			// 法線.
		DirectX::XMFLOAT2 UV;				// UV座標.
		uint32_t BoneIDs[4];				// ボーンインデックス.
		float Weights[4];					// ウェイト.
	};

	// レイ構造体.
	struct Ray
	{
		DirectX::XMFLOAT3 Origin;			// レイの始点.
		DirectX::XMFLOAT3 Direction;		// レイの方向（正規化済み）.
	};

	// ヒット情報構造体.
	struct HitInfo
	{
		bool IsHit;							// ヒットしたか.
		float Distance;						// 交差点までの距離.
		DirectX::XMFLOAT3 Position;			// 交差点の位置.
		DirectX::XMFLOAT3 Normal;			// 交差点の法線.
		uint32_t TriangleIndex;				// 交差した三角形のインデックス.
		uint32_t MeshIndex;					// 交差したメッシュのインデックス.

		HitInfo()
			: IsHit(false)
			, Distance(FLT_MAX)
			, Position(0.0f, 0.0f, 0.0f)
			, Normal(0.0f, 1.0f, 0.0f)
			, TriangleIndex(0)
			, MeshIndex(0)
		{
		}
	};

	// マテリアルデータ構造体.
	struct MaterialData
	{
		DirectX::XMFLOAT4 Diffuse;			// 拡散反射色.
		DirectX::XMFLOAT4 Ambient;			// 環境色.
		DirectX::XMFLOAT4 Specular;			// 鏡面反射色.
		float Shininess;					// 光沢度.
		ID3D11ShaderResourceView* pTexture;	// ディフューズテクスチャ.
		std::wstring TexturePath;			// テクスチャパス.

		MaterialData()
			: Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
			, Ambient(0.2f, 0.2f, 0.2f, 1.0f)
			, Specular(1.0f, 1.0f, 1.0f, 1.0f)
			, Shininess(32.0f)
			, pTexture(nullptr)
		{
		}
	};

public:
	FBXMesh();
	~FBXMesh() noexcept;

	// 描画.
	bool Render(DirectX11* pDX11);

	// レイキャスト(スタティックメッシュのみ).
	bool IsRayCast(const Ray& Ray, HitInfo& OutHit) const;

	uint32_t GetVertexCount() const noexcept { return static_cast<uint32_t>(m_Vertices.size()); }	// 頂点数取得.
	uint32_t GetIndexCount() const noexcept { return m_IndexCount; }				// インデックス数取得.
	const MaterialData& GetMaterial() const noexcept { return m_Material; }			// マテリアル取得.
	const std::vector<Vertex>& GetVertices() const noexcept { return m_Vertices; }	// 頂点データ取得.
	const std::vector<uint32_t>& GetIndices() const noexcept { return m_Indices; }	// インデックスデータ取得.

private:
	// リソース解放.
	void Release();

	/**********************************************************
	* @brief     : 三角形とレイの交差判定.
	* @param Ray : レイ.
	* @param V0  : 三角形の頂点0.
	* @param V1  : 三角形の頂点1.
	* @param V2  : 三角形の頂点2.
	* @param OutT: 交差点までの距離.
	* @return    : 交差したかどうか.
	**********************************************************/
	bool IntersectTriangle(
		const Ray& Ray,
		const DirectX::XMFLOAT3& V0,
		const DirectX::XMFLOAT3& V1,
		const DirectX::XMFLOAT3& V2,
		float& OutT) const;

	/**********************************************************
	* @brief               : バッファ作成.
	* @param pDX11         : DirectX11インスタンス.
	* @param Vertices      : 頂点データ.
	* @param Indices       : インデックスデータ.
	* @param Material      : マテリアルデータ.
	* @param IsKeepCPUData : CPUデータを保持するか.
	* @return              : 成功したかどうか.
	**********************************************************/
	bool CreateBuffers(
		DirectX11* pDX11,
		const std::vector<Vertex>& Vertices,
		const std::vector<uint32_t>& Indices,
		const MaterialData& Material,
		bool IsKeepCPUData = false);

private:
	ID3D11Buffer* m_pVB;				// 頂点バッファ.
	ID3D11Buffer* m_pIB;				// インデックスバッファ.
	uint32_t m_IndexCount;				// インデックス数.
	MaterialData m_Material;			// マテリアルデータ.

	// CPU側データ（レイ判定用、スタティックのみ保持）.
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	bool m_IsKeepCPUData;
};
