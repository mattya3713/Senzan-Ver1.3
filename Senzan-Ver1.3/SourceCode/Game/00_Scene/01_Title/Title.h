#pragma once
#include "../00_Base/SceneBase.h"

class UITitle;

/*********************************************
*	タイトルクラス.
**/

class Title
	: public SceneBase
{
public:
	Title();
	~Title() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;
	
private: 	
	std::shared_ptr<UITitle> m_pUI;
};
