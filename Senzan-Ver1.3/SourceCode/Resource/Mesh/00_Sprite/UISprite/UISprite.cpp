#include "UISprite.h"

#include <array>
#include <fstream>

#include "Graphic/DirectX/DirectX12/DirectX12.h"
#include "MyMacro.h"

namespace
{
	constexpr TCHAR VS_FILE_PATH[] = _T("Data\\Shader\\Sprite2D\\Sprite2DVS.hlsl");
	constexpr TCHAR PS_FILE_PATH[] = _T("Data\\Shader\\Sprite2D\\Sprite2DPS.hlsl");
}

Sprite2D::Sprite2D()
	: m_pDx12(nullptr)
	, m_pDevice12(nullptr)
	, m_pCmdList12(nullptr)
	, m_pRectTransform(std::make_unique<RectTransform>())
	, m_pCashVertexBuffers()
	, m_pConstantBuffer(nullptr)
	, m_pTexture(nullptr)
	, m_pSampleLinear(nullptr)
	, m_ResourceName()
	, m_WorldMatrix()
	, m_DrawSize()
	, m_Color({ 1.0f, 1.0f ,1.0f , 1.0f })
{
}

Sprite2D::~Sprite2D()
{
	SAFE_RELEASE(m_pSampleLinear);
	SAFE_RELEASE(m_pTexture);

	for (auto& pair_resource : m_pCashVertexBuffers)
	{
		SAFE_RELEASE(pair_resource.second);
	}

	SAFE_RELEASE(m_pConstantBuffer);
}

bool Sprite2D::Initialize(const std::filesystem::path& FilePath)
{
	UNREFERENCED_PARAMETER(VS_FILE_PATH);
	UNREFERENCED_PARAMETER(PS_FILE_PATH);

	m_ResourceName = FilePath.stem().string();
	LoadImageSize(FilePath);

	CreateShader();
	CreateModel();
	CreateTexture(FilePath.wstring());
	CreateSampler();

	return true;
}

void Sprite2D::Render()
{
	CalcWorldMatrix();

	// TODO : DirectX12 の UI 描画パス（PSO/RootSignature/DescriptorHeap）に接続する.
}

const std::unique_ptr<RectTransform>& Sprite2D::GetRectTransform() const
{
	return m_pRectTransform;
}

void Sprite2D::SetDrawSize(const DirectX::XMFLOAT2& DrawSize)
{
	m_DrawSize = DrawSize;
}

void Sprite2D::SetColor(const DirectX::XMFLOAT4& Color)
{
	m_Color = Color;
}

const std::string& Sprite2D::GetResourceName() const
{
	return m_ResourceName;
}

void Sprite2D::SetResourceName(const std::string& Name)
{
	m_ResourceName = Name;
}

ID3D12Resource* Sprite2D::GetTexture()
{
	return m_pTexture;
}

void Sprite2D::LoadImageSize(const std::filesystem::path& FilePath)
{
	std::ifstream file_stream(FilePath, std::ios::binary);
	if (!file_stream.is_open())
	{
		return;
	}

	file_stream.seekg(16);

	std::array<uint32_t, 2> size = {};
	file_stream.read(reinterpret_cast<char*>(size.data()), sizeof(size));

	constexpr auto swap_endian = [](uint32_t Value) -> uint32_t
		{
			return ((Value >> 24) & 0x000000FF)
				| ((Value >> 8) & 0x0000FF00)
				| ((Value << 8) & 0x00FF0000)
				| ((Value << 24) & 0xFF000000);
		};

	const float width = static_cast<float>(swap_endian(size[0]));
	const float height = static_cast<float>(swap_endian(size[1]));

	m_pRectTransform->SetSize({ width, height });
	m_DrawSize = { width, height };
}

void Sprite2D::CreateShader()
{
	// TODO : DirectX12 の PSO/RootSignature 生成へ置き換える.
}

HRESULT Sprite2D::CreateModel()
{
	if (m_pDevice12 == nullptr)
	{
		return S_OK;
	}

	const float width = m_pRectTransform->GetSize().x;
	const float height = m_pRectTransform->GetSize().y;
	const DirectX::XMFLOAT2 pivot = m_pRectTransform->GetPivot();

	const float origin_x = -width * pivot.x;
	const float origin_y = -height * pivot.y;

	VERTEX vertices[] =
	{
		{ DirectX::XMFLOAT3(origin_x,         origin_y + height, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(origin_x,         origin_y,          0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(origin_x + width, origin_y + height, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(origin_x + width, origin_y,          0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
	};

	const UINT vertex_buffer_size = static_cast<UINT>(sizeof(vertices));

	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resource_desc.Width = vertex_buffer_size;
	resource_desc.Height = 1;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* p_vertex_buffer = nullptr;
	if (FAILED(m_pDevice12->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&p_vertex_buffer))))
	{
		return E_FAIL;
	}

	void* p_mapped_data = nullptr;
	if (FAILED(p_vertex_buffer->Map(0, nullptr, &p_mapped_data)))
	{
		SAFE_RELEASE(p_vertex_buffer);
		return E_FAIL;
	}
	memcpy(p_mapped_data, vertices, sizeof(vertices));
	p_vertex_buffer->Unmap(0, nullptr);

	if (m_pCashVertexBuffers.find(pivot) != m_pCashVertexBuffers.end())
	{
		SAFE_RELEASE(m_pCashVertexBuffers[pivot]);
	}
	m_pCashVertexBuffers[pivot] = p_vertex_buffer;

	return S_OK;
}

HRESULT Sprite2D::CreateTexture(const std::wstring& FilePath)
{
	UNREFERENCED_PARAMETER(FilePath);
	// TODO : DirectX12 のテクスチャロード + SRV 作成へ置き換える.
	return S_OK;
}

HRESULT Sprite2D::CreateSampler()
{
	// TODO : DirectX12 の StaticSampler / Descriptor 経由へ置き換える.
	return S_OK;
}

void Sprite2D::CalcWorldMatrix()
{
	const Transform& transform = m_pRectTransform->GetTransform();

	DirectX::XMMATRIX m_trans;
	DirectX::XMMATRIX m_rot;
	DirectX::XMMATRIX m_scale;

	m_scale = DirectX::XMMatrixScaling(
		transform.Scale.x,
		transform.Scale.y,
		transform.Scale.z);

	DirectX::XMMATRIX m_yaw = DirectX::XMMatrixRotationY(transform.Rotation.y);
	DirectX::XMMATRIX m_pitch = DirectX::XMMatrixRotationX(transform.Rotation.x);
	DirectX::XMMATRIX m_roll = DirectX::XMMatrixRotationZ(transform.Rotation.z);

	m_rot = DirectX::XMMatrixMultiply(m_yaw, m_pitch);
	m_rot = DirectX::XMMatrixMultiply(m_rot, m_roll);

	const DirectX::XMFLOAT3 anchored_position = m_pRectTransform->CalcAnchoredPosition();
	m_trans = DirectX::XMMatrixTranslation(
		anchored_position.x,
		anchored_position.y,
		anchored_position.z);

	DirectX::XMMATRIX m_world_temp = DirectX::XMMatrixMultiply(m_scale, m_rot);
	m_WorldMatrix = DirectX::XMMatrixMultiply(m_world_temp, m_trans);
}

ID3D12Resource* Sprite2D::GetUseVertexBuffer()
{
	const DirectX::XMFLOAT2& pivot = m_pRectTransform->GetPivot();

	if (m_pCashVertexBuffers.find(pivot) == m_pCashVertexBuffers.end())
	{
		if (FAILED(CreateModel()))
		{
			return nullptr;
		}
	}

	return m_pCashVertexBuffers[pivot];
}
