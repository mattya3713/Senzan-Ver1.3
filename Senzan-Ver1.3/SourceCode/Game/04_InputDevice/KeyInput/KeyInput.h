#pragma once
#include "System/Singleton/SingletonTemplate.h"


/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : キー入力判定クラス.
**********************************************************************************/
class KeyInput final
	: public Singleton<KeyInput>
{
private:
	friend class Singleton<KeyInput>;
	KeyInput();
public:
	~KeyInput();

	/**********************************************************
	* @brief 更新.
	**********************************************************/
	static void Update();

public:// 押下しているか.

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

public: // 押下した瞬間.

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

public: // 離した瞬間.

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

public:// 押下し続けているか.

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

private:
	static constexpr int KEY_MAX = 256; // キーの最大値.
private:
	BYTE m_NowKeyState[KEY_MAX];// 現在の入力状態.
	BYTE m_OldKeyState[KEY_MAX];// 1F前の入力状態.
};
