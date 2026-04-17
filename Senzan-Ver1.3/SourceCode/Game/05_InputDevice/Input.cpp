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


bool Input::IsKeyPress(int key)
{
	return KeyInput::IsKeyPress(key);
}


bool Input::IsKeyPress(const std::vector<int>& keyList)
{
	return KeyInput::IsKeyPress(keyList);
}


bool Input::IsKeyDown(int key)
{
	return KeyInput::IsKeyDown(key);
}


bool Input::IsKeyDown(const std::vector<int>& keyList)
{
	return KeyInput::IsKeyDown(keyList);
}


bool Input::IsKeyUp(int key)
{
	return KeyInput::IsKeyUp(key);
}


bool Input::IsKeyRepeat(int key)
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


bool Input::IsCursorInWindow()
{
	return Mouse::IsCursorInWindow();
}


bool Input::IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size)
{
	return Mouse::IsCursorInRegion(position, size);
}


DirectX::XMFLOAT2 Input::GetCursorPosition()
{
	return Mouse::GetCursorPosition();
}


DirectX::XMFLOAT2 Input::GetClientCursorPosition()
{
	return Mouse::GetClientCursorPosition();
}


DirectX::XMFLOAT2 Input::GetPastCursorPosition()
{
	return Mouse::GetPastCursorPosition();
}


DirectX::XMFLOAT2 Input::GetPastClientCursorPosition()
{
	return Mouse::GetPastClientCursorPosition();
}

DirectX::XMFLOAT2 Input::GetClientCursorDelta()
{
	return Mouse::GetClientCursorDelta();
}


int Input::GetWheelDirection()
{
	return Mouse::GetWheelDirection();
}


void Input::SetWheelDirection(int direction)
{
	Mouse::SetWheelDirection(direction);
}


bool Input::IsCenterMouseCursor()
{
	return Mouse::IsCenterMouseCursor();
}


void Input::SetCenterMouseCursor(bool isCenter)
{
	Mouse::SetCenterMouseCursor(isCenter);
}


bool Input::IsMouseGrab()
{
	return Mouse::IsMouseGrab();
}


void Input::SetMouseGrab(bool isGrab)
{
	Mouse::SetMouseGrab(isGrab);
}


void Input::SetShowCursor(bool isShowCursor)
{
	Mouse::SetShowCursor(isShowCursor);
}


bool Input::IsButtonDown(XInput::Key key, int id)
{	
	return GetInstance().m_pControllers[id]->IsDown(key);
}


bool Input::IsButtonUp(XInput::Key key, int id)
{
	return GetInstance().m_pControllers[id]->IsUp(key);
}


bool Input::IsButtonRepeat(XInput::Key key, int id)
{
	return GetInstance().m_pControllers[id]->IsRepeat(key);
}


bool Input::IsLStickDirectionDown(XInput::StickState dir, bool isFirstPress, int id)
{
	return GetInstance().m_pControllers[id]->IsLStickDirectionDown(dir, isFirstPress);
}


bool Input::IsRStickDirectionDown(XInput::StickState dir, bool isFirstPress, int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionDown(dir, isFirstPress);
}


bool Input::IsLStickDirectionUp(int id)
{	
	return GetInstance().m_pControllers[id]->IsLStickDirectionUp();
}


bool Input::IsRStickDirectionUp(int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionUp();
}


bool Input::IsLStickDirectionRepeat(XInput::StickState dir, int id)
{
	return GetInstance().m_pControllers[id]->IsLStickDirectionRepeat(dir);
}


bool Input::IsRStickDirectionRepeat(XInput::StickState dir, int id)
{
	return GetInstance().m_pControllers[id]->IsRStickDirectionRepeat(dir);
}


bool Input::IsLStickActive(float deadZone, int id)
{
	return GetInstance().m_pControllers[id]->IsLStickActive(deadZone);
}


bool Input::IsRStickActive(float deadZone, int id)
{	
	return GetInstance().m_pControllers[id]->IsRStickActive(deadZone);
}


std::shared_ptr<XInput> Input::GetController(int id)
{
	if (id >= CONTROLLER_MAX) 
	{
		assert(0 && "参照出来ないコントローラーの番号です");
	}

	return GetInstance().m_pControllers[id];
}


DirectX::XMFLOAT2 Input::GetLStickDirection(int id)
{
	return GetInstance().m_pControllers[id]->GetLStickDirection();
}


DirectX::XMFLOAT2 Input::GetRStickDirection(int id)
{
	return GetInstance().m_pControllers[id]->GetRStickDirection();
}

float Input::GetLTriggerRaw(int id)
{
	return GetInstance().m_pControllers[id]->GetLTriggerRaw();
}

float Input::GetRTriggerRaw(int id)
{
	return GetInstance().m_pControllers[id]->GetRightTrigger();
}

float Input::GetLTrigger(int id)
{
	return GetInstance().m_pControllers[id]->GetLeftTrigger();
}

float Input::GetRTrigger(int id)
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
