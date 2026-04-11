//#include "Loader.h"
//#include "Graphic/DirectX/DirectX11/DirectX11.h"
//#include "System/Singleton/ResourceManager/ResourceManager.h"
//#include "02_UIObject/UIObject.h"
//#include "Utility/Math/Random/Random.h"
//#include <chrono>
//
//Loader::Loader()
//	: m_IsLoadResult        ()
//	, m_pBackGroundImage    ( std::make_shared<Sprite2D>() )
//	, m_pLoadImage          ( std::make_shared<Sprite2D>() )
//	, m_pLineImage          ( std::make_shared<Sprite2D>() )
//	, m_pBackGroundObject   ( std::make_unique<UIObject>() )
//	, m_pLoadObject         ( std::make_unique<UIObject>() )
//	, m_LoadRotSpeed        ( 0.05f )
//	, m_Lines               ( 5 )
//	, m_LineInitTickness    ( 0.01f )
//	, m_LineGenerateRate    ( 0.5f )
//    , m_LineGenerateTime    ( 0.0f )
//	, m_LineDecAlpha        ( 0.01f )
//	, m_LineDecTickness     ( 0.0001f )
//{
//}
//
//
//Loader::~Loader()
//{
//}
//
//
//void Loader::LoadData()
//{
//	// 画像の読み込み.
//	m_pBackGroundImage->Initialize("Data\\Image\\Sprite\\Other\\Black.png");
//	m_pLoadImage->Initialize("Data\\Image\\Sprite\\Other\\Loading.png");
//	m_pLineImage->Initialize("Data\\Image\\Sprite\\Other\\White.png");
//
//
//	// 画像を接続.
//	m_pBackGroundObject->AttachSprite(m_pBackGroundImage);
//	m_pLoadObject->AttachSprite(m_pLoadImage);
//
//    m_pLoadObject->SetPosition(
//        WND_WF/2 - m_pLoadObject->GetDrawSize().x,
//        WND_HF/2 - m_pLoadObject->GetDrawSize().y,
//        0.0f);
//
//	for (int i = 0; i < m_Lines; ++i) {
//		auto line = std::make_shared<UIObject>();
//		line->AttachSprite(m_pLineImage);
//        line->SetAlpha(0.0f);
//		m_pWhiteLine.push_back(line);
//	}
//}
//
//
//void Loader::Update()
//{
//    m_pLoadObject->SetRotationZ(m_pLoadObject->GetRotationZ() + m_LoadRotSpeed);
//
//    m_LineGenerateTime -= 0.01f;
//	for (auto& line : m_pWhiteLine)
//	{
//        if (m_LineGenerateTime < 0.0f &&  line->GetAlpha() < 0.0f){
//            m_LineGenerateTime = m_LineGenerateRate;
//            LineInit(line);
//        }
//
//		line->SetAlpha(line->GetAlpha() - m_LineDecAlpha);
//		line->SetScaleY(line->GetScaleY() - m_LineDecTickness);
//	}
//}
//
//
//void Loader::Draw()
//{
//	//バックバッファをクリアにする.
//	DirectX11::GetInstance().ClearBackBuffer();
//	DirectX11::GetInstance().SetDepth(false);
//
//	m_pBackGroundObject->Draw();
//	m_pLoadObject->Draw();
//
//	for (auto& line : m_pWhiteLine) {
//		line->Draw();
//	}
//
//	DirectX11::GetInstance().SetDepth(true);
//	DirectX11::GetInstance().Present();
//}
//
//
//void Loader::StartLoading()
//{
//	// スレッドに関数を渡して非同期で読み込み開始.
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadGraphicsAssets, this));
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadSounds , this));
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadEffects, this));
//}
//
//
//const bool Loader::IsLoadCompletion()
//{
//	// 一つでも読み込みが完了していなければfalseを返す.
//	for (const auto& result : m_IsLoadResult) {
//		if (result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
//			return false;
//		}
//	}
//	return true;
//}
//
//
//bool Loader::LoadGraphicsAssets()
//{
//	return ResourceManager::LoadMesh();
//}
//
//
//bool Loader::LoadSounds()
//{
//	return ResourceManager::LoadSounds();
//}
//
//
//bool Loader::LoadEffects()
//{
//	return ResourceManager::LoadEffects();
//}
//
//
//void Loader::LineInit(std::shared_ptr<UIObject> obje)
//{
//	// アルファをマイナス値で初期化することで「出現までの待機時間」とする.
//	// m_LineGenerateRateを使って待ち時間の幅を調節.
//	obje->SetAlpha(1.0f);
//	obje->SetScaleY(m_LineInitTickness);
//	obje->SetScaleX(3.0f);
//
//	float range = 400.0f;
//	obje->SetPosition(MyRand::GetRandomPercentage(-range, range),
//		MyRand::GetRandomPercentage(-range, range), 0.0f);
//
//	obje->SetRotationZ(MyRand::GetRandomPercentage(0.0f, 1.64f));
//}
#include "Loader.h"
//#include "Graphic/DirectX/DirectX11/DirectX11.h"
//#include "System/Singleton/ResourceManager/ResourceManager.h"
//#include "02_UIObject/UIObject.h"
//#include "Utility/Math/Random/Random.h"
//#include <chrono>
//
//Loader::Loader()
//	: m_IsLoadResult        ()
//	, m_pBackGroundImage    ( std::make_shared<Sprite2D>() )
//	, m_pLoadImage          ( std::make_shared<Sprite2D>() )
//	, m_pLineImage          ( std::make_shared<Sprite2D>() )
//	, m_pBackGroundObject   ( std::make_unique<UIObject>() )
//	, m_pLoadObject         ( std::make_unique<UIObject>() )
//	, m_LoadRotSpeed        ( 0.05f )
//	, m_Lines               ( 5 )
//	, m_LineInitTickness    ( 0.01f )
//	, m_LineGenerateRate    ( 0.5f )
//    , m_LineGenerateTime    ( 0.0f )
//	, m_LineDecAlpha        ( 0.01f )
//	, m_LineDecTickness     ( 0.0001f )
//{
//}
//
//
//Loader::~Loader()
//{
//}
//
//
//void Loader::LoadData()
//{
//	// 画像の読み込み.
//	m_pBackGroundImage->Initialize("Data\\Image\\Sprite\\Other\\Black.png");
//	m_pLoadImage->Initialize("Data\\Image\\Sprite\\Other\\Loading.png");
//	m_pLineImage->Initialize("Data\\Image\\Sprite\\Other\\White.png");
//
//
//	// 画像を接続.
//	m_pBackGroundObject->AttachSprite(m_pBackGroundImage);
//	m_pLoadObject->AttachSprite(m_pLoadImage);
//
//    m_pLoadObject->SetPosition(
//        WND_WF/2 - m_pLoadObject->GetDrawSize().x,
//        WND_HF/2 - m_pLoadObject->GetDrawSize().y,
//        0.0f);
//
//	for (int i = 0; i < m_Lines; ++i) {
//		auto line = std::make_shared<UIObject>();
//		line->AttachSprite(m_pLineImage);
//        line->SetAlpha(0.0f);
//		m_pWhiteLine.push_back(line);
//	}
//}
//
//
//void Loader::Update()
//{
//    m_pLoadObject->SetRotationZ(m_pLoadObject->GetRotationZ() + m_LoadRotSpeed);
//
//    m_LineGenerateTime -= 0.01f;
//	for (auto& line : m_pWhiteLine)
//	{
//        if (m_LineGenerateTime < 0.0f &&  line->GetAlpha() < 0.0f){
//            m_LineGenerateTime = m_LineGenerateRate;
//            LineInit(line);
//        }
//
//		line->SetAlpha(line->GetAlpha() - m_LineDecAlpha);
//		line->SetScaleY(line->GetScaleY() - m_LineDecTickness);
//	}
//}
//
//
//void Loader::Draw()
//{
//	//バックバッファをクリアにする.
//	DirectX11::GetInstance().ClearBackBuffer();
//	DirectX11::GetInstance().SetDepth(false);
//
//	m_pBackGroundObject->Draw();
//	m_pLoadObject->Draw();
//
//	for (auto& line : m_pWhiteLine) {
//		line->Draw();
//	}
//
//	DirectX11::GetInstance().SetDepth(true);
//	DirectX11::GetInstance().Present();
//}
//
//
//void Loader::StartLoading()
//{
//	// スレッドに関数を渡して非同期で読み込み開始.
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadGraphicsAssets, this));
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadSounds , this));
//	m_IsLoadResult.emplace_back(std::async(std::launch::async, &Loader::LoadEffects, this));
//}
//
//
//const bool Loader::IsLoadCompletion()
//{
//	// 一つでも読み込みが完了していなければfalseを返す.
//	for (const auto& result : m_IsLoadResult) {
//		if (result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
//			return false;
//		}
//	}
//	return true;
//}
//
//
//bool Loader::LoadGraphicsAssets()
//{
//	return ResourceManager::LoadMesh();
//}
//
//
//bool Loader::LoadSounds()
//{
//	return ResourceManager::LoadSounds();
//}
//
//
//bool Loader::LoadEffects()
//{
//	return ResourceManager::LoadEffects();
//}
//
//
//void Loader::LineInit(std::shared_ptr<UIObject> obje)
//{
//	// アルファをマイナス値で初期化することで「出現までの待機時間」とする.
//	// m_LineGenerateRateを使って待ち時間の幅を調節.
//	obje->SetAlpha(1.0f);
//	obje->SetScaleY(m_LineInitTickness);
//	obje->SetScaleX(3.0f);
//
//	float range = 400.0f;
//	obje->SetPosition(MyRand::GetRandomPercentage(-range, range),
//		MyRand::GetRandomPercentage(-range, range), 0.0f);
//
//	obje->SetRotationZ(MyRand::GetRandomPercentage(0.0f, 1.64f));
//}
