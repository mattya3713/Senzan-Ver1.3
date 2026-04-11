#pragma once

#include "../00_Base/SceneBase.h"

#include <vector>


/*********************************************
*	ゲームメインクラス.
**/

class GameMain
	: public SceneBase
{
public:
	GameMain();
	~GameMain() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();

private:
	void UIUpdate();

	// コンテニュー回数（シーン間で保持）.
	static int s_ContinueCount;

private:
		
	float m_TimeLimit;
};
