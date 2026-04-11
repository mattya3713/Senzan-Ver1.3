#include "WorldSprite.h"
#include "Graphic/DirectX/DirectX12/DirectX12.h"
#include "System/99_Utility/Assert/Assert.inl"
#include "MyMacro.h"

//シェーダファイル名（ディレクトリも含む）.
const TCHAR SHADER_NAME[] = _T("Data\\Shader\\Sprite3D.hlsl");

//コンストラクタ.
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

//デストラクタ.
WorldSprite::~WorldSprite()
{
	Release();
	m_pDx12 = nullptr;
}

//初期化.
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

//解放.
void WorldSprite::Release()
{
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pConstantBuffer);

	m_pCmdList12 = nullptr;
	m_pDevice12 = nullptr;
}

//===========================================================.
//	HLSLファイルを読み込みシェーダを作成する.
//	HLSL: High Level Shading Language の略.
//===========================================================.
HRESULT WorldSprite::CreateShader()
{
	// TODO : DirectX12用のPSO/RootSignature連携を実装する.
	return S_OK;
}

//モデル作成.
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

//テクスチャ作成.
HRESULT WorldSprite::CreateTexture(LPCTSTR pFileName)
{
	UNREFERENCED_PARAMETER(pFileName);
	// TODO : DirectX12のSRV/DescriptorHeap経由のテクスチャ生成を実装する.
	return S_OK;
}

//サンプラ作成.
HRESULT WorldSprite::CreateSampler()
{
	// TODO : DirectX12のStaticSampler/DescriptorHeap経由に置き換える.
	return S_OK;
}

//レンダリング用.
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
		// TODO : CameraManager(DX12) 連携後にビルボード行列へ置き換える.
	}

	UNREFERENCED_PARAMETER(m_world);
	// TODO : DirectX12 のコマンドリストで描画する実装へ置き換える.
}
