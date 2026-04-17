#pragma once

//警告についてのコード分析を無効にする.4005:再定義.
#pragma warning(disable:4005)

#include <d3d12.h>

//前方宣言.
class DirectX12;

/**************************************************
*	スプライト3Dクラス.
**/
class WorldSprite
{
public:
	//======================================.
	//	構造体.
	//======================================.
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
		WHSIZE Stride;	//１コマあたりの幅高さ.
	};

	//コンスタントバッファのアプリ側の定義.
	//※シェーダ内のコンスタントバッファと一致している必要あり.
	struct SHADER_CONSTANT_BUFFER
	{
		DirectX::XMMATRIX	mWVP;		//ワールド,ビュー,プロジェクションの合成変換行列.	
		DirectX::XMFLOAT4	vColor;		//カラー（RGBAの型に合わせる）.
		DirectX::XMFLOAT4	vUV;		//UV座標（x,yのみ使用）.
	};
	//頂点の構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3 Pos;	//頂点座標.
		DirectX::XMFLOAT2	Tex;	//テクスチャ座標.
	};

public:
	WorldSprite();	//コンストラクタ.
	~WorldSprite();	//デストラクタ.

	//初期化.
	HRESULT Init(DirectX12& pDx12, LPCTSTR pFileName, SPRITE_STATE& pSs);

	//解放.
	void Release();

	//シェーダ作成.
	HRESULT CreateShader();
	//モデル作成.
	HRESULT CreateModel();
	//テクスチャ作成.
	HRESULT CreateTexture(LPCTSTR pFileName);
	//サンプラ作成.
	HRESULT CreateSampler();

	//レンダリング用.
	void Render();

	//座標情報を設定.
	void SetPosition(const DirectX::XMFLOAT3& vPos) { m_Position = vPos; }
	//座標xを設定.
	void SetPositionX(float x) { m_Position.x = x; }
	//座標yを設定.
	void SetPositionY(float y) { m_Position.y = y; }
	//座標zを設定.
	void SetPositionZ(float z) { m_Position.z = z; }

	//回転情報を設定.
	void SetRotation(const DirectX::XMFLOAT3& vRot) { m_Rotation = vRot; }
	//回転軸Yを設定(Yaw).
	void SetRotationY(float y) { m_Rotation.y = y; }
	//回転軸Xを設定(Pitch).
	void SetRotationX(float x) { m_Rotation.x = x; }
	//回転軸Zを設定(Roll).
	void SetRotationZ(float z) { m_Rotation.z = z; }

	//拡縮情報を設定.
	void SetScale(const DirectX::XMFLOAT3& vScale) { m_Scale = vScale; }

	//α値を設定.
	void SetAlpha(float alpha) { m_Alpha = alpha; }

	//パターン番号(マス目)を設定.
	void SetPatternNo(SHORT x, SHORT y)
	{
		m_PatternNo.x = x;
		m_PatternNo.y = y;
	}
	//最大パターン数(マス目)を取得.
	POINTS GetPatternMax() const { return m_PatternMax; }

	//ビルボードのON/OFF切り替え.
	void SetBillboard(bool flag) { m_Billboard = flag; }

private:
	DirectX12* m_pDx12;
	ID3D12Device* m_pDevice12;
	ID3D12GraphicsCommandList* m_pCmdList12;

	ID3D12Resource* m_pConstantBuffer;	//コンスタントバッファ.
	ID3D12Resource* m_pVertexBuffer;	//頂点バッファ.
	ID3D12Resource* m_pTexture;			//テクスチャ.

	DirectX::XMFLOAT3 m_Position;	//座標.
	DirectX::XMFLOAT3 m_Rotation;	//回転.
	DirectX::XMFLOAT3 m_Scale;		//拡縮.

	DirectX::XMFLOAT2 m_UV;		//テクスチャUV座標.

	float m_Alpha;	//α値(0:透明、1:完全不透明).

	SPRITE_STATE m_SpriteState;	//スプライト情報.
	POINTS m_PatternNo;	//パターン番号(マス目).
	POINTS m_PatternMax;	//最大パターン(マスの最大値).

	bool m_Billboard;	//ビルボードON/OFF.
};
