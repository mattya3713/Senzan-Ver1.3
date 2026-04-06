#pragma once

//Windows.hのmin/maxマクロを無効化.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <tchar.h>
#include <crtdbg.h>

//=============================================================================
// アプリケーション定数
//=============================================================================
constexpr float WND_W = 1280.0f;	// ウィンドウ幅
constexpr float WND_H = 720.0f;		// ウィンドウ高さ
constexpr float FPS = 60.0f;		// フレームレート

//=============================================================================
// 安全解放マクロ
//=============================================================================
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = nullptr; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p) = nullptr; } }
#endif
