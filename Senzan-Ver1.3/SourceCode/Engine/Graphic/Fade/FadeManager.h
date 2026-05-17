#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include "Engine/Graphic/Fade/Fade.h"

/**********************************************************************************
* @author    : mattya3713.
* @date      : 2025/02/18.
* @brief     : フェード管理クラス.
**********************************************************************************/
class FadeManager final
	: public Singleton<FadeManager>
{
private:
	friend class Singleton<FadeManager>;
	FadeManager();
public:
	~FadeManager()override;

	void Update();	
	void Draw();	

	/****************************************
	* @brief フェードを開始.
	* @param type : 開始するフェードの種類.
	****************************************/
	void StartFade(const Fade::FadeType type);

public: // Getter、Setter.

	// フェード中か判定.
	const bool IsFading() const;

	/***************************************
	* @brief フェードが終了しているか判定.
	* @param type・終了判定をするフェードの種類.
	****************************************/
	const bool IsFadeCompleted(const Fade::FadeType type) const;

private:
	std::unique_ptr<Fade> m_pFade;
};
