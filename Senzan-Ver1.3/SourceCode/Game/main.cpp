#include "MyMacro.h"
#include "Graphic/DirectX/DirectX12/DirectX12.h"
#include "Graphic/FBX/FBXModel.h"
#include <array>
#include <cmath>
#include <iostream>

#pragma comment(lib, "user32.lib")

HWND g_HWnd = nullptr;
HINSTANCE g_HInstance = nullptr;
DirectX12* g_pDirectX12 = nullptr;
FBXModel* g_pModel = nullptr;

DirectX::XMFLOAT3 g_CameraPosition(0.0f, 100.0f, -200.0f);
float g_CameraYaw = 0.0f;
float g_CameraPitch = 0.0f;

bool InitWindow(HINSTANCE HInstance, int CommandShow);
bool LoadYBotModel();
void HandleAnimationInput();
void HandleCameraInput(float DeltaTime);
void Update(float DeltaTime);
void RenderYBot();
LRESULT CALLBACK WndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam);

int WINAPI WinMain(HINSTANCE HInstance, HINSTANCE, LPSTR, int CommandShow)
{
	g_HInstance = HInstance;

	if (!InitWindow(HInstance, CommandShow))
	{
		MessageBox(nullptr, _T("ウィンドウの作成に失敗しました"), _T("エラー"), MB_OK);
		return 1;
	}

	g_pDirectX12 = new DirectX12();
	if (!g_pDirectX12->Create(g_HWnd))
	{
		MessageBox(nullptr, _T("DirectX12の初期化に失敗しました"), _T("エラー"), MB_OK);
		delete g_pDirectX12;
		g_pDirectX12 = nullptr;
		return 1;
	}

	g_pModel = new FBXModel();
	if (!LoadYBotModel())
	{
		MessageBox(nullptr, _T("YBotの読み込みに失敗しました"), _T("エラー"), MB_OK);
		delete g_pModel;
		g_pModel = nullptr;
		delete g_pDirectX12;
		g_pDirectX12 = nullptr;
		return 1;
	}
	g_pModel->SetAnimationIndex(0);

	MSG msg = {};
	LARGE_INTEGER freq = {};
	LARGE_INTEGER prev = {};
	LARGE_INTEGER now = {};
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&prev);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			QueryPerformanceCounter(&now);
			double delta_time_sec = static_cast<double>(now.QuadPart - prev.QuadPart) / static_cast<double>(freq.QuadPart);
			float delta_time = static_cast<float>(delta_time_sec);
			prev = now;

			Update(delta_time);
			g_pDirectX12->Update();
			g_pDirectX12->BeginDraw();
			RenderYBot();
			g_pDirectX12->EndDraw();
		}
	}

	delete g_pModel;
	g_pModel = nullptr;
	delete g_pDirectX12;
	g_pDirectX12 = nullptr;

	return static_cast<int>(msg.wParam);
}

bool LoadYBotModel()
{
	if (!g_pModel || !g_pDirectX12)
	{
		OutputDebugStringA("[LoadYBotModel] ERROR: g_pModel or g_pDirectX12 is null\n");
		return false;
	}

	auto p_device12 = g_pDirectX12->GetDevice();
	if (!p_device12)
	{
		OutputDebugStringA("[LoadYBotModel] ERROR: Failed to get D3D12 device\n");
		return false;
	}

	auto p_cmd_queue = g_pDirectX12->GetCommandQueue();
	if (!p_cmd_queue)
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
		std::string msg = "[LoadYBotModel] Trying to load: " + std::string(model_path) + "\n";
		OutputDebugStringA(msg.c_str());
		if (g_pModel->Load(p_device12.Get(), p_cmd_queue.Get(), model_path))
		{
			msg = "[LoadYBotModel] SUCCESS: Model loaded from " + std::string(model_path) + "\n";
			OutputDebugStringA(msg.c_str());
			return true;
		}
		msg = "[LoadYBotModel] FAILED: " + std::string(model_path) + "\n";
		OutputDebugStringA(msg.c_str());
	}

	OutputDebugStringA("[LoadYBotModel] ERROR: All model paths failed\n");
	return false;
}

void HandleAnimationInput()
{
	if (!g_pModel)
	{
		return;
	}

	static bool is_prev_pressed[9] = { false, false, false, false, false, false, false, false, false };

	for (int i = 0; i < 9; ++i)
	{
		const int vk_code = '1' + i;
		const bool is_pressed = (GetAsyncKeyState(vk_code) & 0x8000) != 0;

		if (is_pressed && !is_prev_pressed[i])
		{
			const uint32_t anim_index = static_cast<uint32_t>(i);
			if (anim_index < g_pModel->GetAnimationCount())
			{
				if (g_pModel->SetAnimationIndex(anim_index))
				{
					const std::string msg = "[Input] Animation switched to index: " + std::to_string(anim_index) + "\n";
					OutputDebugStringA(msg.c_str());
				}
			}
		}

		is_prev_pressed[i] = is_pressed;
	}
}

void HandleCameraInput(float DeltaTime)
{
	if (!g_HWnd)
	{
		return;
	}

	static bool s_IsFirstMouse = true;
	static POINT s_PrevMousePos = {};

	const bool is_right_mouse = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	const float rotate_sensitivity = 0.0035f;
	const float base_move_speed = 120.0f;
	const float fast_rate = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) ? 3.0f : 1.0f;
	const float move_speed = base_move_speed * fast_rate * DeltaTime;

	if (is_right_mouse)
	{
		POINT mouse_pos = {};
		GetCursorPos(&mouse_pos);
		ScreenToClient(g_HWnd, &mouse_pos);

		if (s_IsFirstMouse)
		{
			s_PrevMousePos = mouse_pos;
			s_IsFirstMouse = false;
		}

		const int delta_x = mouse_pos.x - s_PrevMousePos.x;
		const int delta_y = mouse_pos.y - s_PrevMousePos.y;
		s_PrevMousePos = mouse_pos;

		g_CameraYaw += static_cast<float>(delta_x) * rotate_sensitivity;
		g_CameraPitch += static_cast<float>(delta_y) * rotate_sensitivity;

		const float max_pitch = DirectX::XM_PIDIV2 - 0.01f;
		if (g_CameraPitch > max_pitch) { g_CameraPitch = max_pitch; }
		if (g_CameraPitch < -max_pitch) { g_CameraPitch = -max_pitch; }
	}
	else
	{
		s_IsFirstMouse = true;
	}

	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		sinf(g_CameraYaw) * cosf(g_CameraPitch),
		sinf(g_CameraPitch),
		cosf(g_CameraYaw) * cosf(g_CameraPitch),
		0.0f);
	v_forward = DirectX::XMVector3Normalize(v_forward);

	DirectX::XMVECTOR v_world_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR v_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_world_up, v_forward));

	DirectX::XMVECTOR v_position = DirectX::XMLoadFloat3(&g_CameraPosition);

	if (GetAsyncKeyState('W') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_forward, move_speed)); }
	if (GetAsyncKeyState('S') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_forward, move_speed)); }
	if (GetAsyncKeyState('A') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_right, move_speed)); }
	if (GetAsyncKeyState('D') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_right, move_speed)); }
	if (GetAsyncKeyState('Q') & 0x8000) { v_position = DirectX::XMVectorSubtract(v_position, DirectX::XMVectorScale(v_world_up, move_speed)); }
	if (GetAsyncKeyState('E') & 0x8000) { v_position = DirectX::XMVectorAdd(v_position, DirectX::XMVectorScale(v_world_up, move_speed)); }

	DirectX::XMStoreFloat3(&g_CameraPosition, v_position);
}

void Update(float DeltaTime)
{
	HandleCameraInput(DeltaTime);

	if (g_pModel)
	{
		HandleAnimationInput();
		g_pModel->Update(DeltaTime);
	}
}

void RenderYBot()
{
	if (!g_pDirectX12 || !g_pModel)
	{
		return;
	}

	auto p_command_list = g_pDirectX12->GetCommandList();
	if (!p_command_list)
	{
		return;
	}

	// PSO と Root Signature をセット.
	auto p_pso = g_pDirectX12->GetPipelineState();
	auto p_root_sig = g_pDirectX12->GetRootSignature();
	if (!p_pso || !p_root_sig)
	{
		OutputDebugStringA("[RenderYBot] PSO or Root Signature is null\n");
		return;
	}

	p_command_list->SetPipelineState(p_pso.Get());
	p_command_list->SetGraphicsRootSignature(p_root_sig.Get());

	// ビューポート、シザー矩形をセット.
	auto p_viewport_ptr = g_pDirectX12->GetViewport();
	auto p_scissor_rect_ptr = g_pDirectX12->GetScissorRect();
	if (p_viewport_ptr && p_scissor_rect_ptr && *p_viewport_ptr && *p_scissor_rect_ptr)
	{
		p_command_list->RSSetViewports(1, p_viewport_ptr->get());
		p_command_list->RSSetScissorRects(1, p_scissor_rect_ptr->get());
	}

	DirectX::XMVECTOR v_eye = DirectX::XMLoadFloat3(&g_CameraPosition);
	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		sinf(g_CameraYaw) * cosf(g_CameraPitch),
		sinf(g_CameraPitch),
		cosf(g_CameraYaw) * cosf(g_CameraPitch),
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

	g_pModel->SetWorldMatrix(DirectX::XMMatrixIdentity());
	g_pModel->SetViewMatrix(view);
	g_pModel->SetProjectionMatrix(proj);
	g_pModel->Render(p_command_list.Get());
}

bool InitWindow(HINSTANCE HInstance, int CommandShow)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = HInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszClassName = _T("DirectX12DemoClass");
	RegisterClassEx(&wc);

	RECT rc = { 0, 0, static_cast<LONG>(WND_W), static_cast<LONG>(WND_H) };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_HWnd = CreateWindowEx(
		0,
		_T("DirectX12DemoClass"),
		_T("Senzan DirectX12"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr,
		nullptr,
		HInstance,
		nullptr);

	if (!g_HWnd)
	{
		return false;
	}

	ShowWindow(g_HWnd, CommandShow);
	UpdateWindow(g_HWnd);
	return true;
}

LRESULT CALLBACK WndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(HWnd, Message, WParam, LParam);
	}
}

int main()
{
	return WinMain(GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOWDEFAULT);
}
