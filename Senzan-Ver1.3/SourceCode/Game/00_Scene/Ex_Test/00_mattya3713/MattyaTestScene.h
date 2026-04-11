#pragma once

#include "../../00_Base/SceneBase.h"
/*********************************************
*	ゲームメイン画像クラス.
**/

class MattyaTestScene
	: public SceneBase
{
public:
	MattyaTestScene();
	~MattyaTestScene() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();

private:

};
