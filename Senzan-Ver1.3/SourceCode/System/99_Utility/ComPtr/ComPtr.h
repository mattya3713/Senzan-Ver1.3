#pragma once
#include <utility>

/**********************************************************
* @author      : mattya3713.
* @date        : 2025/XX/XX.
* @brief       : DirectX COM リソース専用軽量スマートポインタ.
*              : 参照カウントを管理し、所有権を安全に扱う.
**********************************************************/
template <typename T>
class MyComPtr final
{
public:
    // デフォルトコンストラクタ.
    MyComPtr() noexcept : m_pPtr(nullptr) {}

    // ポインタを受け取るコンストラクタ.
    explicit MyComPtr(T* pPtr) noexcept : m_pPtr(pPtr)
    {
        if (m_pPtr) { m_pPtr->AddRef(); }
    }

    // コピーコンストラクタ.
    MyComPtr(const MyComPtr& Other) noexcept : m_pPtr(Other.m_pPtr)
    {
        if (m_pPtr) { m_pPtr->AddRef(); }
    }

    // ムーブコンストラクタ.
    MyComPtr(MyComPtr&& Other) noexcept : m_pPtr(Other.m_pPtr)
    {
        Other.m_pPtr = nullptr;
    }

    // デストラクタ.
    ~MyComPtr() noexcept
    {
        if (m_pPtr) { m_pPtr->Release(); }
    }

    // コピー代入演算子.
    MyComPtr& operator=(const MyComPtr& Other) noexcept
    {
        if (this != &Other)
        {
            // 先に新しい参照を増やしてから古いのを解放（安全順序）.
            T* p_old = m_pPtr;
            m_pPtr = Other.m_pPtr;
            if (m_pPtr) { m_pPtr->AddRef(); }
            if (p_old) { p_old->Release(); }
        }
        return *this;
    }

    // ムーブ代入演算子.
    MyComPtr& operator=(MyComPtr&& Other) noexcept
    {
        if (this != &Other)
        {
            if (m_pPtr) { m_pPtr->Release(); }
            m_pPtr = Other.m_pPtr;
            Other.m_pPtr = nullptr;
        }
        return *this;
    }

    // 内部ポインタの取得.
    T* Get() const noexcept { return m_pPtr; }

    // ポインタへのアクセス.
    T* operator->() const noexcept { return m_pPtr; }

    // ポインタのデリファレンス.
    T& operator*() const noexcept { return *m_pPtr; }

    // nullptr でなければ true を返す.
    explicit operator bool() const noexcept { return m_pPtr != nullptr; }

    // nullptr チェック.
    bool operator!() const noexcept { return m_pPtr == nullptr; }

    // 内部ポインタのアドレスを取得（nullptr前提・Releaseしない）.
    T** GetAddressOf() noexcept
    {
        return &m_pPtr;
    }

    // 現在のポインタを解放し、アドレスを返す（上書き・再代入用）.
    T** ReleaseAndGetAddressOf() noexcept
    {
        if (m_pPtr) { m_pPtr->Release(); m_pPtr = nullptr; }
        return &m_pPtr;
    }

    // 新しいポインタを設定する.
    void Reset(T* pPtr = nullptr) noexcept
    {
        T* p_old = m_pPtr;
        m_pPtr = pPtr;
        if (m_pPtr) { m_pPtr->AddRef(); }
        if (p_old) { p_old->Release(); }
    }

    // 所有権を放棄して生ポインタを返す.
    T* Detach() noexcept
    {
        T* p_ptr = m_pPtr;
        m_pPtr = nullptr;
        return p_ptr;
    }

    // ポインタを交換する.
    void Swap(MyComPtr& Other) noexcept
    {
        std::swap(m_pPtr, Other.m_pPtr);
    }

private:
    T* m_pPtr;
};
