#include "MyMacro.h"
#include "Graphic/DirectX/DirectX11.h"

// コンストラクタ.
DirectX11::DirectX11()
	: m_pDevice11				( nullptr )
	, m_pContext11				( nullptr )
	, m_pSwapChain				( nullptr )
	, m_pBackBuffer_TexRTV		( nullptr )
	, m_pBackBuffer_DSTex		( nullptr )
	, m_pBackBuffer_DSTexDSV	( nullptr )

	, m_pDepthStencilStateOn	( nullptr )
	, m_pDepthStencilStateOff	( nullptr )

	, m_pAlphaBlendOn			( nullptr )
	, m_pAlphaBlendOff			( nullptr )
{
}

// デストラクタ.
DirectX11::~DirectX11()
{
	Release();
}

// DirectX初期化.
HRESULT DirectX11::Create(HWND HWnd)
{
	// デバイスとスワップチェーンを作成.
	if( FAILED(
		CreateDeviceAndSwapChain(
			HWnd,
			static_cast<UINT>(FPS),
			static_cast<UINT>(WND_W),
			static_cast<UINT>(WND_H) )))
	{
		return E_FAIL;
	}

	// カラーバックバッファのレンダーターゲットビューを作成.
	// バックバッファを色バッファとして描画ターゲットに設定.
	if( FAILED(
		CreateColorBackBufferRTV() ) )
	{
		return E_FAIL;
	}

	// 深度バッファ用の深度ステンシルビューを作成.
	if( FAILED(
		CreateDepthStencilBackBufferRTV() ) )
	{
		return E_FAIL;
	}

	// レンダーターゲットビューと深度ステンシルビューをパイプラインに設定.
	m_pContext11->OMSetRenderTargets(
		1,
		&m_pBackBuffer_TexRTV,
		m_pBackBuffer_DSTexDSV );

	// 深度ステンシル状態を作成.
	if( FAILED( CreateDepthStencilState() ) ){
		return E_FAIL;
	}

	// アルファブレンド状態を作成.
	if( FAILED( CreateAlphaBlendState() ) ){
		return E_FAIL;
	}

	//------------------------------------------------
	// ビューポート設定.
	//------------------------------------------------
	D3D11_VIEWPORT vp = {};
	vp.Width	= WND_W;	// 幅.
	vp.Height	= WND_H;	// 高さ.
	vp.MinDepth = 0.0f;		// 最小深度.
	vp.MaxDepth = 1.0f;		// 最大深度.
	vp.TopLeftX = 0.0f;		// 左上x.
	vp.TopLeftY = 0.0f;		// 左上y.

	m_pContext11->RSSetViewports( 1, &vp );

	// ラスタライザ状態を作成.
	if( FAILED( CreateRasterizer() ) ){
		return E_FAIL;
	}

	return S_OK;
}

// リソース解放.
// 解放時は作成順と逆順で開放する.
void DirectX11::Release()
{
	SAFE_RELEASE( m_pAlphaBlendOff );
	SAFE_RELEASE( m_pAlphaBlendOn );

	SAFE_RELEASE( m_pDepthStencilStateOff );
	SAFE_RELEASE( m_pDepthStencilStateOn );

	SAFE_RELEASE( m_pBackBuffer_DSTexDSV );
	SAFE_RELEASE( m_pBackBuffer_DSTex );
	SAFE_RELEASE( m_pBackBuffer_TexRTV );

	SAFE_RELEASE( m_pSwapChain );
	SAFE_RELEASE( m_pContext11 );
	SAFE_RELEASE( m_pDevice11 );
}

// デバイスとスワップチェーンの作成.
HRESULT DirectX11::CreateDeviceAndSwapChain(
	HWND HWnd, UINT FPS, UINT Width, UINT Height )
{
	// スワップチェーン設定.
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount			= 1;		// バックバッファ数.
	sd.BufferDesc.Width		= Width;	// バックバッファ幅.
	sd.BufferDesc.Height	= Height;	// バックバッファ高さ.
	sd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;	// フォーマット(32bitカラー).
	sd.BufferDesc.RefreshRate.Numerator		= FPS;	// リフレッシュレート(分子) FPS:60.
	sd.BufferDesc.RefreshRate.Denominator	= 1;	// リフレッシュレート(分母).
	sd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画用.
	sd.OutputWindow			= HWnd;		// ウィンドウハンドル.
	sd.SampleDesc.Count		= 1;		// マルチサンプル数.
	sd.SampleDesc.Quality	= 0;		// マルチサンプル品質.
	sd.Windowed				= TRUE;		// ウィンドウモード.

	// 作成する機能レベルの優先順位を指定.
	// GPU対応の機能セット定義.
	// D3D_FEATURE_LEVEL_11_0: Direct3D 11.0 対応GPU.
	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = nullptr;	// 配列の要素用.

	// デバイスとスワップチェーンを作成.
	// ハードウェアGPUで作成.
	if( FAILED(
		D3D11CreateDeviceAndSwapChain(
			nullptr,					// ビデオアダプタへのポインタ.
			D3D_DRIVER_TYPE_HARDWARE,	// 作成するデバイスの種類.
			nullptr,					// ソフトウェアラスタライザDLLのハンドル.
			0,							// 有効なランタイムオプション.
			&pFeatureLevels,			// 作成する機能レベル配列へのポインタ.
			1,							// 配列数.
			D3D11_SDK_VERSION,			// SDKバージョン.
			&sd,						// スワップチェーン設定のポインタ.
			&m_pSwapChain,				// (out) スワップチェーン.
			&m_pDevice11,				// (out) デバイス.
			pFeatureLevel,				// 機能レベル配列の先頭要素ポインタ.
			&m_pContext11 ) ) )			// (out) デバイスコンテキスト.
	{
		// WARPデバイスの作成.
		// D3D_FEATURE_LEVEL_9_1 ～ D3D_FEATURE_LEVEL_10_1.
		if( FAILED(
			D3D11CreateDeviceAndSwapChain(
				nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
				0, &pFeatureLevels, 1, D3D11_SDK_VERSION,
				&sd, &m_pSwapChain, &m_pDevice11,
				pFeatureLevel, &m_pContext11 ) ) )
		{
			// リファレンスデバイスの作成.
			// DirectX SDK未インストールでは使用不可.
			if( FAILED(
				D3D11CreateDeviceAndSwapChain(
					nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr,
					0, &pFeatureLevels, 1, D3D11_SDK_VERSION,
					&sd, &m_pSwapChain, &m_pDevice11,
					pFeatureLevel, &m_pContext11 ) ) )
			{
				MessageBox( nullptr,
					_T( "デバイスとスワップチェーンの作成に失敗しました" ),
					_T( "Error" ), MB_OK );
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

// ラスタライザ作成.
// ラスタライズ設定(三角形の描画状態)を設定する.
HRESULT DirectX11::CreateRasterizer()
{
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.FillMode = D3D11_FILL_SOLID;	// 塗りつぶし。

	// カリング設定.
	// D3D11_CULL_BACK	: 背面を描画しない.
	// D3D11_CULL_FRONT	: 正面を描画しない.
	// D3D11_CULL_NONE	: カリングしない.
	rdc.CullMode = D3D11_CULL_NONE;

	// ポリゴンの表裏判定.
	// TRUE  : 反時計回りが正面.
	// FALSE : 時計回りが正面.
	rdc.FrontCounterClockwise = FALSE;

	// 深度方向のクリッピングを無効化.
	rdc.DepthClipEnable = FALSE;

	ID3D11RasterizerState* pRs = nullptr;
	m_pDevice11->CreateRasterizerState( &rdc, &pRs );
	m_pContext11->RSSetState( pRs );
	SAFE_RELEASE( pRs );

	return S_OK;
}

// 深度ステンシル状態を作成.
// この関数でON/OFFの2種類を作成する.
HRESULT DirectX11::CreateDepthStencilState()
{
	// 深度テスト設定.
	// ON/OFF共通の項目のみ設定.
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc		= D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable	= FALSE;
	dsDesc.StencilReadMask	= D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask	= D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// 深度テストを有効にする.
	dsDesc.DepthEnable = TRUE;
	if( FAILED(
		m_pDevice11->CreateDepthStencilState(
			&dsDesc, &m_pDepthStencilStateOn ) ) )
	{
		_ASSERT_EXPR( false, _T("深度ON状態の作成に失敗しました") );
		return E_FAIL;
	}

	// 深度テストを無効にする.
	dsDesc.DepthEnable = FALSE;
	if( FAILED(
		m_pDevice11->CreateDepthStencilState(
			&dsDesc, &m_pDepthStencilStateOff ) ) )
	{
		_ASSERT_EXPR( false, _T( "深度OFF状態の作成に失敗しました" ) );
		return E_FAIL;
	}

	return S_OK;
}

// ブレンド状態を作成.
// アルファブレンド用のON/OFFを作成する.
HRESULT DirectX11::CreateAlphaBlendState()
{
	// アルファブレンド用ブレンド状態設定.
	// pngなどでアルファ値を持つため、透過表現に対応する.
	D3D11_BLEND_DESC BlendDesc = {};

	BlendDesc.IndependentBlendEnable = false;
	BlendDesc.AlphaToCoverageEnable = false;

	// 画素に対する設定.
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	// アルファブレンドを有効にする.
	BlendDesc.RenderTarget[0].BlendEnable = true;
	if( FAILED(
		m_pDevice11->CreateBlendState( &BlendDesc, &m_pAlphaBlendOn ) ) )
	{
		_ASSERT_EXPR( false, _T( "アルファブレンド状態(ON)の作成に失敗しました" ) );
		return E_FAIL;
	}

	// アルファブレンドを無効にする.
	BlendDesc.RenderTarget[0].BlendEnable = false;
	if( FAILED(
		m_pDevice11->CreateBlendState( &BlendDesc, &m_pAlphaBlendOff ) ) )
	{
		_ASSERT_EXPR( false, _T( "アルファブレンド状態(OFF)の作成に失敗しました" ) );
		return E_FAIL;
	}

	return S_OK;
}

// ブレンド設定の切り替え.
void DirectX11::SetAlphaBlend(bool IsEnable)
{
	UINT mask = 0xffffffff;
	ID3D11BlendState* p_tmp = IsEnable ? m_pAlphaBlendOn : m_pAlphaBlendOff;

	// アルファブレンド設定を適用.
	m_pContext11->OMSetBlendState( p_tmp, nullptr, mask );
}

// 深度テストON/OFF切り替え.
void DirectX11::SetDepth(bool IsEnable)
{
	ID3D11DepthStencilState* p_tmp
		= IsEnable ? m_pDepthStencilStateOn : m_pDepthStencilStateOff;

	// 深度設定を適用.
	m_pContext11->OMSetDepthStencilState( p_tmp, 1 );
}

// バックバッファクリア関数.
// この関数を呼ぶ前にレンダリングすること.
void DirectX11::ClearBackBuffer()
{
	// 画面のクリア.
	float ClearColor[4] = { 0.0f, 0.0f, 0.6f, 1.0f };	// クリア色(RGBA).
	// カラーバックバッファ.
	m_pContext11->ClearRenderTargetView(
		m_pBackBuffer_TexRTV, ClearColor );
	// 深度ステンシルバックバッファ.
	m_pContext11->ClearDepthStencilView(
		m_pBackBuffer_DSTexDSV,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0 );
}

// 表示.
void DirectX11::Present()
{
	m_pSwapChain->Present( 0, 0 );
}

// バックバッファ作成: カラー用レンダーターゲットビュー作成.
HRESULT DirectX11::CreateColorBackBufferRTV()
{
	// バックバッファテクスチャを取得(内部で作成済みのため新規作成ではない).
	ID3D11Texture2D* pBackBuffer_Tex = nullptr;
	if( FAILED(
		m_pSwapChain->GetBuffer(
			0,
			__uuidof( ID3D11Texture2D ),
			(LPVOID*)&pBackBuffer_Tex ) ) )
	{
		_ASSERT_EXPR( false, _T( "スワップチェーンからバックバッファの取得に失敗しました" ) );
		return E_FAIL;
	}

	// このテクスチャに対してレンダーターゲットビュー(RTV)を作成.
	if( FAILED(
		m_pDevice11->CreateRenderTargetView(
			pBackBuffer_Tex,
			nullptr,
			&m_pBackBuffer_TexRTV ) ) )
	{
		_ASSERT_EXPR( false, _T( "レンダーターゲットビューの作成に失敗しました" ) );
		return E_FAIL;
	}
	SAFE_RELEASE( pBackBuffer_Tex );

	return S_OK;
}

// バックバッファ作成: 深度ステンシル用レンダーターゲットビュー作成.
HRESULT DirectX11::CreateDepthStencilBackBufferRTV()
{
	// 深度(またはステンシル)用のテクスチャを作成.
	D3D11_TEXTURE2D_DESC	descDepth = {};
	descDepth.Width = static_cast<UINT>(WND_W);		// 幅.
	descDepth.Height = static_cast<UINT>(WND_H);	// 高さ.
	descDepth.MipLevels = 1;		// ミップマップレベル.
	descDepth.ArraySize	= 1;		// 配列数.
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;	// 32ビット深度フォーマット.
	descDepth.SampleDesc.Count = 1;				// マルチサンプル数.
	descDepth.SampleDesc.Quality = 0;			// マルチサンプル品質.
	descDepth.Usage = D3D11_USAGE_DEFAULT;		// 標準.
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// 深度ステンシルとして使用.
	descDepth.CPUAccessFlags = 0;				// CPUアクセスなし.
	descDepth.MiscFlags = 0;					// その他設定なし.

	if( FAILED(
		m_pDevice11->CreateTexture2D(
			&descDepth,
			nullptr,
			&m_pBackBuffer_DSTex ) ) )
	{
		return E_FAIL;
	}

	// このテクスチャに対して深度ステンシルビュー(DSV)を作成.
	if( FAILED( m_pDevice11->CreateDepthStencilView(
		m_pBackBuffer_DSTex,
		nullptr,
		&m_pBackBuffer_DSTexDSV ) ) )
	{
		_ASSERT_EXPR( false, _T( "深度ステンシルビューの作成に失敗しました" ) );
		return E_FAIL;
	}

	return S_OK;
}
