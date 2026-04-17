#include "Title.h"
//#include "02_UIObject/UIObject.h"
//#include "02_UIObject/UITitle/UITitle.h"
//#include "Game/05_InputDevice/Input.h"
//#include "Singleton/ResourceManager/ResourceManager.h"
//#include "SceneManager/SceneManager.h"

//#include "Singleton/ImGui/CImGuiManager.h"

Title::Title()
	: SceneBase		(  )
//	, m_pUI			(/*std::make_shared<UITitle>()*/)
{
}

Title::~Title()
{
	//SoundManager::GetInstance().AllStop();
}

void Title::Initialize()
{
}

void Title::Create()
{
	//SoundManager::GetInstance().Play("Title", true);
	//SoundManager::GetInstance().SetVolume("Title", 8000);
}

void Title::Update()
{
	/*Input::Update();

	m_pUI->Update();

	if (Input::IsKeyDown(VK_SPACE)
	||	Input::IsKeyDown('C')
	||	Input::IsButtonDown(XInput::Key::B))
	{
        SoundManager::GetInstance().Play("Decide");
        SoundManager::GetInstance().SetVolume("Decide",8000);
		if (m_pUI->GetSelected() == m_pUI->Items::Start) {
			SceneManager::LoadScene(eList::GameMain);
			return;
		}
		else if (m_pUI->GetSelected() == m_pUI->Items::End) {
			DestroyWindow(ResourceManager::GethWnd());
		}
	}*/
}

void Title::LateUpdate()
{
	//m_pUI->LateUpdate();
}

void Title::Draw()
{
	//m_pUI->Draw();
}
