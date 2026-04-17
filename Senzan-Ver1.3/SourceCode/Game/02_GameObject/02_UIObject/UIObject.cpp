#include "UIObject.h"
#include "Engine/Resource/Mesh/00_Sprite/UISprite/UISprite.h"


UIObject::UIObject()
	: GameObject		()
	, m_pSprite			()
	, m_Anchor			( {0.5f, 0.5f} )
	, m_Pivot			( {0.5f, 0.5f} )
	, m_DrawSize		()
	, m_Color			( {1.0f, 1.0f, 1.0f, 1.0f} )
	, m_UIType			( UIType::Image )
	, m_Layer			( 0 )
{
}


UIObject::~UIObject()
{
}


void UIObject::Update()
{
	if (!m_IsActive) { return; }
}

void UIObject::LateUpdate()
{
}


void UIObject::Draw()
{
	if (!m_IsRenderActive) { return; }
	
	if (auto sprite = m_pSprite.lock())
	{		
		const auto& rectTransform = sprite->GetRectTransform();

		Transform newTransform = *m_spTransform;
		rectTransform->SetTransform(newTransform);
		rectTransform->SetAnchor(m_Anchor);
		rectTransform->SetPivot(m_Pivot);
		sprite->SetDrawSize(m_DrawSize);
		sprite->SetColor(m_Color);

		sprite->Render();
	}
}


void UIObject::AttachSprite(const std::shared_ptr<Sprite2D>& Sprite)
{
 if (Sprite == nullptr) { return; }

   m_pSprite	= Sprite;
	m_DrawSize	= Sprite->GetRectTransform()->GetSize();
}


std::shared_ptr<Sprite2D> UIObject::GetSprite()
{
    return m_pSprite.lock();
}


void UIObject::DetachSprite()
{
	m_pSprite.reset();
}


DirectX::XMFLOAT3 UIObject::GetPositionWithAnchorPivot() const
{
	if (auto sprite = m_pSprite.lock())
	{
		return sprite->GetRectTransform()->CalcAnchoredPosition();
	}
	return Axis::ZERO;
}




std::string UIObject::GetResourceName() const
{
	if (auto lockedSprite = m_pSprite.lock())
	{
		return lockedSprite->GetResourceName();
	}
	return std::string("Non");
}


