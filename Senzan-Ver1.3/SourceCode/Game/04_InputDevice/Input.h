#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "KeyInput\KeyInput.h"
#include "Mouse\Mouse.h"
#include "XInput\XInput.h"

/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : 入力機のラッパークラス.
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


	/*********************************************
	* @brief ウィンドウハンドルを設定.
	*********************************************/
	static void SethWnd(HWND hWnd);

public: // キーボード.

	/************************************************************
	* @brief 入力判定.
	*
	* @param[in] key：判定するキー.
	*
	* @return キーを押下しているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyPress('A')){
	*		Aキーを押下しているときの処理.
	*	}
	*	if(IsKeyPress(VK_RETURN)){
	*		Enterキーを押下しているときの処理.
	*	}
	************************************************************/
	static bool IsKeyPress(const int& key);


	/************************************************************
	* @brief 入力判定（複数）.
	*
	* @param[in] keyList：判定するキー.
	*
	* @return 'keyList'内の全てのキーを押下しているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyPress({VK_CONTROL,'C'})){
	*		ctrlキーとCキーを押下しているときの処理.
	*	}
	************************************************************/
	static bool IsKeyPress(const std::vector<int>& keyList);


	/************************************************************
	* @brief 押下した瞬間の判定.
	*
	* @param[in] key：判定するキー.
	*
	* @return キーを押下した瞬間ならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyDown('A')){
	*		Aキーを押下した瞬間の処理.
	*	}
	*	if(IsKeyDown(VK_RETURN)){
	*		Enterキーを押下した瞬間の処理.
	*	}
	************************************************************/
	static bool IsKeyDown(const int& key);


	/************************************************************
	* @brief 押下した瞬間の判定（複数）.
	*
	* @param[in] keyList：判定するキー.
	*
	* @return 'keyList'の最後のキーを押下した瞬間ならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyDown({VK_CONTROL,'C'})){
	*		ctrlキーが入力状態でCキーを押下した瞬間の処理.
	*	}
	************************************************************/
	static bool IsKeyDown(const std::vector<int>& keyList);


	/************************************************************
	* @brief 離した瞬間の判定.
	*
	* @param[in] key：判定するキー.
	*
	* @return キーを離した瞬間ならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyUp('A')){
	*		Aキーを離した瞬間の処理.
	*	}
	*	if(IsKeyUp(VK_RETURN)){
	*		Enterキーを押下した瞬間の処理.
	*	}
	************************************************************/
	static bool IsKeyUp(const int& key);


	/************************************************************
	* @brief 押下し続けているか判定.
	*
	* @param[in] key：判定するキー.
	*
	* @return キーを押下し続けているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyRepeat('A')){
	*		Aキーを押下し続けているときの処理.
	*	}
	*	if(IsKeyRepeat(VK_RETURN)){
	*		Enterキーを押下し続けているときの処理.
	*	}
	************************************************************/
	static bool IsKeyRepeat(const int& key);


	/************************************************************
	* @brief 押下し続けているか判定（複数）.
	*
	* @param[in] keyList：判定するキー.
	*
	* @return 'keyList'の最後のキーを押下し続けているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsKeyRepeat({VK_CONTROL,'C'})){
	*		ctrlキーが入力状態でCキーを押下し続けているときの処理.
	*	}
	************************************************************/
	static bool IsKeyRepeat(const std::vector<int>& keyList);


public: // マウス.

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
	static const bool IsCursorInWindow();


	/************************************************************
	* @brief マウスカーソルが指定領域にあるか.
	* @param[in] position	：判定座標.
	* @param[in] size		：判定の幅高さ.
	************************************************************/
	static const bool IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size);
	

	/************************************************************
	* @brief 現在のカーソル座標を取得.
	* @note GetCursorPosition		：ディスプレイ基準.
	* @note GetClientCursorPosition	：ウィンドウ基準.
	************************************************************/
	static const DirectX::XMFLOAT2 GetCursorPosition();
	static const DirectX::XMFLOAT2 GetClientCursorPosition();


	/************************************************************
	* @brief 前回のカーソル座標を取得.
	* @note GetPastCursorPosition		：ディスプレイ基準.
	* @note GetPastClientCursorPosition	：ウィンドウ基準.
	************************************************************/
	static const DirectX::XMFLOAT2 GetPastCursorPosition();
	static const DirectX::XMFLOAT2 GetPastClientCursorPosition();

	/************************************************************
	* @brief 前フレームから今フレームへの移動間隔を取得.
	* @note GetPastCursorPosition		：ディスプレイ基準.
	************************************************************/
	static const DirectX::XMFLOAT2 GetClientCursorDelta();


	/************************************************************
	* @brief マウスホイールの操作方向を取得・設定.
	* @note GetWheelDirection	：取得.
	* @note SetWheelDirection	：設定.
	************************************************************/
	static const int GetWheelDirection();	
	static void SetWheelDirection(const int direction);


	/************************************************************
	* @brief カーソルをウィンドウの中心に固定するか判定・設定.
	* @note IsCenterMouseCursor		：判定.
	* @note SetCenterMouseCursor	：設定.
	************************************************************/
	static const bool IsCenterMouseCursor();	
	static void SetCenterMouseCursor(const bool isCenter);


	/************************************************************
	* @brief マウスの掴み状態を判定・設定.
	* @note IsMouseGrab	：判定.
	* @note SetMouseGrab：設定.
	************************************************************/
	static const bool IsMouseGrab();
	static void SetMouseGrab(const bool isGrab);


	/************************************************************
	* @brief カーソルの表示を設定.	
	************************************************************/
	static void SetShowCursor(const bool& isShowCursor);

public:// コントローラー.

	/************************************************************
	* @brief ボタンを押下した瞬間か判定.
	*
	* @param[in] key：判定するキー.
	* @param[in] id	：コントローラー番号（0～3）.
	*
	* @return ボタンを押下した瞬間ならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsButtonDown(CXInput::Key::A)){
	*		Aボタンを押下した瞬間の処理.
	*	}
	************************************************************/
	static const bool IsButtonDown(const XInput::Key key, const int id = 0);


	/************************************************************
	* @brief ボタンを離した瞬間か判定.
	*
	* @param[in] key：判定するキー.
	* @param[in] id	：コントローラー番号（0～3）.
	* 
	* @return ボタンを離した瞬間ならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsButtonUp(CXInput::Key::A)){
	*		Aボタンを離した瞬間の処理.
	*	}
	************************************************************/
	static const bool IsButtonUp(const XInput::Key key, const int id = 0);


	/************************************************************
	* @brief ボタンを押下し続けているか判定.
	*
	* @param[in] key：判定するキー.
	* @param[in] id	：コントローラー番号（0～3）.
	*
	* @return ボタンを押下し続けているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsButtonRepeat(CXInput::Key::A)){
	*		Aボタンを押下し続けているときの処理.
	*	}
	************************************************************/
	static const bool IsButtonRepeat(const XInput::Key key, const int id = 0);


	/************************************************************
	* @brief 指定方向にスティックが入力された瞬間か判定.
	*
	* @param[in] dir			：判定方向.
	* @param[in] isFirstPress	：初回入力かに切り替えるフラグ.
	* @param[in] id			：コントローラー番号（0～3）.
	*
	* @return 入力された瞬間ならtrue、それ以外はfalse.
	*
	* @note IsLStickDirectionDown：左スティック.
	* @note IsRStickDirectionDown：右スティック.
	*
	* @details
	*  使用例：
	*	if(IsLStickDirectionDown(CXInput::StickState::Up)){
	*		スティックが上方向に入力された瞬間の処理.
	*	}
	************************************************************/
	static const bool IsLStickDirectionDown(const XInput::StickState dir, const bool isFirstPress = false, const int id = 0);
	static const bool IsRStickDirectionDown(const XInput::StickState dir, const bool isFirstPress = false, const int id = 0);


	/************************************************************
	* @brief 指定方向にスティックが未入力になった瞬間か判定.
	*
	* @param[in] id	：コントローラー番号（0～3）.
	* 
	* @note IsLStickDirectionUp：左スティック.
	* @note IsRStickDirectionUp：右スティック.
	*
	* @details
	*  使用例：
	*	if(IsLStickDirectionUp()){
	*		スティックが未入力になった瞬間の処理.
	*	}
	************************************************************/
	static const bool IsLStickDirectionUp(const int id = 0);
	static const bool IsRStickDirectionUp(const int id = 0);


	/************************************************************
	* @brief 指定方向にスティックが入力し続けているか判定.
	*
	* @param[in] dir：判定方向.
	* @param[in] id	：コントローラー番号（0～3）.
	*
	* @note IsLStickDirectionRepeat：左スティック.
	* @note IsRStickDirectionRepeat：右スティック.
	*
	* @details
	*  使用例：
	*	if(IsLStickDirectionRepeat(CXInput::StickState::Up)){
	*		スティックが上方向に入力された瞬間の処理.
	*	}
	************************************************************/
	static const bool IsLStickDirectionRepeat(const XInput::StickState dir, const int id = 0);
	static const bool IsRStickDirectionRepeat(const XInput::StickState dir, const int id = 0);


	/*******************************************************
	* @brief スティック入力があるか判定.
	*
	* @param[in] deadZone	：最低傾き範囲.
	* @param[in] id			：コントローラー番号（0～3）.
	*
	* @return 入力があるならtrue、それ以外はfalse.
	*
	* @note IsLStickActive：左スティックの入力判定.
	* @note IsRStickActive：右スティックの入力判定.
	*******************************************************/
	static const bool IsLStickActive(const float deadZone = 0.0f, const int id = 0);
	static const bool IsRStickActive(const float deadZone = 0.0f, const int id = 0);


	/*******************************************************
	* @brief コントローラーを取得.
	* @param[in] id	：コントローラー番号（0～3）.
	*******************************************************/
	static const std::shared_ptr<XInput> GetController(const int id = 0);

	/*******************************************************
	* @brief スティックの入力方向を取得.
	* @param[in] id	：コントローラー番号（0～3）.
	* @note：GetLStickDirectionで右スティックの入力方向を取得.
	* @note：GetRStickDirectionで右スティックの入力方向を取得.
	*******************************************************/
	static const DirectX::XMFLOAT2 GetLStickDirection(const int id = 0);
	static const DirectX::XMFLOAT2 GetRStickDirection(const int id = 0);
	
	/*******************************************************
	* @brief スティックの入力方向を取得.
	* @param[in] id	：コントローラー番号（0～3）.
	* @note：GetLTriggerで右スティックの入力方向を取得.
	* @note：GetRTriggerで右スティックの入力方向を取得.
	*******************************************************/
	static const float GetLTriggerRaw(const int id = 0);
	static const float GetRTriggerRaw(const int id = 0);
	static const float GetLTrigger(const int id = 0);
	static const float GetRTrigger(const int id = 0);

private:

	/*********************************************
	* @brief 全てのコントローラーの更新.
	*********************************************/
	void UpdateForAllController();


	/*********************************************
	* @brief コントローラーの作成.
	*********************************************/
	void CreateController();

public:// 定数.

	// コントローラーの最大数.
	static constexpr int CONTROLLER_MAX = 4;
private:
	HWND m_hWnd;
	std::array<std::shared_ptr<XInput>, CONTROLLER_MAX> m_pControllers;
};
