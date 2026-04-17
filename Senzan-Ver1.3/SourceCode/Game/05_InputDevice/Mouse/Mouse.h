#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"

/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : マウスクラス.
**********************************************************************************/
class Mouse final
	: public Singleton<Mouse>
{
private:
	friend class Singleton<Mouse>;
	Mouse();
public:
	~Mouse();

	// 更新.
	static void Update();


	// マウスカーソルをウィンドウ中央に移動.
	static void CenterMouseCursor();


	// マウスカーソルを画面内に収める.
	static void WrapCursorInScreen();


	// カーソルがウィンドウ内にあるか.
	static bool IsCursorInWindow();


	// 指定矩形にカーソルがあるか.
	static bool IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size);

public: // Getter / Setter.

	// ウィンドウハンドルを設定.
	static void SethWnd(HWND hWnd);


	// 現在のカーソル座標を取得.
	static DirectX::XMFLOAT2 GetCursorPosition();
	static DirectX::XMFLOAT2 GetClientCursorPosition();


	// 前フレームのカーソル座標を取得.
	static DirectX::XMFLOAT2 GetPastCursorPosition();
	static DirectX::XMFLOAT2 GetPastClientCursorPosition();

	// カーソルの移動量を取得.
	static DirectX::XMFLOAT2 GetClientCursorDelta();


	// マウスホイール値の取得/設定.
	static int&	GetWheelDirection();
	static void SetWheelDirection(const int& direction);


	// カーソル中央固定状態の取得/設定.
	static bool& IsCenterMouseCursor();
	static void SetCenterMouseCursor(const bool& isCenter);


	// マウスグラブ状態の取得/設定.
	static bool& IsMouseGrab();
	static void SetMouseGrab(const bool& isGrab);


	// カーソル表示を設定.
	static void SetShowCursor(const bool& isShowCursor);

private:
	HWND	m_hWnd;
	POINT	m_NowMousePoint;		// 現在のマウス座標.
	POINT	m_PastMousePoint;		// 前フレームのマウス座標.
	POINT	m_NowClientMousePoint;	// 現在のクライアント座標.
	POINT	m_PastClientMousePoint;	// 前フレームのクライアント座標.
	DirectX::XMFLOAT2 m_ClientCursorDelta;	// カーソルの移動量.
	int		m_WheelDirection;		// ホイール方向.
	bool	m_IsGrab;				// マウスグラブ状態.
	bool	m_IsCenterMouseCursor;	// カーソル中央固定状態.
	bool	m_IsShowCursor;			// カーソル表示状態.
};

