#pragma once
#include <queue>
#include <mutex>

/**********************************************************
* @author      : mattya3713.
* @date        : 2026/02/18.
* @brief       : マルチスレッド環境でのスレッドセーフなキュー実装.
**********************************************************/

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> m_Queue;
    std::mutex    m_Mutex;

public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // データを安全に追加する.
    void Push(const T& Value) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Queue.push(Value);
    }

    // データを安全に取り出す(空ならfalseを返す).
    bool Pop(T& OutValue) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.empty()) {
            return false;
        }
        OutValue = m_Queue.front();
        m_Queue.pop();
        return true;
    }

    // キューが空かどうかを確認する.
    bool IsEmpty() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Queue.empty();
    }
};