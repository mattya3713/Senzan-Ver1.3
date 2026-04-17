#include "Main.h"
#include "System/00_GameLoop/Loader.h"

#include "Graphic/DirectX/DirectX12/DirectX12.h"
#include "Resource/Mesh/FBX/FBXModel.h"

#include <array>
#include <cmath>
#include <string>

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
	, m_upFbxModel	( std::make_unique<FBXModel>() )
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

	if (!LoadYBotModel()) {
		OutputDebugStringA("[Main::LoadData] WARNING: YBotモデルの読み込みに失敗しました\n");
	}

	return S_OK;
}

void Main::Create()
{
   if (m_upFbxModel != nullptr) {
		m_upFbxModel->SetAnimationIndex(0);
	}
}

// 更新処理.
void Main::Update()
{
	m_upDirectX12->Update();

	DebugImgui();

	const float delta_time = Time::GetInstance().GetDeltaTime();
	HandleCameraInput(delta_time);

	if (m_upFbxModel != nullptr) {
		HandleAnimationInput();
		m_upFbxModel->Update(delta_time);
	}

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
	RenderYBot();
	SceneManager::Draw();
	m_upDirectX12->EndDraw();
}

// 解放処理.
void Main::Release()
{
  if (m_upFbxModel != nullptr) {
		m_upFbxModel->Release();
		m_upFbxModel.reset();
	}

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

bool Main::LoadYBotModel()
{
	if (m_upFbxModel == nullptr || m_upDirectX12 == nullptr)
	{
		OutputDebugStringA("[LoadYBotModel] ERROR: model or directx is null\n");
		return false;
	}

	auto p_device12 = m_upDirectX12->GetDevice();
	if (!p_device12)
	{
		OutputDebugStringA("[LoadYBotModel] ERROR: Failed to get D3D12 device\n");
		return false;
	}

	auto p_command_queue = m_upDirectX12->GetCommandQueue();
	if (!p_command_queue)
	{
		OutputDebugStringA("[LoadYBotModel] ERROR: Failed to get command queue\n");
		return false;
	}

	const std::array<const char*, 4> model_paths =
	{
		"Data/Model/FBX/Y-Bot-Anim-In-And-Parts.fbx",
		"../Data/Model/FBX/Y-Bot-Anim-In-And-Parts.fbx",
		"../../Data/Model/FBX/Y-Bot-Anim-In-And-Parts.fbx",
		"../../Senzan-Ver1.3/Data/Model/FBX/Y-Bot-Anim-In-And-Parts.fbx"
	};

	for (const char* model_path : model_paths)
	{
		std::string log_message = "[LoadYBotModel] Trying to load: " + std::string(model_path) + "\n";
		OutputDebugStringA(log_message.c_str());

		if (m_upFbxModel->Load(p_device12.Get(), p_command_queue.Get(), model_path))
		{
			log_message = "[LoadYBotModel] SUCCESS: Model loaded from " + std::string(model_path) + "\n";
			OutputDebugStringA(log_message.c_str());
			return true;
		}

		log_message = "[LoadYBotModel] FAILED: " + std::string(model_path) + "\n";
		OutputDebugStringA(log_message.c_str());
	}

	OutputDebugStringA("[LoadYBotModel] ERROR: All model paths failed\n");
	return false;
}

void Main::HandleAnimationInput()
{
	if (m_upFbxModel == nullptr)
	{
		return;
	}

	for (int i = 0; i < 9; ++i)
	{
		const int vk_code = '1' + i;
		const bool is_pressed = (GetAsyncKeyState(vk_code) & 0x8000) != 0;

		if (is_pressed && !m_IsPrevAnimKeyPressed[static_cast<size_t>(i)])
		{
			const uint32_t anim_index = static_cast<uint32_t>(i);
			if (anim_index < m_upFbxModel->GetAnimationCount())
			{
				if (m_upFbxModel->SetAnimationIndex(anim_index))
				{
					const std::string log_message = "[Input] Animation switched to index: " + std::to_string(anim_index) + "\n";
					OutputDebugStringA(log_message.c_str());
				}
			}
		}

		m_IsPrevAnimKeyPressed[static_cast<size_t>(i)] = is_pressed;
	}
}

void Main::HandleCameraInput(float DeltaTime)
{
	if (m_hWnd == nullptr)
	{
		return;
	}

	const bool is_right_mouse = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	const float rotate_sensitivity = 0.0035f;
	const float base_move_speed = 120.0f;
	const float fast_rate = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) ? 3.0f : 1.0f;
	const float move_speed = base_move_speed * fast_rate * DeltaTime;

	if (is_right_mouse)
	{
		POINT mouse_pos = {};
		GetCursorPos(&mouse_pos);
		ScreenToClient(m_hWnd, &mouse_pos);

		if (m_IsFirstMouse)
		{
			m_PrevMousePos = mouse_pos;
			m_IsFirstMouse = false;
		}

		const int delta_x = mouse_pos.x - m_PrevMousePos.x;
		const int delta_y = mouse_pos.y - m_PrevMousePos.y;
		m_PrevMousePos = mouse_pos;

		m_CameraYaw += static_cast<float>(delta_x) * rotate_sensitivity;
		m_CameraPitch += static_cast<float>(delta_y) * -rotate_sensitivity;

		const float max_pitch = DirectX::XM_PIDIV2 - 0.01f;
		if (m_CameraPitch > max_pitch) { m_CameraPitch = max_pitch; }
		if (m_CameraPitch < -max_pitch) { m_CameraPitch = -max_pitch; }
	}
	else
	{
		m_IsFirstMouse = true;
	}

	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		sinf(m_CameraYaw) * cosf(m_CameraPitch),
		sinf(m_CameraPitch),
		cosf(m_CameraYaw) * cosf(m_CameraPitch),
		0.0f);
	v_forward = DirectX::XMVector3Normalize(v_forward);

	DirectX::XMVECTOR v_world_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR v_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_world_up, v_forward));
	DirectX::XMVECTOR v_position = DirectX::XMLoadFloat3(&m_CameraPosition);

	if (GetAsyncKeyState('W') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_forward, move_speed)); }
	if (GetAsyncKeyState('S') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_forward, move_speed)); }
	if (GetAsyncKeyState('A') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_right, move_speed)); }
	if (GetAsyncKeyState('D') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_right, move_speed)); }
	if (GetAsyncKeyState('Q') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_world_up, move_speed)); }
	if (GetAsyncKeyState('E') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_world_up, move_speed)); }

	DirectX::XMStoreFloat3(&m_CameraPosition, v_position);
}

void Main::RenderYBot()
{
	if (m_upDirectX12 == nullptr || m_upFbxModel == nullptr)
	{
		return;
	}

	auto p_command_list = m_upDirectX12->GetCommandList();
	if (!p_command_list)
	{
		return;
	}

	auto p_pso = m_upDirectX12->GetPipelineState();
	auto p_root_sig = m_upDirectX12->GetRootSignature();
	if (!p_pso || !p_root_sig)
	{
		OutputDebugStringA("[RenderYBot] PSO or Root Signature is null\n");
		return;
	}

	p_command_list->SetPipelineState(p_pso.Get());
	p_command_list->SetGraphicsRootSignature(p_root_sig.Get());

	auto p_viewport_ptr = m_upDirectX12->GetViewport();
	auto p_scissor_rect_ptr = m_upDirectX12->GetScissorRect();
	if (p_viewport_ptr && p_scissor_rect_ptr && *p_viewport_ptr && *p_scissor_rect_ptr)
	{
		p_command_list->RSSetViewports(1, p_viewport_ptr->get());
		p_command_list->RSSetScissorRects(1, p_scissor_rect_ptr->get());
	}

	DirectX::XMVECTOR v_eye = DirectX::XMLoadFloat3(&m_CameraPosition);
	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		sinf(m_CameraYaw) * cosf(m_CameraPitch),
		sinf(m_CameraPitch),
		cosf(m_CameraYaw) * cosf(m_CameraPitch),
		0.0f);
	v_forward = DirectX::XMVector3Normalize(v_forward);

	DirectX::XMVECTOR v_at = DirectX::XMVectorAdd(v_eye, v_forward);
	DirectX::XMVECTOR v_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(v_eye, v_at, v_up);
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		static_cast<float>(WND_W) / static_cast<float>(WND_H),
		0.1f,
		1000.0f);

	m_upFbxModel->SetWorldMatrix(DirectX::XMMatrixIdentity());
	m_upFbxModel->SetViewMatrix(view);
	m_upFbxModel->SetProjectionMatrix(proj);
	m_upFbxModel->Render(p_command_list.Get());
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
