#pragma once
#include "Game\01_GameObject\02_UIObject\UIObject.h"

class Fade
	: public UIObject
{
public:
	enum class FadeType : char
	{
		FadeIn,
		FadeOut,
	};
public:
	Fade();
	virtual ~Fade() override;

	virtual void Update()	override;
	virtual void Draw()		override;

	// フェードを開始.
	void StartFade(const FadeType type);

public: // Getter、Setter.

	// フェード中か.
	const bool IsFading() const;

	// フェードが完了しているか.
	const bool IsFadeCompleted(const FadeType type) const;

private:
	FadeType m_LastFadeType;// 最後に使用したフェードの種類.
	float m_AlphaSpeed;		// α値の減算速度.
	bool m_IsStartFade;		// フェードを開始するか.	
	bool m_IsFadeCompleted;	// フェードが完了したか.
};
