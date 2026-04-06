#pragma once
#include <DirectXMath.h>
//=================================================.
//	定数.
//=================================================.

static constexpr int	WND_W = 1280;		// ウィンドウの幅.
static constexpr float	WND_WF = 1280.f;	// ウィンドウの幅.
static constexpr int	WND_H = 720;		// ウィンドウの高さ.
static constexpr float	WND_HF = 720.f;	// ウィンドウの高さ.
static constexpr int	FPS = 60;		// フレームレート.
static constexpr float  EPSILON_E3 = 1e-3f;	// 0.001f.
static constexpr float  EPSILON_E4 = 1e-4f;	// 0.0001f.
static constexpr float  EPSILON_E5 = 1e-5f;	// 0.00001f.
static constexpr float  EPSILON_E6 = 1e-6f;	// 0.000001f.
static constexpr float  EPSILON_E7 = 1e-7f;	// 0.0000001f.

static constexpr float  M_PI_F = 3.1415926535f; // 円周率.

static constexpr float STICK_THRESHOLD = 0.15f; // 入力の閾値.


// 単位ベクトル.
namespace Axis {
    // XMFLOAT3 は静的初期化子リストで初期化可能.
    static const DirectX::XMFLOAT3 ZERO = { 0.0f, 0.0f, 0.0f };
    static const DirectX::XMFLOAT3 UP = { 0.0f, 1.0f, 0.0f };
    static const DirectX::XMFLOAT3 DOWN = { 0.0f, -1.0f, 0.0f };
    static const DirectX::XMFLOAT3 LEFT = { -1.0f, 0.0f, 0.0f };
    static const DirectX::XMFLOAT3 RIGHT = { 1.0f, 0.0f, 0.0f };
    static const DirectX::XMFLOAT3 FORWARD = { 0.0f, 0.0f, 1.0f };
    static const DirectX::XMFLOAT3 BACK = { 0.0f, 0.0f, -1.0f };

    // XMFLOAT4からロードすることで、SIMDレジスタに正しい値が設定される.
    static const DirectX::XMVECTOR ZERO_VECTOR  = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    static const DirectX::XMVECTOR UP_VECTOR    = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    static const DirectX::XMVECTOR DOWN_VECTOR  = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    static const DirectX::XMVECTOR LEFT_VECTOR  = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
    static const DirectX::XMVECTOR RIGHT_VECTOR = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    static const DirectX::XMVECTOR FORWARD_VECTOR = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    static const DirectX::XMVECTOR BACK_VECTOR  = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
}

// 色系の定義.
namespace Color {

// RGBの定義.
enum class eColor
{
    Red = 0,
    Green,
    Blue,
    Yellow,
    Cyan,   // 水色.
    Magenta, // マゼンタ.
    Gray,   // 灰色.
    Black,
    White,
};

static DirectX::XMFLOAT4 GetColorValue(eColor debugColor)
{
    switch (debugColor)
    {
    case eColor::Red:
        return DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

    case eColor::Green:
        return DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    case eColor::Blue:
        return DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

    case eColor::Yellow:
        return DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    case eColor::Cyan:
        return DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);

    case eColor::Magenta:
        return DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

    case eColor::Gray:
        return DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

    case eColor::Black:
        return DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    case eColor::White:
        return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    default:
        return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
};
}
