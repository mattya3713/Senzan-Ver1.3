#pragma once

#include "Engine/Utility/MultiThread/ThreadSafeQueue.h" // スレッドセーフなキュー.
#include "Engine/Resource/ResourceCache.h"              // リソースキャッシュ.

/**********************************************************
* @author   : mattya3713.
* @date     : 2026/05/17.
* @brief    : リソースの非同期読み込みを管理する.
**********************************************************/

class ResourceLoader final {

public:
    void Initialize(int ThreadCount);
    void Shutdown();

    void PushLoadRequest(const LoadRequest& Request);
    bool PopLoadedAsset(LoadedAsset& OutAsset);

private:
    void LoadWorkerLoop();  // ワーカースレッドで実行.

private:
    std::vector<std::thread>     m_LoadThreads;
    ThreadSafeQueue<LoadRequest> m_RequestQueue;
    ThreadSafeQueue<LoadedAsset> m_CompletedQueue;
    std::atomic<bool>            m_IsRunning;

};