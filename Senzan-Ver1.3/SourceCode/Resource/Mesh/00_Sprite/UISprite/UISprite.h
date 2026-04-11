#pragma once

#include <filesystem>
#include <unordered_map>
#include <d3d12.h>

#include "System/99_Utility/Transform/RectTransform.h"

#pragma warning(disable:4005)
#define ALIGN16	_declspec( align(16) )

class DirectX12;

class Sprite2D
{
public:

	//頂点の構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3 pos;	//頂点座標.
		DirectX::XMFLOAT2 tex;	//テクスチャ座標.
	};

	//幅高さ構造体.
	struct WHSIZE
	{
		float w;
		float h;
	};

	//スプライト構造体.
	struct SPRITE_STATE
	{
		WHSIZE Disp;	//表示幅高さ.
		WHSIZE Base;	//元画像幅高さ.
		WHSIZE Stride;	//1コマあたりの幅高さ.
	};

	//コンスタントバッファのアプリ側の定義.
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16	DirectX::XMMATRIX mWorld;		//ワールド行列.
		ALIGN16	DirectX::XMFLOAT4 vColor;		//カラー（RGBAの型に合わせる）.
		ALIGN16	DirectX::XMFLOAT4 vUV;			//UV座標（x,yのみ使用）.
		ALIGN16	float fViewPortWidth;			//ビューポート幅.
		ALIGN16	float fViewPortHeight;			//ビューポート高さ.
		ALIGN16	DirectX::XMFLOAT2 DawSize;	//描画幅高さ.
	};

	// DirectX::XMFLOAT2をキーとして使用するためのハッシュ.
	struct HASH_D3DXVECTER2
	{
		size_t operator()(const DirectX::XMFLOAT2& Key) const
		{
			return std::hash<float>()(Key.x) ^ std::hash<float>()(Key.y);
		}
	};

	// DirectX::XMFLOAT2を比較するための等価比較関数.
	struct EQUAL_XMFLOAT2
	{
		bool operator()(const DirectX::XMFLOAT2& Left, const DirectX::XMFLOAT2& Right) const
		{
			return Left.x == Right.x && Left.y == Right.y;
		}
	};

public:
	Sprite2D();
	~Sprite2D();

	/*********************************************************
	* @brief 初期化.
	* @param FilePath：画像のファイルパス.
	*********************************************************/
	bool Initialize(const std::filesystem::path& FilePath);

	/*********************************************************
	* @brief 描画.
	*********************************************************/
	void Render();

public: // Getter、Setter.

	/*********************************************************
	* @brief RectTransformを取得.
	*********************************************************/
	const std::unique_ptr<RectTransform>& GetRectTransform() const;

	/*********************************************************
	* @brief 描画幅、高さを設定.
	*********************************************************/
	void SetDrawSize(const DirectX::XMFLOAT2& DrawSize);

	/*********************************************************
	* @brief 色を設定.
	*********************************************************/
	void SetColor(const DirectX::XMFLOAT4& Color);

	/*********************************************************
	* @brief 資源名を取得.
	*********************************************************/
	const std::string& GetResourceName() const;

	/*********************************************************
	* @brief 資源名を設定.
	*********************************************************/
	void SetResourceName(const std::string& Name);

	/*********************************************************
	* @brief テクスチャを取得.
	*********************************************************/
	ID3D12Resource* GetTexture();

private:

	/*********************************************************
	* @brief 画像サイズの読み込み.
	*********************************************************/
	void LoadImageSize(const std::filesystem::path& FilePath);

	/*********************************************************
	* @brief シェーダー作成.
	*********************************************************/
	void CreateShader();

	/*********************************************************
	* @brief モデル作成.
	*********************************************************/
	HRESULT CreateModel();

	/*********************************************************
	* @brief テクスチャ作成.
	*********************************************************/
	HRESULT CreateTexture(const std::wstring& FilePath);

	/*********************************************************
	* @brief サンプラ作成.
	*********************************************************/
	HRESULT CreateSampler();

	/*********************************************************
	* @brief ワールド行列を計算.
	*********************************************************/
	void CalcWorldMatrix();

	/*********************************************************
	* @brief 描画に使用する頂点バッファを取得.
	*********************************************************/
	ID3D12Resource* GetUseVertexBuffer();

private:
	DirectX12* m_pDx12;
	ID3D12Device* m_pDevice12;
	ID3D12GraphicsCommandList* m_pCmdList12;

	std::unique_ptr<RectTransform> m_pRectTransform;

	std::unordered_map<DirectX::XMFLOAT2, ID3D12Resource*, HASH_D3DXVECTER2, EQUAL_XMFLOAT2> m_pCashVertexBuffers;
	ID3D12Resource* m_pConstantBuffer;	//コンスタントバッファ.
	ID3D12Resource* m_pTexture;			//テクスチャ.
	ID3D12Resource* m_pSampleLinear;	//DX12移行互換用（未使用）.

	std::string m_ResourceName;			//使用している資源の名前.
	DirectX::XMMATRIX m_WorldMatrix;	//ワールド行列.
	DirectX::XMFLOAT2 m_DrawSize;		//表示幅、高さ.
	DirectX::XMFLOAT4 m_Color;			//色（R,G,B,A）.
};
