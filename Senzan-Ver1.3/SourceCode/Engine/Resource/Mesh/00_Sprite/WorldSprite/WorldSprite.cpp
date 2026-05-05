#include "WorldSprite.h"
#include "Graphic/DirectX/DirectX12/DirectX12.h"
#include "Engine/Utility/Assert/Assert.inl"
#include "MyMacro.h"

//繧ｷ繧ｧ繝ｼ繝繝輔ぃ繧､繝ｫ蜷搾ｼ医ョ繧｣繝ｬ繧ｯ繝医Μ繧ょ性繧・・
const TCHAR SHADER_NAME[] = _T("Data\\Shader\\Sprite3D.hlsl");

//繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ.
WorldSprite::WorldSprite()
	: m_pDx12(nullptr)
	, m_pDevice12(nullptr)
	, m_pCmdList12(nullptr)
	, m_pConstantBuffer(nullptr)
	, m_pVertexBuffer(nullptr)
	, m_pTexture(nullptr)
	, m_Position()
	, m_Rotation()
	, m_Scale(1.0f, 1.0f, 1.0f)
	, m_UV(0.0f, 0.0f)
	, m_Alpha(1.0f)
	, m_SpriteState()
	, m_PatternNo()
	, m_PatternMax()
	, m_Billboard(false)
{
}

//繝・せ繝医Λ繧ｯ繧ｿ.
WorldSprite::~WorldSprite()
{
	Release();
	m_pDx12 = nullptr;
}

//蛻晄悄蛹・
HRESULT WorldSprite::Init(DirectX12& pDx12, LPCTSTR pFileName, SPRITE_STATE& pSs)
{
	m_pDx12 = &pDx12;
	m_pDevice12 = m_pDx12->GetDevice().Get();
	m_pCmdList12 = m_pDx12->GetCommandList().Get();

	m_SpriteState = pSs;

	if (FAILED(CreateShader()))
	{
		return E_FAIL;
	}
	if (FAILED(CreateModel()))
	{
		return E_FAIL;
	}
	if (FAILED(CreateTexture(pFileName)))
	{
		return E_FAIL;
	}
	if (FAILED(CreateSampler()))
	{
		return E_FAIL;
	}

	return S_OK;
}

//隗｣謾ｾ.
void WorldSprite::Release()
{
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pConstantBuffer);

	m_pCmdList12 = nullptr;
	m_pDevice12 = nullptr;
}

//===========================================================.
//	HLSL繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ縺ｿ繧ｷ繧ｧ繝ｼ繝繧剃ｽ懈・縺吶ｋ.
//	HLSL: High Level Shading Language 縺ｮ逡･.
//===========================================================.
HRESULT WorldSprite::CreateShader()
{
	// TODO : DirectX12逕ｨ縺ｮPSO/RootSignature騾｣謳ｺ繧貞ｮ溯｣・☆繧・
	return S_OK;
}

//繝｢繝・Ν菴懈・.
HRESULT WorldSprite::CreateModel()
{
	const float w = m_SpriteState.Disp.w / 2.0f;
	const float h = m_SpriteState.Disp.h / 2.0f;

	m_PatternMax.x = static_cast<SHORT>(m_SpriteState.Base.w / m_SpriteState.Stride.w);
	m_PatternMax.y = static_cast<SHORT>(m_SpriteState.Base.h / m_SpriteState.Stride.h);

	VERTEX vertices[] =
	{
		{ DirectX::XMFLOAT3(-w, -h, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-w,  h, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3( w, -h, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3( w,  h, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
	};

	const UINT vertex_buffer_size = static_cast<UINT>(sizeof(vertices));

	D3D12_HEAP_PROPERTIES heap_props = {};
	heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resource_desc.Width = vertex_buffer_size;
	resource_desc.Height = 1;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	if (FAILED(m_pDevice12->CreateCommittedResource(
		&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer))))
	{
		return E_FAIL;
	}

	void* p_mapped = nullptr;
	if (FAILED(m_pVertexBuffer->Map(0, nullptr, &p_mapped)))
	{
		return E_FAIL;
	}
	memcpy(p_mapped, vertices, sizeof(vertices));
	m_pVertexBuffer->Unmap(0, nullptr);

	return S_OK;
}

//繝・け繧ｹ繝√Ε菴懈・.
HRESULT WorldSprite::CreateTexture(LPCTSTR pFileName)
{
	UNREFERENCED_PARAMETER(pFileName);
	// TODO : DirectX12縺ｮSRV/DescriptorHeap邨檎罰縺ｮ繝・け繧ｹ繝√Ε逕滓・繧貞ｮ溯｣・☆繧・
	return S_OK;
}

//繧ｵ繝ｳ繝励Λ菴懈・.
HRESULT WorldSprite::CreateSampler()
{
	// TODO : DirectX12縺ｮStaticSampler/DescriptorHeap邨檎罰縺ｫ鄂ｮ縺肴鋤縺医ｋ.
	return S_OK;
}

//繝ｬ繝ｳ繝繝ｪ繝ｳ繧ｰ逕ｨ.
void WorldSprite::Render()
{
	DirectX::XMMATRIX m_world;
	DirectX::XMMATRIX m_trans;
	DirectX::XMMATRIX m_rot;
	DirectX::XMMATRIX m_scale;

	m_scale = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

	DirectX::XMMATRIX m_yaw = DirectX::XMMatrixRotationY(m_Rotation.y);
	DirectX::XMMATRIX m_pitch = DirectX::XMMatrixRotationX(m_Rotation.x);
	DirectX::XMMATRIX m_roll = DirectX::XMMatrixRotationZ(m_Rotation.z);
	m_rot = DirectX::XMMatrixMultiply(m_yaw, m_pitch);
	m_rot = DirectX::XMMatrixMultiply(m_rot, m_roll);

	m_trans = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	DirectX::XMMATRIX m_world_temp = DirectX::XMMatrixMultiply(m_scale, m_rot);
	m_world = DirectX::XMMatrixMultiply(m_world_temp, m_trans);

    if (m_Billboard)
	{
		// TODO : CameraManager(DX12) 騾｣謳ｺ蠕後↓繝薙Ν繝懊・繝芽｡悟・縺ｸ鄂ｮ縺肴鋤縺医ｋ.
	}

	UNREFERENCED_PARAMETER(m_world);
	// TODO : DirectX12 縺ｮ繧ｳ繝槭Φ繝峨Μ繧ｹ繝医〒謠冗判縺吶ｋ螳溯｣・∈鄂ｮ縺肴鋤縺医ｋ.
}

