#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h" 
#include "../../../Data/Library/Effekseer/include/Effekseer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

// ライブラリのリンク設定は .vcxproj ファイルで管理する.
// 将来選択で LNK1104 が発生しないため pragma 指定の使用はしない.

class ResourceManager;  // Forward declaration.

class EffectResource final : public Singleton<EffectResource>
{
public:
	EffectResource();
	~EffectResource();

	bool Create();

	bool LoadData();

	static ::Effekseer::EffectRef GetResource(const std::string& name);

private:
	// エフェクトデータのマッピング.
	// 名前:エフェクト参照.
	std::unordered_map<std::string, ::Effekseer::EffectRef> m_pEffects;
};
