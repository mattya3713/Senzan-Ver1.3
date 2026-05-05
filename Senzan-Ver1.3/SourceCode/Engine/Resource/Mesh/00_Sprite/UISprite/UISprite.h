#pragma once

#include <filesystem>
#include <unordered_map>
#include <d3d12.h>

#include "Engine/Utility/Transform/RectTransform.h"

#pragma warning(disable:4005)
#define ALIGN16	_declspec( align(16) )

class DirectX12;

class Sprite2D
{
public:

	//鬆らせ縺ｮ讒矩菴・
	struct VERTEX
	{
		DirectX::XMFLOAT3 pos;	//鬆らせ蠎ｧ讓・
		DirectX::XMFLOAT2 tex;	//繝・け繧ｹ繝√Ε蠎ｧ讓・
	};

	//蟷・ｫ倥＆讒矩菴・
	struct WHSIZE
	{
		float w;
		float h;
	};

	//繧ｹ繝励Λ繧､繝域ｧ矩菴・
	struct SPRITE_STATE
	{
		WHSIZE Disp;	//陦ｨ遉ｺ蟷・ｫ倥＆.
		WHSIZE Base;	//蜈・判蜒丞ｹ・ｫ倥＆.
		WHSIZE Stride;	//1繧ｳ繝槭≠縺溘ｊ縺ｮ蟷・ｫ倥＆.
	};

	//繧ｳ繝ｳ繧ｹ繧ｿ繝ｳ繝医ヰ繝・ヵ繧｡縺ｮ繧｢繝励Μ蛛ｴ縺ｮ螳夂ｾｩ.
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16	DirectX::XMMATRIX mWorld;		//繝ｯ繝ｼ繝ｫ繝芽｡悟・.
		ALIGN16	DirectX::XMFLOAT4 vColor;		//繧ｫ繝ｩ繝ｼ・・GBA縺ｮ蝙九↓蜷医ｏ縺帙ｋ・・
		ALIGN16	DirectX::XMFLOAT4 vUV;			//UV蠎ｧ讓呻ｼ・,y縺ｮ縺ｿ菴ｿ逕ｨ・・
		ALIGN16	float fViewPortWidth;			//繝薙Η繝ｼ繝昴・繝亥ｹ・
		ALIGN16	float fViewPortHeight;			//繝薙Η繝ｼ繝昴・繝磯ｫ倥＆.
		ALIGN16	DirectX::XMFLOAT2 DawSize;	//謠冗判蟷・ｫ倥＆.
	};

	// DirectX::XMFLOAT2繧偵く繝ｼ縺ｨ縺励※菴ｿ逕ｨ縺吶ｋ縺溘ａ縺ｮ繝上ャ繧ｷ繝･.
	struct HASH_D3DXVECTER2
	{
		size_t operator()(const DirectX::XMFLOAT2& Key) const
		{
			return std::hash<float>()(Key.x) ^ std::hash<float>()(Key.y);
		}
	};

	// DirectX::XMFLOAT2繧呈ｯ碑ｼ・☆繧九◆繧√・遲我ｾ｡豈碑ｼ・未謨ｰ.
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
	* @brief 蛻晄悄蛹・
	* @param FilePath・夂判蜒上・繝輔ぃ繧､繝ｫ繝代せ.
	*********************************************************/
	bool Initialize(const std::filesystem::path& FilePath);

	/*********************************************************
	* @brief 謠冗判.
	*********************************************************/
	void Render();

public: // Getter縲ヾetter.

	/*********************************************************
	* @brief RectTransform繧貞叙蠕・
	*********************************************************/
	const std::unique_ptr<RectTransform>& GetRectTransform() const;

	/*********************************************************
	* @brief 謠冗判蟷・・ｫ倥＆繧定ｨｭ螳・
	*********************************************************/
	void SetDrawSize(const DirectX::XMFLOAT2& DrawSize);

	/*********************************************************
	* @brief 濶ｲ繧定ｨｭ螳・
	*********************************************************/
	void SetColor(const DirectX::XMFLOAT4& Color);

	/*********************************************************
	* @brief 雉・ｺ仙錐繧貞叙蠕・
	*********************************************************/
	const std::string& GetResourceName() const;

	/*********************************************************
	* @brief 雉・ｺ仙錐繧定ｨｭ螳・
	*********************************************************/
	void SetResourceName(const std::string& Name);

	/*********************************************************
	* @brief 繝・け繧ｹ繝√Ε繧貞叙蠕・
	*********************************************************/
	ID3D12Resource* GetTexture();

private:

	/*********************************************************
	* @brief 逕ｻ蜒上し繧､繧ｺ縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ.
	*********************************************************/
	void LoadImageSize(const std::filesystem::path& FilePath);

	/*********************************************************
	* @brief 繧ｷ繧ｧ繝ｼ繝繝ｼ菴懈・.
	*********************************************************/
	void CreateShader();

	/*********************************************************
	* @brief 繝｢繝・Ν菴懈・.
	*********************************************************/
	HRESULT CreateModel();

	/*********************************************************
	* @brief 繝・け繧ｹ繝√Ε菴懈・.
	*********************************************************/
	HRESULT CreateTexture(const std::wstring& FilePath);

	/*********************************************************
	* @brief 繧ｵ繝ｳ繝励Λ菴懈・.
	*********************************************************/
	HRESULT CreateSampler();

	/*********************************************************
	* @brief 繝ｯ繝ｼ繝ｫ繝芽｡悟・繧定ｨ育ｮ・
	*********************************************************/
	void CalcWorldMatrix();

	/*********************************************************
	* @brief 謠冗判縺ｫ菴ｿ逕ｨ縺吶ｋ鬆らせ繝舌ャ繝輔ぃ繧貞叙蠕・
	*********************************************************/
	ID3D12Resource* GetUseVertexBuffer();

private:
	DirectX12* m_pDx12;
	ID3D12Device* m_pDevice12;
	ID3D12GraphicsCommandList* m_pCmdList12;

	std::unique_ptr<RectTransform> m_pRectTransform;

	std::unordered_map<DirectX::XMFLOAT2, ID3D12Resource*, HASH_D3DXVECTER2, EQUAL_XMFLOAT2> m_pCashVertexBuffers;
	ID3D12Resource* m_pConstantBuffer;	//繧ｳ繝ｳ繧ｹ繧ｿ繝ｳ繝医ヰ繝・ヵ繧｡.
	ID3D12Resource* m_pTexture;			//繝・け繧ｹ繝√Ε.
	ID3D12Resource* m_pSampleLinear;	//DX12遘ｻ陦御ｺ呈鋤逕ｨ・域悴菴ｿ逕ｨ・・

	std::string m_ResourceName;			//菴ｿ逕ｨ縺励※縺・ｋ雉・ｺ舌・蜷榊燕.
	DirectX::XMMATRIX m_WorldMatrix;	//繝ｯ繝ｼ繝ｫ繝芽｡悟・.
	DirectX::XMFLOAT2 m_DrawSize;		//陦ｨ遉ｺ蟷・・ｫ倥＆.
	DirectX::XMFLOAT4 m_Color;			//濶ｲ・・,G,B,A・・
};

