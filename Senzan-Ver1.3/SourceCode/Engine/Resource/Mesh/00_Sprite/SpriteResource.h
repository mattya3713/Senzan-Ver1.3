#pragma once

#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <d3d12.h>

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : スプライト画像リソースの管理.
**********************************************************/

class SpriteResource final : public Singleton<SpriteResource> {
	friend class Singleton<SpriteResource>;

private:
	SpriteResource() = default;
	~SpriteResource() = default;

public:
	/*******************************************
	* @brief DirectX12 デバイスの初期化が必要な場合.
	* @param pDevice DirectX12デバイス.
	* @param pCommandQueue コマンドキュー.
	* @return 成功したかどうか.
	*******************************************/
	bool Init(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue);

	bool LoadData();

	ID3D12Resource* GetResource(const std::string& Name);

private:
	// スプライト画像データのマッピング（名前:GPU リソース）.
	std::unordered_map<std::string, ID3D12Resource*> m_pSpriteResources;

	ID3D12Device* m_pDevice;
	ID3D12CommandQueue* m_pCommandQueue;
};
