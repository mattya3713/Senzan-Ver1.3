#include "Input.h"

Input::Input()
	: m_hWnd		()
	, m_pControllers()
{
	// コントローラーの作成.
	CreateController();
}


Input::~Input()
{
}


void Input::Update()
{
	// キーボードの更新.
	KeyInput::Update();

	// マウスの更新.
	Mouse::Update();

	// コントローラーの更新.
	GetInstance().UpdateForAllController();
}


void Input::SethWnd(HWND hWnd)
{
	GetInstance().m_hWnd = hWnd;
	Mouse::SethWnd(hWnd);
}


bool Input::IsKeyPress(const int& key)
{
	return KeyInput::IsKeyPress(key);
}


bool Input::IsKeyPress(const std::vector<int>& keyList)
{
	return KeyInput::IsKeyPress(keyList);
}


bool Input::IsKeyDown(const int& key)
{
	return KeyInput::IsKeyDown(key);
}


bool Input::IsKeyDown(const std::vector<int>& keyList)
{
	return KeyInput::IsKeyDown(keyList);
}


bool Input::IsKeyUp(const int& key)
{
	return KeyInput::IsKeyUp(key);
}


bool Input::IsKeyRepeat(const int& key)
{
	return KeyInput::IsKeyRepeat(key);
}


bool Input::IsKeyRepeat(const std::vector<int>& keyList)
{
	return KeyInput::IsKeyRepeat(keyList);
}


void Input::CenterMouseCursor()
{
	Mouse::CenterMouseCursor();
}


void Input::WrapCursorInScreen()
{
	Mouse::WrapCursorInScreen();
}


const bool Input::IsCursorInWindow()
{
	return Mouse::IsCursorInWindow();
}


const bool Input::IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size)
{
	return Mouse::IsCursorInRegion(position, size);
}


const DirectX::XMFLOAT2 Input::GetCursorPosition()
{
	return Mouse::GetCursorPosition();
}


const DirectX::XMFLOAT2 Input::GetClientCursorPosition()
{
	return Mouse::GetClientCursorPosition();
}


const DirectX::XMFLOAT2 Input::GetPastCursorPosition()
{
	return Mouse::GetPastCursorPosition();
}


const DirectX::XMFLOAT2 Input::GetPastClientCursorPosition()
{
	return Mouse::GetPastClientCursorPosition();
}

const DirectX::XMFLOAT2 Input::GetClientCursorDelta()
{
	return Mouse::GetClientCursorDelta();
}


const int Input::GetWheelDirection()
{
	return Mouse::GetWheelDirection();
}


void Input::SetWheelDirection(const int direction)
{
	Mouse::SetWheelDirection(direction);
}


const bool Input::IsCenterMouseCursor()
{
	return Mouse::IsCenterMouseCursor();
}


void Input::SetCenterMouseCursor(const bool isCenter)
{
	Mouse::SetCenterMouseCursor(isCenter);
}


const bool Input::IsMouseGrab()
{
	return Mouse::IsMouseGrab();
}


void Input::SetMouseGrab(const bool isGrab)
{
	Mouse::SetMouseGrab(isGrab);
}


void Input::SetShowCursor(const bool& isShowCursor)
{
	Mouse::SetShowCursor(isShowCursor);
}


const bool Input::IsButtonDown(const XInput::Key key, const int id)
{	
	return GetInstance().m_pControllers[id]->IsDown(key);
}


const bool Input::IsButtonUp(const XInput::Key key, const int id)
{
	return GetInstance().m_pControllers[id]->IsUp(key);
}


const bool Input::IsButtonRepeat(const XInput::Key key, const int id)
{
	return GetInstance().m_pControllers[id]->IsRepeat(key);
}


const bool Input::IsLStickDirectionDown(const XInput::StickState dir, const bool isFirstPress, const int id)
{
	return GetInstance().m_pControllers[id]->IsLStickDirectionDown(dir, isFirstPress);
}


const bool Input::IsRStickDirectionDown(const XInput::StickState dir, const bool isFirstPress, const int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionDown(dir, isFirstPress);
}


const bool Input::IsLStickDirectionUp(const int id)
{	
	return GetInstance().m_pControllers[id]->IsLStickDirectionUp();
}


const bool Input::IsRStickDirectionUp(const int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionUp();
}


const bool Input::IsLStickDirectionRepeat(const XInput::StickState dir, const int id)
{
	return GetInstance().m_pControllers[id]->IsLStickDirectionRepeat(dir);
}


const bool Input::IsRStickDirectionRepeat(const XInput::StickState dir, const int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionRepeat(dir);
}


const bool Input::IsLStickActive(const float deadZone, const int id)
{
	return GetInstance().m_pControllers[id]->IsLStickActive(deadZone);
}


const bool Input::IsRStickActive(const float deadZone, const int id)
{	
	return GetInstance().m_pControllers[id]->IsRStickActive(deadZone);
}


const std::shared_ptr<XInput> Input::GetController(const int id)
{
	if (id >= CONTROLLER_MAX) 
	{
		assert(0 && "参照出来ないコントローラーの番号です");
	}

	return GetInstance().m_pControllers[id];
}


const DirectX::XMFLOAT2 Input::GetLStickDirection(const int id)
{
	return GetInstance().m_pControllers[id]->GetLStickDirection();
}


const DirectX::XMFLOAT2 Input::GetRStickDirection(const int id)
{
	return GetInstance().m_pControllers[id]->GetRStickDirection();
}

const float Input::GetLTriggerRaw(const int id)
{
	return GetInstance().m_pControllers[id]->GetLTriggerRaw();
}

const float Input::GetRTriggerRaw(const int id)
{
	return GetInstance().m_pControllers[id]->GetRightTrigger();
}

const float Input::GetLTrigger(const int id)
{
	return GetInstance().m_pControllers[id]->GetLeftTrigger();
}

const float Input::GetRTrigger(const int id)
{
	return GetInstance().m_pControllers[id]->GetRightTrigger();
}


void Input::UpdateForAllController()
{
	for (const auto& controller : m_pControllers)
	{
		controller->Update();
	}
}


void Input::CreateController()
{
	for (int i = 0; i < CONTROLLER_MAX; i++)
	{
		DWORD padID = static_cast<DWORD>(i);
		m_pControllers[i] = std::make_shared<XInput>(padID);
	}
}
