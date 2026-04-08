#include "Mouse.h"
#include "Game/05_InputDevice/KeyInput/KeyInput.h"

Mouse::Mouse()
    : m_hWnd				()
	, m_NowMousePoint       ()
    , m_PastMousePoint      ()
	, m_NowClientMousePoint	()
	, m_PastClientMousePoint()
	, m_ClientCursorDelta()
	, m_WheelDirection		( 0 )
    , m_IsGrab              ( false )
    , m_IsCenterMouseCursor ( false )
	, m_IsShowCursor		( true )
{					
}


Mouse::~Mouse()
{
}


void Mouse::Update()
{
	Mouse& pI = GetInstance();

	// 過去の座標を保存.
	pI.m_PastMousePoint		= pI.m_NowMousePoint;
	pI.m_PastClientMousePoint	= pI.m_NowClientMousePoint;

	// マウス座標を取得.
	GetCursorPos(&pI.m_NowMousePoint);

	// マウス座標をクライアント座標に変換.
	pI.m_NowClientMousePoint = pI.m_NowMousePoint;
	ScreenToClient(pI.m_hWnd, &pI.m_NowClientMousePoint);

	// 移動距離調べる.
	pI.m_ClientCursorDelta.x =
		static_cast<float>(pI.m_NowClientMousePoint.x - pI.m_PastClientMousePoint.x);

	pI.m_ClientCursorDelta.y =
		static_cast<float>(pI.m_NowClientMousePoint.y - pI.m_PastClientMousePoint.y);

}


void Mouse::CenterMouseCursor()
{
    if (GetInstance().m_IsCenterMouseCursor == false) { return; }

	// クライアント領域の位置とサイズを取得.
	RECT rect;
	GetClientRect(GetInstance().m_hWnd, &rect);

	// ウィンドウの中心を計算.
	int centerX = (rect.right - rect.left) / 2;
	int centerY = (rect.bottom - rect.top) / 2;

	// クライアント座標をスクリーン座標に変換.
	POINT centerPoint = { centerX, centerY };
	ClientToScreen(GetInstance().m_hWnd, &centerPoint);

	// マウス座標をウィンドウの中心に固定.
	SetCursorPos(centerPoint.x, centerPoint.y);

	// 中心の位置をNowClientMousePointとして更新.
	GetInstance().m_NowClientMousePoint.x = centerX;
	GetInstance().m_NowClientMousePoint.y = centerY;
}


void Mouse::WrapCursorInScreen()
{
    Mouse& pI = GetInstance();

    // 画面の解像度を取得.
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int newCursorPosX = pI.m_NowMousePoint.x;
    int newCursorPosY = pI.m_NowMousePoint.y;

	if (newCursorPosX <= 0) {
		newCursorPosX = screenWidth - 1;
	}
	else if (newCursorPosX >= screenWidth - 1) {
		newCursorPosX = 0;
	}

	if (newCursorPosY <= 0) {
		newCursorPosY = screenHeight - 1;
	}
	else if (newCursorPosY >= screenHeight - 1) {
		newCursorPosY = 0;
	}

	SetCursorPos(newCursorPosX, newCursorPosY);
}


bool Mouse::IsCursorInWindow()
{
	Mouse& pI = GetInstance();

	// クライアント領域の位置とサイズを取得.
	RECT rect;
	GetClientRect(pI.m_hWnd, &rect);

	bool isRightOfLeft = pI.m_NowMousePoint.x >= rect.left;	// x座標が左端より右にあるか.
	bool isLeftOfRight = pI.m_NowMousePoint.x <= rect.right;	// x座標が右端より左にあるか.

	bool isBottomOfTop = pI.m_NowMousePoint.y >= rect.top;		// y座標が上端より下にあるか.
	bool isTopOfBottom = pI.m_NowMousePoint.y <= rect.bottom;	// y座標が下端より上にあるか.


	// X軸とY軸の境界を判定.
	bool isWithinX = isRightOfLeft && isLeftOfRight;
	bool isWithinY = isBottomOfTop && isTopOfBottom;

	// クライアント領域内か判定.
	return isWithinX && isWithinY;
}


bool Mouse::IsCursorInRegion(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& size)
{
	Mouse& pI = GetInstance();

	return pI.m_NowClientMousePoint.x >= position.x && pI.m_NowClientMousePoint.x <= position.x + size.x &&
		pI.m_NowClientMousePoint.y >= position.y && pI.m_NowClientMousePoint.y <= position.y + size.y;
}


void Mouse::SethWnd(HWND hWnd)
{
	GetInstance().m_hWnd = hWnd;
}


DirectX::XMFLOAT2 Mouse::GetCursorPosition()
{
    Mouse& pI = GetInstance();
    float x = static_cast<float>(pI.m_NowMousePoint.x);
    float y = static_cast<float>(pI.m_NowMousePoint.y);

    return DirectX::XMFLOAT2(x, y);
}


DirectX::XMFLOAT2 Mouse::GetClientCursorPosition()
{
	Mouse& pI = GetInstance();
	float x = static_cast<float>(pI.m_NowClientMousePoint.x);
	float y = static_cast<float>(pI.m_NowClientMousePoint.y);

	return DirectX::XMFLOAT2(x, y);
}


DirectX::XMFLOAT2 Mouse::GetPastCursorPosition()
{
    Mouse& pI = GetInstance();
    float x = static_cast<float>(pI.m_PastMousePoint.x);
    float y = static_cast<float>(pI.m_PastMousePoint.y);

    return DirectX::XMFLOAT2(x, y);
}


DirectX::XMFLOAT2 Mouse::GetPastClientCursorPosition()
{
	Mouse& pI = GetInstance();
	float x = static_cast<float>(pI.m_PastClientMousePoint.x);
	float y = static_cast<float>(pI.m_PastClientMousePoint.y);

	return DirectX::XMFLOAT2(x, y);
}

DirectX::XMFLOAT2 Mouse::GetClientCursorDelta()
{
	Mouse& pI = GetInstance();
	float x = static_cast<float>(pI.m_ClientCursorDelta.x);
	float y = static_cast<float>(pI.m_ClientCursorDelta.y);

	return DirectX::XMFLOAT2(x, y);
}


int& Mouse::GetWheelDirection()
{
	return GetInstance().m_WheelDirection;
}


void Mouse::SetWheelDirection(const int& direction)
{
	GetInstance().m_WheelDirection = direction;
}


bool& Mouse::IsCenterMouseCursor()
{
	return GetInstance().m_IsCenterMouseCursor;
}


void Mouse::SetCenterMouseCursor(const bool& isCenter)
{
	GetInstance().m_IsCenterMouseCursor = isCenter;
}


bool& Mouse::IsMouseGrab()
{
    return GetInstance().m_IsGrab;
}


void Mouse::SetMouseGrab(const bool& isGrab)
{
    GetInstance().m_IsGrab = isGrab;
}


void Mouse::SetShowCursor(const bool& isShowCursor)
{
	Mouse& pI = GetInstance();
	
	if (pI.m_IsShowCursor == isShowCursor) { return; }
	pI.m_IsShowCursor = isShowCursor;
	ShowCursor(isShowCursor);
}
