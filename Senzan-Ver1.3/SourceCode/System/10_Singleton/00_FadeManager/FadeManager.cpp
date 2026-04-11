#include "FadeManager.h"

FadeManager::FadeManager()
    : m_pFade( std::make_unique<Fade>() )
{
}

FadeManager::~FadeManager()
{
}

void FadeManager::Update()
{
    m_pFade->Update();
}

void FadeManager::Draw()
{
    m_pFade->Draw();
}

void FadeManager::StartFade(const Fade::FadeType type)
{
    m_pFade->StartFade(type);
}

const bool FadeManager::IsFading() const
{
    return m_pFade->IsFading();
}

const bool FadeManager::IsFadeCompleted(const Fade::FadeType type) const
{
    return m_pFade->IsFadeCompleted(type);
}
