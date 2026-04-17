#include "RectTransform.h"

RectTransform::RectTransform()
    : m_spTransform   ()
    , m_Size        ()
    , m_Anchor      ( {0.5f, 0.5f} )
    , m_Pivot       ( {0.5f, 0.5f} )
{
}


RectTransform::~RectTransform()
{
}


const DirectX::XMFLOAT3 RectTransform::CalcAnchoredPosition()
{
    float offsetX = static_cast<float>(WND_W) * m_Anchor.x;
    float offsetY = static_cast<float>(WND_H) *m_Anchor.y;

    float offsetPivotX = m_Size.x * (m_Pivot.x - 0.5f );
    float offsetPivotY = m_Size.y * (m_Pivot.y - 0.5f );

    float x = m_spTransform.Position.x + offsetX + offsetPivotX;
    float y = m_spTransform.Position.y + offsetY + offsetPivotY;
    float z = m_spTransform.Position.z;

    // 新しい位置を計算.
    return DirectX::XMFLOAT3(x, y, z);
}


const Transform& RectTransform::GetTransform() const
{
    return m_spTransform;
}


void RectTransform::SetTransform(const Transform& transform)
{
    m_spTransform = transform;
}


const DirectX::XMFLOAT2& RectTransform::GetSize() const
{
    return m_Size;
}


void RectTransform::SetSize(const DirectX::XMFLOAT2& size)
{
    m_Size = size;
}


const DirectX::XMFLOAT2& RectTransform::GetAnchor() const
{
    return m_Anchor;
}


void RectTransform::SetAnchor(const DirectX::XMFLOAT2& anchor)
{
    m_Anchor = anchor;
}


const DirectX::XMFLOAT2& RectTransform::GetPivot() const
{
    return m_Pivot;
}


void RectTransform::SetPivot(const DirectX::XMFLOAT2& pivot)
{
    m_Pivot = pivot;
}
