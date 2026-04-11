#include "Main.h"
#include "System/00_GameLoop/Loader.h"

#include "Graphic/DirectX/DirectX12/DirectX12.h"

#include "Game/04_InputDevice/Input.h"
#include "System/00_GameLoop/Time/Time.h"
#include "System/10_Singleton/01_SceneManager/SceneManager.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

// ウィンドウを画面中央で起動を有効にする.
#define ENABLE_WINDOWS_CENTERING

// 定数.
const TCHAR WND_TITLE[] = _T("閃斬-Ver1.3");
const TCHAR APP_NAME[] = _T("閃斬-Ver1.3");

// コンストラクタ.
Main::Main()
	: m_hWnd		( nullptr )
	, m_upDirectX12	( std::make_unique<DirectX12>() )
//	, m_pResourceLoader(std::make_unique<Loader>())
{
}

// デストラクタ.
Main::~Main()
{
	//CImGuiManager::Relese(); // ImGuiの終了処理.
}

// データロード処理.
HRESULT Main::LoadData()
{
 if (m_upDirectX12 == nullptr) {
		return E_FAIL;
	}

	if (!m_upDirectX12->Create(m_hWnd)) {
		_ASSERT_EXPR(false, _T("DirectX12の初期化に失敗"));
		return E_FAIL;
	}

	// ウィンドウハンドルを設定.
	Input::SethWnd(m_hWnd);

	SceneManager::GetInstance().LoadData();

	return S_OK;
}

void Main::Create()
{
  // 生成処理が必要になった場合はここへ追加.
}

// 更新処理.
void Main::Update()
{
	m_upDirectX12->Update();

	DebugImgui();

	SceneManager::GetInstance().Update();

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
    if (m_upDirectX12 == nullptr) {
		return;
	}

	m_upDirectX12->BeginDraw();
	SceneManager::Draw();
	m_upDirectX12->EndDraw();
}

// 解放処理.
void Main::Release()
{
	m_upDirectX12.reset();
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
	//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	//{
	//	return true;
	// 　
	//}
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
   bool was_esc_pressed = !Input::IsKeyPress(Esc);
	UNREFERENCED_PARAMETER(was_esc_pressed);

  float current_time = Time::GetInstance().GetNowTime(); // 現在のゲーム内時刻を取得.

	if (Input::IsKeyDown(Esc)) // Escキーが押された瞬間.
	{
		// 前回からの経過時間を計算.
       float elapsed_time = current_time - m_LastEscPressTime;

		// ダブルタップの判定.
        if (elapsed_time < DOUBLE_TAP_TIME_THRESHOLD)
		{
			if (MessageBox(m_hWnd, _T("ゲームを終了しますか？"), _T("警告"), MB_YESNO) == IDYES) {
				DestroyWindow(m_hWnd);
			}
			m_LastEscPressTime = 0.0f;
		}
		else
		{
			// シングルタップとみなし、次回判定のために時刻を更新.
           m_LastEscPressTime = current_time;
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
