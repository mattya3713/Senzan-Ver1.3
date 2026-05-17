#include "Engine/Resource/Mesh/01_FBX/FBXModelResource.h"
#include "Engine/Resource/Mesh/01_FBX/FBXModel.h"

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : FBX モデルリソースの管理実装.
**********************************************************/

bool FBXModelResource::Create(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue) {
	m_pDevice = pDevice;
	m_pCommandQueue = pCommandQueue;

	if (!pDevice || !pCommandQueue) {
		return false;
	}

	return true;
}

bool FBXModelResource::LoadData() {
	// TODO: 対象ディレクトリをスキャンし、FBX ファイルを Assimp で読み込む.
	// TODO: 読み込んだモデルを m_pFBXModels にキャッシング.
	// TODO: モデルごとに FBXModel::Load() を呼び出し GPU リソース生成.
	// TODO: ロード失敗時のエラーハンドリング.
	// TODO: ボーン情報、アニメーション情報の初期化.

	return true;
}

FBXModel* FBXModelResource::GetResource(const std::string& Name) {
	auto it = m_pFBXModels.find(Name);
	if (it == m_pFBXModels.end()) {
		return nullptr;
	}
	return it->second;
}
