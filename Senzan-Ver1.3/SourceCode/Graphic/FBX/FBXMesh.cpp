#include "Graphic/FBX/FBXMesh.h"

FBXMesh::FBXMesh()
	: m_IndexCount(0)
	, m_IsKeepCPUData(false)
{
}

FBXMesh::~FBXMesh() noexcept
{
	Release();
}

bool FBXMesh::CreateBuffers(
	ID3D12Device* pDevice,
	ID3D12CommandQueue* pCommandQueue,
	const std::vector<Vertex>& Vertices,
	const std::vector<uint32_t>& Indices,
	const MaterialData& Material,
	bool IsKeepCPUData)
{
	if (!pDevice || !pCommandQueue || Vertices.empty() || Indices.empty()) 
	{ 
		return false; 
	}

	m_Material = Material;
	m_IndexCount = static_cast<uint32_t>(Indices.size());
	m_IsKeepCPUData = IsKeepCPUData;

	if (m_IsKeepCPUData)
	{
		m_Vertices = Vertices;
		m_Indices = Indices;
	}

	UINT vb_size = static_cast<UINT>(Vertices.size() * sizeof(Vertex));
	UINT ib_size = static_cast<UINT>(Indices.size() * sizeof(uint32_t));

	// ===== Upload Heap リソース作成 =====
	D3D12_HEAP_PROPERTIES upload_heap_prop = {};
	upload_heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	upload_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upload_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC buf_res_desc = {};
	buf_res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buf_res_desc.Alignment = 0;
	buf_res_desc.Width = vb_size;
	buf_res_desc.Height = 1;
	buf_res_desc.DepthOrArraySize = 1;
	buf_res_desc.MipLevels = 1;
	buf_res_desc.Format = DXGI_FORMAT_UNKNOWN;
	buf_res_desc.SampleDesc.Count = 1;
	buf_res_desc.SampleDesc.Quality = 0;
	buf_res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	buf_res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// Upload VB 作成.
	ComPtr<ID3D12Resource> upload_vb;
	if (FAILED(pDevice->CreateCommittedResource(
		&upload_heap_prop,
		D3D12_HEAP_FLAG_NONE,
		&buf_res_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload_vb))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create upload VB\n");
		return false;
	}

	// Upload IB 作成.
	buf_res_desc.Width = ib_size;
	ComPtr<ID3D12Resource> upload_ib;
	if (FAILED(pDevice->CreateCommittedResource(
		&upload_heap_prop,
		D3D12_HEAP_FLAG_NONE,
		&buf_res_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload_ib))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create upload IB\n");
		return false;
	}

	// ===== Default Heap リソース作成 =====
	D3D12_HEAP_PROPERTIES default_heap_prop = {};
	default_heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	default_heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	default_heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	buf_res_desc.Width = vb_size;
	if (FAILED(pDevice->CreateCommittedResource(
		&default_heap_prop,
		D3D12_HEAP_FLAG_NONE,
		&buf_res_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create default VB\n");
		return false;
	}

	buf_res_desc.Width = ib_size;
	if (FAILED(pDevice->CreateCommittedResource(
		&default_heap_prop,
		D3D12_HEAP_FLAG_NONE,
		&buf_res_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pIndexBuffer))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create default IB\n");
		return false;
	}

	// ===== CPU→GPU コピー =====
	// VB にデータをコピー.
	void* p_vb_mapped = nullptr;
	if (FAILED(upload_vb->Map(0, nullptr, &p_vb_mapped)))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to map upload VB\n");
		return false;
	}
	memcpy(p_vb_mapped, Vertices.data(), vb_size);
	upload_vb->Unmap(0, nullptr);

	// IB にデータをコピー.
	void* p_ib_mapped = nullptr;
	if (FAILED(upload_ib->Map(0, nullptr, &p_ib_mapped)))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to map upload IB\n");
		return false;
	}
	memcpy(p_ib_mapped, Indices.data(), ib_size);
	upload_ib->Unmap(0, nullptr);

	// ===== コマンドリスト経由で GPU にコピー =====
	ComPtr<ID3D12CommandAllocator> cmd_allocator;
	ComPtr<ID3D12GraphicsCommandList> cmd_list;

	if (FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmd_allocator))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create command allocator\n");
		return false;
	}

	if (FAILED(pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmd_allocator.Get(),
		nullptr,
		IID_PPV_ARGS(&cmd_list))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create command list\n");
		return false;
	}

	// VB コピーコマンド.
	D3D12_RESOURCE_BARRIER vb_barrier_before = {};
	vb_barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vb_barrier_before.Transition.pResource = m_pVertexBuffer.Get();
	vb_barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vb_barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	vb_barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmd_list->CopyResource(m_pVertexBuffer.Get(), upload_vb.Get());

	// IB コピーコマンド.
	cmd_list->CopyResource(m_pIndexBuffer.Get(), upload_ib.Get());

	// リソース状態遷移：COPY_DEST → GENERIC_READ.
	D3D12_RESOURCE_BARRIER barriers[2] = {};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Transition.pResource = m_pVertexBuffer.Get();
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[1].Transition.pResource = m_pIndexBuffer.Get();
	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmd_list->ResourceBarrier(2, barriers);

	if (FAILED(cmd_list->Close()))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to close command list\n");
		return false;
	}

	// コマンド実行.
	ID3D12CommandList* p_cmd_lists[] = { cmd_list.Get() };
	pCommandQueue->ExecuteCommandLists(1, p_cmd_lists);

	// GPU 完了待ち.
	ComPtr<ID3D12Fence> fence;
	if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create fence\n");
		return false;
	}

	pCommandQueue->Signal(fence.Get(), 1);

	HANDLE h_event = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	if (h_event == nullptr)
	{
		OutputDebugStringA("[FBXMesh::CreateBuffers] Failed to create event\n");
		return false;
	}

	fence->SetEventOnCompletion(1, h_event);
	WaitForSingleObject(h_event, INFINITE);
	CloseHandle(h_event);

	// ===== VB, IB View 設定 =====
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = vb_size;
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);

	m_IndexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = ib_size;
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	OutputDebugStringA("[FBXMesh::CreateBuffers] Buffers created and data copied successfully\n");
	return true;
}

bool FBXMesh::Render(ID3D12GraphicsCommandList* pCommandList)
{
	if (!pCommandList || m_IndexCount == 0 || !m_pVertexBuffer || !m_pIndexBuffer)
	{
		OutputDebugStringA("[FBXMesh::Render] Invalid parameters or resources\n");
		return false;
	}

	// VB, IB をセット.
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	pCommandList->IASetIndexBuffer(&m_IndexBufferView);

	// プリミティブトポロジー設定（三角形リスト）.
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ここでマテリアル定数バッファをバインドする
	// b2 レジスター: マテリアル定数バッファ
	// （注：FBXModel::Render() で b0, b1 をバインド済み）
	// マテリアル定数バッファはここでバインドする必要があります
	// ただし、現在 FBXMesh がマテリアル定数バッファを持たないため、
	// FBXModel から渡されるべきです

	// 描画コマンド発行.
	pCommandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);

	OutputDebugStringA("[FBXMesh::Render] Draw call issued successfully\n");
	return true;
}



bool FBXMesh::IsRayCast(const Ray& Ray, HitInfo& OutHit) const
{
	if (!m_IsKeepCPUData || m_Vertices.empty() || m_Indices.empty())
	{
		OutHit.IsHit = false;
		return false;
	}

	OutHit.IsHit = false;
	OutHit.Distance = FLT_MAX;

	for (size_t i = 0; i < m_Indices.size(); i += 3)
	{
		const Vertex& v0 = m_Vertices[m_Indices[i]];
		const Vertex& v1 = m_Vertices[m_Indices[i + 1]];
		const Vertex& v2 = m_Vertices[m_Indices[i + 2]];

		float t = {};
		if (IntersectTriangle(Ray, v0.Position, v1.Position, v2.Position, t))
		{
			if (t > 0.0f && t < OutHit.Distance)
			{
				OutHit.IsHit = true;
				OutHit.Distance = t;
				OutHit.TriangleIndex = static_cast<uint32_t>(i / 3);

				DirectX::XMVECTOR v_ray_origin = DirectX::XMLoadFloat3(&Ray.Origin);
				DirectX::XMVECTOR v_ray_dir = DirectX::XMLoadFloat3(&Ray.Direction);
				DirectX::XMVECTOR v_hit_pos = DirectX::XMVectorAdd(v_ray_origin, DirectX::XMVectorScale(v_ray_dir, t));
				DirectX::XMStoreFloat3(&OutHit.Position, v_hit_pos);

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

bool FBXMesh::IntersectTriangle(
	const Ray& Ray,
	const DirectX::XMFLOAT3& V0,
	const DirectX::XMFLOAT3& V1,
	const DirectX::XMFLOAT3& V2,
	float& OutT) const
{
	const float EPSILON = 0.0001f;

	DirectX::XMVECTOR v_v0 = DirectX::XMLoadFloat3(&V0);
	DirectX::XMVECTOR v_v1 = DirectX::XMLoadFloat3(&V1);
	DirectX::XMVECTOR v_v2 = DirectX::XMLoadFloat3(&V2);
	DirectX::XMVECTOR v_ray_origin = DirectX::XMLoadFloat3(&Ray.Origin);
	DirectX::XMVECTOR v_ray_dir = DirectX::XMLoadFloat3(&Ray.Direction);

	DirectX::XMVECTOR v_edge1 = DirectX::XMVectorSubtract(v_v1, v_v0);
	DirectX::XMVECTOR v_edge2 = DirectX::XMVectorSubtract(v_v2, v_v0);
	DirectX::XMVECTOR v_normal = DirectX::XMVector3Cross(v_edge1, v_edge2);
	v_normal = DirectX::XMVector3Normalize(v_normal);

	float denom = DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_ray_dir));
	if (fabsf(denom) < EPSILON) { return false; }

	DirectX::XMVECTOR v_to_origin = DirectX::XMVectorSubtract(v_v0, v_ray_origin);
	float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_to_origin)) / denom;
	if (t < EPSILON) { return false; }

	DirectX::XMVECTOR v_hit_point = DirectX::XMVectorAdd(v_ray_origin, DirectX::XMVectorScale(v_ray_dir, t));

	DirectX::XMVECTOR v_c0 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v1, v_v0),
		DirectX::XMVectorSubtract(v_hit_point, v_v0));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c0)) < 0.0f) { return false; }

	DirectX::XMVECTOR v_c1 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v2, v_v1),
		DirectX::XMVectorSubtract(v_hit_point, v_v1));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c1)) < 0.0f) { return false; }

	DirectX::XMVECTOR v_c2 = DirectX::XMVector3Cross(
		DirectX::XMVectorSubtract(v_v0, v_v2),
		DirectX::XMVectorSubtract(v_hit_point, v_v2));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(v_normal, v_c2)) < 0.0f) { return false; }

	OutT = t;
	return true;
}

void FBXMesh::Release()
{
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();
	m_Vertices.clear();
	m_Indices.clear();
	m_IndexCount = 0;
	m_IsKeepCPUData = false;
}
