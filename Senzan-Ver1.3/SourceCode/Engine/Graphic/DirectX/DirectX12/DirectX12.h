#pragma once
#pragma warning(disable:4005)

#include "Engine/Utility/ComPtr/ComPtr.h"
#include "Engine/Utility/String/FilePath/FilePath.h"
#include "Engine/Utility/Assert/Assert.inl"

// Header includes.
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>


#include <d3d12.h>
#include "..\..\..\..\..\Data\Library\DirectXTex\Common\d3dx12.h"
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "..\..\..\..\..\Data\Library\DirectXTex\DirectXTex\DirectXTex.h"
#include <d3dcompiler.h>
#include "..\..\..\..\..\Data\Library\Effekseer\include\Effekseer.h"

namespace DirectX
{
	struct TexMetadata;
	class ScratchImage;
}

// ライブラリ宣言
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib") 
#pragma comment(lib, "DirectXTex.lib") 
#pragma comment(lib, "dxguid.lib") 

/**********************************************************
* @author      : mattya3713.
* @date        : 2026/02/18.
* @brief       : DirectX12 初期化とセットアップ.
**********************************************************/

class DirectX12
{
public:

	// HLSL用マテリアルデータ.
	struct MaterialForHlsl {
		DirectX::XMFLOAT3	Diffuse;	// 拡散色.		
		float				Alpha;		// 透明度.		
		DirectX::XMFLOAT3	Specular;	// 鏡面色.		
		float				Specularity;// 鏡面強度.		
		DirectX::XMFLOAT3	Ambient;	// 環境光色.

		MaterialForHlsl()
			: Diffuse		(0.0f, 0.0f, 0.0f)
			, Alpha			(0.0f)
			, Specular		(0.0f, 0.0f, 0.0f)
			, Specularity	(0.0f)
			, Ambient		(0.0f, 0.0f, 0.0f)
		{}
	};

	// 追加マテリアルデータ.
	struct AdditionalMaterial {
		std::string TexPath;	// テクスチャファイルパス.
		int			ToonIdx;	// トゥーンインデックス.
		bool		EdgeFlg;	// マテリアルエッジフラグ.

		AdditionalMaterial()
			: TexPath		{}
			, ToonIdx		(0)
			, EdgeFlg		(false)
		{}
	};

	// マテリアルデータ統合
	struct Material {
		unsigned int		IndicesNum;		// インデックス数.
		MaterialForHlsl		Materialhlsl;	// HLSL用マテリアル.
		AdditionalMaterial	Additional;		// 追加マテリアルデータ.

		Material()
			: IndicesNum	(0)
			, Materialhlsl	{}
			, Additional	{}
		{}
	};

	// シーンデータ構造体.
	struct SceneData {
		DirectX::XMMATRIX View;	// ビュー行列.
		DirectX::XMMATRIX Proj;	// プロジェクション行列.
		DirectX::XMFLOAT3 Eye;	// カメラ位置.
	};

public:
	DirectX12();
	~DirectX12();

	// DirectX12初期化
	bool Create(HWND hWnd);
	void Update();
	void UpdateSceneBuffer();

	void BeginDraw();
	void EndDraw();

	MyComPtr<IDXGISwapChain4> const GetSwapChain()				{ return m_pSwapChain; };
	MyComPtr<ID3D12Device> const GetDevice()					{ return m_pDevice12; };
	MyComPtr<ID3D12GraphicsCommandList> const GetCommandList()	{ return m_pCmdList; };
	MyComPtr<ID3D12CommandQueue> const GetCommandQueue()		{ return m_pCmdQueue; };
	ID3D12Resource* GetSceneConstantBuffer() const				{ return m_pSceneConstBuff.Get(); }
	SceneData* GetMappedSceneData() const						{ return m_pMappedSceneData; }
	MyComPtr<ID3D12PipelineState> GetPipelineState() const		{ return m_pPipelineState; }
	MyComPtr<ID3D12RootSignature> GetRootSignature() const		{ return m_pRootSignature; }
	std::unique_ptr<D3D12_VIEWPORT>* GetViewport()				{ return &m_pViewport; }
	std::unique_ptr<D3D12_RECT>* GetScissorRect()				{ return &m_pScissorRect; }

	// パスからテクスチャを取得(パスが未定義の場合生成を行う).
	MyComPtr<ID3D12Resource> GetTextureByPath(const char* texpath);

	// GPU処理完了を待機.
	void WaitForGPU();

	/*******************************************
	* @brief	エフェクトの描画.
	* @param Effect   : Effekseer::EffectRef（リソースマネージャー経由で取得）.
	* @param Position : 描画位置.
	* @param Rotation : 回転情報.
	* @param Scale	  : スケール.
	*******************************************/
	void DrawEffect(
		::Effekseer::EffectRef Effect,
		const DirectX::XMFLOAT3& Position, 
		const DirectX::XMFLOAT3& Rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 
		const DirectX::XMFLOAT3& Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));

	void DrawSprite(ID3D12Resource* Texture, const DirectX::XMFLOAT2& Position, const DirectX::XMFLOAT4& Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), float Width = 100.0f, float Height = 100.0f);

	void DrawFBXModel(class FBXModel* pModel);

private:

	// DXGIファクトリーを作成.
	void CreateDXGIFactory(MyComPtr<IDXGIFactory6>& DxgiFactory);

	// コマンドオブジェクトを作成.
	void CreateCommandObject(
		MyComPtr<ID3D12CommandAllocator>&	CmdAllocator,
		MyComPtr<ID3D12GraphicsCommandList>&CmdList,
		MyComPtr<ID3D12CommandQueue>&		CmdQueue);

	// スワップチェーンを作成.
	void CreateSwapChain(MyComPtr<IDXGISwapChain4>& SwapChain);

	// レンダーターゲットを作成.
	void CreateRenderTarget(
		MyComPtr<ID3D12DescriptorHeap>&			RenderTargetViewHeap,
		std::vector<MyComPtr<ID3D12Resource>>&	BackBuffer);

	// 深度ディスクリプタを作成.
	void CreateDepthDesc(
		MyComPtr<ID3D12Resource>&		DepthBuffer,
		MyComPtr<ID3D12DescriptorHeap>&	DepthHeap,
		MyComPtr<ID3D12DescriptorHeap>&	DepthSRVHeap);

	// シーンディスクリプタを作成.
	void CreateSceneDesc();

	// PSOとルートシグネチャを作成.
	bool CreatePipelineStateAndRootSignature();
	
	// フェンスを作成.
	void CreateFance(MyComPtr<ID3D12Fence>& Fence);



private:
	/*******************************************
	* @brief	名前でアダプターを検索.
	* @param	検索ワード.
	* @return   見つかったアダプターポインター.
	*******************************************/
	IDXGIAdapter* FindAdapter(std::wstring FindWord);

	/*******************************************
	* @brief	ファイルからテクスチャバッファを作成してデータをコピー.
	* @param	ファイルパス.
	* @param	リソースポインター戻り値.
	*******************************************/
	MyComPtr<ID3D12Resource> CreateTextureFromFile(const char* Texpath);

	// デバッグレイヤーを有効化.
	void EnableDebuglayer();
	// テクスチャロード関数テーブルを作成.
	void CreateTextureLoadTable();


private:
	HWND m_hWnd;	// ウィンドウハンドル

	// DXGI
	MyComPtr<IDXGIFactory6>					m_pDxgiFactory;			// ディスプレイAPIファクトリー.
	MyComPtr<IDXGISwapChain4>				m_pSwapChain;			// スワップチェーン.
	DXGI_SWAP_CHAIN_DESC1                   m_SwapChainDesc;		// スワップチェーンディスクリプタ.

	// DirectX12
	MyComPtr<ID3D12Device>					m_pDevice12;			// DirectX12デバイスコンテキスト.
	MyComPtr<ID3D12CommandAllocator>		m_pCmdAllocator;		// コマンドアロケーター(コマンドのメモリを割り当て).
	MyComPtr<ID3D12GraphicsCommandList>		m_pCmdList;				// コマンドリスト.
	MyComPtr<ID3D12CommandQueue>			m_pCmdQueue;			// コマンドキュー.

	// レンダーターゲット.
	MyComPtr<ID3D12DescriptorHeap>			m_pRenderTargetViewHeap;// レンダーターゲットビューヒープ.
	std::vector<MyComPtr<ID3D12Resource>>	m_pBackBuffer;			// バックバッファー.

	// 深度バッファー.
	MyComPtr<ID3D12Resource>				m_pDepthBuffer;			// 深度バッファー.
	MyComPtr<ID3D12DescriptorHeap>			m_pDepthHeap;			// 深度SRV.
	MyComPtr<ID3D12DescriptorHeap>			m_pDepthSRVHeap;		// 深度SRVディスクリプタヒープ.
	D3D12_CLEAR_VALUE						m_DepthClearValue;		// 深度クリア値.

	// シーン定数バッファー.
	MyComPtr<ID3D12Resource>				m_pSceneConstBuff;		// シーン定数バッファーリソース.
	SceneData*								m_pMappedSceneData;		// シーン定数バッファーCPUマップポインター.

	// フェンス.
	MyComPtr<ID3D12Fence>					m_pFence;				// 処理同期フラグ.
	UINT64									m_FenceValue;			// 処理カウンター.
	HANDLE									m_hFenceEvent;			// フェンスイベントハンドル.

	// レンダリング設定.
	MyComPtr<ID3D12PipelineState>			m_pPipelineState;		// パイプラインステート.
	MyComPtr<ID3D12RootSignature>			m_pRootSignature;		// ルートシグネチャ
	std::unique_ptr<D3D12_VIEWPORT>			m_pViewport;			// ビューポート.
	std::unique_ptr<D3D12_RECT>				m_pScissorRect;			// シザー矩形.

	using LoadLambda_t = std::function<HRESULT(const std::wstring& Path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLambda_t>		m_LoadLambdaTable;

	// ファイルパスとリソースのマッピングテーブル.
	std::map<std::string, MyComPtr<ID3D12Resource>>	m_ResourceTable;

};
