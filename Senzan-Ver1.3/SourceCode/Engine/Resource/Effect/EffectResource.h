#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h" 
#include "../../../Data/Library/Effekseer/include/Effekseer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

// 繝ｩ繧､繝悶Λ繝ｪ縺ｮ繝ｪ繝ｳ繧ｯ險ｭ螳壹・ .vcxproj 蛛ｴ縺ｧ邂｡逅・☆繧・
// ・育腸蠅・ｷｮ蛻・〒 LNK1104 縺檎匱逕溘＠繧・☆縺・◆繧√｝ragma 謖・ｮ壹・菴ｿ逕ｨ縺励↑縺・ｼ・

class EffectResource final : public Singleton<EffectResource>
{
public:
    EffectResource();
    ~EffectResource();

    // 隱ｭ縺ｿ霎ｼ縺ｿ髢｢謨ｰ・・esourceManager縺九ｉ蜻ｼ縺ｰ繧後ｋ諠ｳ螳夲ｼ・
    bool Create();   // 蛻晄悄蛹悶′蠢・ｦ√↑蝣ｴ蜷・
    bool LoadData(); // 蜈ｨ繝・・繧ｿ隱ｭ縺ｿ霎ｼ縺ｿ.

    // 繧ｨ繝輔ぉ繧ｯ繝医・蜿門ｾ・
    static ::Effekseer::EffectRef GetResource(const std::string& name);

private:
    // 繧ｨ繝輔ぉ繧ｯ繝医ョ繝ｼ繧ｿ縺ｮ繝槭ャ繝暦ｼ亥錐蜑・ 繧ｨ繝輔ぉ繧ｯ繝域悽菴難ｼ・
    std::unordered_map<std::string, ::Effekseer::EffectRef> m_pEffects;
};

