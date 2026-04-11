#pragma once
#include "Transform.h"

class RectTransform final
{
public:
	RectTransform();
	~RectTransform();

	/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : アンカーとピボットを考慮した座標を計算して返す.
**********************************************************************************/
	const DirectX::XMFLOAT3 CalcAnchoredPosition();

public: // Getter、Setter.

	/*************************************************************
	* @breif Transform情報を取得・設定.
	* @note GetTransform：取得.
	* @note SetTransform：設定.
	* ************************************************************/
	const Transform& GetTransform() const;
	void SetTransform(const Transform& transform);


	/*************************************************************
	* @breif 幅高さを取得・設定.
	* @note GetSize：取得.
	* @note SetSize：設定.
	* ************************************************************/
	const DirectX::XMFLOAT2& GetSize() const;
	void SetSize(const DirectX::XMFLOAT2& size);


	/*************************************************************
	* @breif アンカーを取得・設定.
	* @note GetAnchor：取得.
	* @note SetAnchor：設定.
	* ************************************************************/
	const DirectX::XMFLOAT2& GetAnchor() const;
	void SetAnchor(const DirectX::XMFLOAT2& anchor);


	/*************************************************************
	* @breif ピボットを取得・設定.
	* @note GetPivot：取得.
	* @note SetPivot：設定.
	* ************************************************************/
	const DirectX::XMFLOAT2& GetPivot() const;
	void SetPivot(const DirectX::XMFLOAT2& pivot);

private:
	Transform	m_spTransform;
	DirectX::XMFLOAT2	m_Size;		// 幅高さ.
	DirectX::XMFLOAT2	m_Anchor;	// アンカー.
	DirectX::XMFLOAT2	m_Pivot;	// ピボット.
};
