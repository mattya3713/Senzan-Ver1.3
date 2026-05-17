#include "Engine/Resource/Mesh/00_Sprite/SpriteResource.h"

/*******************************************
* @brief DirectX12 デバイスの初期化が必要な場合.
* @param pDevice DirectX12デバイス.
* @param pCommandQueue コマンドキュー.
* @return 成功したかどうか.
*******************************************/
bool SpriteResource::Init(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue) {
	m_pDevice = pDevice;
	m_pCommandQueue = pCommandQueue;

	if (!pDevice || !pCommandQueue) {
		return false;
	}

	return true;
}

bool SpriteResource::LoadData() {
	// TODO: 対象ディレクトリをスキャンし、画像ファイルを DirectXTex で読み込む.
	// TODO: 読み込んだテクスチャを m_pSpriteResources にキャッシング.
	// TODO: 複数フォーマット対応（PNG, JPG, BMP 等）.
	// TODO: ロード失敗時のエラーハンドリング.

	return true;
}

ID3D12Resource* SpriteResource::GetResource(const std::string& Name) {
	auto it = m_pSpriteResources.find(Name);
	if (it == m_pSpriteResources.end()) {
		return nullptr;
	}
	return it->second;
}
