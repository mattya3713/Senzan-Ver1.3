#pragma once
#include "Game/02_GameObject/GameObject.h"

class Sprite2D;

/**********************************************************
* @author      : mattya3713.
* @date        : 2026/04/17.
* @brief       : UIオブジェクト.
**********************************************************/
class UIObject
	: public GameObject
{
public:
	// UIの種類.
	enum class UIType : unsigned char
	{
		Image,	// 画像.
		Button,	// ボタン.
		Text,	// テキスト.
		Gage,	// ゲージ.
	};

public:
	UIObject();
	virtual ~UIObject() override;

	// 更新.
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Draw() override;

	// 画像の接続.
	void AttachSprite(const std::shared_ptr<Sprite2D>& Sprite);
	// 画像の取得.
	std::shared_ptr<Sprite2D> GetSprite();
	// 画像の分離.
	void DetachSprite();

public: // Getter、Setter.
	// アンカー、ピボットを考慮した座標を取得.
	DirectX::XMFLOAT3 GetPositionWithAnchorPivot() const;

	// Anchorの取得/設定.
    inline const DirectX::XMFLOAT2& GetAnchor() const noexcept { return m_Anchor; }
	inline void SetAnchor(const DirectX::XMFLOAT2& Anchor) { m_Anchor = Anchor; }

	// Pivotの取得/設定.
	inline const DirectX::XMFLOAT2& GetPivot() const noexcept { return m_Pivot; }
	inline void SetPivot(const DirectX::XMFLOAT2& Pivot) { m_Pivot = Pivot; }

	// DrawSizeの取得/設定.
	inline const DirectX::XMFLOAT2& GetDrawSize() const noexcept { return m_DrawSize; }
	inline void SetDrawSize(const DirectX::XMFLOAT2& DrawSize) { m_DrawSize = DrawSize; }

	// Colorの取得/設定.
	inline const DirectX::XMFLOAT4& GetColor() const noexcept { return m_Color; }
	inline void SetColor(const DirectX::XMFLOAT4& Color) { m_Color = Color; }

	// Alphaの取得/設定.
    inline float GetAlpha() const noexcept { return m_Color.w; }
	inline void SetAlpha(float Alpha) { m_Color.w = Alpha; }

	// UITypeの取得.
	inline UIType GetUIType() const noexcept { return m_UIType; }

	// Layerの取得/設定.
	inline int GetLayer() const noexcept { return m_Layer; }
	inline void SetLayer(int Layer) { m_Layer = Layer; }

	// 接続している資源名を取得.
	std::string GetResourceName() const;

	// UI名の取得/設定.
	inline const std::string& GetUIName() const noexcept { return m_UIName; }
	inline void SetUIName(const std::string& Name) { m_UIName = Name; }

protected:
	std::weak_ptr<Sprite2D> m_pSprite;
	std::string m_UIName;			// UI名.
	DirectX::XMFLOAT2 m_Anchor;		// アンカー.
	DirectX::XMFLOAT2 m_Pivot;		// ピボット.
	DirectX::XMFLOAT2 m_DrawSize;	// 描画幅・高さ.
	DirectX::XMFLOAT4 m_Color;		// 色（R,G,B,A）.
	UIType m_UIType;				// UIの種類.
	int m_Layer;					// レイヤー番号.
};
