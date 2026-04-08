#include "KeyInput.h"

KeyInput::KeyInput()
	: m_NowKeyState()
	, m_OldKeyState()
{
}


KeyInput::~KeyInput()
{

}


void KeyInput::Update()
{
	KeyInput& pI = GetInstance();

	//更新前の現在の状態をコピー.
	memcpy_s(pI.m_OldKeyState, sizeof(m_OldKeyState), pI.m_NowKeyState, sizeof(pI.m_NowKeyState));

	// 入力されているキーを知らべる.
	if (GetKeyboardState(pI.m_NowKeyState) == false) { return; };
}


bool KeyInput::IsKeyPress(const int& key)
{
	KeyInput& pI = GetInstance();
	if ((pI.m_NowKeyState[key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyPress(const std::vector<int>& keyList)
{
	for (const auto& Key : keyList)
	{
		if (IsKeyPress(Key) == false)
		{ 
			return false; 
		}
	}
	return true;
}


bool KeyInput::IsKeyDown(const int& key)
{
	KeyInput& pI = GetInstance();

	// 現在入力で前回未入力なら押した瞬間.
	if ((pI.m_NowKeyState[key] & 0x80) != 0 &&
		(pI.m_OldKeyState[key] & 0x80) == 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyDown(const std::vector<int>& keyList)
{
	if (IsKeyDown(keyList.back()) == false) { return false; }
	for (const auto& Key : keyList)
	{
		if (IsKeyPress(Key) == false)
		{
			return false;
		}
	}
	return true;
}


bool KeyInput::IsKeyUp(const int& key)
{
	KeyInput& pI = GetInstance();

	// 現在未入力で前回入力なら離した瞬間.
	if ((pI.m_NowKeyState[key] & 0x80) == 0 &&
		(pI.m_OldKeyState[key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyRepeat(const int& key)
{
	KeyInput& pI = GetInstance();

	// 現在入力で前回入力なら押し続けている.
	if ((pI.m_NowKeyState[key] & 0x80) != 0 &&
		(pI.m_OldKeyState[key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyRepeat(const std::vector<int>& keyList)
{
	for (const auto& Key : keyList)
	{
		if (IsKeyRepeat(Key) == false)
		{
			return false;
		}
	}
	return true;
}
