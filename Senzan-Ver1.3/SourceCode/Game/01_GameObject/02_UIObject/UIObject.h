#pragma once
#include "..\GameObject.h"

class Sprite2D;

/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
* @brief     : UIオブジェクト.
**********************************************************************************/
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
	
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Draw() override;
	

	/******************************************************
	* @brief 画像の接続・分離.
	* @note AttachSprite：接続.
	* @note DetachSprite：分離.
	******************************************************/
	void AttachSprite(const std::shared_ptr<Sprite2D>& pSprite);
    std::shared_ptr<Sprite2D> GetSprite();
	void DetachSprite();

public: // Getter、Setter.

	/******************************************************
	* @brief アンカー、ピボットを考慮した座標を取得.	
	******************************************************/
	const DirectX::XMFLOAT3 GetPositionWithAnchorPivot() const;


	/******************************************************
	* @brief アンカーの取得・設定.
	* @note GetAnchor：取得.
	* @note SetAnchor：設定.
	******************************************************/
	const DirectX::XMFLOAT2& GetAnchor() const;
	void SetAnchor(const DirectX::XMFLOAT2& anchor);


	/******************************************************
	* @brief ピボットの取得・設定.
	* @note GetPivot：取得.
	* @note SetPivot：設定.
	******************************************************/
	const DirectX::XMFLOAT2& GetPivot() const;
	void SetPivot(const DirectX::XMFLOAT2& pivot);


	/******************************************************
	* @brief 描画幅、高さを取得・設定.
	* @note GetDrawSize：取得.
	* @note SetDrawSize：設定.
	******************************************************/
	const DirectX::XMFLOAT2& GetDrawSize() const;
	void SetDrawSize(const DirectX::XMFLOAT2& drawSize);


	/******************************************************
	* @brief 色を取得・設定.
	* @note GetColor：取得.
	* @note SetColor：設定.
	******************************************************/
	const DirectX::XMFLOAT4& GetColor() const;
	void SetColor(const DirectX::XMFLOAT4& Color);
	

	/******************************************************
	* @brief アルファ値を取得・設定.
	* @note GetAlpha：取得.
	* @note SetAlpha：設定.
	******************************************************/
	const float& GetAlpha() const;
	void SetAlpha(const float& alpha);


	/******************************************************
	* @brief UIの種類を取得.
	******************************************************/
	const UIObject::UIType& GetUIType() const;


	/******************************************************
	* @brief レイヤー番号を取得・設定.
	* @note GetLayer：取得.
	* @note SetLayer：設定.
	******************************************************/
	const int& GetLayer() const;
	void SetLayer(const int& layer);


	/******************************************************
	* @brief 接続している資源名を取得.
	******************************************************/
	const std::string GetResourceName() const;


	/******************************************************
	* @brief 接続している資源名を取得.
	******************************************************/
	const std::string& GetUIName() const;
	void SetUIName(const std::string& name);


protected:
	std::weak_ptr<Sprite2D> m_pSprite;
	std::string m_UIName;	// UIの自体名前.
	DirectX::XMFLOAT2 m_Anchor;	// アンカー.
	DirectX::XMFLOAT2 m_Pivot;	// ピボット.
	DirectX::XMFLOAT2 m_DrawSize;	// 描画幅、高さ.
	DirectX::XMFLOAT4 m_Color;	// 色（R,G,B,A）.	
	UIType		m_UIType;	// UIの種類.
	int			m_Layer;	// レイヤー番号.
};
