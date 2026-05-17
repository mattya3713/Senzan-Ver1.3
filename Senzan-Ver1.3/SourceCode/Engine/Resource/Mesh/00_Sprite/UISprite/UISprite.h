#pragma once

#include <filesystem>
#include <unordered_map>
#include <d3d12.h>

#include "Engine/Utility/Transform/RectTransform.h"

#pragma warning(disable:4005)
#define ALIGN16	_declspec( align(16) )

class DirectX12;

class UISprite
{
public:

	// 頂点の構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3 pos;	// 頂点座標.
		DirectX::XMFLOAT2 tex;	// テクスチャ座標.
	};

	// 幅・高さ構造体.
	struct WHSIZE
	{
		float w;
		float h;
	};

	// スプライト状態構造体.
	struct SPRITE_STATE
	{
		WHSIZE Disp;	// 表示幅 / 高さ.
		WHSIZE Base;	// 画像本体幅 / 高さ.
		WHSIZE Stride;	// 1コマあたりの幅 / 高さ.
	};

	// コンスタントバッファのアプリ側の設定
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16	DirectX::XMMATRIX mWorld;		// ワールド行列.
		ALIGN16	DirectX::XMFLOAT4 vColor;		// カラー.
		ALIGN16	DirectX::XMFLOAT4 vUV;			// UV座標(x,y のみ使用).
		ALIGN16	float fViewPortWidth;			// ビューポート幅.
		ALIGN16	float fViewPortHeight;			// ビューポート高さ.
		ALIGN16	DirectX::XMFLOAT2 DawSize;		// 描画幅 / 高さ.
	};

	// DirectX::XMFLOAT2 をキーとして使用するためのハッシュ.
	struct HASH_D3DXVECTER2
	{
		size_t operator()(const DirectX::XMFLOAT2& Key) const
		{
			return std::hash<float>()(Key.x) ^ std::hash<float>()(Key.y);
		}
	};

	// DirectX::XMFLOAT2 を比較するための比較演算子関数.
	struct EQUAL_XMFLOAT2
	{
		bool operator()(const DirectX::XMFLOAT2& Left, const DirectX::XMFLOAT2& Right) const
		{
			return Left.x == Right.x && Left.y == Right.y;
		}
	};

public:
	UISprite();
	~UISprite();

	/*********************************************************
	* @brief 初期化.
	* @param FilePath・画像ファイルパス.
	*********************************************************/
	bool Initialize(const std::filesystem::path& FilePath);

	// 描画.
	void Render();

public: // Getter、Setter.
	
	const std::unique_ptr<RectTransform>& GetRectTransform() const;

	// 描画幅 / 高さを設定.
	void SetDrawSize(const DirectX::XMFLOAT2& DrawSize);

	// 色を設定.
	void SetColor(const DirectX::XMFLOAT4& Color);

	// リソース名を設定.
	const std::string& GetResourceName() const;
	void SetResourceName(const std::string& Name);

	// テクスチャを取得.
	ID3D12Resource* GetTexture();

private:

	// 画像サイズの読み込み.
	void LoadImageSize(const std::filesystem::path& FilePath);

	//  シェーダー作成.
	void CreateShader();

	// モデル作成
	HRESULT CreateModel();

		
	// テクスチャ作成.
	HRESULT CreateTexture(const std::wstring& FilePath);

	// サンプラー作成.
	HRESULT CreateSampler();

	// ワールド行列の計算.
	void CalcWorldMatrix();

	// 描画に使用する頂点バッファを取得.
	ID3D12Resource* GetUseVertexBuffer();

private:
	DirectX12* m_pDx12;
	ID3D12Device* m_pDevice12;
	ID3D12GraphicsCommandList* m_pCmdList12;

	std::unique_ptr<RectTransform> m_pRectTransform;

	std::unordered_map<DirectX::XMFLOAT2, ID3D12Resource*, HASH_D3DXVECTER2, EQUAL_XMFLOAT2> m_pCashVertexBuffers;
	ID3D12Resource* m_pConstantBuffer;	// コンスタントバッファ.
	ID3D12Resource* m_pTexture;			// テクスチャ.
	ID3D12Resource* m_pSampleLinear;	// DX12 移動用.

	std::string m_ResourceName;			// 使用している リソースの名前.
	DirectX::XMMATRIX m_WorldMatrix;	// ワールド行列.
	DirectX::XMFLOAT2 m_DrawSize;		// 表示幅 / 高さ.
	DirectX::XMFLOAT4 m_Color;			// 色（R,G,B,A）.
};
