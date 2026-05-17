#pragma once

#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <d3d12.h>

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : FBX モデルリソースの管理.
**********************************************************/

class FBXModel;  // Forward declaration.

class FBXModelResource final : public Singleton<FBXModelResource> {
	friend class Singleton<FBXModelResource>;

private:
	FBXModelResource() = default;
	~FBXModelResource() = default;

public:
	// 初期化・読み込み関数（ResourceManager から呼ばれる想定）.
	// @brief DirectX12 デバイスの初期化が必要な場合.
	bool Create(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue);

	// @brief FBX モデルデータの一括読み込み.
	bool LoadData();

	// FBX モデルの取得.
	// @param Name リソース名（モデル名等）.
	// @return FBXModel ポインタ、見つからなければ nullptr.
	FBXModel* GetResource(const std::string& Name);

private:
	// FBX モデルデータのマッピング（名前:モデル）.
	std::unordered_map<std::string, FBXModel*> m_pFBXModels;

	ID3D12Device* m_pDevice;
	ID3D12CommandQueue* m_pCommandQueue;
};
