#include "SceneRegistry.generated.h"

#include "Game/00_Scene/00_Base/SceneBase.h"
#include "Game/00_Scene/01_Title/Title.h"
#include "Game/00_Scene/02_GameMain/GameMain.h"
#include "Game/00_Scene/Ex_Test/00_mattya3713/MattyaTestScene.h"

namespace
{
    constexpr eList SCENE_LIST[] =
    {
        eList::Title,
        eList::GameMain,
        eList::MattyaTestScene,
    };
}

std::unique_ptr<SceneBase> CreateSceneById(eList SceneId)
{
    switch (SceneId)
    {
    case eList::Title:
        return std::make_unique<Title>();
    case eList::GameMain:
        return std::make_unique<GameMain>();
    case eList::MattyaTestScene:
        return std::make_unique<MattyaTestScene>();
    case eList::MAX:
    default:
        return nullptr;
    }
}

const char* GetSceneName(eList SceneId)
{
    switch (SceneId)
    {
    case eList::Title:
        return "Title";
    case eList::GameMain:
        return "GameMain";
    case eList::MattyaTestScene:
        return "MattyaTestScene";
    case eList::MAX:
    default:
        return "Unknown";
    }
}

std::size_t GetSceneCount()
{
    return sizeof(SCENE_LIST) / sizeof(SCENE_LIST[0]);
}

eList GetSceneByIndex(std::size_t Index)
{
    const std::size_t scene_count = GetSceneCount();
    if (Index >= scene_count)
    {
        return eList::MAX;
    }

    return SCENE_LIST[Index];
}

bool TryGetSceneIdByName(std::string_view SceneName, eList& OutSceneId)
{
    const std::size_t scene_count = GetSceneCount();
    for (std::size_t index = 0; index < scene_count; ++index)
    {
        const eList scene_id = GetSceneByIndex(index);
        if (SceneName == GetSceneName(scene_id))
        {
            OutSceneId = scene_id;
            return true;
        }
    }

    OutSceneId = eList::MAX;
    return false;
}
