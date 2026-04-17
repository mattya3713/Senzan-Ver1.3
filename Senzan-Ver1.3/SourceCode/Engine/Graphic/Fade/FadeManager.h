#pragma once
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"
#include "Engine/Graphic/Fade/Fade.h"

/**********************************************************************************
* @author    : 譛ｪ螳・
* @date      : 譛ｪ螳・
* @brief     : 繝輔ぉ繝ｼ繝臥ｮ｡逅・け繝ｩ繧ｹ.
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
	* @brief 繝輔ぉ繝ｼ繝峨ｒ髢句ｧ・
	* @param type・夐幕蟋九☆繧九ヵ繧ｧ繝ｼ繝峨・遞ｮ鬘・
	****************************************/
	void StartFade(const Fade::FadeType type);

public: // Getter縲ヾetter.

	/****************************************
	* @brief 繝輔ぉ繝ｼ繝我ｸｭ縺句愛螳・
	****************************************/
	const bool IsFading() const;

	/****************************************
	* @brief 繝輔ぉ繝ｼ繝峨′邨ゆｺ・＠縺ｦ縺・ｋ縺句愛螳・
	* @param type・夂ｵゆｺ・愛螳壹ｒ縺吶ｋ繝輔ぉ繝ｼ繝峨・遞ｮ鬘・
	****************************************/
	const bool IsFadeCompleted(const Fade::FadeType type) const;

private:
	std::unique_ptr<Fade> m_pFade;
};

