#include "Engine/Resource/ResourceManager.h"

void ResourceManager::Initialize(int ThreadCount) {
	m_upLoader = std::make_unique<ResourceLoader>();
	m_upLoader->Initialize(ThreadCount);
}

void ResourceManager::Shutdown() {
	if (m_upLoader) {
		m_upLoader->Shutdown();
		m_upLoader.reset();
	}

	// キャッシュをクリア.
	m_ResourceCache.clear();
	m_ResourceTypes.clear();
	m_LoadingStatus.clear();
}

void ResourceManager::Update() {
	if (!m_upLoader) {
		return;
	}

	// ロード完了キューを監視し、完了したリソースをキャッシュに登録.
	LoadedAsset loaded_asset;
	while (m_upLoader->PopLoadedAsset(loaded_asset)) {
		if (loaded_asset.m_IsSuccess) {
			m_ResourceCache[loaded_asset.ResourceKey] = loaded_asset.m_pResource;
			m_LoadingStatus[loaded_asset.ResourceKey] = true;
		} else {
			// ロード失敗時もステータスを更新（後続フェーズでリトライ処理を実装予定）.
			m_LoadingStatus[loaded_asset.ResourceKey] = false;
		}
	}
}

void ResourceManager::RequestLoadEffect(const std::string& Name, const std::string& Path) {
	if (!m_upLoader) {
		return;
	}

	LoadRequest request = {};
	request.ResourceKey = Name;
	request.FilePath = Path;
	request.Type = eResourceType::Effect;

	m_upLoader->PushLoadRequest(request);
	m_LoadingStatus[Name] = false;  // ローディング中フラグ.
	m_ResourceTypes[Name] = eResourceType::Effect;
}

void ResourceManager::RequestLoadTexture(const std::string& Key, const std::string& Path) {
	if (!m_upLoader) {
		return;
	}

	LoadRequest request = {};
	request.ResourceKey = Key;
	request.FilePath = Path;
	request.Type = eResourceType::Texture;

	m_upLoader->PushLoadRequest(request);
	m_LoadingStatus[Key] = false;
	m_ResourceTypes[Key] = eResourceType::Texture;
}

void ResourceManager::RequestLoadFBX(const std::string& Name, const std::string& Path) {
	if (!m_upLoader) {
		return;
	}

	LoadRequest request = {};
	request.ResourceKey = Name;
	request.FilePath = Path;
	request.Type = eResourceType::FBX;

	m_upLoader->PushLoadRequest(request);
	m_LoadingStatus[Name] = false;
	m_ResourceTypes[Name] = eResourceType::FBX;
}

void ResourceManager::RequestLoadSprite(const std::string& Key, const std::string& Path) {
	if (!m_upLoader) {
		return;
	}

	LoadRequest request = {};
	request.ResourceKey = Key;
	request.FilePath = Path;
	request.Type = eResourceType::Sprite;

	m_upLoader->PushLoadRequest(request);
	m_LoadingStatus[Key] = false;
	m_ResourceTypes[Key] = eResourceType::Sprite;
}

// ロード完了確認.
bool ResourceManager::IsResourceReady(const std::string& Key) const {
	auto it = m_LoadingStatus.find(Key);
	if (it == m_LoadingStatus.end()) {
		return false;
	}
	return it->second;
}

// リソース取得.
void* ResourceManager::GetResource(const std::string& Key) {
	auto it = m_ResourceCache.find(Key);
	if (it == m_ResourceCache.end()) {
		return nullptr;
	}
	return it->second;
}

::Effekseer::EffectRef ResourceManager::GetEffect(const std::string& Name) {
	// TODO: EffectResource との統合実装.
	// TODO: ロード完了確認後、キャッシュから Effekseer::EffectRef を型キャストして返す.
	return nullptr;
}
