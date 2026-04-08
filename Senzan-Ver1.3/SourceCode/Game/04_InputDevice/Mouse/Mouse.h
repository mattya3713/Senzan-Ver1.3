#pragma once
#include "System/Singleton/SingletonTemplate.h"

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

	/************************************************************
	* @brief 更新.
	************************************************************/
	static void Update();


	/************************************************************
	* @brief マウスカーソルをウィンドウの中心に固定する.
	************************************************************/
	static void CenterMouseCursor();


	/************************************************************
	* @brief マウスカーソルがスクリーン内に収める.
	************************************************************/
	static void WrapCursorInScreen();


	/************************************************************
	* @brief マウスカーソルがウィンドウ内にあるか判定.
	************************************************************/
	static bool IsCursorInWindow();


	/************************************************************
	* @brief マウスカーソルが指定領域にあるか.
	* @param[in] position	：判定座標.
	* @param[in] size		：判定の幅高さ.
	************************************************************/
	static bool IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size);

public: // Getter、Setter.

	/************************************************************
	* @brief ウィンドウハンドルを設定.
	* @param[in] hWnd：ウィンドウハンドル.
	************************************************************/
	static void SethWnd(HWND hWnd);


	/************************************************************
	* @brief 現在のカーソル座標を取得.
	* @note GetCursorPosition		：ディスプレイ基準.
	* @note GetClientCursorPosition	：ウィンドウ基準.
	************************************************************/
	static DirectX::XMFLOAT2 GetCursorPosition();
	static DirectX::XMFLOAT2 GetClientCursorPosition();


	/************************************************************
	* @brief 前回のカーソル座標を取得.
	* @note GetPastCursorPosition		：ディスプレイ基準.
	* @note GetPastClientCursorPosition	：ウィンドウ基準.
	************************************************************/
	static DirectX::XMFLOAT2 GetPastCursorPosition();
	static DirectX::XMFLOAT2 GetPastClientCursorPosition();

	/************************************************************
	* @brief カーソルデルタタイム取得.
	* @note GetPastClientCursorPosition	：ウィンドウ基準.
	************************************************************/
	static DirectX::XMFLOAT2 GetClientCursorDelta();


	/************************************************************
	* @brief マウスホイールの操作方向を取得・設定.
	* @note GetWheelDirection	：取得.
	* @note SetWheelDirection	：設定.
	************************************************************/
	static int&	GetWheelDirection();
	static void SetWheelDirection(const int& direction);


	/************************************************************
	* @brief カーソルをウィンドウの中心に固定するか判定・設定.
	* @note IsCenterMouseCursor		：判定.
	* @note SetCenterMouseCursor	：設定.
	************************************************************/
	static bool& IsCenterMouseCursor();
	static void SetCenterMouseCursor(const bool& isCenter);


	/************************************************************
	* @brief マウスの掴み状態を判定・設定.
	* @note IsMouseGrab	：判定.
	* @note SetMouseGrab：設定.
	************************************************************/	
	static bool& IsMouseGrab();
	static void SetMouseGrab(const bool& isGrab);


	/************************************************************
	* @brief カーソルの表示を設定.	
	************************************************************/
	static void SetShowCursor(const bool& isShowCursor);

private:
	HWND	m_hWnd;
	POINT	m_NowMousePoint;		// 現在のマウス座標.
	POINT	m_PastMousePoint;		// 過去のマウス座標.
	POINT	m_NowClientMousePoint;	// 現在のマウス座標（クライアント座標）.
	POINT	m_PastClientMousePoint;	// 過去のマウス座標（クライアント座標）.
	DirectX::XMFLOAT2 m_ClientCursorDelta;	// マウスの移動間隔.
	int		m_WheelDirection;		// マウスホイールを動かした方向.
	bool	m_IsGrab;				// マウスの掴み状態.
	bool	m_IsCenterMouseCursor;	// マウスカーソルを固定するか.
	bool	m_IsShowCursor;			// カーソルを表示するか.
};
