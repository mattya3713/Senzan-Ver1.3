#include "UIObject.h"
#include "Resource/Mesh/00_Sprite/UISprite/UISprite.h"


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


void UIObject::AttachSprite(const std::shared_ptr<Sprite2D>& pSprite)
{
	if (pSprite == nullptr) { return; }

	m_pSprite	= pSprite;
	m_DrawSize	= pSprite->GetRectTransform()->GetSize();
}


std::shared_ptr<Sprite2D> UIObject::GetSprite()
{
    return m_pSprite.lock();
}


void UIObject::DetachSprite()
{
	m_pSprite.reset();
}


const DirectX::XMFLOAT3 UIObject::GetPositionWithAnchorPivot() const
{
	if (auto sprite = m_pSprite.lock())
	{
		return sprite->GetRectTransform()->CalcAnchoredPosition();
	}
	return Axis::ZERO;
}


const DirectX::XMFLOAT2& UIObject::GetAnchor() const
{
	return m_Anchor;
}


void UIObject::SetAnchor(const DirectX::XMFLOAT2& anchor)
{
	m_Anchor = anchor;
}


const DirectX::XMFLOAT2& UIObject::GetPivot() const
{
	return m_Pivot;
}


void UIObject::SetPivot(const DirectX::XMFLOAT2& pivot)
{
	m_Pivot = pivot;
}

const DirectX::XMFLOAT2& UIObject::GetDrawSize() const
{
	return m_DrawSize;
}


void UIObject::SetDrawSize(const DirectX::XMFLOAT2& drawSize)
{
	m_DrawSize = drawSize;
}


const DirectX::XMFLOAT4& UIObject::GetColor() const
{
	return m_Color;
}


void UIObject::SetColor(const DirectX::XMFLOAT4& color)
{
	m_Color = color;
}


const float& UIObject::GetAlpha() const
{
	return m_Color.w;
}


void UIObject::SetAlpha(const float& alpha)
{
	m_Color.w = alpha;
}


const UIObject::UIType& UIObject::GetUIType() const
{
	return m_UIType;
}


const int& UIObject::GetLayer() const
{
	return m_Layer;
}


void UIObject::SetLayer(const int& layer)
{
	m_Layer = layer;
}


const std::string UIObject::GetResourceName() const
{
	if (auto lockedSprite = m_pSprite.lock())
	{
		return lockedSprite->GetResourceName();
	}
	return std::string("Non");
}


const std::string& UIObject::GetUIName() const
{
	return m_UIName;
}


void UIObject::SetUIName(const std::string& name)
{
	m_UIName = name;
}
