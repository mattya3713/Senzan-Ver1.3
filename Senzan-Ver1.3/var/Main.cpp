#include "Main.h"
#include "System/Singleton/SceneManager/SceneManager.h"
#include "Game/04_Time/Time.h"
#include "Game/05_InputDevice/Input.h"
#include "Game/05_InputDevice/VirtualPad.h"
#include "System/Singleton/ResourceManager/ResourceManager.h"
#include "System/GameLoop/Loader.h"
#include "Graphic/DirectX/DirectX9/DirectX9.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Singleton/PostEffectManager/PostEffectManager.h"
#include "Singleton/FrameCaptureManager/FrameCaptureManager.h"
#include "02_UIObject/Fade/FadeManager.h"

#include "System/Singleton/ResourceManager/EffectManager/EffekseerManager.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Singleton/ResourceManager/MeshManager/MeshManager.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

// ウィンドウを画面中央で起動を有効にする.
#define ENABLE_WINDOWS_CENTERING

//=================================================.
// 定数.
//=================================================.
const TCHAR WND_TITLE[] = _T("閃斬");
const TCHAR APP_NAME[] = _T("閃斬");

//=================================================.
// コンストラクタ.
//=================================================.
Main::Main()
	: m_hWnd            ( nullptr )
	, m_pResourceLoader(std::make_unique<Loader>())
{
}

//=================================================.
// デストラクタ.
//=================================================.
Main::~Main()
{
	CImGuiManager::Relese(); // ImGuiの終了処理.
}

// データロード処理.
HRESULT Main::LoadData()
{
	// DirectXの初期化.
	if (FAILED(DirectX9::GetInstance().Create(m_hWnd))) {
		_ASSERT_EXPR(false, "DirectX9の初期化に失敗");
	}

	if (FAILED(DirectX11::GetInstance().Create(m_hWnd))) {
		_ASSERT_EXPR(false, "DirectX11の初期化に失敗");
	}
	
	// ウィンドウハンドルを設定.
	Input::SethWnd(m_hWnd);
	VirtualPad::GetInstance().SetupDefaultBindings();
	ResourceManager::SethWnd(m_hWnd);

	// ロード画面で使用するデータの読み込み.
	m_pResourceLoader->LoadData();

	// リソースの読み込み開始.
	m_pResourceLoader->StartLoading();

	// 必要に応じてデータロード処理を追加.
	return S_OK;
}

void Main::Create()
{
	CImGuiManager::Init(m_hWnd);

	SceneManager::GetInstance().LoadData();
	PostEffectManager::GetInstance().Initialize();
	FrameCaptureManager::GetInstance().Initialize();
}

// 更新処理.
void Main::Update()
{
	CImGuiManager::NewFrameSetting();

	DebugImgui();

	// フレームキャプチャマネージャ更新.
	FrameCaptureManager::GetInstance().Update(Time::GetInstance().GetDeltaTime());

	// 再生中はゲームロジックをスキップ.
	if (FrameCaptureManager::GetInstance().IsPlaying())
	{
		return;
	}

	SceneManager::GetInstance().Update();

    // SoundManager の更新（自動復帰タイマー等）.
    SoundManager::GetInstance().Update(Time::GetInstance().GetDeltaTime());

	// マウスホイールのスクロール方向を初期化.
	Input::SetWheelDirection(0);

	// マウスを画面中央に固定する.
	constexpr int Esc = VK_ESCAPE;
	static bool wasEscPressed = false;
#if ENABLE_FRAMECAPTURE_IMGUI
	if (Input::IsKeyDown(Esc))
	{
		wasEscPressed = !wasEscPressed;
	}
#endif // _DEBUG.

	Input::SetCenterMouseCursor(wasEscPressed);
	Input::SetShowCursor(!wasEscPressed);

	// マウスを画面中央に固定する.
	Input::CenterMouseCursor();

	IsExitGame();
}

// 描画処理.
void Main::Draw()
{
	// リソースの読み込みが終わるまでゲームの描画を行わない.
	if (!m_pResourceLoader->IsLoadCompletion())
	{
		m_pResourceLoader->Draw();
		return;
	}

	auto& fc = FrameCaptureManager::GetInstance();

	// 再生中はキャプチャしたフレームを描画.
	if (fc.IsPlaying())
	{
		fc.RenderPlayback(Time::GetInstance().GetDeltaTime());
		CImGuiManager::Render();
		DirectX11::GetInstance().Present();
		return;
	}

	// バックバッファのクリア.
	DirectX11::GetInstance().ClearBackBuffer();

	auto& pe = PostEffectManager::GetInstance();

	if (pe.IsGray()) {
		pe.BeginSceneRender();
		SceneManager::Draw();

		// 【歪みエフェクト用】シーンをResolveしてから背景テクスチャとしてEffekseerに渡す.
		auto ctx = DirectX11::GetInstance().GetContext();
		ctx->ResolveSubresource(pe.GetSceneResolvedTex(), 0, pe.GetSceneMSAATex(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		EffekseerManager::GetInstance().SetBackgroundTexture(pe.GetSceneSRV());

		pe.DrawToBackBuffer();
	}
	else {
		// 通常描画（レンダーターゲットを戻してから描画）.
		DirectX11::GetInstance().ResetRenderTarget();
		SceneManager::Draw();
	}

	// キャプチャ中はフレームをコピー.
	if (fc.IsCapturing())
	{
		fc.CaptureFrame();
	}

	CImGuiManager::Render();


	// 画面に表示.
	DirectX11::GetInstance().Present();
}

// 解放処理.
void Main::Release()
{
   
}

// メッセージループ.

void Main::Loop()
{
	// ゲームの構築.
	if (FAILED(LoadData())) {
		return;
	}

	// タイマ精度を向上させる.
	timeBeginPeriod(1);

	// 読み込み完了待ち処理.
	DWORD lastTime = timeGetTime(); // 前のフレームの時間.
	const float loadUpdateInterval = 1.0f / 60.0f; // 読み込み更新の間隔 (60FPS目安).
	float accumulatedTime = 0.0f;

	while (!m_pResourceLoader->IsLoadCompletion()
		|| !FadeManager::GetInstance().IsFading()) {
		DWORD currentTime = timeGetTime();                 // 現在の時間を取得.
		float deltaTime = (currentTime - lastTime) / 1000.0f; // ミリ秒から秒に変換.
		lastTime = currentTime;                            // 前の時間を更新.

		accumulatedTime += deltaTime;
		if (accumulatedTime >= loadUpdateInterval) {
			accumulatedTime = 0.0f; // タイマーをリセット.

			// データ読み込み.
			m_pResourceLoader->Update();
			m_pResourceLoader->Draw();
		}

		if (m_pResourceLoader->IsLoadCompletion()) {
			FadeManager::GetInstance().StartFade(Fade::FadeType::FadeOut);
		}
	}
	while (!FadeManager::GetInstance().IsFadeCompleted(Fade::FadeType::FadeOut))
	{
		// Update fade and ensure the backbuffer is cleared each frame.
		// so the fade sprite doesn't composite over an uninitialized/back buffer.
		FadeManager::GetInstance().Update();
		DirectX11::GetInstance().ClearBackBuffer();
		FadeManager::GetInstance().Draw();
		DirectX11::GetInstance().Present();
	}


	// データの読み込みが終わったらゲームを構築.
	Create();

	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// Time の更新は Loop が責務.
		Time::GetInstance().Update();
		// ゲーム更新と描画.
		Update();
		Draw();

		// フレームレート維持.
		Time::GetInstance().MaintainFPS();
	}

	// タイマ設定を戻す.
	timeEndPeriod(1);
}

// ウィンドウ初期化関数.
HRESULT Main::InitWindow(HINSTANCE hInstance, INT x, INT y, INT width, INT height)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MsgProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = APP_NAME;

	if (!RegisterClassEx(&wc)) {
		return E_FAIL;
	}

	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	INT winWidth = rect.right - rect.left;
	INT winHeight = rect.bottom - rect.top;
	INT winX = (GetSystemMetrics(SM_CXSCREEN) - winWidth) / 2;
	INT winY = (GetSystemMetrics(SM_CYSCREEN) - winHeight) / 2;

	m_hWnd = CreateWindow(
		APP_NAME, WND_TITLE,
		WS_OVERLAPPEDWINDOW,
		winX, winY, winWidth, winHeight,
		nullptr, nullptr, hInstance, this
	);

	if (!m_hWnd) {
		return E_FAIL;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return S_OK;
}

// ウィンドウ関数（メッセージ毎の処理）.
LRESULT CALLBACK Main::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}
	// hWndに関連付けられたCMainを取得.
	// MEMO : ウィンドウが作成されるまでは nullptr になる可能性がある.
	Main* pMain = reinterpret_cast<Main*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// ウィンドウが初めて作成された時.
	if (uMsg == WM_NCCREATE) {
		// CREATESTRUCT構造体からCMainのポインタを取得.
		CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		// SetWindowLongPtrを使用しhWndにCMainインスタンスを関連付ける.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		// デフォルトのウィンドウプロシージャを呼び出して処理を進める.
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	if (pMain) {
		switch (uMsg) {
			// ウィンドウが破棄されるとき.
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_ACTIVATEAPP:
			// アプリがアクティブになったときにタイマーをリセットして大きなデルタを防止.
			if (wParam != 0) {
				Time::GetInstance().ResetOnResume();
			}
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Escキーのダブルタップでゲームを終了する.
void Main::IsExitGame()
{
	constexpr int Esc = VK_ESCAPE;
	bool wasEscPressed = !Input::IsKeyPress(Esc);

	float currentTime = Time::GetInstance().GetNowTime(); // 現在のゲーム内時刻を取得.

	if (Input::IsKeyDown(Esc)) // Escキーが押された瞬間.
	{
		// 前回からの経過時間を計算.
		float elapsedTime = currentTime - m_LastEscPressTime;

		// ダブルタップの判定.
		if (elapsedTime < DOUBLE_TAP_TIME_THRESHOLD)
		{
			if (MessageBox(m_hWnd, _T("ゲームを終了しますか？"), _T("警告"), MB_YESNO) == IDYES) {
				DestroyWindow(m_hWnd);
			}
			m_LastEscPressTime = 0.0f;
		}
		else
		{
			// シングルタップとみなし、次回判定のために時刻を更新.
			m_LastEscPressTime = currentTime;
		}
	}
}

void Main::DebugImgui()
{
#if ENABLE_FRAMECAPTURE_IMGUI
	ImGui::Begin("Performance Monitor");

	// 基本的なFPS表示.
	float fps = ImGui::GetIO().Framerate;
	float ms = 1000.0f / fps;

	ImGui::Text("Average: %.1f FPS (%.3f ms/frame)", fps, ms);

	// 状態に応じた警告表示.
	if (fps < 50.0f) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Low FPS!");
	}
	else {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Stable");
	}

	ImGui::Separator();

	// --- World Time Scale の操作UI ---.
	ImGui::Text("World Time Scale");

	// 現在の time scale を取得（Time クラスの GetWorldTimeScale() を使用）.
	float timeScale = Time::GetInstance().GetWorldTimeScale();

	// スライダーで time scale を調整（0.0f から 4.0f）.
	if (ImGui::SliderFloat("Scale", &timeScale, 0.0f, 4.0f, "%.2f")) {
		// 負の値は許可しない.
		if (timeScale < 0.0f) {
			timeScale = 0.0f;
		}
		Time::GetInstance().SetWorldTimeScale(timeScale);
	}

	// リセットと一時停止ボタン.
	if (ImGui::Button("Reset##TimeScale")) {
		Time::GetInstance().SetWorldTimeScale(1.0f);
		timeScale = 1.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause/Resume##TimeScale")) {
		float cur = Time::GetInstance().GetWorldTimeScale();
		Time::GetInstance().SetWorldTimeScale((cur > 0.0f) ? 0.0f : 1.0f);
	}

	// 現在の time scale をテキスト表示.
	ImGui::Text("Current: %.2f", Time::GetInstance().GetWorldTimeScale());

	ImGui::Separator();

	// --- ディゾルブエフェクト デバッグ ---.
	ImGui::Text(IMGUI_JP("ディゾルブエフェクト"));
	
	// スキンメッシュリストを取得.
	static int selectedMeshIndex = 0;
	auto meshList = MeshManager::GetSkinMeshList();
	
	if (!meshList.empty())
	{
		// メッシュ選択コンボボックス.
		if (ImGui::BeginCombo(IMGUI_JP("対象メッシュ"), meshList[selectedMeshIndex].c_str()))
		{
			for (int i = 0; i < static_cast<int>(meshList.size()); i++)
			{
				bool isSelected = (selectedMeshIndex == i);
				if (ImGui::Selectable(meshList[i].c_str(), isSelected))
				{
					selectedMeshIndex = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		
		// 選択されたメッシュのディゾルブ設定.
		auto pMesh = MeshManager::GetSkinMesh(meshList[selectedMeshIndex]);
		if (pMesh)
		{
			bool dissolveEnabled = pMesh->IsDissolveEnabled();
			if (ImGui::Checkbox(IMGUI_JP("ディゾルブ有効"), &dissolveEnabled))
			{
				pMesh->SetDissolveEnabled(dissolveEnabled);
			}
			
			if (dissolveEnabled)
			{
				float threshold = pMesh->GetDissolveThreshold();
				if (ImGui::SliderFloat(IMGUI_JP("消失度 (閾値)"), &threshold, 0.0f, 1.0f, "%.2f"))
				{
					pMesh->SetDissolveThreshold(threshold);
				}
				
				float edgeWidth = pMesh->GetDissolveEdgeWidth();
				if (ImGui::SliderFloat(IMGUI_JP("エッジ幅"), &edgeWidth, 0.0f, 0.3f, "%.3f"))
				{
					pMesh->SetDissolveEdgeWidth(edgeWidth);
				}
				
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
					IMGUI_JP("※ 閾値を上げると徐々に消えます"));
			}
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
			IMGUI_JP("スキンメッシュが読み込まれていません"));
	}

	ImGui::Separator();

	ImGui::End();
#endif
}
