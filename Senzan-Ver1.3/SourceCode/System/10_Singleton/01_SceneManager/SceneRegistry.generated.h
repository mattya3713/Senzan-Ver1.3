#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

class SceneBase;

enum class eList
{
    Title,
    GameMain,
    MattyaTestScene,
    MAX,
};

std::unique_ptr<SceneBase> CreateSceneById(eList SceneId);
const char* GetSceneName(eList SceneId);
std::size_t GetSceneCount();
eList GetSceneByIndex(std::size_t Index);
bool TryGetSceneIdByName(std::string_view SceneName, eList& OutSceneId);
