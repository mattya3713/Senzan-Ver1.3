#include "Main.h"
#include "Game/00_GameLoop/Loader.h"

#include "Engine/Graphic/DirectX/DirectX12/DirectX12.h"
#include "Engine/Resource/Mesh/01_FBX/FBXModel.h"

#include <array>
#include <cmath>
#include <string>

#include "Game/05_InputDevice/Input.h"
#include "Game/00_GameLoop/Time/Time.h"
#include "Game/00_GameLoop/Scene/SceneManager.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

// 繧ｦ繧｣繝ｳ繝峨え繧堤判髱｢荳ｭ螟ｮ縺ｧ襍ｷ蜍輔☆繧・
#define ENABLE_WINDOWS_CENTERING

// 螳壽焚.
const TCHAR WND_TITLE[] = _T("髢・脈-Ver1.3");
const TCHAR APP_NAME[] = _T("髢・脈-Ver1.3");

// 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ.
Main::Main()
	: m_hWnd		( nullptr )
	, m_upDirectX12	( std::make_unique<DirectX12>() )
	, m_upFbxModel	( std::make_unique<FBXModel>() )
//	, m_pResourceLoader(std::make_unique<Loader>())
{
}

// 繝・せ繝医Λ繧ｯ繧ｿ.
Main::~Main()
{
    //CImGuiManager::Relese(); // ImGui縺ｮ邨ゆｺ・・逅・
}

// 繝・・繧ｿ繝ｭ繝ｼ繝牙・逅・
HRESULT Main::LoadData()
{
    if (m_upDirectX12 == nullptr) {
		return E_FAIL;
	}

   if (!m_upDirectX12->Create(m_hWnd)) {
		_ASSERT_EXPR(false, _T("DirectX12 initialization failed."));
		return E_FAIL;
	}

    // 繧ｦ繧｣繝ｳ繝峨え繝上Φ繝峨Ν繧定ｨｭ螳・
	Input::SethWnd(m_hWnd);

	SceneManager::GetInstance().LoadData();

	if (!LoadYBotModel()) {
      OutputDebugStringA("[Main::LoadData] WARNING: YBot繝｢繝・Ν縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ縺ｫ螟ｱ謨励＠縺ｾ縺励◆\n");
	}

	return S_OK;
}

void Main::Create()
{
   if (m_upFbxModel != nullptr) {
		m_upFbxModel->SetAnimationIndex(0);
	}
}

// 譖ｴ譁ｰ蜃ｦ逅・
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

   // 繝槭え繧ｹ繝帙う繝ｼ繝ｫ縺ｮ繧ｹ繧ｯ繝ｭ繝ｼ繝ｫ譁ｹ蜷代ｒ蛻晄悄蛹・
	Input::SetWheelDirection(0);

  // 繝槭え繧ｹ繧堤判髱｢荳ｭ螟ｮ縺ｫ蝗ｺ螳壹☆繧・
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

  // 繝槭え繧ｹ繧堤判髱｢荳ｭ螟ｮ縺ｫ蝗ｺ螳壹☆繧・
	Input::CenterMouseCursor();

	IsExitGame();
}

// 謠冗判蜃ｦ逅・
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

// 隗｣謾ｾ蜃ｦ逅・
void Main::Release()
{
  if (m_upFbxModel != nullptr) {
		m_upFbxModel->Release();
		m_upFbxModel.reset();
	}

	m_upDirectX12.reset();
}

// 繝｡繧､繝ｳ繝ｫ繝ｼ繝・
void Main::Loop()
{
 // 繧ｲ繝ｼ繝縺ｮ讒狗ｯ・
	if (FAILED(LoadData())) {
		return;
	}

 // 繧ｿ繧､繝樒ｲｾ蠎ｦ繧貞髄荳翫＆縺帙ｋ.
	timeBeginPeriod(1);

    // 繝・・繧ｿ隱ｭ縺ｿ霎ｼ縺ｿ蠕後↓繧ｲ繝ｼ繝繧呈ｧ狗ｯ・
	Create();

	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

        // Time 縺ｮ譖ｴ譁ｰ縺ｯ Loop 縺瑚ｲｬ蜍・
		Time::GetInstance().Update();
     // 繧ｲ繝ｼ繝譖ｴ譁ｰ縺ｨ謠冗判.
		Update();
		Draw();

        // 繝輔Ξ繝ｼ繝繝ｬ繝ｼ繝育ｶｭ謖・
		Time::GetInstance().MaintainFPS();
	}

  // 繧ｿ繧､繝櫁ｨｭ螳壹ｒ謌ｻ縺・
	timeEndPeriod(1);
}

// 繧ｦ繧｣繝ｳ繝峨え蛻晄悄蛹夜未謨ｰ.
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

// 繧ｦ繧｣繝ｳ繝峨え髢｢謨ｰ・医Γ繝・そ繝ｼ繧ｸ豈弱・蜃ｦ逅・ｼ・
LRESULT CALLBACK Main::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	//{
	//	return true;
	// 邵ｲﾂ
	//}
  // hWnd縺ｫ髢｢騾｣莉倥￠繧峨ｌ縺櫪ain繧貞叙蠕・
	// MEMO : 繧ｦ繧｣繝ｳ繝峨え菴懈・蜑阪・ nullptr 縺ｮ蜿ｯ閭ｽ諤ｧ縺後≠繧・
	Main* pMain = reinterpret_cast<Main*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // 繧ｦ繧｣繝ｳ繝峨え縺悟・繧√※菴懈・縺輔ｌ縺滓凾.
	if (uMsg == WM_NCCREATE) {
      // CREATESTRUCT縺九ｉMain繝昴う繝ｳ繧ｿ繧貞叙蠕・
		CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
     // hWnd縺ｫMain繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧帝未騾｣莉倥￠繧・
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
      // 繝・ヵ繧ｩ繝ｫ繝医・繧ｦ繧｣繝ｳ繝峨え繝励Ο繧ｷ繝ｼ繧ｸ繝｣繧貞他縺ｶ.
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	if (pMain) {
		switch (uMsg) {
          // 繧ｦ繧｣繝ｳ繝峨え縺檎ｴ譽・＆繧後ｋ縺ｨ縺・
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_ACTIVATEAPP:
         // 繧｢繝励Μ蠕ｩ蟶ｰ譎ゅ↓繧ｿ繧､繝槭ｒ繝ｪ繧ｻ繝・ヨ縺励※螟ｧ縺阪↑繝・Ν繧ｿ繧帝亟縺・
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

// Esc繧ｭ繝ｼ縺ｮ繝繝悶Ν繧ｿ繝・・縺ｧ繧ｲ繝ｼ繝繧堤ｵゆｺ・☆繧・
void Main::IsExitGame()
{
	constexpr int Esc = VK_ESCAPE;
  bool was_esc_pressed = !Input::IsKeyPress(Esc);
	UNREFERENCED_PARAMETER(was_esc_pressed);

   float current_time = Time::GetInstance().GetNowTime(); // 迴ｾ蝨ｨ縺ｮ繧ｲ繝ｼ繝蜀・凾蛻ｻ繧貞叙蠕・

    if (Input::IsKeyDown(Esc)) // Esc繧ｭ繝ｼ縺梧款縺輔ｌ縺溽椪髢・
	{
     // 蜑榊屓縺九ｉ縺ｮ邨碁℃譎る俣繧定ｨ育ｮ・
		float elapsed_time = current_time - m_LastEscPressTime;

     // 繝繝悶Ν繧ｿ繝・・蛻､螳・
       if (elapsed_time < DOUBLE_TAP_TIME_THRESHOLD)
       {
			if (MessageBox(m_hWnd, _T("Exit the game?"), _T("Confirm"), MB_YESNO) == IDYES) {
				DestroyWindow(m_hWnd);
			}
			m_LastEscPressTime = 0.0f;
		}
		else
		{
            // 繧ｷ繝ｳ繧ｰ繝ｫ繧ｿ繝・・譎ゅ・谺｡蝗槫愛螳夂畑縺ｫ譎ょ綾繧呈峩譁ｰ.
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

  // 蝓ｺ譛ｬ逧・↑FPS陦ｨ遉ｺ.
	float fps = ImGui::GetIO().Framerate;
	float ms = 1000.0f / fps;

	ImGui::Text("Average: %.1f FPS (%.3f ms/frame)", fps, ms);

  // 迥ｶ豕√↓蠢懊§縺溯ｭｦ蜻願｡ｨ遉ｺ.
	if (fps < 50.0f) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Low FPS!");
	}
	else {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Stable");
	}

	ImGui::Separator();

    // --- World Time Scale 縺ｮ隱ｿ謨ｴUI ---.
	ImGui::Text("World Time Scale");

   // 迴ｾ蝨ｨ縺ｮ time scale 繧貞叙蠕・
	float timeScale = Time::GetInstance().GetWorldTimeScale();

   // 繧ｹ繝ｩ繧､繝繝ｼ縺ｧ time scale 繧定ｪｿ謨ｴ・・.0f ・・4.0f・・
	if (ImGui::SliderFloat("Scale", &timeScale, 0.0f, 4.0f, "%.2f")) {
        // 雋縺ｮ蛟､縺ｯ辟｡蜉ｹ.
		if (timeScale < 0.0f) {
			timeScale = 0.0f;
		}
		Time::GetInstance().SetWorldTimeScale(timeScale);
	}

   // 繝ｪ繧ｻ繝・ヨ縺ｨ荳譎ょ●豁｢繝懊ち繝ｳ.
	if (ImGui::Button("Reset##TimeScale")) {
		Time::GetInstance().SetWorldTimeScale(1.0f);
		timeScale = 1.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause/Resume##TimeScale")) {
		float cur = Time::GetInstance().GetWorldTimeScale();
		Time::GetInstance().SetWorldTimeScale((cur > 0.0f) ? 0.0f : 1.0f);
	}

 // 迴ｾ蝨ｨ縺ｮ time scale 繧定｡ｨ遉ｺ.
	ImGui::Text("Current: %.2f", Time::GetInstance().GetWorldTimeScale());

	ImGui::Separator();

 // --- 繝・ぅ繧ｾ繝ｫ繝悶お繝輔ぉ繧ｯ繝・繝・ヰ繝・げ ---.
	ImGui::Text(IMGUI_JP("繝・ぅ繧ｾ繝ｫ繝悶お繝輔ぉ繧ｯ繝・繝・ヰ繝・げ"));
	
 // 繧ｹ繧ｭ繝ｳ繝｡繝・す繝･繝ｪ繧ｹ繝医ｒ蜿門ｾ・
	static int selectedMeshIndex = 0;
	auto meshList = MeshManager::GetSkinMeshList();
	
	if (!meshList.empty())
	{
      // 繝｡繝・す繝･驕ｸ謚槭さ繝ｳ繝懊・繝・け繧ｹ.
        if (ImGui::BeginCombo(IMGUI_JP("蟇ｾ雎｡繝｡繝・す繝･"), meshList[selectedMeshIndex].c_str()))
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
		
       // 驕ｸ謚槭Γ繝・す繝･縺ｮ繝・ぅ繧ｾ繝ｫ繝冶ｨｭ螳・
		auto pMesh = MeshManager::GetSkinMesh(meshList[selectedMeshIndex]);
		if (pMesh)
		{
			bool dissolveEnabled = pMesh->IsDissolveEnabled();
            if (ImGui::Checkbox(IMGUI_JP("繝・ぅ繧ｾ繝ｫ繝匁怏蜉ｹ"), &dissolveEnabled))
			{
				pMesh->SetDissolveEnabled(dissolveEnabled);
			}
			
			if (dissolveEnabled)
			{
				float threshold = pMesh->GetDissolveThreshold();
              if (ImGui::SliderFloat(IMGUI_JP("髢ｾ蛟､ (縺励″縺・､)"), &threshold, 0.0f, 1.0f, "%.2f"))
				{
					pMesh->SetDissolveThreshold(threshold);
				}
				
				float edgeWidth = pMesh->GetDissolveEdgeWidth();
                if (ImGui::SliderFloat(IMGUI_JP("繧ｨ繝・ず蟷・), &edgeWidth, 0.0f, 0.3f, "%.3f"))
				{
					pMesh->SetDissolveEdgeWidth(edgeWidth);
				}
				
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
					IMGUI_JP("窶ｻ 髢ｾ蛟､繧剃ｸ翫￡繧九⊇縺ｩ貅ｶ縺代∪縺吶・));
			}
		}
	}
	else
	{
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
			IMGUI_JP("繧ｹ繧ｭ繝ｳ繝｡繝・す繝･縺瑚ｪｭ縺ｿ霎ｼ縺ｾ繧後※縺・∪縺帙ｓ縲・));
	}

	ImGui::Separator();

	ImGui::End();
#endif
}


