#pragma once

#include "Constant.h"								// このゲームの定数.	
#include "Game/00_GameLoop/Time/Time.h"			// このゲームの時間軸.

#include "Engine/Utility/Assert/Assert.inl"			// HRESULTのtrycatchをする.
#include "Engine/Utility/Math/Math.h"				// 算数系.	
#include "Engine/Utility/ComPtr/ComPtr.h"			// Microsoft::WRL::ComPtrのようなもの.	
#include "Engine/Utility/CustomSTL/pair/Pair.h"		// 添え字が使えるpair.	
#include "Engine/Utility/EnumFlags/EnumFlags.h"		// Enumのビット演算子オーバーロード.
#include "Engine/Utility/Transform/Transform.h"		// Transform.	
#include "Engine/Utility/SingletonTemplate/SingletonTemplate.h"	// シングルトンテンプレト.	

// MEMO : Windows ヘッダの min/max マクロを無効化.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#pragma warning(push)
#pragma warning(disable : 4005)
#include <Windows.h>
#include <crtdbg.h>
#pragma warning(pop)
#include <stdio.h>
#include <tchar.h>
#include <functional>
#include <cassert>
#include <filesystem>	// ファイル関連.
#include <vector>       // 可変長配列.	
#include <array>        // 定数長配列.
#include <algorithm>    // アルゴリズム.
#include <map>          // マップ.
#include <unordered_map>// マップ.
#include <cmath>        // 数学関数.
#include <memory>       // メモリ管理.
#include <string>       // 文字列.
#include <fstream>		// ファイル入出力.	
#include <chrono>		// 時間計測.	
#include <iostream>
#include <stdexcept>
#include <type_traits>



#define _SOUND_STOP_DEBUG 0

#if _DEBUG
#include <dxgidebug.h>	// メモリリークの検出.
#endif // _DEBUG.

// DirectX12.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// DirectSound.
#include <dsound.h>

//ライブラリ読み込み.
#pragma comment(lib, "winmm.lib")
//DirectSound.
#pragma comment( lib, "dsound.lib" )