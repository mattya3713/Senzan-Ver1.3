#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <memory>

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : リソース全般をキャッシュ.
**********************************************************/

 // リソースの種類を定義.
enum class eResourceType {
    Effect,
    Texture,
    FBX,
    Sprite
};

// メインスレッド -> ロードスレッドへの「お願い用」構造体.
struct LoadRequest {
    std::string ResourceKey; // 識別用のキー.
    std::string FilePath;    // ロードするファイルのパス.
    eResourceType Type;      // リソースの種類.
};

// ロードスレッド -> メインスレッドへの「できたよ用」構造体.
struct LoadedAsset {
	std::string ResourceKey; // 識別用のキー.
    void* m_pResource;       // ロード済みリソース, メイン側でキャストして使うため一旦void*.
    bool m_IsSuccess;        // ロードが成功したかどうか.
};
