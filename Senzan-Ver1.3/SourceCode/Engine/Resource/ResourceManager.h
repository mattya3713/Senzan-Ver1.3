#pragma once

#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include "Engine/Resource/ResourceLoader.h"
#include "Engine/Resource/ResourceCache.h"

#include <unordered_map>
#include <string>
#include <memory>
#include "../../../Data/Library/Effekseer/include/Effekseer.h"

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : リソース管理の統括(Presenter層).
**********************************************************/

class ResourceManager final : public Singleton<ResourceManager> {
	friend class Singleton<ResourceManager>;

private:
	ResourceManager() = default;
	~ResourceManager() = default;

public:
	// 初期化・終了処理.
	// @param ThreadCount ロードスレッドの起動数（デフォルト値は4）.
	void Initialize(int ThreadCount = 4);
	void Shutdown();

	// 毎フレーム実行：ロード完了キューを監視し、キャッシュに登録する.
	void Update();

	// リソースロード要求.
	void RequestLoadEffect(const std::string& Name, const std::string& Path);
	void RequestLoadTexture(const std::string& Key, const std::string& Path);
	void RequestLoadFBX(const std::string& Name, const std::string& Path);
	void RequestLoadSprite(const std::string& Key, const std::string& Path);

	// ロード完了確認.
	// @param Key リソースキー.
	// @return ロード完了なら true、未完了なら false.
	bool IsResourceReady(const std::string& Key) const;

	// リソース取得（現段階では void* で返す）.
	void* GetResource(const std::string& Key);

	// 型別リソース取得.
	// @param Name リソース名.
	// @return Effekseer::EffectRef、見つからなければ nullptr.
	::Effekseer::EffectRef GetEffect(const std::string& Name);

	// TODO: 後続フェーズで型チェック・安全な取得を実装.
	// ID3D12Resource* GetTexture(const std::string& Key);
	// FBXModel* GetFBXModel(const std::string& Name);

private:
	std::unique_ptr<ResourceLoader> m_upLoader;
	std::unordered_map<std::string, void*> m_ResourceCache;           // キャッシュ済みリソース.
	std::unordered_map<std::string, eResourceType> m_ResourceTypes;   // 各リソースの型情報.
	std::unordered_map<std::string, bool> m_LoadingStatus;            // ローディング状態追跡用.
};
