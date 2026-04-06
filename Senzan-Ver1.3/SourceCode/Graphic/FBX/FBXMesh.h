#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

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
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 UV;
		uint32_t BoneIDs[4];
		float Weights[4];
	};

	// レイ構造体.
	struct Ray
	{
		DirectX::XMFLOAT3 Origin;
		DirectX::XMFLOAT3 Direction;
	};

	// ヒット情報構造体.
	struct HitInfo
	{
		bool IsHit;
		float Distance;
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		uint32_t TriangleIndex;
		uint32_t MeshIndex;

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
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Specular;
		float Shininess;
		std::wstring TexturePath;

		MaterialData()
			: Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
			, Ambient(0.2f, 0.2f, 0.2f, 1.0f)
			, Specular(1.0f, 1.0f, 1.0f, 1.0f)
			, Shininess(32.0f)
		{
		}
	};

public:
	FBXMesh();
	~FBXMesh() noexcept;

	// 描画.
	bool Render(ID3D12GraphicsCommandList* pCommandList);

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
	* @brief                : バッファ作成.
	* @param pDevice        : D3D12デバイス.
	* @param pCommandQueue  : コマンドキュー（CPU→GPU コピー用）.
	* @param Vertices       : 頂点データ.
	* @param Indices        : インデックスデータ.
	* @param Material       : マテリアルデータ.
	* @param IsKeepCPUData  : CPUデータを保持するか.
	* @return               : 成功したかどうか.
	**********************************************************/
	bool CreateBuffers(
		ID3D12Device* pDevice,
		ID3D12CommandQueue* pCommandQueue,
		const std::vector<Vertex>& Vertices,
		const std::vector<uint32_t>& Indices,
		const MaterialData& Material,
		bool IsKeepCPUData = false);

private:
	uint32_t m_IndexCount;
	MaterialData m_Material;

	// GPU リソース（Default Heap）.
	ComPtr<ID3D12Resource> m_pVertexBuffer;
	ComPtr<ID3D12Resource> m_pIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	// CPU側データ（レイ判定用、スタティックのみ保持）.
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	bool m_IsKeepCPUData;
};