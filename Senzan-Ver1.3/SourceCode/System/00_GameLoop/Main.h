#pragma once

#include <array>
#include <memory>
#include <DirectXMath.h>

//クラスの前方宣言.
class DirectX12;
class FBXModel;
class Time;
class SceneManager;

class Loader;
class Input;


/**************************************************
*	メインクラス.
**/
class Main
{
public:
	Main();		// コンストラクタ.
	~Main();	// デストラクタ.

	void Update();		// 更新処理.
	void Draw();		// 描画処理.
	HRESULT LoadData();	// データロード処理.
	void Create();		// 初期処理.
	void Release();		// 解放処理.

	void Loop();		// メインループ.

	// ウィンドウ初期化関数.
	HRESULT InitWindow(
		HINSTANCE hInstance,
		INT x, INT y,
		INT width, INT height );

private:
	// ウィンドウ関数.
	static LRESULT CALLBACK MsgProc(
		HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam );


	// Escキーのダブルタップでゲームを終了する.
	void IsExitGame();
	bool LoadYBotModel();
	void HandleAnimationInput();
	void HandleCameraInput(float DeltaTime);
	void RenderYBot();

    void DebugImgui(); // デバッグ表示.

private:
	HWND			m_hWnd;	 // ウィンドウハンドル.
 std::unique_ptr<DirectX12> m_upDirectX12;
	std::unique_ptr<FBXModel> m_upFbxModel;
//	std::unique_ptr<Loader>			m_pResourceLoader;

	DirectX::XMFLOAT3 m_CameraPosition = DirectX::XMFLOAT3(0.0f, 100.0f, -200.0f);
	float m_CameraYaw = 0.0f;
	float m_CameraPitch = 0.0f;
	bool m_IsFirstMouse = true;
	POINT m_PrevMousePos = {};
	std::array<bool, 9> m_IsPrevAnimKeyPressed = { false, false, false, false, false, false, false, false, false };
	
	// ゲームを終了するためのデバッグフラグ.
	float m_LastEscPressTime = 0.0f; // 前回Escが押されたゲーム内時刻.
	const float DOUBLE_TAP_TIME_THRESHOLD = 0.3f; // ダブルタップとみなす時間 (例: 0.3秒).

	float m_SomeFloatValue = 0.0f;
	bool m_bFeatureEnabled = false;
};
