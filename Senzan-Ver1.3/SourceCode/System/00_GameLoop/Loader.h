//#pragma once
//#include <future>
//
//class Sprite2D;
//class UIObject;
//
///**********************************************************************************
//* @author    : 未定.
//* @date      : 未定.
//* @brief     : 読み込みクラス.
//**********************************************************************************/
//class Loader
//{
//public:
//	Loader();
//	~Loader();
//
//	/****************************************************
//	* @brief ローディング画面に使用するデータの読み込み.
//	****************************************************/
//	void LoadData();
//	
//
//	/****************************************************
//	* @brief 更新.
//	****************************************************/
//	void Update();	
//
//
//	/****************************************************
//	* @brief 描画.
//	****************************************************/
//	void Draw();
//
//
//	/****************************************************
//	* @brief 非同期でアセットの読み込み.
//	****************************************************/
//	void StartLoading();
//
//
//	/****************************************************
//	* @brief アセットの読み込みが完了したか判定.
//	****************************************************/
//	const bool IsLoadCompletion();
//
//private:	
//
//	/****************************************************
//	* @brief 3Dモデル、UIの読み込み.
//	****************************************************/
//	bool LoadGraphicsAssets();
//
//
//	/****************************************************
//	* @brief サウンドの読み込み.
//	****************************************************/
//	bool LoadSounds();
//
//
//	/****************************************************
//	* @brief エフェクトの読み込み.
//	****************************************************/
//	bool LoadEffects();
//
//
//	/****************************************************
//	* @brief Lineの初期化処理.
//	****************************************************/
//	void LineInit(std::shared_ptr<UIObject> obje);
//private:
//	// 非同期の結果を取得するフラグ.
//	std::vector<std::future<bool>> m_IsLoadResult;
//	std::vector<std::shared_ptr<UIObject>> m_pWhiteLine;
//
//	// Spite.
//	std::shared_ptr<Sprite2D> m_pBackGroundImage;
//	std::shared_ptr<Sprite2D> m_pLoadImage;
//	std::shared_ptr<Sprite2D> m_pLineImage;
//
//	// UIObject.
//	std::unique_ptr<UIObject> m_pBackGroundObject;
//	std::unique_ptr<UIObject> m_pLoadObject;
//
//	// テキストのα値.
//	float m_LoadRotSpeed;
//
//	int   m_Lines;
//	float m_LineInitTickness;
//	float m_LineGenerateRate;
//	float m_LineGenerateTime;
//	float m_LineDecAlpha;
//	float m_LineDecTickness;
//};
