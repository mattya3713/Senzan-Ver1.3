#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include "KeyInput/KeyInput.h"
#include "Mouse/Mouse.h"
#include "XInput/XInput.h"

/**********************************************************************************
* @author    : mattya3713.
* @date      : 2026/04/16.
* @brief     : 入力管理クラス.
**********************************************************************************/

class Input final
	: public Singleton<Input>
{
private:
	friend class Singleton<Input>;
	Input();
public:
	~Input() override;

	static void Update();


	// ウィンドウハンドルを設定.
	static void SethWnd(HWND hWnd);

public: // キーボード.

	// キーが押されている間.
	static bool IsKeyPress(int key);

	// 指定した全キーが押されている間.
	static bool IsKeyPress(const std::vector<int>& keyList);

	// キーが押された瞬間.
	static bool IsKeyDown(int key);

	// 指定した全キーが押された瞬間.
	static bool IsKeyDown(const std::vector<int>& keyList);

	// キーが離された瞬間.
    static bool IsKeyUp(int key);

	// キーがリピート中なら.
    static bool IsKeyRepeat(int key);

	// 指定した全キーがリピート中なら.
	static bool IsKeyRepeat(const std::vector<int>& keyList);


public: // マウス.

	// マウスカーソルをウィンドウ中央に移動.
	static void CenterMouseCursor();


	// マウスカーソルを画面内に収める.
	static void WrapCursorInScreen();


	// カーソルがウィンドウ内にあるか.
   static bool IsCursorInWindow();


	// 指定矩形にカーソルがあるか.
   static bool IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size);
	

	// 現在のカーソル座標を取得.
	static DirectX::XMFLOAT2 GetCursorPosition();
	static DirectX::XMFLOAT2 GetClientCursorPosition();


	// 前フレームのカーソル座標を取得.
	static DirectX::XMFLOAT2 GetPastCursorPosition();
	static DirectX::XMFLOAT2 GetPastClientCursorPosition();

	// クライアント座標の移動量を取得.
	static DirectX::XMFLOAT2 GetClientCursorDelta();


	// ホイール方向の取得/設定.
    static int GetWheelDirection();	
	static void SetWheelDirection(int direction);


	// 中央固定モードの取得/設定.
    static bool IsCenterMouseCursor();	
	static void SetCenterMouseCursor(bool isCenter);


	// マウスグラブ状態の取得/設定.
    static bool IsMouseGrab();
    static void SetMouseGrab(bool isGrab);


	// カーソル表示を設定.
    static void SetShowCursor(bool isShowCursor);

public:// コントローラー.

	// ボタンが押された瞬間.
	static bool IsButtonDown(XInput::Key key, int id = 0);


	// ボタンが離された瞬間.
    static bool IsButtonUp(XInput::Key key, int id = 0);


	// ボタンがリピート中なら.
    static bool IsButtonRepeat(XInput::Key key, int id = 0);


	// スティック方向が押された瞬間.
	static bool IsLStickDirectionDown(XInput::StickState dir, bool isFirstPress = false, int id = 0);
	static bool IsRStickDirectionDown(XInput::StickState dir, bool isFirstPress = false, int id = 0);

	// スティック入力が離された瞬間.
	static bool IsLStickDirectionUp(int id = 0);
	static bool IsRStickDirectionUp(int id = 0);

	// スティック方向がリピート中なら.
	static bool IsLStickDirectionRepeat(XInput::StickState dir, int id = 0);
	static bool IsRStickDirectionRepeat(XInput::StickState dir, int id = 0);

    // スティック入力が有効か判定.
	static bool IsLStickActive(float deadZone = 0.0f, int id = 0);
	static bool IsRStickActive(float deadZone = 0.0f, int id = 0);

    // コントローラーを取得.
	static std::shared_ptr<XInput> GetController(int id = 0);

    // スティック入力値を取得.
	static DirectX::XMFLOAT2 GetLStickDirection(int id = 0);
	static DirectX::XMFLOAT2 GetRStickDirection(int id = 0);
	
    // トリガー入力値を取得.
	static float GetLTriggerRaw(int id = 0);
	static float GetRTriggerRaw(int id = 0);
	static float GetLTrigger(int id = 0);
	static float GetRTrigger(int id = 0);

private:

	// 全コントローラーの更新.
	void UpdateForAllController();


	// コントローラーを生成.
	void CreateController();

public:// 定数.

	// コントローラー最大数.
	static constexpr int CONTROLLER_MAX = 4;
private:
	HWND m_hWnd;
	std::array<std::shared_ptr<XInput>, CONTROLLER_MAX> m_pControllers;
};

