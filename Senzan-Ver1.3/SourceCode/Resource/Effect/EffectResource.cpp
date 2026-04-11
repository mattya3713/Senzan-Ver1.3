#include "EffectResource.h"
//#include "System\10_Singleton\ResourceManager\EffectManager\EffekseerManager.h"
#include <filesystem>

namespace {
static constexpr char EFFECT_FILE_PATH[] = "Data\\Effect";
}

EffectResource::EffectResource() : m_pEffects() {}
EffectResource::~EffectResource() { m_pEffects.clear(); }

bool EffectResource::Create()
{
    // 必要に応じてEffekseer側の初期化確認などを行う.
    return true;
}

bool EffectResource::LoadData()
{
    //EffectResource& pI = GetInstance();

    //try {
    //    // 指定したディレクトリ内を再帰的に検索.
    //    for (const auto& entry : std::filesystem::recursive_directory_iterator(EFFECT_FILE_PATH)) {
    //        const std::string extension = entry.path().extension().string();

    //        // Effekseerの拡張子以外は無視.
    //        if (extension != ".efkefc" && extension != ".efk") continue;

    //        const std::string fileName = entry.path().stem().string();
    //        const std::string filePath = entry.path().string();

    //        // エフェクトの生成とロード.
    //        auto effect = ::Effekseer::Effect::Create(
    //            EffekseerManager::GetInstance().GetManager(),
    //            reinterpret_cast<const char16_t*>(entry.path().u16string().c_str())
    //        );

    //        if (effect != nullptr) {
    //            pI.m_pEffects[fileName] = effect;
    //        }
    //        else {
    //            _ASSERT_EXPR(false, L"エフェクトのロードに失敗しました");
    //        }
    //    }
    //}
    //catch (const std::exception& e) {
    //    _ASSERT_EXPR(false, e.what());
    //    return false;
    //}

    return true;
}

::Effekseer::EffectRef EffectResource::GetResource(const std::string& name)
{
  /*  auto& pI = GetInstance();
    if (pI.m_pEffects.count(name) > 0) {
        return pI.m_pEffects[name];
    }*/
    return nullptr;
}
