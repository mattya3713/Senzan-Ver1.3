#include "Graphic/FBX/FBXMesh.h"

// コンストラクタ.
FBXMesh::FBXMesh()
	: m_pVB(nullptr)
	, m_pIB(nullptr)
	, m_IndexCount(0)
	, m_IsKeepCPUData(false)
{
}

// デストラクタ.
FBXMesh::~FBXMesh() noexcept
{
	Release();
}

// バッファ作成.
bool FBXMesh::CreateBuffers(
	DirectX11* pDX11,
	const std::vector<Vertex>& Vertices,
	const std::vector<uint32_t>& Indices,
	const MaterialData& Material,
	bool IsKeepCPUData)
{
	if (!pDX11 || Vertices.empty() || Indices.empty()) { return false; }

	ID3D11Device* p_device = pDX11->GetDevice();
	if (!p_device) { return false; }

	m_Material = Material;
	m_IndexCount = static_cast<uint32_t>(Indices.size());
	m_IsKeepCPUData = IsKeepCPUData;

	// CPU側データを保持する場合はレイキャスト用に保存.
	if (m_IsKeepCPUData)
	{
		m_Vertices = Vertices;
		m_Indices = Indices;
	}

	// 頂点バッファ作成.
	D3D11_BUFFER_DESC vb_desc = {};
	vb_desc.Usage = D3D11_USAGE_DEFAULT;
	vb_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * Vertices.size());
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vb_init_data = {};
	vb_init_data.pSysMem = Vertices.data();

	HRESULT vb_hr = p_device->CreateBuffer(&vb_desc, &vb_init_data, &m_pVB);
	if (FAILED(vb_hr)) { return false; }

	// インデックスバッファ作成.
	D3D11_BUFFER_DESC ib_desc = {};
	ib_desc.Usage = D3D11_USAGE_DEFAULT;
	ib_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * Indices.size());
	ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ib_init_data = {};
	ib_init_data.pSysMem = Indices.data();

	HRESULT ib_hr = p_device->CreateBuffer(&ib_desc, &ib_init_data, &m_pIB);
	if (FAILED(ib_hr))
	{
		if (m_pVB) m_pVB->Release();
		m_pVB = nullptr;
		return false;
	}

	return true;
}

// 描画.
bool FBXMesh::Render(DirectX11* pDX11)
{
	if (!pDX11 || !m_pVB || !m_pIB) { return false; }

	ID3D11DeviceContext* p_context = pDX11->GetContext();
	if (!p_context) { return false; }

	// 頂点バッファ設定.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	p_context->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);

	// インデックスバッファ設定.
	p_context->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジー設定.
	p_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// テクスチャ設定.
	if (m_Material.pTexture)
	{
		p_context->PSSetShaderResources(0, 1, &m_Material.pTexture);
	}

	// 描画.
	p_context->DrawIndexed(m_IndexCount, 0, 0);

	return true;
}

// レイキャスト処理（スタティックメッシュのみ）.
bool FBXMesh::IsRayCast(const Ray& Ray, HitInfo& OutHit) const
{
	if (!m_IsKeepCPUData || m_Vertices.empty() || m_Indices.empty())
	{
		OutHit.IsHit = false;
		return false;
	}

	OutHit.IsHit = false;
	OutHit.Distance = FLT_MAX;

	// 全三角形をチェック.
	for (size_t i = 0; i < m_Indices.size(); i += 3)
	{
		const Vertex& v0 = m_Vertices[m_Indices[i]];
		const Vertex& v1 = m_Vertices[m_Indices[i + 1]];
		const Vertex& v2 = m_Vertices[m_Indices[i + 2]];

		float t = {};
		// 三角形とレイの交差判定.
		if (IntersectTriangle(Ray, v0.Position, v1.Position, v2.Position, t))
		{
			if (t > 0.0f && t < OutHit.Distance)
			{
				OutHit.IsHit = true;
				OutHit.Distance = t;
				OutHit.TriangleIndex = static_cast<uint32_t>(i / 3);

				// 交差点の位置を計算.
				DirectX::XMVECTOR v_ray_origin = DirectX::XMLoadFloat3(&Ray.Origin);
				DirectX::XMVECTOR v_ray_dir = DirectX::XMLoadFloat3(&Ray.Direction);
				DirectX::XMVECTOR v_hit_pos = DirectX::XMVectorAdd(v_ray_origin, DirectX::XMVectorScale(v_ray_dir, t));
				DirectX::XMStoreFloat3(&OutHit.Position, v_hit_pos);

				// 法線を計算.
				DirectX::XMVECTOR v_v0 = DirectX::XMLoadFloat3(&v0.Position);
				DirectX::XMVECTOR v_v1 = DirectX::XMLoadFloat3(&v1.Position);
				DirectX::XMVECTOR v_v2 = DirectX::XMLoadFloat3(&v2.Position);
				DirectX::XMVECTOR v_edge1 = DirectX::XMVectorSubtract(v_v1, v_v0);
				DirectX::XMVECTOR v_edge2 = DirectX::XMVectorSubtract(v_v2, v_v0);
				DirectX::XMVECTOR v_normal = DirectX::XMVector3Cross(v_edge1, v_edge2);
				v_normal = DirectX::XMVector3Normalize(v_normal);
				DirectX::XMStoreFloat3(&OutHit.Normal, v_normal);
			}
		}
	}

	return OutHit.IsHit;
}

//=============================================================================
// 平面交差 + 内外判定法による三角形交差判定.
//
// 【アルゴリズム概要】
// Step1: 三角形の法線を計算する.
// Step2: レイと三角形平面の交差点を求める.
// Step3: 交差点が三角形の内部にあるか判定する.
//=============================================================================
bool FBXMesh::IntersectTriangle(
	const Ray& Ray,
	const DirectX::XMFLOAT3& V0,
	const DirectX::XMFLOAT3& V1,
	const DirectX::XMFLOAT3& V2,
	float& OutT) const
{
	const float EPSILON = 0.0001f;

	// 頂点をベクトルに変換.
	DirectX::XMVECTOR v_v0 = DirectX::XMLoadFloat3(&V0);
	DirectX::XMVECTOR v_v1 = DirectX::XMLoadFloat3(&V1);
	DirectX::XMVECTOR v_v2 = DirectX::XMLoadFloat3(&V2);
	DirectX::XMVECTOR v_ray_origin = DirectX::XMLoadFloat3(&Ray.Origin);
	DirectX::XMVECTOR v_ray_dir = DirectX::XMLoadFloat3(&Ray.Direction);

	//=========================================================================
	// Step1: 三角形の法線を計算.
	// 2辺の外積で法線ベクトルを求める.
	//=========================================================================
	DirectX::XMVECTOR v_edge1 = DirectX::XMVectorSubtract(v_v1, v_v0);  // v0 → v1.
	DirectX::XMVECTOR v_edge2 = DirectX::XMVectorSubtract(v_v2, v_v0);  // v0 → v2.
	DirectX::XMVECTOR v_normal = DirectX::XMVector3Cross(v_edge1, v_edge2);
	v_normal = DirectX::XMVector3Normalize(v_normal);

	//=========================================================================
	// Step2: レイと三角形平面の交差点を求める.
	// 平面方程式: dot(N, P - V0) = 0
	// レイ方程式: P = Origin + t * Direction
	// これを解くと: t = dot(N, V0 - Origin) / dot(N, Direction)
	//=========================================================================

	// レイ方向と法線の内積.
	float denom = DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_ray_dir));

	// レイが平面と平行な場合は交差しない.
	if (fabsf(denom) < EPSILON) { return false; }

	// 交差点までの距離 t を計算.
	DirectX::XMVECTOR v_to_origin = DirectX::XMVectorSubtract(v_v0, v_ray_origin);
	float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_to_origin)) / denom;

	// t が負の場合、交差点はレイの後方.
	if (t < EPSILON) { return false; }

	//=========================================================================
	// Step3: 交差点が三角形の内部にあるか判定.
	// 各辺に対して、交差点が辺の左側にあるかを外積で判定.
	// 3辺すべてで同じ側にあれば、点は三角形内部にある.
	//=========================================================================

	// 交差点を計算.
	DirectX::XMVECTOR v_hit_point = DirectX::XMVectorAdd(v_ray_origin, DirectX::XMVectorScale(v_ray_dir, t));

	// 辺1 (v0 → v1) に対するチェック.
	DirectX::XMVECTOR v_c0 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v1, v_v0),
		DirectX::XMVectorSubtract(v_hit_point, v_v0));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c0)) < 0.0f) { return false; }

	// 辺2 (v1 → v2) に対するチェック.
	DirectX::XMVECTOR v_c1 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v2, v_v1),
		DirectX::XMVectorSubtract(v_hit_point, v_v1));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c1)) < 0.0f) { return false; }

	// 辺3 (v2 → v0) に対するチェック.
	DirectX::XMVECTOR v_c2 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v0, v_v2),
		DirectX::XMVectorSubtract(v_hit_point, v_v2));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c2)) < 0.0f) { return false; }

	// 全チェックを通過 → 三角形内部で交差.
	OutT = t;
	return true;
}

// リソース解放.
void FBXMesh::Release()
{
	if (m_Material.pTexture)
	{
		m_Material.pTexture->Release();
		m_Material.pTexture = nullptr;
	}

	if (m_pIB)
	{
		m_pIB->Release();
		m_pIB = nullptr;
	}

	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}

	m_Vertices.clear();
	m_Indices.clear();
	m_IndexCount = 0;
	m_IsKeepCPUData = false;
}
