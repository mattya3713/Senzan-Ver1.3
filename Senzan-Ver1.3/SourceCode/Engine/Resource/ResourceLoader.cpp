#include "Engine/Resource/ResourceLoader.h"

#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : リソースの非同期読み込みを管理する実装.
**********************************************************/

// ヘルパー関数：エフェクトファイルの読み込み.
// @param FilePath ファイルパス.
// @return 成功なら true、失敗なら false.
static bool LoadEffectFile(const std::string& FilePath) {
    // ファイル存在確認.
    if (!std::filesystem::exists(FilePath)) {
        return false;
    }

    // TODO: Effekseer マネージャー経由でエフェクトファイルを読み込む.
    // TODO: EffectRef を生成し、中間データとして返す構造体に格納.
    // 現段階では、ファイル存在確認のみで成功を返す.
    return true;
}

// ヘルパー関数：テクスチャファイルの読み込み（DirectXTex）.
// @param FilePath ファイルパス.
// @return 成功なら true、失敗なら false.
static bool LoadTextureFile(const std::string& FilePath) {
    // ファイル存在確認.
    if (!std::filesystem::exists(FilePath)) {
        return false;
    }

    // TODO: DirectXTex を使用して画像ファイルを読み込む.
    // TODO: メモリバッファ（バイナリデータ）として ID3D12Resource 生成前の状態で保持.
    // TODO: GPU リソース生成はメインスレッド側で実行（スレッドセーフティのため）.
    // 現段階では、ファイル存在確認のみで成功を返す.
    return true;
}

// ヘルパー関数：FBX ファイルの読み込み（Assimp）.
// @param FilePath ファイルパス.
// @return 成功なら true、失敗なら false.
static bool LoadFBXFile(const std::string& FilePath) {
    // ファイル存在確認.
    if (!std::filesystem::exists(FilePath)) {
        return false;
    }

    // TODO: Assimp を使用して FBX ファイルを読み込む.
    // TODO: メッシュデータ（頂点、インデックス、ボーン）をバッファ化.
    // TODO: アニメーション情報も同様にバッファ化.
    // TODO: GPU リソース生成はメインスレッド側で実行.
    // 現段階では、ファイル存在確認のみで成功を返す.
    return true;
}

// ヘルパー関数：スプライト画像の読み込み（DirectXTex）.
// @param FilePath ファイルパス.
// @return 成功なら true、失敗なら false.
static bool LoadSpriteFile(const std::string& FilePath) {
    // ファイル存在確認.
    if (!std::filesystem::exists(FilePath)) {
        return false;
    }

    // TODO: DirectXTex を使用して画像ファイルを読み込む.
    // TODO: Texture と同じ処理だが、スプライト用メタデータ（幅・高さ等）も付加.
    // TODO: GPU リソース生成はメインスレッド側で実行.
    // 現段階では、ファイル存在確認のみで成功を返す.
    return true;
}

void ResourceLoader::Initialize(int ThreadCount) {
    m_IsRunning = true;

    // ワーカースレッドを指定数起動する.
    for (int i = 0; i < ThreadCount; ++i) {
        m_LoadThreads.emplace_back(&ResourceLoader::LoadWorkerLoop, this);
    }
}

void ResourceLoader::Shutdown() {
    m_IsRunning = false;

    // すべてのワーカースレッドの終了を待機.
    for (auto& thread : m_LoadThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    m_LoadThreads.clear();
}

void ResourceLoader::PushLoadRequest(const LoadRequest& Request) {
    m_RequestQueue.Push(Request);
}

bool ResourceLoader::PopLoadedAsset(LoadedAsset& OutAsset) {
    return m_CompletedQueue.Pop(OutAsset);
}

void ResourceLoader::LoadWorkerLoop() {
    LoadRequest load_request;

    while (m_IsRunning) {
        // リクエストキューから読み込み要求を取得.
        if (m_RequestQueue.Pop(load_request)) {
            LoadedAsset loaded_asset;
            loaded_asset.ResourceKey = load_request.ResourceKey;
            loaded_asset.m_pResource = nullptr;
            loaded_asset.m_IsSuccess = false;

            // リソースタイプに応じた読み込み処理を実行.
            switch (load_request.Type) {
                case eResourceType::Effect:
                    loaded_asset.m_IsSuccess = LoadEffectFile(load_request.FilePath);
                    break;

                case eResourceType::Texture:
                    loaded_asset.m_IsSuccess = LoadTextureFile(load_request.FilePath);
                    break;

                case eResourceType::FBX:
                    loaded_asset.m_IsSuccess = LoadFBXFile(load_request.FilePath);
                    break;

                case eResourceType::Sprite:
                    loaded_asset.m_IsSuccess = LoadSpriteFile(load_request.FilePath);
                    break;

                default:
                    break;
            }

            // ロード完了をメインスレッドに通知.
            m_CompletedQueue.Push(loaded_asset);
        } else {
            // リクエストがない場合は少し待機.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
