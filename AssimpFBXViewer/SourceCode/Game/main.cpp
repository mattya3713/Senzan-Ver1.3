// DirectX 11 デモアプリケーション
// D3DXライブラリを使用せず、DirectXMathを使用

#include "MyMacro.h"
#include "Graphic/DirectX/DirectX11.h"
#include "Graphic/FBX/FBXModel.h"
#include "System/Utility/Debug/DebugDraw/DebugDraw.h"
#include <DirectXMath.h>
#include <iostream>
#include <cstdio>
#include <Windows.h>
#include <windowsx.h>

#pragma comment(lib, "user32.lib")

//=============================================================================
// グローバル変数
//=============================================================================
HWND			g_hWnd = nullptr;
HINSTANCE		g_hInstance = nullptr;
DirectX11*		g_pDirectX11 = nullptr;
FBXModel*		g_pModel = nullptr;
DebugDraw*		g_pDebugDraw = nullptr;

// カメラパラメータ.
float g_CameraDistance = 200.0f;
float g_CameraAngleY = 0.0f;
// 可動カメラ.
DirectX::XMFLOAT3 g_CameraPos = { 0.0f, 100.0f, -200.0f };
float g_CameraYaw = 0.0f;   // 回転(Y軸、ラジアン)
float g_CameraPitch = 0.0f; // 回転(X軸、ラジアン)
bool g_RButtonDown = false;
POINT g_LastMousePos = { 0, 0 };
float g_CameraMoveSpeed = 200.0f; // units per second
float g_MouseSensitivity = 0.005f; // radians per pixel

// アニメーション切り替え用.
int g_CurrentAnimIndex = 0;

bool InitWindow(HINSTANCE HInstance, int CommandShow);
void Update(float DeltaTime);
void Render();
void PrintModelInfo();
void TestBoneAPI();
void TestRaycastAPI();
LRESULT CALLBACK WndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam);


int WINAPI WinMain(HINSTANCE HInstance, HINSTANCE, LPSTR, int CommandShow)
{
	g_hInstance = HInstance;

	// コンソールウィンドウを作成（デバッグ用）.
	AllocConsole();
	FILE* fp = nullptr;
	(void)freopen_s(&fp, "CONOUT$", "w", stdout);
	(void)freopen_s(&fp, "CONOUT$", "w", stderr);

	// ウィンドウ初期化
	if (!InitWindow(HInstance, CommandShow))
	{
		MessageBox(nullptr, _T("ウィンドウの作成に失敗しました"), _T("エラー"), MB_OK);
		return 1;
	}

	// DirectX11初期化
	g_pDirectX11 = new DirectX11();
	if (FAILED(g_pDirectX11->Create(g_hWnd)))
	{
		MessageBox(nullptr, _T("DirectX11の初期化に失敗しました"), _T("エラー"), MB_OK);
		SAFE_DELETE(g_pDirectX11);
		return 1;
	}

	// FBXモデル読み込み
	g_pModel = new FBXModel();
	if (!g_pModel->Load(g_pDirectX11, "Data/YBot/Y-Bot-Anim-In-And-Parts.fbx"))
	{
		MessageBox(nullptr, _T("FBXモデルの読み込みに失敗しました"), _T("エラー"), MB_OK);
		SAFE_DELETE(g_pModel);
		SAFE_DELETE(g_pDirectX11);
		return 1;
	}

	// DebugDraw初期化
	g_pDebugDraw = new DebugDraw();
	if (!g_pDebugDraw->Create(g_pDirectX11))
	{
		MessageBox(nullptr, _T("DebugDrawの初期化に失敗しました"), _T("エラー"), MB_OK);
		SAFE_DELETE(g_pDebugDraw);
		SAFE_DELETE(g_pModel);
		SAFE_DELETE(g_pDirectX11);
		return 1;
	}

	// モデル情報を表示.
	PrintModelInfo();

	// ボーンAPIのテスト.
	TestBoneAPI();

	// 最初のアニメーションを選択.
	g_pModel->SetAnimationIndex(0);
	std::cout << "Selected Animation Index: 0" << std::endl;

	// 操作説明を表示.
	std::cout << "\n=== Controls ===" << std::endl;
	std::cout << "WASD: Move camera" << std::endl;
	std::cout << "Space/Ctrl: Move up/down" << std::endl;
	std::cout << "Right Mouse + Drag: Look around" << std::endl;
	std::cout << "1-9: Switch animation" << std::endl;
	std::cout << "R: Test raycast" << std::endl;
	std::cout << "B: Print bone positions" << std::endl;
	std::cout << "ESC: Exit" << std::endl;
	std::cout << "================\n" << std::endl;

	// メインループ
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
			Render();
		}
	}

	// 解放
	SAFE_DELETE(g_pDebugDraw);
	SAFE_DELETE(g_pModel);
	SAFE_DELETE(g_pDirectX11);

	return static_cast<int>(msg.wParam);
}

// モデル情報を表示.
void PrintModelInfo()
{
	if (!g_pModel) return;

	std::cout << "\n=== Model Info ===" << std::endl;
	std::cout << "Mesh Count: " << g_pModel->GetMeshCount() << std::endl;
	std::cout << "Bone Count: " << g_pModel->GetBoneCount() << std::endl;
	std::cout << "Is Skinned: " << (g_pModel->IsSkinned() ? "Yes" : "No") << std::endl;

	// アニメーション一覧.
	std::cout << "\nAnimation Count: " << g_pModel->GetAnimationCount() << std::endl;
	auto anim_names = g_pModel->GetAnimationNames();
	for (size_t i = 0; i < anim_names.size(); ++i)
	{
		float duration = g_pModel->GetAnimationDuration(static_cast<uint32_t>(i));
		std::cout << "  [" << i << "] \"" << anim_names[i] << "\" (Duration: " << duration << "s)" << std::endl;
	}

	std::cout << "==================\n" << std::endl;
}

// ボーンAPIのテスト.
void TestBoneAPI()
{
	if (!g_pModel || !g_pModel->IsSkinned()) return;

	std::cout << "\n=== Bone API Test ===" << std::endl;

	// ボーン名一覧を取得.
	auto bone_names = g_pModel->GetBoneNames();
	std::cout << "Total Bones: " << bone_names.size() << std::endl;

	// 最初の5つのボーンの位置を表示.
	int count = 0;
	for (const auto& bone_name : bone_names)
	{
		if (count >= 5) break;

		DirectX::XMFLOAT3 pos = {};
		if (g_pModel->GetBoneWorldPosition(bone_name, pos))
		{
			std::cout << "  " << bone_name << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
		}
		++count;
	}

	std::cout << "======================\n" << std::endl;
}

// レイキャストAPIのテスト.
void TestRaycastAPI()
{
	if (!g_pModel) return;

	std::cout << "\n=== Raycast Test ===" << std::endl;

	// カメラ位置・向きからレイを生成.
	FBXMesh::Ray ray =
	{
		g_CameraPos,
		{ cosf(g_CameraPitch) * sinf(g_CameraYaw), sinf(g_CameraPitch), cosf(g_CameraPitch) * cosf(g_CameraYaw) }
	};

	std::cout << "Ray Origin: (" << ray.Origin.x << ", " << ray.Origin.y << ", " << ray.Origin.z << ")" << std::endl;
	std::cout << "Ray Direction: (" << ray.Direction.x << ", " << ray.Direction.y << ", " << ray.Direction.z << ")" << std::endl;

	// 三角形判定.
	FBXMesh::HitInfo hit = {};
	if (g_pModel->RayCast(ray, hit))
	{
		std::cout << "Triangle Hit!" << std::endl;
		std::cout << "  Distance: " << hit.Distance << std::endl;
		std::cout << "  Position: (" << hit.Position.x << ", " << hit.Position.y << ", " << hit.Position.z << ")" << std::endl;
		std::cout << "  Normal: (" << hit.Normal.x << ", " << hit.Normal.y << ", " << hit.Normal.z << ")" << std::endl;
		std::cout << "  Mesh Index: " << hit.MeshIndex << std::endl;
		std::cout << "  Triangle Index: " << hit.TriangleIndex << std::endl;
	}
	else
	{
		std::cout << "Triangle Miss (or Skinned mesh - no CPU data)" << std::endl;
	}

	std::cout << "====================\n" << std::endl;
}

// ウィンドウ初期化.
bool InitWindow(HINSTANCE HInstance, int CommandShow)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = HInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszClassName = _T("DirectX11DemoClass");
	RegisterClassEx(&wc);

	RECT rc = { 0, 0, static_cast<LONG>(WND_W), static_cast<LONG>(WND_H) };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindowEx(
		0,
		_T("DirectX11DemoClass"),
		_T("Assimp FBX Viewer - Multiple Animation & Bone API Demo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, nullptr, HInstance, nullptr);

	if (!g_hWnd)
		return false;

	ShowWindow(g_hWnd, CommandShow);
	UpdateWindow(g_hWnd);

	return true;
}

// 更新.
void Update(float DeltaTime)
{
	// カメラ移動（WASD + Space/Ctrl）およびマウスルックで操作

	float move_speed = g_CameraMoveSpeed;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		move_speed *= 3.0f;

	// 方向ベクトルを yaw/pitch から計算
	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		cosf(g_CameraPitch) * sinf(g_CameraYaw),
		sinf(g_CameraPitch),
		cosf(g_CameraPitch) * cosf(g_CameraYaw),
		0.0f);
	DirectX::XMVECTOR v_up    = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR v_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_up, v_forward));
	DirectX::XMVECTOR v_pos   = DirectX::XMLoadFloat3(&g_CameraPos);

	if (GetAsyncKeyState('W') & 0x8000) {
		v_pos = DirectX::XMVectorAdd(v_pos, DirectX::XMVectorScale(v_forward, move_speed * DeltaTime));
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		v_pos = DirectX::XMVectorSubtract(v_pos, DirectX::XMVectorScale(v_forward, move_speed * DeltaTime));
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		v_pos = DirectX::XMVectorSubtract(v_pos, DirectX::XMVectorScale(v_right, move_speed * DeltaTime));
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		v_pos = DirectX::XMVectorAdd(v_pos, DirectX::XMVectorScale(v_right, move_speed * DeltaTime));
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		v_pos = DirectX::XMVectorAdd(v_pos, DirectX::XMVectorScale(v_up, move_speed * DeltaTime));
	}
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		v_pos = DirectX::XMVectorSubtract(v_pos, DirectX::XMVectorScale(v_up, move_speed * DeltaTime));
	}
	DirectX::XMStoreFloat3(&g_CameraPos, v_pos);

	// アニメーション更新.
	if (g_pModel)
	{
		g_pModel->Update(DeltaTime);
	}
}

// 描画.
void Render()
{
	if (!g_pDirectX11) return;

	// バックバッファクリア.
	g_pDirectX11->ClearBackBuffer();

	// 深度テストON.
	g_pDirectX11->SetDepth(true);

	// アルファブレンドON.
	g_pDirectX11->SetAlphaBlend(true);

	// ビュー・プロジェクション行列（デバッグ描画と共用）.
	DirectX::XMVECTOR v_eye     = DirectX::XMLoadFloat3(&g_CameraPos);
	DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(
		cosf(g_CameraPitch) * sinf(g_CameraYaw),
		sinf(g_CameraPitch),
		cosf(g_CameraPitch) * cosf(g_CameraYaw),
		0.0f);

	DirectX::XMVECTOR v_at = DirectX::XMVectorAdd(v_eye, v_forward);
	DirectX::XMVECTOR v_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(v_eye, v_at, v_up);
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		WND_W / WND_H,
		0.1f,
		1000.0f);

	DirectX::XMMATRIX view_proj = DirectX::XMMatrixMultiply(view, proj);

	// モデル描画.
	if (g_pModel)
	{
		g_pModel->SetWorldMatrix(DirectX::XMMatrixIdentity());
		g_pModel->SetViewMatrix(view);
		g_pModel->SetProjectionMatrix(proj);
		g_pModel->Render(g_pDirectX11);
	}

	// デバッグ描画（深度テストOFF → 常に手前に表示）.
	if (g_pDebugDraw)
	{
		g_pDirectX11->SetDepth(false);
		g_pDebugDraw->Clear();

		// ボーンを緑クロスで表示.
		if (g_pModel && g_pModel->IsSkinned())
		{
			DirectX::XMFLOAT4 bone_color(0.0f, 1.0f, 0.0f, 1.0f);
			auto bone_names = g_pModel->GetBoneNames();
			for (const auto& name : bone_names)
			{
				DirectX::XMFLOAT3 bone_pos = {};
				if (g_pModel->GetBoneWorldPosition(name, bone_pos))
				{
					g_pDebugDraw->AddCross(bone_pos, 3.0f, bone_color);
				}
			}
		}

		// レティクル：カメラ正面でレイキャストし、ヒット→赤、ミス→黄.
		if (g_pModel)
		{
			FBXMesh::Ray center_ray =
			{
				g_CameraPos,
				{ cosf(g_CameraPitch) * sinf(g_CameraYaw), sinf(g_CameraPitch), cosf(g_CameraPitch) * cosf(g_CameraYaw) }
			};

			FBXMesh::HitInfo center_hit = {};
			bool did_hit = g_pModel->RayCast(center_ray, center_hit);

			// ヒット: 赤、ミス: 黄.
			DirectX::XMFLOAT4 reticle_color = did_hit
				? DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
				: DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

			// 縦横比を考慮したアーム長（約20ピクセル相当）.
			float arm_x = 20.0f / (WND_W * 0.5f);
			float arm_y = 20.0f / (WND_H * 0.5f);
			g_pDebugDraw->AddCrossNDC(0.0f, 0.0f, arm_x, arm_y, reticle_color);
		}

		g_pDebugDraw->Render(g_pDirectX11, view_proj);
		g_pDirectX11->SetDepth(true);
	}

	// 表示
	g_pDirectX11->Present();
}

// ウィンドウプロシージャ.
LRESULT CALLBACK WndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (WParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		// 数字キー1-9でアニメーション切り替え.
		else if (WParam >= '1' && WParam <= '9')
		{
			int anim_index = static_cast<int>(WParam - '1');
			if (g_pModel && anim_index < static_cast<int>(g_pModel->GetAnimationCount()))
			{
				// アニメーション切り替え.
				g_pModel->SetAnimationIndex(anim_index);
				std::cout << "Animation[" << anim_index << "]: "
					<< g_pModel->GetAnimationName(anim_index) << std::endl;
			}
		}
		// Rキーでレイキャストテスト（コンソール出力のみ）.
		else if (WParam == 'R')
		{
			TestRaycastAPI();
		}
		// Bキーでボーン位置表示.
		else if (WParam == 'B')
		{
			if (g_pModel && g_pModel->IsSkinned())
			{
				std::cout << "\n=== Current Bone Positions ===" << std::endl;
				auto bone_names = g_pModel->GetBoneNames();
				for (const auto& name : bone_names)
				{
					DirectX::XMFLOAT3 pos = {};
					if (g_pModel->GetBoneWorldPosition(name, pos))
					{
						std::cout << "  " << name << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
					}
				}
				std::cout << "==============================\n" << std::endl;
			}
		}
		return 0;

	case WM_RBUTTONDOWN:
		g_RButtonDown = true;
		SetCapture(HWnd);
		g_LastMousePos.x = GET_X_LPARAM(LParam);
		g_LastMousePos.y = GET_Y_LPARAM(LParam);
		return 0;

	case WM_RBUTTONUP:
		g_RButtonDown = false;
		ReleaseCapture();
		return 0;

	case WM_MOUSEMOVE:
		if (g_RButtonDown)
		{
			POINT pt = { GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam) };

			int dx = pt.x - g_LastMousePos.x;
			int dy = pt.y - g_LastMousePos.y;

			g_CameraYaw += static_cast<float>(dx) * g_MouseSensitivity;
			g_CameraPitch += -static_cast<float>(dy) * g_MouseSensitivity;

			// ピッチのクランプ
			const float limit = DirectX::XM_PIDIV2 - 0.01f;
			if (g_CameraPitch > limit) g_CameraPitch = limit;
			if (g_CameraPitch < -limit) g_CameraPitch = -limit;

			g_LastMousePos = pt;
		}
		return 0;

	default:
		return DefWindowProc(HWnd, Message, WParam, LParam);
	}

}

//=============================================================================
// コンソールアプリケーション用エントリーポイント
//=============================================================================
int main()
{
	return WinMain(GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOWDEFAULT);
}
