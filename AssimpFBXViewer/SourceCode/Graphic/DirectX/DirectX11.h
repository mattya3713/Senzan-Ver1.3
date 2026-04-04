#pragma once

// ビット演算に関する警告を無効化する. 4005: 再定義.
#pragma warning(disable:4005)

// Windows.h の min/max マクロを無効化.
#ifndef NOMINMAX
#define NOMINMAX
#endif

// ヘッダ読み込み.
#include <Windows.h>
#include <tchar.h>
#include <crtdbg.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

// ライブラリ読み込み.
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


/**************************************************
* DirectX11 セットアップ.
**/
class DirectX11
{
public:
	DirectX11();
	~DirectX11();

	HRESULT Create(HWND HWnd);
	void Release();

	void SetDepth(bool IsEnable);
	void SetAlphaBlend(bool IsEnable);

	void ClearBackBuffer();
	void Present();

	ID3D11Device* GetDevice() const { return m_pDevice11; }
	ID3D11DeviceContext* GetContext() const { return m_pContext11; }

private:
	HRESULT CreateDeviceAndSwapChain(HWND HWnd, UINT FPS, UINT Width, UINT Height);

	HRESULT CreateColorBackBufferRTV();
	HRESULT CreateDepthStencilBackBufferRTV();
	HRESULT CreateRasterizer();
	HRESULT CreateDepthStencilState();

	HRESULT CreateAlphaBlendState();

private:
	ID3D11Device*				m_pDevice11;			// デバイスオブジェクト.
	ID3D11DeviceContext*		m_pContext11;			// デバイスコンテキスト.
	IDXGISwapChain*				m_pSwapChain;			// スワップチェーン.
	ID3D11RenderTargetView*		m_pBackBuffer_TexRTV;	// レンダーターゲットビュー.
	ID3D11Texture2D*			m_pBackBuffer_DSTex;		// 深度ステンシル用テクスチャ.
	ID3D11DepthStencilView*		m_pBackBuffer_DSTexDSV;	// 深度ステンシルビュー.

	// 深度ステンシル状態.
	ID3D11DepthStencilState*	m_pDepthStencilStateOn;		// 有効.
	ID3D11DepthStencilState*	m_pDepthStencilStateOff;	// 無効.

	// アルファブレンド.
	ID3D11BlendState*		m_pAlphaBlendOn;		// 有効.
	ID3D11BlendState*		m_pAlphaBlendOff;	// 無効.
};