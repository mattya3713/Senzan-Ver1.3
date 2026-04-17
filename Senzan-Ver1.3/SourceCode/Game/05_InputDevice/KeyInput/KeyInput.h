#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include <vector>


/**********************************************************************************
* @author    : mattya3713.
* @date      : 2026/04/17.
* @pattern   : Singleton.
* @brief     : キー入力管理クラス.
**********************************************************************************/
class KeyInput final
	: public Singleton<KeyInput>
{
private:
	friend class Singleton<KeyInput>;
	KeyInput();
public:
	~KeyInput();

	// 更新.
	static void Update();

public:
	// キーが押されている間 true.
	static bool IsKeyPress(int Key);


	// 指定した全キーが押されている間 true.
    static bool IsKeyPress(const std::vector<int>& KeyList);

	// キーが押された瞬間 true.
	static bool IsKeyDown(int Key);


	// 指定した全キーが押された瞬間 true.
    static bool IsKeyDown(const std::vector<int>& KeyList);

	// キーが離された瞬間 true.
	static bool IsKeyUp(int Key);

	// キーがリピート中なら true.
	static bool IsKeyRepeat(int Key);


	// 指定した全キーがリピート中なら true.
    static bool IsKeyRepeat(const std::vector<int>& KeyList);

private:
   // キーコードの有効範囲を判定.
	static bool IsValidKey(int Key);

	static constexpr int KEY_MAX = 256; // キーの最大数.
private:
    BYTE m_NowKeyState[KEY_MAX]; // 現在の入力状態.
	BYTE m_OldKeyState[KEY_MAX]; // 1フレーム前の入力状態.
};

