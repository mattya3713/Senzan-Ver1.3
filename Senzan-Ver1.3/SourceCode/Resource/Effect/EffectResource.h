#pragma once
#include "System/10_Singleton/SingletonTemplate.h" 
#include "../../../Data/Library/Effekseer/include/Effekseer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

// ライブラリのリンク設定は .vcxproj 側で管理する.
// （環境差分で LNK1104 が発生しやすいため、pragma 指定は使用しない）.

class EffectResource final : public Singleton<EffectResource>
{
public:
    EffectResource();
    ~EffectResource();

    // 読み込み関数（ResourceManagerから呼ばれる想定）.
    bool Create();   // 初期化が必要な場合.
    bool LoadData(); // 全データ読み込み.

    // エフェクトの取得.
    static ::Effekseer::EffectRef GetResource(const std::string& name);

private:
    // エフェクトデータのマップ（名前, エフェクト本体）.
    std::unordered_map<std::string, ::Effekseer::EffectRef> m_pEffects;
};
