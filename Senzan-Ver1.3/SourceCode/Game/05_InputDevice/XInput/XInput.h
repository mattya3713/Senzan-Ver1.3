#pragma once
#include <Xinput.h>

#pragma comment(lib,"xinput.lib")

class XInput final
{
public:
	
	enum Key
	{
		None = -1,
		Up,		//方向パッド:上.
		Down,	//方向パッド:下.
		Left,	//方向パッド:左.
		Right,	//方向パッド:右.
		Start,	//ボタン:スタート.
		Back,	//ボタン:バック.
		LStick,	//ボタン:左スティック.
		RStick,	//ボタン:右スティック.
		LB,		//ボタン:LB.
		RB,		//ボタン:RB.
		A,		//ボタン:A.
		B,		//ボタン:B.
		X,		//ボタン:X.
		Y,		//ボタン:Y.

		Max,
		First = Up,
		Last = Y,
	};

	enum class StickState
	{		
		None = -1,	// 未入力.
		Up = 0,		// 上.
		Down,		// 下.
		Left,		// 右.
		Right,		// 左.

		Max,		
	};
public:
	XInput(DWORD padId);
	~XInput();

	bool Update();
	void EndProc();

	/************************************************************
	* @brief ボタンを押下した瞬間か判定.
	* 
	* @param[in] key：判定するキー.
	* 
	* @return ボタンを押下した瞬間ならtrue、それ以外はfalse.
	* 
	* @details
	*  使用例：
	*	if(IsDown(CXInput::Key::A)){
	*		Aボタンを押下した瞬間の処理.
	*	}	
	************************************************************/
	bool IsDown(const Key key);


	/************************************************************
	* @brief ボタンを離した瞬間か判定.
	* 
	* @param[in] key：判定するキー.
	* 
	* @return ボタンを離した瞬間ならtrue、それ以外はfalse.
	* 
	* @details
	*  使用例：
	*	if(IsUp(CXInput::Key::A)){
	*		Aボタンを離した瞬間の処理.
	*	}	
	************************************************************/
	bool IsUp(const Key key);


	/************************************************************
	* @brief ボタンを押下し続けているか判定.
	* 
	* @param[in] key：判定するキー.
	*
	* @return ボタンを押下し続けているならtrue、それ以外はfalse.
	*
	* @details
	*  使用例：
	*	if(IsRepeat(CXInput::Key::A)){
	*		Aボタンを押下し続けているときの処理.
	*	}	
	************************************************************/
	bool IsRepeat(const Key key);


	/************************************************************
	* @brief 指定方向にスティックが入力された瞬間か判定.
	* 
	* @param[in] dir			：判定方向.
	* @param[in] isFirstPress	：初回入力かに切り替えるフラグ.
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
	bool IsLStickDirectionDown(StickState dir, const bool isFirstPress = false);
	bool IsRStickDirectionDown(StickState dir, const bool isFirstPress = false);


	/************************************************************
	* @brief 指定方向にスティックが未入力になった瞬間か判定.
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
	bool IsLStickDirectionUp();
	bool IsRStickDirectionUp();


	/************************************************************
	* @brief 指定方向にスティックが入力し続けているか判定.
	* 
	* @param[in] dir：判定方向.
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
	bool IsLStickDirectionRepeat(StickState dir);
	bool IsRStickDirectionRepeat(StickState dir);


	/*******************************************************
	* @brief スティック入力があるか判定.
	* 
	* @param[in] deadZone：最低傾き範囲.
	* 
	* @return 入力があるならtrue、それ以外はfalse.
	* 
	* @note IsLStickActiveで左スティックの入力判定.
	* @note IsRStickActiveで右スティックの入力判定.
	*******************************************************/
	bool IsLStickActive(const float deadZone = 0.0f);
	bool IsRStickActive(const float deadZone = 0.0f);

	
public: // Getter、Setter.

	/*******************************************************
	* @brief トリガーの生の値 (0～255) を取得.
	*******************************************************/
	const BYTE GetLTriggerRaw() const;
	const BYTE GetRTriggerRaw() const;

	/*******************************************************
	* @brief トリガーの正規化された値 (0.0f～1.0f) を取得.
	*******************************************************/
	const float GetLeftTrigger() const; 
	const float GetRightTrigger() const; 


	/*******************************************************
	* @brief スティックの入力方向を取得.
	* @note：GetLStickDirectionで右スティックの入力方向を取得.
	* @note：GetRStickDirectionで右スティックの入力方向を取得.
	*******************************************************/
	const DirectX::XMFLOAT2 GetLStickDirection() const;
	const DirectX::XMFLOAT2 GetRStickDirection() const;


	/*******************************************************
	* @brief 左スティックの入力値を取得.
	* @note：GetLThumbで左スティックの入力値を取得.
	* @note：GetLThumbXで左スティックのX軸の入力値を取得.
	* @note：GetLThumbYで左スティックのY軸の入力値を取得.
	*******************************************************/
	const DirectX::XMFLOAT2 GetLThumb() const;
	const float GetLThumbX() const;
	const float GetLThumbY() const;

	/*******************************************************
	* @brief 左スティックの入力値を取得.
	* @note：GetLThumbで左スティックの入力値を取得.
	* @note：GetLThumbXで左スティックのX軸の入力値を取得.
	* @note：GetLThumbYで左スティックのY軸の入力値を取得.
	*******************************************************/
	const DirectX::XMFLOAT2 GetLThumb_Clamp() const;
	const float GetLThumbX_Clamp() const;
	const float GetLThumbY_Clamp() const;


	/*******************************************************
	* @brief 右スティックの入力値を取得.
	* @note：GetRThumbで右スティックの入力値を取得.
	* @note：GetRThumbXで右スティックのX軸の入力値を取得.
	* @note：GetRThumbYで右スティックのY軸の入力値を取得.
	*******************************************************/
	const DirectX::XMFLOAT2 GetRThumb() const;
	const float GetRThumbX() const;
	const float GetRThumbY() const;

	/*******************************************************
	* @brief 右スティックの入力値を取得.
	* @note：GetRThumbで右スティックの-1~1入力値を取得.
	* @note：GetRThumbXで右スティックのX軸の-1~1入力値を取得.
	* @note：GetRThumbYで右スティックのY軸の-1~1入力値を取得.
	*******************************************************/
	const DirectX::XMFLOAT2 GetRThumb_Clamp() const;
	const float GetRThumbX_Clamp() const;
	const float GetRThumbY_Clamp() const;


	/*******************************************************
	* @brief バッド番号を取得.	
	*******************************************************/
	const DWORD GetPadID() const;


	/*******************************************************
	* @brief 接続状態を取得.
	*******************************************************/
	const bool IsConnect() const;


	/*******************************************************
	* @brief 振動を設定.
	* @param[in] leftMotorSpd	：左モータの速度.
	* @param[in] rightMotorSpd	：右モータの速度.
	* @return 設定できたならtrue、それ以外はfalse.
	*******************************************************/
	const bool SetVibration(const WORD& leftMotorSpd, const WORD& rightMotorSpd);

private:

	/*******************************************************
	* @brief 入力状態の更新.
	* @return 更新できたらtrue、それ以外はfalse.
	*******************************************************/
	bool UpdateStatus();


	/*******************************************************
	* @brief 入力判定.
	* @param[in] gamePad：パッド番号.
	* @param[in] state	：判定ボタン.
	* @return 入力状態ならtrue、それ以外はfalse.
	*******************************************************/
	bool IsKeyCore(WORD gamePad, const XINPUT_STATE& state);


	/*******************************************************
	* @brief ボタン入力をWORD型に変換.	
	* @param[in] key	：変換するボタン.
	* @return ボタンを変換したWORD型の値.
	*******************************************************/
	WORD GenerateGamePadValue(const Key key);


	/*******************************************************
	* @brief スティック入力がデッドゾーンを超えているか判定.	
	* @param[in] deadZone	：デッドゾーンの値.
	* @param[in] stickSlope	：スティック入力の値.
	* @return デッドゾーンを超えているならtrue、それ以外はfalse.
	*******************************************************/
	bool IsOutsideDeadZone(const float& deadZone, const DirectX::XMFLOAT2& stickSlope);


	/*******************************************************
	* @brief スティック入力がデッドゾーンを超えているか判定.	
	* @param[in] useStickDir：スティック入力の方向.
	* @param[in] state		：判定する方向.
	* @return 指定方向に入力があるならtrue、それ以外はfalse.
	*******************************************************/
	bool IsStickInput(const DirectX::XMFLOAT2& useStickDir,StickState& state);	

public: // 定数.

	//トリガー入力範囲.
	static constexpr BYTE TRIGGER_MIN = 0;
	static constexpr BYTE TRIGGER_MAX = 255;

	//スティック入力範囲.
	static constexpr SHORT THUMB_MIN = -32768;
	static constexpr SHORT THUMB_MAX = 32767;

	// 振動範囲.
	static constexpr WORD VIBRATION_MIN = 0;
	static constexpr WORD VIBRATION_MAX = 65535;

private:
	DWORD				m_PadID;			// パッド番号（0～3）.
	XINPUT_STATE        m_State;			// キー入力情報.
	XINPUT_STATE		m_PastState;		// キー入力情報（キーストローク判定用）.
	XINPUT_VIBRATION    m_Vibration;		// 振動.
	StickState			m_LStickState;		// 左スティック入力情報.
	StickState			m_RStickState;		// 右スティック入力情報.
	StickState			m_PastLStickState;	// 前回の左スティック入力情報.
	StickState			m_PastRStickState;	// 前回の右スティック入力情報.
	int					m_VibrationTime;	// 振動処理の時間計測用.
	bool				m_IsConnect;		// 接続判定.
	bool				m_IsVibration;		// 振動があったか.
};
