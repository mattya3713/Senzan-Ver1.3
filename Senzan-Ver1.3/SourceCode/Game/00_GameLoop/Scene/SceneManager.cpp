#include "SceneManager.h"
#include "Game/01_Scene/00_Base/SceneBase.h"
#include "Engine/Graphic/Fade/FadeManager.h"
#include "Game/00_GameLoop/Time/Time.h"

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
    // 譛蛻昴↓繝ｭ繝ｼ繝峨☆繧九す繝ｼ繝ｳ繧堤腸蠅・↓蠢懊§縺ｦ豎ｺ螳・
    eList initial_scene = GetSceneByIndex(0);
    if (initial_scene == eList::MAX)
    {
        return;
    }

	// --- 迺ｰ蠅・＃縺ｨ縺ｮ蛻晄悄繧ｷ繝ｼ繝ｳ險ｭ螳・---.

#if _DEBUG
#ifdef MATTYA_PC
  TryGetSceneIdByName("MattyaTestScene", initial_scene);
#elif defined(MEMEU_PC)
   TryGetSceneIdByName("MemeuTestScene", initial_scene);
#elif defined(L_PC)
   TryGetSceneIdByName("LTestScene", initial_scene);
#elif _DEBUG
	// 荳願ｨ倥・迺ｰ蠅・・繧ｯ繝ｭ縺悟ｮ夂ｾｩ縺輔ｌ縺ｦ縺翫ｉ縺壹√ョ繝舌ャ繧ｰ繝薙Ν繝峨・蝣ｴ蜷・
	// 騾壼ｸｸ縺ｯ髢狗匱荳ｭ縺ｮ繝｡繧､繝ｳ繧ｷ繝ｼ繝ｳ縺九ｉ髢句ｧ・
 //TryGetSceneIdByName("GameMain", initial_scene);

#else
	// 繝ｪ繝ｪ繝ｼ繧ｹ繝薙Ν繝峨∪縺溘・荳肴・縺ｪ迺ｰ蠅・・蝣ｴ蜷医√ち繧､繝医Ν縺九ｉ髢句ｧ・
   TryGetSceneIdByName("Title", initial_scene);

#endif 
#endif // _DEBUG.

    // 繝輔ぉ繝ｼ繝峨う繝ｳ髢句ｧ・
    FadeManager::GetInstance().StartFade(Fade::FadeType::FadeIn);

    // 蛻晏屓繝ｭ繝ｼ繝牙・逅・
    if (m_StartFade) {
        MakeScene(initial_scene); // 逶ｴ謗･菴懈・.
        if (m_pScene) {
            m_pScene->Create();
        }
        m_StartFade = false;      // 谺｡蝗槭°繧峨・繝輔ぉ繝ｼ繝峨ｒ譛牙柑縺ｫ縺吶ｋ.
    }
    else {
        LoadScene(initial_scene);
    }
}

void SceneManager::Update()
{
    SceneManager& pI = GetInstance();
    FadeManager& fade = FadeManager::GetInstance();

    // 繧ｷ繝ｼ繝ｳ蛻・ｊ譖ｿ縺井ｸｭ縺ｮ蜃ｦ逅・
    if (pI.m_IsSceneChanging)
    {
        // 逕ｻ髱｢縺梧囓縺上↑縺｣縺溘ｉ繧ｷ繝ｼ繝ｳ繧貞ｷｮ縺玲崛縺医ｋ.
        if (fade.IsFadeCompleted(Fade::FadeType::FadeOut))
        {
            pI.m_pScene.reset();
            pI.MakeScene(pI.m_NextSceneID);

            if (pI.m_pScene) {
                pI.m_pScene->Create();
            }

            // 譁ｰ縺励＞繧ｷ繝ｼ繝ｳ縺ｮ貅門ｙ縺後〒縺阪◆繧峨ヵ繧ｧ繝ｼ繝峨う繝ｳ髢句ｧ・
            fade.StartFade(Fade::FadeType::FadeIn);
        }

        // 逕ｻ髱｢縺梧・繧九￥縺ｪ縺｣縺溘ｉ驕ｷ遘ｻ螳御ｺ・
        if (fade.IsFadeCompleted(Fade::FadeType::FadeIn))
        {
            pI.m_IsSceneChanging = false;
        }
    }

    // 繝輔ぉ繝ｼ繝芽・菴薙・譖ｴ譁ｰ.
    fade.Update();

    // 繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ.
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

//繧ｷ繝ｼ繝ｳ菴懈・.
void SceneManager::MakeScene(eList Scene)
{
#if _DEBUG
	Time::GetInstance().SetWorldTimeScale(1.0f);
	m_CurrentSceneID = Scene;
#endif
  m_pScene = CreateSceneById(Scene);
}

