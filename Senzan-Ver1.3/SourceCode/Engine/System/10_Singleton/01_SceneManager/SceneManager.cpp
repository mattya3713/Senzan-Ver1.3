#include "SceneManager.h"
#include "../../../Game/00_Scene/00_Base/SceneBase.h"
#include "System/10_Singleton/00_FadeManager/FadeManager.h"
#include "System/00_GameLoop/Time/Time.h"

SceneManager::SceneManager()
   : m_pScene	( nullptr )
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
    eList initial_scene = GetSceneByIndex(0);
    if (initial_scene == eList::MAX)
    {
        return;
    }

	// --- 環境ごとの初期シーン設定 ---.

#if _DEBUG
#ifdef MATTYA_PC
  TryGetSceneIdByName("MattyaTestScene", initial_scene);
#elif defined(MEMEU_PC)
   TryGetSceneIdByName("MemeuTestScene", initial_scene);
#elif defined(L_PC)
   TryGetSceneIdByName("LTestScene", initial_scene);
#elif _DEBUG
	// 上記の環境マクロが定義されておらず、デバッグビルドの場合.
	// 通常は開発中のメインシーンから開始.
 //TryGetSceneIdByName("GameMain", initial_scene);

#else
	// リリースビルドまたは不明な環境の場合、タイトルから開始.
   TryGetSceneIdByName("Title", initial_scene);

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

//        CollisionDetector::GetInstance().ExecuteCollisionDetection();

        pI.m_pScene->LateUpdate();
    }
	
//#if _DEBUG
//	ImGui::Begin("Scene");
//  ImGui::Text(GetSceneName(pI.m_CurrentSceneID));
//
//    const std::size_t scene_count = GetSceneCount();
//    for (std::size_t index = 0; index < scene_count; ++index)
//    {
//        const eList scene_id = GetSceneByIndex(index);
//        const char* scene_name = GetSceneName(scene_id);
//        if (ImGui::Button(scene_name))
//        {
//            LoadScene(scene_id);
//        }
//    }
//
//	ImGui::End();
//#endif // _DEBUG.
}

void SceneManager::Draw()
{
	SceneManager& pI = GetInstance();
    if (pI.m_pScene)
    {
        pI.m_pScene->Draw();
    }
    FadeManager::GetInstance().Draw();
}

bool SceneManager::IsCurrentSceneMattya() const
{
#if _DEBUG
 eList mattya_scene = eList::MAX;
    if (TryGetSceneIdByName("MattyaTestScene", mattya_scene))
    {
        return (m_CurrentSceneID == mattya_scene);
    }

    return false;
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
  m_pScene = CreateSceneById(Scene);
}
