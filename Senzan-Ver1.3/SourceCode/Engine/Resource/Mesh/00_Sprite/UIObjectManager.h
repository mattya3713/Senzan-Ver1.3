#pragma once

//隴ｦ蜻翫↓縺､縺・※縺ｮ繧ｳ繝ｼ繝牙・譫舌ｒ辟｡蜉ｹ縺ｫ縺吶ｋ.4005:蜀榊ｮ夂ｾｩ.
#pragma warning(disable:4005)

#include <d3d12.h>

//蜑肴婿螳｣險.
class DirectX12;

/**************************************************
*	繧ｹ繝励Λ繧､繝・D繧ｯ繝ｩ繧ｹ.
**/
class WorldSprite
{
public:
	//======================================.
	//	讒矩菴・
	//======================================.
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
		WHSIZE Stride;	//・代さ繝槭≠縺溘ｊ縺ｮ蟷・ｫ倥＆.
	};

	//繧ｳ繝ｳ繧ｹ繧ｿ繝ｳ繝医ヰ繝・ヵ繧｡縺ｮ繧｢繝励Μ蛛ｴ縺ｮ螳夂ｾｩ.
	//窶ｻ繧ｷ繧ｧ繝ｼ繝蜀・・繧ｳ繝ｳ繧ｹ繧ｿ繝ｳ繝医ヰ繝・ヵ繧｡縺ｨ荳閾ｴ縺励※縺・ｋ蠢・ｦ√≠繧・
	struct SHADER_CONSTANT_BUFFER
	{
		DirectX::XMMATRIX	mWVP;		//繝ｯ繝ｼ繝ｫ繝・繝薙Η繝ｼ,繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ縺ｮ蜷域・螟画鋤陦悟・.	
		DirectX::XMFLOAT4	vColor;		//繧ｫ繝ｩ繝ｼ・・GBA縺ｮ蝙九↓蜷医ｏ縺帙ｋ・・
		DirectX::XMFLOAT4	vUV;		//UV蠎ｧ讓呻ｼ・,y縺ｮ縺ｿ菴ｿ逕ｨ・・
	};
	//鬆らせ縺ｮ讒矩菴・
	struct VERTEX
	{
		DirectX::XMFLOAT3 Pos;	//鬆らせ蠎ｧ讓・
		DirectX::XMFLOAT2	Tex;	//繝・け繧ｹ繝√Ε蠎ｧ讓・
	};

public:
	WorldSprite();	//繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ.
	~WorldSprite();	//繝・せ繝医Λ繧ｯ繧ｿ.

	//蛻晄悄蛹・
	HRESULT Init(DirectX12& pDx12, LPCTSTR pFileName, SPRITE_STATE& pSs);

	//隗｣謾ｾ.
	void Release();

	//繧ｷ繧ｧ繝ｼ繝菴懈・.
	HRESULT CreateShader();
	//繝｢繝・Ν菴懈・.
	HRESULT CreateModel();
	//繝・け繧ｹ繝√Ε菴懈・.
	HRESULT CreateTexture(LPCTSTR pFileName);
	//繧ｵ繝ｳ繝励Λ菴懈・.
	HRESULT CreateSampler();

	//繝ｬ繝ｳ繝繝ｪ繝ｳ繧ｰ逕ｨ.
	void Render();

	//蠎ｧ讓呎ュ蝣ｱ繧定ｨｭ螳・
	void SetPosition(const DirectX::XMFLOAT3& vPos) { m_Position = vPos; }
	//蠎ｧ讓凅繧定ｨｭ螳・
	void SetPositionX(float x) { m_Position.x = x; }
	//蠎ｧ讓凉繧定ｨｭ螳・
	void SetPositionY(float y) { m_Position.y = y; }
	//蠎ｧ讓凛繧定ｨｭ螳・
	void SetPositionZ(float z) { m_Position.z = z; }

	//蝗櫁ｻ｢諠・ｱ繧定ｨｭ螳・
	void SetRotation(const DirectX::XMFLOAT3& vRot) { m_Rotation = vRot; }
	//蝗櫁ｻ｢霆ｸY繧定ｨｭ螳・Yaw).
	void SetRotationY(float y) { m_Rotation.y = y; }
	//蝗櫁ｻ｢霆ｸX繧定ｨｭ螳・Pitch).
	void SetRotationX(float x) { m_Rotation.x = x; }
	//蝗櫁ｻ｢霆ｸZ繧定ｨｭ螳・Roll).
	void SetRotationZ(float z) { m_Rotation.z = z; }

	//諡｡邵ｮ諠・ｱ繧定ｨｭ螳・
	void SetScale(const DirectX::XMFLOAT3& vScale) { m_Scale = vScale; }

	//ﾎｱ蛟､繧定ｨｭ螳・
	void SetAlpha(float alpha) { m_Alpha = alpha; }

	//繝代ち繝ｼ繝ｳ逡ｪ蜿ｷ(繝槭せ逶ｮ)繧定ｨｭ螳・
	void SetPatternNo(SHORT x, SHORT y)
	{
		m_PatternNo.x = x;
		m_PatternNo.y = y;
	}
	//譛螟ｧ繝代ち繝ｼ繝ｳ謨ｰ(繝槭せ逶ｮ)繧貞叙蠕・
	POINTS GetPatternMax() const { return m_PatternMax; }

	//繝薙Ν繝懊・繝峨・ON/OFF蛻・ｊ譖ｿ縺・
	void SetBillboard(bool flag) { m_Billboard = flag; }

private:
	DirectX12* m_pDx12;
	ID3D12Device* m_pDevice12;
	ID3D12GraphicsCommandList* m_pCmdList12;

	ID3D12Resource* m_pConstantBuffer;	//繧ｳ繝ｳ繧ｹ繧ｿ繝ｳ繝医ヰ繝・ヵ繧｡.
	ID3D12Resource* m_pVertexBuffer;	//鬆らせ繝舌ャ繝輔ぃ.
	ID3D12Resource* m_pTexture;			//繝・け繧ｹ繝√Ε.

	DirectX::XMFLOAT3 m_Position;	//蠎ｧ讓・
	DirectX::XMFLOAT3 m_Rotation;	//蝗櫁ｻ｢.
	DirectX::XMFLOAT3 m_Scale;		//諡｡邵ｮ.

	DirectX::XMFLOAT2 m_UV;		//繝・け繧ｹ繝√ΕUV蠎ｧ讓・

	float m_Alpha;	//ﾎｱ蛟､(0:騾乗・縲・:螳悟・荳埼乗・).

	SPRITE_STATE m_SpriteState;	//繧ｹ繝励Λ繧､繝域ュ蝣ｱ.
	POINTS m_PatternNo;	//繝代ち繝ｼ繝ｳ逡ｪ蜿ｷ(繝槭せ逶ｮ).
	POINTS m_PatternMax;	//譛螟ｧ繝代ち繝ｼ繝ｳ(繝槭せ縺ｮ譛螟ｧ蛟､).

	bool m_Billboard;	//繝薙Ν繝懊・繝碓N/OFF.
};

