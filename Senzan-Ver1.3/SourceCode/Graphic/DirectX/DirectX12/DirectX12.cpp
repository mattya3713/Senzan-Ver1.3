#include "DirectX12.h"
#include "MyMacro.h"
#include "Utility/String/FilePath/FilePath.h"
#include "Utility/Assert/Assert.inl"
#include <array>
#include <iostream>
#include <stdexcept>


DirectX12::DirectX12()
	: m_hWnd			( nullptr )
	, m_pDxgiFactory	( nullptr )
	, m_pSwapChain		( nullptr )
	, m_pDevice12		( nullptr )
	, m_pCmdAllocator	( nullptr )
	, m_pCmdList		( nullptr )
	, m_pCmdQueue		( nullptr )
	, m_pRenderTargetViewHeap( nullptr )	
	, m_pBackBuffer		( )
	, m_pDepthBuffer	( nullptr ) 
	, m_pDepthHeap		( nullptr ) 
	, m_DepthClearValue	( ) 
	, m_pSceneConstBuff	( nullptr )
	, m_pMappedSceneData( nullptr )
	, m_pFence			( nullptr )
	, m_FenceValue		( 0 )
	, m_pPipelineState	( nullptr )	
	, m_pRootSignature	( nullptr )
	, m_LoadLambdaTable	()
	, m_ResourceTable	()
{							 
}

DirectX12::~DirectX12()
{
	if (m_hFenceEvent != nullptr)
	{
		CloseHandle(m_hFenceEvent);
		m_hFenceEvent = nullptr; // nullptr にして二重解放を防ぐ
	}
}

bool DirectX12::Create(HWND hWnd)
{
	m_hWnd = hWnd;

#if _DEBUG
	// デバッグレイヤーをオン.
	EnableDebuglayer();

#endif

	try {
		
		// DXGIの生成.
		CreateDXGIFactory(
			m_pDxgiFactory);
	
		// コマンド類の生成.
		CreateCommandObject(
			m_pCmdAllocator,
			m_pCmdList,
			m_pCmdQueue);
		
		// スワップチェーンの生成.
		CreateSwapChain(
			m_pSwapChain);

		// レンダーターゲットの作成.
		CreateRenderTarget(
			m_pRenderTargetViewHeap,
			m_pBackBuffer);

		// テクスチャロードテーブルの作成.
		CreateTextureLoadTable();

		// 深度バッファの作成.
		CreateDepthDesc(
			m_pDepthBuffer, 
			m_pDepthHeap,
			m_pDepthSRVHeap);
		
		// ビューの設定.
		CreateSceneDesc();

		// PSO と Root Signature の作成.
		if (!CreatePipelineStateAndRootSignature())
		{
			throw std::runtime_error("PSO and Root Signature creation failed.");
		}

		// フェンスの表示.
		CreateFance(
			m_pFence);
	}
	catch (const std::runtime_error& Msg) {

		// エラーメッセージを表示.
		std::string message = Msg.what();
		std::wstring w_message(message.begin(), message.end());
		_ASSERT_EXPR(false, w_message.c_str());
		return false;
	}
	
	return true;
}
#include <DirectXMath.h>

DirectX::XMVECTOR m_Position = DirectX::XMVectorSet(0.0f, 20.0f, -40.0f, 1.0f);
// メンバー変数に追加 (カメラのターゲット位置)
DirectX::XMVECTOR m_TargetPosition = DirectX::XMVectorSet(0.0f, 15.0f, 0.0f, 1.0f); // モデルの中心あたりを見るように調整

// 更新
void DirectX12::Update()
{
	// 既存のカメラ位置とターゲット位置を取得
	DirectX::XMVECTOR currentEye = m_Position;
	float m_MoveSpeed = 1.0f;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		m_MoveSpeed = 0.1f; // Shiftキーで移動速度を遅くする
	}
	DirectX::XMVECTOR currentTarget = m_TargetPosition;

	// 現在のカメラの向きを表すビュー行列を計算 (後で逆行列を使って軸を抽出するため)
	// ここで計算するビュー行列は、UpdateSceneBufferで計算するview行列と同じロジックであるべき
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(currentEye, currentTarget, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	// カメラのローカル軸（前方、右、上）をビュー行列の逆行列から抽出
	// ビュー行列の逆行列は、ワールド空間におけるカメラの変換行列になる
	DirectX::XMMATRIX invViewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);

	// カメラのローカル前方ベクトル (Z軸の逆方向)
	// 行列の3列目が前方ベクトル。LookAtLHでは-Zが前方なので、反転させる
	// HLSLではmul(vector, matrix)で使う場合、列ベクトルが行列の各列に格納される。
	// DirectX::XMMATRIXは行優先なので、行ベクトルは行列の各行に格納される。
	// ビュー行列はワールド空間からカメラ空間への変換なので、その逆行列はカメラ空間からワールド空間への変換。
	// 逆ビュー行列のZ軸成分 (3行目) がワールド空間でのカメラの「Z軸」の方向。
	// LookAtLHの場合、カメラは-Z方向を見ているので、カメラの「前方」はワールドの-Z方向に近くなります。
	// そのため、invViewMatrixの3行目（Z軸）を取り出すと、それはカメラのローカルZ軸がワールド空間のどちらを向いているかを示すベクトルになります。
	// カメラの「前方」は、このZ軸を反転させた方向になります。
	DirectX::XMVECTOR cameraForward = DirectX::XMVector3Normalize(invViewMatrix.r[2]); // Z軸
	DirectX::XMVECTOR cameraRight = DirectX::XMVector3Normalize(invViewMatrix.r[0]); // X軸
	DirectX::XMVECTOR cameraUp = DirectX::XMVector3Normalize(invViewMatrix.r[1]); // Y軸

	// Q/EキーはワールドのY軸に沿って移動する方が直感的かもしれません
	DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	if (GetAsyncKeyState('W') & 0x8000) {
		// 前方移動 (LookAtLHのデフォルトのカメラ方向は-Zなので、逆行列のZ軸の向きがワールド空間のZ軸方向を向く。
		// カメラの進行方向は、そのベクトルの逆方向 (モデルに向かう方向) になることが多い)
		// もしカメラがZ軸正方向を向いているとしたら XMVECTOR cameraForward = invViewMatrix.r[2];
		// XMMatrixLookAtLHの場合、カメラは-Z方向を見ているので、invViewMatrix.r[2] はカメラのローカルZ軸（後方）
		// 前進はこれの逆方向
		currentEye = DirectX::XMVectorAdd(currentEye, DirectX::XMVectorScale(cameraForward, m_MoveSpeed));
		currentTarget = DirectX::XMVectorAdd(currentTarget, DirectX::XMVectorScale(cameraForward, m_MoveSpeed));
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		// 後方移動
		currentEye = DirectX::XMVectorSubtract(currentEye, DirectX::XMVectorScale(cameraForward, m_MoveSpeed));
		currentTarget = DirectX::XMVectorSubtract(currentTarget, DirectX::XMVectorScale(cameraForward, m_MoveSpeed));
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		// 左移動
		currentEye = DirectX::XMVectorSubtract(currentEye, DirectX::XMVectorScale(cameraRight, m_MoveSpeed));
		currentTarget = DirectX::XMVectorSubtract(currentTarget, DirectX::XMVectorScale(cameraRight, m_MoveSpeed));
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		// 右移動
		currentEye = DirectX::XMVectorAdd(currentEye, DirectX::XMVectorScale(cameraRight, m_MoveSpeed));
		currentTarget = DirectX::XMVectorAdd(currentTarget, DirectX::XMVectorScale(cameraRight, m_MoveSpeed));
	}
	if (GetAsyncKeyState('Q') & 0x8000) { // Qキーで上に移動 (ワールドY軸方向)
		currentEye = DirectX::XMVectorAdd(currentEye, DirectX::XMVectorScale(worldUp, m_MoveSpeed));
		currentTarget = DirectX::XMVectorAdd(currentTarget, DirectX::XMVectorScale(worldUp, m_MoveSpeed));
	}
	if (GetAsyncKeyState('E') & 0x8000) { // Eキーで下に移動 (ワールドY軸方向)
		currentEye = DirectX::XMVectorSubtract(currentEye, DirectX::XMVectorScale(worldUp, m_MoveSpeed));
		currentTarget = DirectX::XMVectorSubtract(currentTarget, DirectX::XMVectorScale(worldUp, m_MoveSpeed));
	}

	m_Position = currentEye; // 更新されたカメラ位置
	m_TargetPosition = currentTarget; // 更新されたターゲット位置

	UpdateSceneBuffer(); // 更新
}

void DirectX12::UpdateSceneBuffer()
{
	if (m_pMappedSceneData)
	{
		DirectX::XMFLOAT3 currentEye = {};
		DirectX::XMStoreFloat3(&currentEye, m_Position); // 更新されたm_Positionを使用

		DirectX::XMFLOAT3 target = {};
		DirectX::XMStoreFloat3(&target, m_TargetPosition); // 更新されたm_TargetPositionを使用

		DirectX::XMFLOAT3 up(0, 1, 0); // ワールドの上方向は固定

		m_pMappedSceneData->view =
			DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&currentEye),
			DirectX::XMLoadFloat3(&target),
			DirectX::XMLoadFloat3(&up));

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		auto result = m_pSwapChain->GetDesc1(&desc);
		m_pMappedSceneData->proj =
			DirectX::XMMatrixPerspectiveFovLH
			(DirectX::XM_PIDIV4, // 画角は45°
			static_cast<float>(desc.Width) / static_cast<float>(desc.Height), // アス比
			0.1f, // 近い方
			1000.0f // 遠い方
			);

		m_pMappedSceneData->eye = currentEye; // 定数バッファのeyeも更新
	}
	else
	{
		std::cerr << "Warning: m_pMappedSceneData is null in UpdateSceneBuffer()." << std::endl;
	}
}

void DirectX12::BeginDraw()
{
	// DirectX処理.
	// バックバッファのインデックスを取得.
	auto BBIdx = m_pSwapChain->GetCurrentBackBufferIndex();
	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pBackBuffer[BBIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_pCmdList->ResourceBarrier(1, &Barrier);

	// レンダーターゲットを指定.
	auto rtvH = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += BBIdx * m_pDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 深度を指定.
	auto DSVHeapPointer = m_pDepthHeap->GetCPUDescriptorHandleForHeapStart();
	m_pCmdList->OMSetRenderTargets(1, &rtvH, false, &DSVHeapPointer);
	m_pCmdList->ClearDepthStencilView(DSVHeapPointer, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 画面クリア.
	float ClearColor[] = { 0.f,0.f,0.f,1.0f };
	m_pCmdList->ClearRenderTargetView(rtvH, ClearColor, 0, nullptr);

	//ビューポート、0.シザー矩形のセット.
	m_pCmdList->RSSetViewports(1, m_pViewport.get());
	m_pCmdList->RSSetScissorRects(1, m_pScissorRect.get());
}

void DirectX12::EndDraw()
{
	auto BBIdx = m_pSwapChain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pBackBuffer[BBIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_pCmdList->ResourceBarrier(1, &barrier);

	// 命令のクローズ.
	m_pCmdList->Close();

	// コマンドリストの実行.
	ID3D12CommandList* cmdlists[] = { m_pCmdList.Get() };
	m_pCmdQueue->ExecuteCommandLists(1, cmdlists);

	// SwapChain の Present を呼び出す (ここで一度だけ行われる)
	m_pSwapChain->Present(1, 0);

	// 待ち.
	WaitForGPU();
	// キューをクリア.
	m_pCmdAllocator->Reset();
	// 再びコマンドリストをためる準備.
	m_pCmdList->Reset(m_pCmdAllocator.Get(), nullptr);
}

// スワップチェーンを取得.
const MyComPtr<IDXGISwapChain4> DirectX12::GetSwapChain()
{
	return m_pSwapChain;
}

// DirectX12デバイスを取得.
const MyComPtr<ID3D12Device> DirectX12::GetDevice()
{
	return m_pDevice12;
}

// コマンドリストを取得.
const MyComPtr<ID3D12GraphicsCommandList> DirectX12::GetCommandList()
{
	return m_pCmdList;
}

// コマンドキューを取得.
const MyComPtr<ID3D12CommandQueue> DirectX12::GetCommandQueue()
{
	return m_pCmdQueue;
}

// テクスチャを取得.
MyComPtr<ID3D12Resource> DirectX12::GetTextureByPath(const char* texpath)
{
	// リソーステーブル内を検索.
	auto [iterator, Result] = m_ResourceTable.emplace(
		texpath,
		nullptr 
	);

	if (Result) {
		// パスが未定義だった場合生成する.
		iterator->second = CreateTextureFromFile(texpath);
	}

	// マップ内のリソースを返す
	return iterator->second;
}

// GPUの完了待ち.
void DirectX12::WaitForGPU()
{
	m_pCmdQueue->Signal(m_pFence.Get(), ++m_FenceValue);

	if (m_pFence->GetCompletedValue() < m_FenceValue) {
		// eventが正常に作成されたかを確認.
		if (m_hFenceEvent != nullptr)
		{
			m_pFence->SetEventOnCompletion(m_FenceValue, m_hFenceEvent);
			WaitForSingleObject(m_hFenceEvent, INFINITE);
		}
		else {
			OutputDebugString(L"Failed to create event!\n");
		}
	}
}

// DXGIの生成.
void DirectX12::CreateDXGIFactory(MyComPtr<IDXGIFactory6>& DxgiFactory)
{
#ifdef _DEBUG
	//HRESULT result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(DxgiFactory.ReleaseAndGetAddressOf()));

	MyAssert::IsFailed(
		_T("DXGIの生成"),
		&CreateDXGIFactory2,
		DXGI_CREATE_FACTORY_DEBUG,			// デバッグモード.
		IID_PPV_ARGS(DxgiFactory.ReleaseAndGetAddressOf()));		// (Out)DXGI.
#else // _DEBUG
	MyAssert::IsFailed(
		_T("DXGIの生成"),
		&CreateDXGIFactory1,
		IID_PPV_ARGS(m_pDxgiFactory.ReleaseAndGetAddressOf()));
#endif

	// フィーチャレベル列挙.
	D3D_FEATURE_LEVEL Levels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	HRESULT Ret = S_OK;
	D3D_FEATURE_LEVEL FeatureLevel;

	for (auto Lv: Levels)
	{
		// DirectX12を実体化.
		if (D3D12CreateDevice(
			FindAdapter(L"NVIDIA"),				// グラボを選択.
			Lv,									// フィーチャーレベル.
			IID_PPV_ARGS(m_pDevice12.ReleaseAndGetAddressOf())) == S_OK)// (Out)Direct12.
		{
			// フィーチャーレベル.
			FeatureLevel = Lv;
			break;
		}
	}

}

// コマンド類の生成.
void DirectX12::CreateCommandObject(
	MyComPtr<ID3D12CommandAllocator>&	CmdAllocator,
	MyComPtr<ID3D12GraphicsCommandList>&CmdList,
	MyComPtr<ID3D12CommandQueue>&		CmdQueue)
{
	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	MyAssert::IsFailed(
		_T("コマンドリストアロケーターの生成"),
		&ID3D12Device::CreateCommandAllocator, m_pDevice12.Get(),
		D3D12_COMMAND_LIST_TYPE_DIRECT,			// 作成するコマンドアロケータの種類.
		IID_PPV_ARGS(CmdAllocator.ReleaseAndGetAddressOf()));		// (Out) コマンドアロケータ.

	MyAssert::IsFailed(
		_T("コマンドリストの生成"),
		&ID3D12Device::CreateCommandList, m_pDevice12.Get(),
		0,									// 単一のGPU操作の場合は0.
		D3D12_COMMAND_LIST_TYPE_DIRECT,		// 作成するコマンド リストの種類.
		CmdAllocator.Get(),					// アロケータへのポインタ.
		nullptr,							// ダミーの初期パイプラインが設定される?
		IID_PPV_ARGS(CmdList.ReleaseAndGetAddressOf()));				// (Out) コマンドリスト.

	// コマンドキュー構造体の作成.
	D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
	CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;				// タイムアウトなし.
	CmdQueueDesc.NodeMask = 0;										// アダプターを一つしか使わないときは0でいい.
	CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// プライオリティは特に指定なし.
	CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;				// コマンドリストと合わせる.

	MyAssert::IsFailed(
		_T("キューの作成"),
		&ID3D12Device::CreateCommandQueue, m_pDevice12.Get(),
		&CmdQueueDesc,
		IID_PPV_ARGS(CmdQueue.ReleaseAndGetAddressOf()));
}

// スワップチェーンの作成.
void DirectX12::CreateSwapChain(MyComPtr<IDXGISwapChain4>& SwapChain)
{
	// スワップ チェーン構造体の設定.
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.Width = WND_W;									//  画面の幅.
	SwapChainDesc.Height = WND_H;									//  画面の高さ.
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//  表示形式.
	SwapChainDesc.Stereo = false;									//  全画面モードかどうか.
	SwapChainDesc.SampleDesc.Count = 1;								//  ピクセル当たりのマルチサンプルの数.
	SwapChainDesc.SampleDesc.Quality = 0;							//  品質レベル(0~1).
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//  ﾊﾞｯｸﾊﾞｯﾌｧのメモリ量.
	SwapChainDesc.BufferCount = 2;									//  ﾊﾞｯｸﾊﾞｯﾌｧの数.
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;					//  ﾊﾞｯｸﾊﾞｯﾌｧのｻｲｽﾞがﾀｰｹﾞｯﾄと等しくない場合のｻｲｽﾞ変更の動作.
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//  ﾌﾘｯﾌﾟ後は素早く破棄.
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;			//  ｽﾜｯﾌﾟﾁｪｰﾝ,ﾊﾞｯｸﾊﾞｯﾌｧの透過性の動作
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//  ｽﾜｯﾌﾟﾁｪｰﾝ動作のｵﾌﾟｼｮﾝ(ｳｨﾝﾄﾞｳﾌﾙｽｸ切り替え可能ﾓｰﾄﾞ).

	MyAssert::IsFailed(
		_T("スワップチェーンの作成"),
		&IDXGIFactory2::CreateSwapChainForHwnd, m_pDxgiFactory.Get(),
		m_pCmdQueue.Get(),								// コマンドキュー.
		m_hWnd,											// ウィンドウハンドル.
		&SwapChainDesc,									// スワップチェーン設定.
		nullptr,										// ひとまずnullotrでよい.TODO : なにこれ
		nullptr,										// これもnulltrでよう
		(IDXGISwapChain1**)SwapChain.ReleaseAndGetAddressOf());	// (Out)スワップチェーン.

	MyAssert::IsFailed(
		_T("スワップチェーンディスクリプションの取得"),
		&IDXGISwapChain1::GetDesc1, SwapChain.Get(),
		&m_SwapChainDesc); // m_SwapChainDesc に格納
}

// レンダーターゲットの作成.
void DirectX12::CreateRenderTarget(
	MyComPtr<ID3D12DescriptorHeap>&			RenderTargetViewHeap,
	std::vector<MyComPtr<ID3D12Resource>>&	BackBuffer)
{
	// ディスクリプタヒープ構造体の作成.
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// RTV用ヒープ.
	HeapDesc.NumDescriptors = 2;						// 2つのディスクリプタ.
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// ヒープのオプション(特になしを設定).
	HeapDesc.NodeMask = 0;								// 単一アダプタ.			

	MyAssert::IsFailed(
		_T("ディスクリプタヒープの作成"),
		&ID3D12Device::CreateDescriptorHeap, m_pDevice12.Get(),
		&HeapDesc,														// ディスクリプタヒープ構造体を登録.
		IID_PPV_ARGS(RenderTargetViewHeap.ReleaseAndGetAddressOf()));	// (Out)ディスクリプタヒープ.

	// スワップチェーン構造体.
	DXGI_SWAP_CHAIN_DESC SwcDesc = {};
	MyAssert::IsFailed(
		_T("スワップチェーン構造体を取得."),
		&IDXGISwapChain4::GetDesc, m_pSwapChain.Get(),
		&SwcDesc);

	// ﾃﾞｨｽｸﾘﾌﾟﾀﾋｰﾌの先頭アドレスを取り出す.
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle = RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

	// バックバッファをヒープの数分宣言.
	m_pBackBuffer.resize(SwcDesc.BufferCount);

	// SRGBレンダーターゲットビュー設定.
	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// バックバファの数分.
	for (UINT i = 0; i < (SwcDesc.BufferCount); ++i)
	{
		MyAssert::IsFailed(
			_T("スワップチェーン内のバッファーとビューを関連づける"),
			&IDXGISwapChain4::GetBuffer, m_pSwapChain.Get(),
			i,
			IID_PPV_ARGS(m_pBackBuffer[i].GetAddressOf()));

		RTVDesc.Format = m_pBackBuffer[i]->GetDesc().Format;

		// レンダーターゲットビューを生成する.
		m_pDevice12->CreateRenderTargetView(
			BackBuffer[i].Get(),
			&RTVDesc,
			DescriptorHandle);

		// ポインタをずらす.
		DescriptorHandle.ptr += m_pDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	DXGI_SWAP_CHAIN_DESC1 Desc = {};
	MyAssert::IsFailed(
		_T("画面幅を取得"),
		&IDXGISwapChain4::GetDesc1, m_pSwapChain.Get(),
		&Desc);

	m_pViewport.reset(new CD3DX12_VIEWPORT(BackBuffer[0].Get()));
	m_pScissorRect.reset(new CD3DX12_RECT(0, 0, Desc.Width, Desc.Height));

}

// 深度バッファ作成.
void DirectX12::CreateDepthDesc(
	MyComPtr<ID3D12Resource>&		DepthBuffer,
	MyComPtr<ID3D12DescriptorHeap>&	DepthHeap,
	MyComPtr<ID3D12DescriptorHeap>& DepthSRVHeap)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	MyAssert::IsFailed(
		_T("スワップチェーンの取り出し"),
		&IDXGISwapChain4::GetDesc1, m_pSwapChain.Get(),
		&desc);

	// 深度バッファの仕様.
	D3D12_RESOURCE_DESC DepthResourceDesc = 
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, 
		desc.Width, desc.Height);
	DepthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// デプス用ヒーププロパティ.
	D3D12_HEAP_PROPERTIES DepthHeapProperty = {};
	DepthHeapProperty.Type = D3D12_HEAP_TYPE_DEFAULT;					// DEFAULTだから後はUNKNOWNでよし.
	DepthHeapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	DepthHeapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// このクリアバリューが重要な意味を持つ.
	m_DepthClearValue.DepthStencil.Depth = 1.0f;		// 深さ１(最大値)でクリア.
	m_DepthClearValue.Format = DXGI_FORMAT_D32_FLOAT;	// 32bit深度値としてクリア.

	MyAssert::IsFailed(
		_T("深度バッファリソースを作成"),
		&ID3D12Device::CreateCommittedResource, m_pDevice12.Get(),
		&DepthHeapProperty,							// ヒーププロパティの設定.
		D3D12_HEAP_FLAG_NONE,						// ヒープのオプション(特になしを設定).
		&DepthResourceDesc,							// リソースの仕様.
		D3D12_RESOURCE_STATE_DEPTH_WRITE,			// リソースの初期状態.
		&m_DepthClearValue,							// 深度バッファをクリアするための設定.
		IID_PPV_ARGS(DepthBuffer.ReleaseAndGetAddressOf())); // (Out)深度バッファ.

	// 深度ステンシルビュー用のデスクリプタヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc = {};
	DsvHeapDesc.NumDescriptors = 1;                   // 深度ビュー1つ.
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;// デスクリプタヒープのタイプ.
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	MyAssert::IsFailed(
		_T("深度ステンシルビュー用のデスクリプタヒープを作成"),
		&ID3D12Device::CreateDescriptorHeap, m_pDevice12.Get(),
		&DsvHeapDesc,										// ヒープの設定.
		IID_PPV_ARGS(DepthHeap.ReleaseAndGetAddressOf()));	// (Out)デスクリプタヒープ.
	
	// 深度ビュー作成.
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
	DsvDesc.Format = DXGI_FORMAT_D32_FLOAT;					// デプスフォーマット.
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ.
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;					// フラグなし.

	D3D12_CPU_DESCRIPTOR_HANDLE handle = DepthHeap->GetCPUDescriptorHandleForHeapStart();

	m_pDevice12->CreateDepthStencilView(
		m_pDepthBuffer.Get(),								// 深度バッファ.
		&DsvDesc,											// 深度ビューの設定.
		DepthHeap->GetCPUDescriptorHandleForHeapStart());	// ヒープ内の位置.

	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HeapDesc.NodeMask = 0;
	HeapDesc.NumDescriptors = 1;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	MyAssert::IsFailed(
		_T("SRVのディスクリプタヒープを作成"),
		&ID3D12Device::CreateDescriptorHeap, m_pDevice12,
		&HeapDesc, IID_PPV_ARGS(DepthSRVHeap.ReleaseAndGetAddressOf()));
	
	D3D12_SHADER_RESOURCE_VIEW_DESC DepthSrvResDesc = {};
	DepthSrvResDesc.Format = DXGI_FORMAT_R32_FLOAT;
	DepthSrvResDesc.Texture2D.MipLevels = 1;
	DepthSrvResDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	DepthSrvResDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	auto srvHandle = DepthSRVHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDevice12->CreateShaderResourceView(DepthBuffer.Get(), &DepthSrvResDesc, srvHandle);

}

// シーンビューの作成.
void DirectX12::CreateSceneDesc()
{
	// ---- リソース作成 ----
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// sizeof(SceneData) を256バイトの倍数に切り上げ
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);

	MyAssert::IsFailed(
		_T("定数バッファ作成 (Scene)"),
		&ID3D12Device::CreateCommittedResource, m_pDevice12.Get(), // クラスメンバーのデバイスを使用
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pSceneConstBuff.ReleaseAndGetAddressOf())); // クラスメンバー変数に代入

	// ---- バッファをマップ ----
	// m_pRawMappedSceneData は生ポインタ
	MyAssert::IsFailed(
		_T("シーン情報のマップ"),
		&ID3D12Resource::Map, m_pSceneConstBuff.Get(),
		0, nullptr,
		(void**)&m_pMappedSceneData); // クラスメンバー変数に代入 (生ポインタ)

	// ---- 初期値設定 ----
	
	// シーンデータの初期値を設定 (m_pRawMappedSceneData を通して書き込む)
	if (m_pMappedSceneData) {
		DirectX::XMFLOAT3 eye_pos(0, 0, -50);
		DirectX::XMFLOAT3 target_pos(0, 0, 0);
		DirectX::XMFLOAT3 up_vec(0, 1, 0);

		m_pMappedSceneData->view =
			DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&eye_pos),
			DirectX::XMLoadFloat3(&target_pos),
			DirectX::XMLoadFloat3(&up_vec));

		// アスペクト比はレンダリングループ内で更新するか、Dx12::Initialize()で一度設定
		// ここでは、既に m_pSwapChain が初期化されていると仮定し、その幅と高さを使用
		float aspectRatio = static_cast<float>(m_SwapChainDesc.Width) / static_cast<float>(m_SwapChainDesc.Height);
		m_pMappedSceneData->proj =
			DirectX::XMMatrixPerspectiveFovLH
			(DirectX::XM_PIDIV4, // 画角は45°
			aspectRatio,         // アス比
			0.1f,                // 近い方
			1000.0f              // 遠い方
			);

		m_pMappedSceneData->eye = eye_pos;
	}
	else {
		std::cerr << "Error: m_pRawMappedSceneData is null after mapping Scene Constant Buffer." << std::endl;
	}
}

// フェンスの作成.
void DirectX12::CreateFance(MyComPtr<ID3D12Fence>& Fence)
{
	MyAssert::IsFailed(
		_T("フェンスの生成"),
		&ID3D12Device::CreateFence, m_pDevice12.Get(),
		m_FenceValue,									// 初期化子.
		D3D12_FENCE_FLAG_NONE,							// フェンスのオプション.
		IID_PPV_ARGS(Fence.ReleaseAndGetAddressOf()));// (Out) フェンス.
}

// テクスチャロードテーブルの作成.
void DirectX12::CreateTextureLoadTable()
{
	m_LoadLambdaTable["sph"] =
		m_LoadLambdaTable["spa"] =
		m_LoadLambdaTable["bmp"] =
		m_LoadLambdaTable["png"] =
		m_LoadLambdaTable["jpg"] =
		[](const std::wstring& path, DirectX::TexMetadata* meta, DirectX::ScratchImage& img)->HRESULT {
		return LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, meta, img);
		};

	m_LoadLambdaTable["tga"] = [](const std::wstring& path, DirectX::TexMetadata* meta, DirectX::ScratchImage& img)->HRESULT {
		return LoadFromTGAFile(path.c_str(), meta, img);
		};

	m_LoadLambdaTable["dds"] = [](const std::wstring& path, DirectX::TexMetadata* meta, DirectX::ScratchImage& img)->HRESULT {
		return LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, meta, img);
		};

}

// テクスチャ名からテクスチャバッファ作成、中身をコピーする.
MyComPtr<ID3D12Resource> DirectX12::CreateTextureFromFile(const char* Texpath)
{
	std::string TexPath = Texpath;
	DirectX::TexMetadata Metadata = {};
	DirectX::ScratchImage ScratchImg = {};

	std::wstring wTexPath = MyString::StringToWString(TexPath);
	auto Extension = MyFilePath::GetExtension(TexPath);

	HRESULT Result = m_LoadLambdaTable[Extension](wTexPath, &Metadata, ScratchImg);

	if (FAILED(Result)) {
		std::string_view ErrorMessage = MyAssert::HResultToJapanese(Result);
		MessageBoxA(nullptr, std::string(ErrorMessage).c_str(), "Texture Load Error", MB_OK | MB_ICONERROR);
		return MyComPtr<ID3D12Resource>();
	}

	// --- 1. GPUがシェーダーから読み取るためのデフォルトヒープ上のテクスチャリソースを作成 ---
	D3D12_RESOURCE_DESC TexResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		Metadata.format,
		Metadata.width,
		static_cast<UINT>(Metadata.height),
		static_cast<UINT16>(Metadata.arraySize),
		static_cast<UINT16>(Metadata.mipLevels)
	);

	MyComPtr<ID3D12Resource> textureResource = {};
	CD3DX12_HEAP_PROPERTIES heapPropsDefault(D3D12_HEAP_TYPE_DEFAULT);

	Result = m_pDevice12->CreateCommittedResource(
		&heapPropsDefault,
		D3D12_HEAP_FLAG_NONE,
		&TexResDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 初期状態はコピー先
		nullptr,
		IID_PPV_ARGS(textureResource.ReleaseAndGetAddressOf())
	);
	if (FAILED(Result)) {
		return MyComPtr<ID3D12Resource>();
	}

	// --- 2. CPUからGPUへのデータ転送用の中間アップロードリソースを作成 ---
	UINT64 uploadBufferSize = 0;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;

	for (size_t arraySlice = 0; arraySlice < Metadata.arraySize; ++arraySlice)
	{
		for (size_t mipLevel = 0; mipLevel < Metadata.mipLevels; ++mipLevel)
		{
			const DirectX::Image* img = ScratchImg.GetImage(mipLevel, arraySlice, 0);
			if (!img) {
				return MyComPtr<ID3D12Resource>();
			}

			D3D12_SUBRESOURCE_DATA sd;
			sd.pData = img->pixels;
			sd.RowPitch = img->rowPitch;
			sd.SlicePitch = img->slicePitch;
			subresources.push_back(sd);
		}
	}

	m_pDevice12->GetCopyableFootprints(
		&TexResDesc,
		0,
		static_cast<UINT>(subresources.size()),
		0,
		nullptr,
		nullptr,
		nullptr,
		&uploadBufferSize
	);

	MyComPtr<ID3D12Resource> textureUploadHeap = {}; // ローカル変数として MyComPtr を宣言

	CD3DX12_HEAP_PROPERTIES heapPropsUpload(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferResDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	Result = m_pDevice12->CreateCommittedResource(
		&heapPropsUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(textureUploadHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(Result)) {
		return MyComPtr<ID3D12Resource>();
	}

	// --- 3. コマンドリストにコピーとバリアコマンドを追加 ---
	// テクスチャロード専用の一時コマンドリストとアロケータを使用する
	MyComPtr<ID3D12CommandAllocator> tempCmdAllocator;
	MyComPtr<ID3D12GraphicsCommandList> tempCmdList;

	MyAssert::IsFailed(
		_T("一時コマンドアロケータの生成"),
		&ID3D12Device::CreateCommandAllocator, m_pDevice12.Get(),
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(tempCmdAllocator.ReleaseAndGetAddressOf()));

	MyAssert::IsFailed(
		_T("一時コマンドリストの生成"),
		&ID3D12Device::CreateCommandList, m_pDevice12.Get(),
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		tempCmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(tempCmdList.ReleaseAndGetAddressOf()));

	// 初期状態はクローズなので、リセットして開く
	tempCmdList->Reset(tempCmdAllocator.Get(), nullptr);

	// UpdateSubresources は CopyCommandList を引数に取る
	UpdateSubresources(
		tempCmdList.Get(), // tempCmdList を使用
		textureResource.Get(),
		textureUploadHeap.Get(),
		0,
		0,
		static_cast<UINT>(subresources.size()),
		subresources.data()
	);

	// テクスチャリソースをシェーダーリソース状態に遷移
	CD3DX12_RESOURCE_BARRIER textureBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		textureResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	tempCmdList->ResourceBarrier(1, &textureBarrier);

	tempCmdList->Close(); // コマンドリストを閉じる

	// --- 4. コマンドリストを実行し、GPUの完了を待機 ---
	ID3D12CommandList* ppCommandLists[] = { tempCmdList.Get() };
	m_pCmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists); // メインのキューで実行

	// --- GPUがこのアップロードを完了するまで同期的に待機 ---
	MyComPtr<ID3D12Fence> uploadFence;
	HANDLE uploadFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	UINT64 fenceValueForUpload = 0;

	MyAssert::IsFailed(
		_T("アップロード用フェンスの生成"),
		&ID3D12Device::CreateFence, m_pDevice12.Get(),
		fenceValueForUpload,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(uploadFence.ReleaseAndGetAddressOf()));

	// キューにシグナルコマンドを積む
	m_pCmdQueue->Signal(uploadFence.Get(), ++fenceValueForUpload);

	// イベントがシグナルされるまで待つ
	if (uploadFence->GetCompletedValue() < fenceValueForUpload) {
		if (uploadFenceEvent != nullptr) {
			MyAssert::IsFailed(
				_T("アップロード用フェンスイベント設定"),
				&ID3D12Fence::SetEventOnCompletion, uploadFence.Get(),
				fenceValueForUpload, uploadFenceEvent);
			WaitForSingleObject(uploadFenceEvent, INFINITE);
		}
		else {
			OutputDebugString(L"Failed to create upload fence event!\n");
		}
	}
	// イベントハンドルをクローズ
	if (uploadFenceEvent != nullptr) {
		CloseHandle(uploadFenceEvent);
	}
	// ここで textureUploadHeap, tempCmdAllocator, tempCmdList, uploadFence は安全に解放される
	// （すべてローカル変数で MyComPtr であるため、スコープを抜けると自動的に Release される）

	// リソースを保持するために m_ResourceTable に追加する
	m_ResourceTable[Texpath] = textureResource;

	return textureResource;
}

// アダプターを見つける.
IDXGIAdapter* DirectX12::FindAdapter(std::wstring FindWord)
{
	// アタブター(見つけたグラボを入れる).
	std::vector <IDXGIAdapter*> Adapter;

	// ここに特定の名前を持つアダプターが入る.
	IDXGIAdapter* TmpAdapter = nullptr;

	// forですべてのアダプターをベクター配列に入れる.
	for (int i = 0; m_pDxgiFactory->EnumAdapters(i, &TmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		Adapter.push_back(TmpAdapter);
	}

	// 取り出したアダプターから情報を持ってくる.
	for (auto Adpt : Adapter) {

		DXGI_ADAPTER_DESC Adesc = {};

		// アダプター情報を取り出す.
		Adpt->GetDesc(&Adesc);

		// 名前を取り出す.
		std::wstring strDesc = Adesc.Description;

		// NVIDIAなら格納.
		if (strDesc.find(FindWord) != std::string::npos) {
			return Adpt;
		}
	}

	return nullptr;
}

// デバッグモードを起動.
void DirectX12::EnableDebuglayer()
{
	ID3D12Debug* DebugLayer = nullptr;
	
	// デバッグレイヤーインターフェースを取得.
	D3D12GetDebugInterface(IID_PPV_ARGS(&DebugLayer));

	// デバッグレイヤーを有効.
	DebugLayer->EnableDebugLayer();	

	// 解放.
	DebugLayer->Release();
}

// PSO と Root Signature を作成
bool DirectX12::CreatePipelineStateAndRootSignature()
{
	if (!m_pDevice12) { return false; }

	// ===== Root Signature 作成 =====
	D3D12_ROOT_PARAMETER root_params[4] = {};

	// b0: World・View・Projection 定数バッファ.
	root_params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	root_params[0].Descriptor.ShaderRegister = 0;
	root_params[0].Descriptor.RegisterSpace = 0;
	root_params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// b1: Bone 定数バッファ.
	root_params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	root_params[1].Descriptor.ShaderRegister = 1;
	root_params[1].Descriptor.RegisterSpace = 0;
	root_params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// b2: Material 定数バッファ.
	root_params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	root_params[2].Descriptor.ShaderRegister = 2;
	root_params[2].Descriptor.RegisterSpace = 0;
	root_params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// t0: Texture + s0: Sampler.
	D3D12_DESCRIPTOR_RANGE desc_range = {};
	desc_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desc_range.NumDescriptors = 1;
	desc_range.BaseShaderRegister = 0;
	desc_range.RegisterSpace = 0;
	desc_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	root_params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	root_params[3].DescriptorTable.NumDescriptorRanges = 1;
	root_params[3].DescriptorTable.pDescriptorRanges = &desc_range;
	root_params[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
	sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
	sampler_desc.ShaderRegister = 0;
	sampler_desc.RegisterSpace = 0;
	sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
	root_sig_desc.NumParameters = 4;
	root_sig_desc.pParameters = root_params;
	root_sig_desc.NumStaticSamplers = 1;
	root_sig_desc.pStaticSamplers = &sampler_desc;
	root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	MyComPtr<ID3DBlob> root_sig_blob;
	MyComPtr<ID3DBlob> error_blob;

	if (FAILED(D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, 
		root_sig_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] Root Signature serialization failed\n");
		if (error_blob)
		{
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
		}
		return false;
	}

	if (FAILED(m_pDevice12->CreateRootSignature(
		0,
		root_sig_blob->GetBufferPointer(),
		root_sig_blob->GetBufferSize(),
		__uuidof(ID3D12RootSignature),
		(void**)m_pRootSignature.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] Root Signature creation failed\n");
		return false;
	}

	// ===== PSO (Pipeline State Object) 作成 =====
	D3D12_INPUT_ELEMENT_DESC input_element[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// シェーダーコンパイル.
	MyComPtr<ID3DBlob> vs_blob, ps_blob;

	// シェーダーファイルのパスを動的に構築.
	wchar_t module_path[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, module_path, MAX_PATH);
	std::wstring exe_dir(module_path);
	exe_dir = exe_dir.substr(0, exe_dir.find_last_of(L"\\"));

	std::array<std::wstring, 4> shader_paths =
	{
		exe_dir + L"\\Data\\Shader\\FBX\\Skinning.hlsl",
		exe_dir + L"\\..\\Data\\Shader\\FBX\\Skinning.hlsl",
		exe_dir + L"\\..\\..\\Data\\Shader\\FBX\\Skinning.hlsl",
		exe_dir + L"\\..\\..\\Senzan-Ver1.3\\Data\\Shader\\FBX\\Skinning.hlsl"
	};

	std::wstring shader_path;
	bool is_shader_found = false;
	for (const auto& path : shader_paths)
	{
		DWORD attr = GetFileAttributesW(path.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			shader_path = path;
			is_shader_found = true;
			break;
		}
	}

	if (!is_shader_found)
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] Shader file not found\n");
		return false;
	}

	if (FAILED(D3DCompileFromFile(
		shader_path.c_str(),
		nullptr,
		nullptr,
		"VS_Main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		vs_blob.ReleaseAndGetAddressOf(),
		error_blob.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] VS compilation failed\n");
		if (error_blob)
		{
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
		}
		return false;
	}

	if (FAILED(D3DCompileFromFile(
		shader_path.c_str(),
		nullptr,
		nullptr,
		"PS_Main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		ps_blob.ReleaseAndGetAddressOf(),
		error_blob.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] PS compilation failed\n");
		if (error_blob)
		{
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
		}
		return false;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.pRootSignature = m_pRootSignature.Get();
	pso_desc.VS.pShaderBytecode = vs_blob->GetBufferPointer();
	pso_desc.VS.BytecodeLength = vs_blob->GetBufferSize();
	pso_desc.PS.pShaderBytecode = ps_blob->GetBufferPointer();
	pso_desc.PS.BytecodeLength = ps_blob->GetBufferSize();

	pso_desc.SampleMask = UINT_MAX;
	pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pso_desc.RasterizerState.DepthBias = 0;
	pso_desc.RasterizerState.DepthBiasClamp = 0.0f;
	pso_desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
	pso_desc.RasterizerState.DepthClipEnable = true;
	pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	pso_desc.RasterizerState.FrontCounterClockwise = false;
	pso_desc.RasterizerState.MultisampleEnable = false;
	pso_desc.RasterizerState.AntialiasedLineEnable = false;

	// ブレンドステート.
	pso_desc.BlendState.AlphaToCoverageEnable = false;
	pso_desc.BlendState.IndependentBlendEnable = false;
	pso_desc.BlendState.RenderTarget[0].BlendEnable = false;
	pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 深度ステンシルステート.
	pso_desc.DepthStencilState.DepthEnable = true;
	pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pso_desc.DepthStencilState.StencilEnable = false;

	pso_desc.InputLayout.pInputElementDescs = input_element;
	pso_desc.InputLayout.NumElements = std::size(input_element);
	pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pso_desc.SampleDesc.Count = 1;
	pso_desc.SampleDesc.Quality = 0;

	if (FAILED(m_pDevice12->CreateGraphicsPipelineState(&pso_desc, __uuidof(ID3D12PipelineState), (void**)m_pPipelineState.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] PSO creation failed\n");
		return false;
	}

	OutputDebugStringA("[DirectX12::CreatePipelineStateAndRootSignature] PSO and Root Signature created successfully\n");
	return true;
}
