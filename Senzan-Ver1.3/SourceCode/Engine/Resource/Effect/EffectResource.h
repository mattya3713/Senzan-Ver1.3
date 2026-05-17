#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h" 
#include "../../../Data/Library/Effekseer/include/Effekseer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

// ライブラリのリンク設定は .vcxproj ファイルで管理する.
// 将来選択で LNK1104 が発生しないため pragma 指定の使用はしない.

class EffectResource final : public Singleton<EffectResource>
{
public:
	EffectResource();
	~EffectResource();

	// 読み込み関連関数.ResourceManager から呼ばれる想定.
	bool Create();   // 初期化が必要な場合.
	bool LoadData(); // 全データ読み込み.

	// エフェクトの取得.
	static ::Effekseer::EffectRef GetResource(const std::string& name);

private:
	// エフェクトデータのマッピング.
	// 名前:エフェクト参照.
	std::unordered_map<std::string, ::Effekseer::EffectRef> m_pEffects;
};
