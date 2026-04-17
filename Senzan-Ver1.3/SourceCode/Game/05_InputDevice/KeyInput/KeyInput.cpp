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
   KeyInput& p_i = GetInstance();

    // 更新前の現在の状態をコピー.
	memcpy_s(p_i.m_OldKeyState, sizeof(m_OldKeyState), p_i.m_NowKeyState, sizeof(p_i.m_NowKeyState));

  // 入力されているキーを調べる.
	if (GetKeyboardState(p_i.m_NowKeyState) == false) { return; }
}


bool KeyInput::IsValidKey(int Key)
{
	return 0 <= Key && Key < KEY_MAX;
}


bool KeyInput::IsKeyPress(int Key)
{
   if (IsValidKey(Key) == false)
	{
		return false;
	}

	KeyInput& p_i = GetInstance();
	if ((p_i.m_NowKeyState[Key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyPress(const std::vector<int>& KeyList)
{
 if (KeyList.empty())
	{
		return false;
	}

	for (const int Key : KeyList)
	{
		if (IsKeyPress(Key) == false)
      {
			return false;
		}
	}
	return true;
}


bool KeyInput::IsKeyDown(int Key)
{
   if (IsValidKey(Key) == false)
	{
		return false;
	}

	KeyInput& p_i = GetInstance();

	// 現在入力で前回未入力なら押した瞬間.
  if ((p_i.m_NowKeyState[Key] & 0x80) != 0 &&
		(p_i.m_OldKeyState[Key] & 0x80) == 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyDown(const std::vector<int>& KeyList)
{
   if (KeyList.empty())
	{
		return false;
	}

	if (IsKeyDown(KeyList.back()) == false)
	{
		return false;
	}

	for (const int Key : KeyList)
	{
		if (IsKeyPress(Key) == false)
		{
			return false;
		}
	}
	return true;
}


bool KeyInput::IsKeyUp(int Key)
{
   if (IsValidKey(Key) == false)
	{
		return false;
	}

	KeyInput& p_i = GetInstance();

	// 現在未入力で前回入力なら離した瞬間.
  if ((p_i.m_NowKeyState[Key] & 0x80) == 0 &&
		(p_i.m_OldKeyState[Key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyRepeat(int Key)
{
   if (IsValidKey(Key) == false)
	{
		return false;
	}

	KeyInput& p_i = GetInstance();

	// 現在入力で前回入力なら押し続けている.
  if ((p_i.m_NowKeyState[Key] & 0x80) != 0 &&
		(p_i.m_OldKeyState[Key] & 0x80) != 0)
	{
		return true;
	}
	return false;
}


bool KeyInput::IsKeyRepeat(const std::vector<int>& KeyList)
{
 if (KeyList.empty())
	{
		return false;
	}

	for (const int Key : KeyList)
	{
		if (IsKeyRepeat(Key) == false)
		{
			return false;
		}
	}
	return true;
}
