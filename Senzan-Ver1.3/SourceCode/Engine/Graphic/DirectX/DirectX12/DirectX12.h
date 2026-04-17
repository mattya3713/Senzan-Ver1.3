#pragma once
#pragma warning(disable:4005)

#include "Engine/Utility/ComPtr/ComPtr.h"
#include "Engine/Utility/String/FilePath/FilePath.h"
#include "Engine/Utility/Assert/Assert.inl"

// 繝倥ャ繝隱ｭ霎ｼ.
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

namespace DirectX
{
	struct TexMetadata;
	class ScratchImage;
}

//繝ｩ繧､繝悶Λ繝ｪ隱ｭ縺ｿ霎ｼ縺ｿ.
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib") 
#pragma comment(lib, "DirectXTex.lib") 
#pragma comment(lib, "dxguid.lib") 

// 繝｢繝・Ν縺ｮ鬆らせ繧ｵ繧､繧ｺ.
constexpr size_t PmdVertexSize = 38;

/**********************************************************
* @author      : mattya3713.
* @date        : 2025/02/18.
* @brief       : DirectX12繧ｻ繝・ヨ繧｢繝・・.
**********************************************************/

class DirectX12
{
public:

	// 繧ｷ繧ｧ繝ｼ繝蛛ｴ縺ｫ謚輔￡繧峨ｌ繧九・繝・Μ繧｢繝ｫ繝・・繧ｿ.
	struct MaterialForHlsl {
		DirectX::XMFLOAT3	Diffuse;	// 繝・ぅ繝輔Η繝ｼ繧ｺ濶ｲ.		
		float				Alpha;		// ﾎｱ蛟､.		
		DirectX::XMFLOAT3	Specular;	// 繧ｹ繝壹く繝･繝ｩ縺ｮ蠑ｷ.		
		float				Specularity;// 繧ｹ繝壹く繝･繝ｩ濶ｲ.		
		DirectX::XMFLOAT3	Ambient;	// 繧｢繝ｳ繝薙お繝ｳ繝郁牡.		

		MaterialForHlsl()
			: Diffuse		(0.0f, 0.0f, 0.0f)
			, Alpha			(0.0f)
			, Specular		(0.0f, 0.0f, 0.0f)
			, Specularity	(0.0f)
			, Ambient		(0.0f, 0.0f, 0.0f)
		{}
	};

	// 縺昴ｌ莉･螟悶・繝槭ユ繝ｪ繧｢繝ｫ繝・・繧ｿ.
	struct AdditionalMaterial {
		std::string TexPath;	// 繝・け繧ｹ繝√Ε繝輔ぃ繧､繝ｫ繝代せ.
		int			ToonIdx;	// 繝医ぇ繝ｼ繝ｳ逡ｪ蜿ｷ.
		bool		EdgeFlg;	// 繝槭ユ繝ｪ繧｢繝ｫ豈弱・霈ｪ驛ｭ邱壹ヵ繝ｩ繧ｰ.

		AdditionalMaterial()
			: TexPath		{}
			, ToonIdx		(0)
			, EdgeFlg		(false)
		{}
	};

	// 縺ｾ縺ｨ繧√◆繧ゅ・.
	struct Material {
		unsigned int IndicesNum;		// 繧､繝ｳ繝・ャ繧ｯ繧ｹ謨ｰ.
		MaterialForHlsl Materialhlsl;	// 繧ｷ繧ｧ繝ｼ繝蛛ｴ縺ｫ謚輔￡繧峨ｌ繧九・繝・Μ繧｢繝ｫ繝・・繧ｿ.
		AdditionalMaterial Additional;	// 縺昴ｌ莉･螟悶・繝槭ユ繝ｪ繧｢繝ｫ繝・・繧ｿ.
		
		Material()
			: IndicesNum	(0)
			, Materialhlsl	{}
			, Additional	{}
		{}
	};

	// TODO : 莉ｮ繧ｷ繝ｼ繝ｳ繝・・繧ｿ.
	struct SceneData {
		DirectX::XMMATRIX view;//繝薙Η繝ｼ陦悟・
		DirectX::XMMATRIX proj;//繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ陦悟・
		DirectX::XMFLOAT3 eye;//隕也せ蠎ｧ讓・
	};

public:
	DirectX12();
	~DirectX12();

	//DirectX12讒狗ｯ・
	bool Create(HWND hWnd);
	void Update();
	void UpdateSceneBuffer();

	void BeginDraw();
	void EndDraw();

	// 繧ｹ繝ｯ繝・・繝√ぉ繝ｼ繝ｳ蜿門ｾ・
	const MyComPtr<IDXGISwapChain4> GetSwapChain();

	// DirextX12繝・ヰ繧､繧ｹ蜿門ｾ・
	const MyComPtr<ID3D12Device> GetDevice();

	// 繧ｳ繝槭Φ繝峨Μ繧ｹ繝亥叙蠕・
	const MyComPtr<ID3D12GraphicsCommandList> GetCommandList();

	// 繧ｳ繝槭Φ繝峨く繝･繝ｼ蜿門ｾ・
	const MyComPtr<ID3D12CommandQueue> GetCommandQueue();

	// 繝・け繧ｹ繝√Ε繧貞叙蠕・
	MyComPtr<ID3D12Resource> GetTextureByPath(const char* texpath);

	ID3D12Resource* GetSceneConstantBuffer() const { return m_pSceneConstBuff.Get(); }
	SceneData* GetMappedSceneData() const { return m_pMappedSceneData; }

	// PSO 縺ｨ Root Signature 蜿門ｾ・
	const MyComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pPipelineState; }
	const MyComPtr<ID3D12RootSignature> GetRootSignature() const { return m_pRootSignature; }
	std::unique_ptr<D3D12_VIEWPORT>* GetViewport() { return &m_pViewport; }
	std::unique_ptr<D3D12_RECT>* GetScissorRect() { return &m_pScissorRect; }

	// GPU縺ｮ螳御ｺ・ｾ・■.
	void WaitForGPU();

private:// 菴懊▲縺ｦ縺・￥繧薙□繧医・縺㍻.

	// DXGI縺ｮ逕滓・.
	void CreateDXGIFactory(MyComPtr<IDXGIFactory6>& DxgiFactory);

	// 繧ｳ繝槭Φ繝蛾｡槭・逕滓・.
	void CreateCommandObject(
		MyComPtr<ID3D12CommandAllocator>&	CmdAllocator,
		MyComPtr<ID3D12GraphicsCommandList>&CmdList,
		MyComPtr<ID3D12CommandQueue>&		CmdQueue);

	// 繧ｹ繝ｯ繝・・繝√ぉ繝ｼ繝ｳ縺ｮ菴懈・.
	void CreateSwapChain(MyComPtr<IDXGISwapChain4>& SwapChain);

	// 繝ｬ繝ｳ繝繝ｼ繧ｿ繝ｼ繧ｲ繝・ヨ縺ｮ菴懈・.
	void CreateRenderTarget(
		MyComPtr<ID3D12DescriptorHeap>&			RenderTargetViewHeap,
		std::vector<MyComPtr<ID3D12Resource>>&	BackBuffer);

	// 豺ｱ蠎ｦ繝舌ャ繝輔ぃ縺ｮ菴懈・.
	void CreateDepthDesc(
		MyComPtr<ID3D12Resource>&		DepthBuffer,
		MyComPtr<ID3D12DescriptorHeap>&	DepthHeap,
		MyComPtr<ID3D12DescriptorHeap>&	DepthSRVHeap);

	// 繧ｷ繝ｼ繝ｳ繝薙Η繝ｼ縺ｮ菴懈・.
	void CreateSceneDesc();

	// PSO 縺ｨ Root Signature 縺ｮ菴懈・.
	bool CreatePipelineStateAndRootSignature();

	// 繝輔ぉ繝ｳ繧ｹ縺ｮ菴懈・.
	void CreateFance(MyComPtr<ID3D12Fence>& Fence);



private:
	/*******************************************
	* @brief	繧｢繝繝励ち繝ｼ繧定ｦ九▽縺代ｋ.
	* @param	讀懃ｴ｢縺吶ｋ譁・ｭ怜・.
	* @return   隕九▽縺代◆繧｢繝繝励ち繝ｼ繧定ｿ斐☆.
	*******************************************/
	IDXGIAdapter* FindAdapter(std::wstring FindWord);

	/*******************************************
	* @brief	繝・ヰ繝・げ繝ｬ繧､繝､繝ｼ繧定ｵｷ蜍・
	*******************************************/
	void EnableDebuglayer();

	/*******************************************
	* @brief	繝・け繧ｹ繝√Ε蜷阪°繧峨ユ繧ｯ繧ｹ繝√Ε繝舌ャ繝輔ぃ菴懈・縲∽ｸｭ霄ｫ繧偵さ繝斐・縺吶ｋ.
	* @param	繝輔ぃ繧､繝ｫ繝代せ.
	* @param	繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ繝昴う繝ｳ繧ｿ繧定ｿ斐☆.
	*******************************************/
	MyComPtr<ID3D12Resource> CreateTextureFromFile(const char* Texpath);

	/*******************************************
	* @brief	 繝・け繧ｹ繝√Ε繝ｭ繝ｼ繝峨ユ繝ｼ繝悶Ν縺ｮ菴懈・.
	*******************************************/
	void CreateTextureLoadTable();


private:
	HWND m_hWnd;	// 繧ｦ繧｣繝ｳ繝峨え繝上Φ繝峨Ν.

	// DXGI.
	MyComPtr<IDXGIFactory6>					m_pDxgiFactory;			// 繝・ぅ繧ｹ繝励Ξ繧､縺ｫ蜃ｺ蜉帙☆繧九◆繧√・API.
	MyComPtr<IDXGISwapChain4>				m_pSwapChain;			// 繧ｹ繝ｯ繝・・繝√ぉ繝ｼ繝ｳ.
	DXGI_SWAP_CHAIN_DESC1                   m_SwapChainDesc;		// 繧ｹ繝ｯ繝・・繝√ぉ繝ｼ繝ｳ縺ｮ繝・ぅ繧ｹ繧ｯ繝ｪ繝励す繝ｧ繝ｳ.

	// DirectX12.
	MyComPtr<ID3D12Device>					m_pDevice12;			// DirectX12縺ｮ繝・ヰ繧､繧ｹ繧ｳ繝ｳ繝・く繧ｹ繝・
	MyComPtr<ID3D12CommandAllocator>		m_pCmdAllocator;		// 繧ｳ繝槭Φ繝峨い繝ｭ繧ｱ繝ｼ繧ｿ(蜻ｽ莉､繧偵◆繧√※縺翫￥繝｡繝｢繝ｪ鬆伜沺).	
	MyComPtr<ID3D12GraphicsCommandList>		m_pCmdList;				// 繧ｳ繝槭Φ繝峨Μ繧ｹ繝・
	MyComPtr<ID3D12CommandQueue>			m_pCmdQueue;			// 繧ｳ繝槭Φ繝峨く繝･繝ｼ.

	// 繝ｬ繝ｳ繝繝ｼ繧ｿ繝ｼ繧ｲ繝・ヨ.
	MyComPtr<ID3D12DescriptorHeap>			m_pRenderTargetViewHeap;// 繝ｬ繝ｳ繝繝ｼ繧ｿ繝ｼ繧ｲ繝・ヨ繝薙Η繝ｼ.
	std::vector<MyComPtr<ID3D12Resource>>	m_pBackBuffer;			// 繝舌ャ繧ｯ繝舌ャ繝輔ぃ.

	// 豺ｱ蠎ｦ繝舌ャ繝輔ぃ.
	MyComPtr<ID3D12Resource>				m_pDepthBuffer;			// 豺ｱ蠎ｦ繝舌ャ繝輔ぃ.
	MyComPtr<ID3D12DescriptorHeap>			m_pDepthHeap;			// 豺ｱ蠎ｦ繧ｹ繝・Φ繧ｷ繝ｫ繝薙Η繝ｼ. 
	MyComPtr<ID3D12DescriptorHeap>			m_pDepthSRVHeap;		// 豺ｱ蠎ｦ繧ｹ繝・Φ繧ｷ繝ｫ繝薙Η繝ｼ縺ｮ繝・せ繧ｯ繝ｪ繝励ち繝偵・繝・ 
	D3D12_CLEAR_VALUE						m_DepthClearValue;		// 豺ｱ蠎ｦ縺ｮ繧ｯ繝ｪ繧｢蛟､.

	//繧ｷ繝ｼ繝ｳ繧呈ｧ区・縺吶ｋ繝舌ャ繝輔ぃ縺ｾ繧上ｊ
	MyComPtr<ID3D12Resource>				m_pSceneConstBuff;		// 繧ｷ繝ｼ繝ｳ螳壽焚繝舌ャ繝輔ぃ縺ｮ繝ｪ繧ｽ繝ｼ繧ｹ
	SceneData*								m_pMappedSceneData;		// 繧ｷ繝ｼ繝ｳ螳壽焚繝舌ャ繝輔ぃ縺ｮCPU蛛ｴ繝槭ャ繝玲ｸ医∩繝昴う繝ｳ繧ｿ.

	// 繝輔ぉ繝ｳ繧ｹ鬘・
	MyComPtr<ID3D12Fence>					m_pFence;				// 蜃ｦ逅・ｾ・■譟ｵ.
	UINT64									m_FenceValue;			// 蜃ｦ逅・き繧ｦ繝ｳ繧ｿ繝ｼ.
	HANDLE									m_hFenceEvent;			// 繝輔ぉ繝ｳ繧ｹ繧､繝吶Φ繝医ワ繝ｳ繝峨Ν. 

	// 謠冗判蜻ｨ繧翫・險ｭ螳・
	MyComPtr<ID3D12PipelineState>			m_pPipelineState;		// 繝代う繝励Λ繧､繝ｳ.
	MyComPtr<ID3D12RootSignature>			m_pRootSignature;		// 繝ｫ繝ｼ繝医す繧ｰ繝阪メ繝｣.
	std::unique_ptr<D3D12_VIEWPORT>			m_pViewport;			// 繝薙Η繝ｼ繝昴・繝・
	std::unique_ptr<D3D12_RECT>				m_pScissorRect;			// 繧ｷ繧ｶ繝ｼ遏ｩ蠖｢.

	using LoadLambda_t = std::function<HRESULT(const std::wstring& Path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLambda_t>		m_LoadLambdaTable;

	// 繝輔ぃ繧､繝ｫ蜷阪ヱ繧ｹ縺ｨ繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ繝槭ャ繝励ユ繝ｼ繝悶Ν.
	std::map<std::string, MyComPtr<ID3D12Resource>>	m_ResourceTable;

};

