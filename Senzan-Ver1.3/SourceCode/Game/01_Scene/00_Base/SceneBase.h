#pragma once

/**********************************************************
*	基底クラス.
**/

class SceneBase	
{
public:
	SceneBase();
	virtual ~SceneBase() {}

	virtual void Initialize() = 0;
	virtual void Create() = 0;
	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Draw() = 0;


protected:
	//ウィンドウハンドル.
	HWND				m_hWnd;

};
