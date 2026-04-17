#pragma once
#include "System/10_Singleton/SingletonTemplate.h"
#include "Graphic/Fade/Fade.h"

/**********************************************************************************
* @author    : 未定.
* @date      : 未定.
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
	* @param type：開始するフェードの種類.
	****************************************/
	void StartFade(const Fade::FadeType type);

public: // Getter、Setter.

	/****************************************
	* @brief フェード中か判定.
	****************************************/
	const bool IsFading() const;

	/****************************************
	* @brief フェードが終了しているか判定.
	* @param type：終了判定をするフェードの種類.
	****************************************/
	const bool IsFadeCompleted(const Fade::FadeType type) const;

private:
	std::unique_ptr<Fade> m_pFade;
};
