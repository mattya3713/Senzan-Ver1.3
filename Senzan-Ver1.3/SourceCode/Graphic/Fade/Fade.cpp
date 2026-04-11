#include "Fade.h"
//#include "Singleton/ResourceManager/SpriteManager/SpriteManager.h"

Fade::Fade()
    : UIObject          ()
    , m_LastFadeType    ()
    , m_AlphaSpeed      ( 0.03f )    
    , m_IsStartFade     ( false )        
    , m_IsFadeCompleted ( false )
{
    // フェード画像を接続.
    //AttachSprite(SpriteManager::GetSprite2D("Black"));
}


Fade::~Fade()
{
}


void Fade::Update()
{
    // フェードを開始していないなら処理しない.
    if (!m_IsStartFade) { return; }

    // α値の更新.
    m_Color.w += m_LastFadeType == FadeType::FadeIn ? -m_AlphaSpeed : m_AlphaSpeed;

    // α値をクランプし、フェード完了設定.
    if (m_Color.w >= 1.0f || m_Color.w <= 0.0f)
    {
        m_Color.w = std::clamp(m_Color.w, 0.0f, 1.0f);
        m_IsStartFade       = false;
        m_IsFadeCompleted   = true;
    }

    UIObject::Update();
}


void Fade::Draw()
{
    // DirectX12 では深度制御は PSO/描画パス側で管理する.
    UIObject::Draw();
}


void Fade::StartFade(const FadeType type)
{
    // 既にフェード開始している場合処理しない.
    if (m_IsStartFade) { return; }

    m_IsStartFade       = true; // フェード開始に設定.
    m_IsFadeCompleted   = false;// フェード未完了に設定.
    m_LastFadeType      = type; // フェードの種類設定.

    // FadeInなら0.0f開始.
    // FadeOutなら1.0f開始.
    m_Color.w = (type == FadeType::FadeIn) ? 1.0f : 0.0f;
}

const bool Fade::IsFading() const
{
    // フェードが開始されているならフェード中.
    return m_IsStartFade;
}


const bool Fade::IsFadeCompleted(const FadeType type) const
{
    return m_IsFadeCompleted && m_LastFadeType == type;
}
