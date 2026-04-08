#include "SceneManager.h"

#include "Graphic/DirectX/DirectX9/DirectX9.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "02_UIObject/Fade/FadeManager.h"

#include "Game/00_Scene/00_Base/SceneBase.h"
#include "Game/00_Scene/01_Title/Title.h"
#include "Game/00_Scene/02_GameMain/GameMain.h"

#include "Game/00_Scene/Ex_Test/00_mattya3713/MattyaTestScene.h"
#include "Game/00_Scene/Ex_Test/01_memeu1101/MemeuTestScene.h"
#include "Game/00_Scene/Ex_Test/02_L/LTestScene.h"
#include "Game/00_Scene/Ex_Test/03_UIEditor/UIEditor.h"
#include "Game/00_Scene/Ex_Test/04_AnimationTuning/AnimationTuningScene.h"
#include "System/Singleton/ResourceManager/EffectManager/EffekseerManager.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"

#if _DEBUG
#include "ImGui/CImGuiManager.h"
#endif // _DEBUG.


SceneManager::SceneManager()
	: m_pScene	( std::make_unique<Title>() )
	, m_pBuffer	( nullptr )
    ,m_NextSceneID(eList::MAX)
    ,m_IsSceneChanging(false)
    , m_StartFade   (true)

#if _DEBUG
	, m_DebugFirstScene()
#endif // _DEBUG.
{

}

SceneManager::~SceneManager()
{
}

void SceneManager::LoadData()
{
	// 最初にロードするシーンを環境に応じて決定.
	eList initial_scene = eList::Title;

	// --- 環境ごとの初期シーン設定 ---.

#if _DEBUG
#ifdef MATTYA_PC
	initial_scene = eList::Mattya;
#elif defined(MEMEU_PC)
	initial_scene = eList::Memeu;
#elif defined(L_PC)
	initial_scene = eList::L;
#elif _DEBUG
	// 上記の環境マクロが定義されておらず、デバッグビルドの場合.
	// 通常は開発中のメインシーンから開始.
	//initial_scene = eList::GameMain;.

#else
	// リリースビルドまたは不明な環境の場合、タイトルから開始.
	initial_scene = eList::Title;

#endif 
#endif // _DEBUG.

    // フェードイン開始.
    FadeManager::GetInstance().StartFade(Fade::FadeType::FadeIn);

    // 初回ロード処理.
    if (m_StartFade) {
        MakeScene(initial_scene); // 直接作成.
        if (m_pScene) {
            m_pScene->Create();
        }
        m_StartFade = false;      // 次回からはフェードを有効にする.
    }
    else {
        LoadScene(initial_scene);
    }
}

void SceneManager::Update()
{
    SceneManager& pI = GetInstance();
    FadeManager& fade = FadeManager::GetInstance();

    // シーン切り替え中の処理.
    if (pI.m_IsSceneChanging)
    {
        // 画面が暗くなったらシーンを差し替える.
        if (fade.IsFadeCompleted(Fade::FadeType::FadeOut))
        {
            pI.m_pScene.reset();
            pI.MakeScene(pI.m_NextSceneID);

            if (pI.m_pScene) {
                pI.m_pScene->Create();
            }

            // 新しいシーンの準備ができたらフェードイン開始.
            fade.StartFade(Fade::FadeType::FadeIn);
        }

        // 画面が明るくなったら遷移完了.
        if (fade.IsFadeCompleted(Fade::FadeType::FadeIn))
        {
            pI.m_IsSceneChanging = false;
        }
    }

    // フェード自体の更新.
    fade.Update();

    // シーンの更新.
    if (pI.m_pScene) {
        pI.m_pScene->Update();

        CollisionDetector::GetInstance().ExecuteCollisionDetection();

        pI.m_pScene->LateUpdate();
    }
	
#if _DEBUG
	ImGui::Begin("Scene");
	ImGui::Text(pI.GetSceneName(pI.m_CurrentSceneID));

	if (ImGui::Button("Title")) { LoadScene(eList::Title); }
	if (ImGui::Button("GameMain")) { LoadScene(eList::GameMain); }

	ImGui::Separator();
	if (ImGui::Button("Mattya")) { LoadScene(eList::Mattya); }
	if (ImGui::Button("Memeu")) { LoadScene(eList::Memeu); }
	if (ImGui::Button("L")) { LoadScene(eList::L); }
	if (ImGui::Button("AnimationTuning")) { LoadScene(eList::AnimationTuning); }

	ImGui::Separator();
	if (ImGui::Button("UIEditor")) { LoadScene(eList::UIEditor); }

	ImGui::End();
#endif // _DEBUG.
}

void SceneManager::Draw()
{
	SceneManager& pI = GetInstance();
	pI.m_pScene->Draw();
    FadeManager::GetInstance().Draw();
}

bool SceneManager::IsCurrentSceneMattya() const
{
#if _DEBUG
    return (m_CurrentSceneID == eList::Mattya);
#else
    return false;
#endif
}

void SceneManager::LoadScene(eList Scene, bool useFade /*= true*/)
{
    SceneManager& pI = GetInstance();
    if (!useFade)
    {
        if (pI.m_pScene) {
            pI.m_pScene.reset();
        }
        pI.MakeScene(Scene);
        if (pI.m_pScene) pI.m_pScene->Create();
        return;
    }

    if (pI.m_IsSceneChanging) return;

    pI.m_NextSceneID = Scene;
    pI.m_IsSceneChanging = true;

    FadeManager::GetInstance().StartFade(Fade::FadeType::FadeOut);
}

//シーン作成.
void SceneManager::MakeScene(eList Scene)
{
#if _DEBUG
	Time::GetInstance().SetWorldTimeScale(1.0f);
	m_CurrentSceneID = Scene;
#endif
	switch (Scene)
	{
		case eList::Title:
			m_pScene = std::make_unique<Title>();
			break;
		case eList::GameMain:
			m_pScene = std::make_unique<GameMain>();
			break;
			
#if _DEBUG
			// テスト用シーン.
		case eList::Mattya:
			m_pScene = std::make_unique<MattyaTestScene>();
			break;
		case eList::Memeu:
			m_pScene = std::make_unique<MemeuTestScene>();
			break;
		case eList::L:
			m_pScene = std::make_unique<LTestScene>();
			break;
		case eList::UIEditor:
			m_pScene = std::make_unique<UIEditor>();
			break;
		case eList::AnimationTuning:
			m_pScene = std::make_unique<AnimationTuningScene>();
			break;
#endif // _DEBUG.
		case eList::MAX:
		default:
			break;
	}
}
