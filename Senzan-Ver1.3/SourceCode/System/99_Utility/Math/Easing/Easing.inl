#pragma once
#include "Easing.h"
#include <type_traits>
#include <DirectXMath.h>
#include <cmath>
#include <algorithm>

/*******************************************************************
*    Easing.inl
*    イージング関数のインライン実装ファイル.
*    各種イージング関数の係数計算と汎用補間ラッパを提供します。
*******************************************************************/

namespace MyEasing {

/**********************************************************
* @brief        : 正規化時間 t に対するイージング係数 f(t) を計算する.
* @param type   : イージングの種類.
* @param t      : 正規化時間(0.0f - 1.0f).
* @return       : 正規化イージング係数(0.0f - 1.0f).
**********************************************************/
inline float EaseFactor(Type type, float t)
{
    // Clamp
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;

    switch (type)
    {
    case Type::Liner:         return t;
    case Type::InSine:        return 1.0f - std::cos((t * M_PI_F) * 0.5f);
    case Type::OutSine:       return std::sin((t * M_PI_F) * 0.5f);
    case Type::InOutSine:     return -0.5f * (std::cos(M_PI_F * t) - 1.0f);
    case Type::InQuad:        return t * t;
    case Type::OutQuad:       return 1.0f - (1.0f - t) * (1.0f - t);
    case Type::InOutQuad:     return (t < 0.5f) ? (2.0f * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f);
    case Type::InCubic:       return t * t * t;
    case Type::OutCubic:      return 1.0f - std::pow(1.0f - t, 3.0f);
    case Type::InOutCubic:    return (t < 0.5f) ? (4.0f * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f);
    case Type::InQuart:       return t * t * t * t;
    case Type::OutQuart:      return 1.0f - std::pow(1.0f - t, 4.0f);
    case Type::InOutQuart:    return (t < 0.5f) ? (8.0f * t * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) * 0.5f);
    case Type::InQuint:       return t * t * t * t * t;
    case Type::OutQuint:      return 1.0f - std::pow(1.0f - t, 5.0f);
    case Type::InOutQuint:    return (t < 0.5f) ? (16.0f * std::pow(t, 5.0f)) : (1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) * 0.5f);
    case Type::InExpo:        return (t == 0.0f) ? 0.0f : std::pow(2.0f, 10.0f * t - 10.0f);
    case Type::OutExpo:       return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
    case Type::InOutExpo:
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return (t < 0.5f) ? (std::pow(2.0f, 20.0f * t - 10.0f) * 0.5f) : (1.0f - std::pow(2.0f, -20.0f * t + 10.0f) * 0.5f);
    case Type::InCirc:        return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f));
    case Type::OutCirc:       return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f));
    case Type::InOutCirc:     return (t < 0.5f) ? ((1.0f - std::sqrt(1.0f - 4.0f * t * t)) * 0.5f) : ((std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) * 0.5f);
    case Type::InBack:
    {
        const float s = 1.70158f;
        return t * t * ((s + 1.0f) * t - s);
    }
    case Type::OutBack:
    {
        const float s = 1.70158f;
        float u = t - 1.0f;
        return 1.0f + (u * u * ((s + 1.0f) * u + s));
    }
    case Type::InOutBack:
    {
        const float s = 1.70158f * 1.525f;
        if (t < 0.5f)
            return (std::pow(2.0f * t, 2.0f) * ((s + 1.0f) * 2.0f * t - s)) * 0.5f;
        else
            return ((std::pow(2.0f * t - 2.0f, 2.0f) * ((s + 1.0f) * (t * 2.0f - 2.0f) + s) + 2.0f) * 0.5f);
    }
    case Type::InElastic:
    {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        float c5 = (2.0f * M_PI_F) / 4.5f;
        return -(std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c5));
    }
    case Type::OutElastic:
    {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        float c5 = (2.0f * M_PI_F) / 4.5f;
        return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c5) + 1.0f;
    }
    case Type::InOutElastic:
    {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        float c5 = (2.0f * M_PI_F) / 4.5f;
        if (t < 0.5f)
            return -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f;
        return (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f + 1.0f;
    }
    case Type::InBounce:
        return 1.0f - EaseFactor(Type::OutBounce, 1.0f - t);
    case Type::OutBounce:
    {
        float n1 = 7.5625f;
        float d1 = 2.75f;
        if (t < 1.0f / d1)
        {
            return n1 * t * t;
        }
        else if (t < 2.0f / d1)
        {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        }
        else if (t < 2.5f / d1)
        {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        }
        else
        {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }
    case Type::InOutBounce:
        return (t < 0.5f) ? ((1.0f - EaseFactor(Type::OutBounce, 1.0f - (t * 2.0f))) * 0.5f) : ((EaseFactor(Type::OutBounce, (t * 2.0f - 1.0f)) * 0.5f) + 0.5f);
    default:
        return t;
    }
}

// イージング種別の名前を返す.
inline const char* GetEasingTypeName(MyEasing::Type type)
{
    switch (type)
    {
    case MyEasing::Type::Liner:         return "Liner";
    case MyEasing::Type::InSine:        return "InSine";
    case MyEasing::Type::OutSine:       return "OutSine";
    case MyEasing::Type::InOutSine:     return "InOutSine";
    case MyEasing::Type::InQuad:        return "InQuad";
    case MyEasing::Type::OutQuad:       return "OutQuad";
    case MyEasing::Type::InOutQuad:     return "InOutQuad";
    case MyEasing::Type::InCubic:       return "InCubic";
    case MyEasing::Type::OutCubic:      return "OutCubic";
    case MyEasing::Type::InOutCubic:    return "InOutCubic";
    case MyEasing::Type::InQuart:       return "InQuart";
    case MyEasing::Type::OutQuart:      return "OutQuart";
    case MyEasing::Type::InOutQuart:    return "InOutQuart";
    case MyEasing::Type::InQuint:       return "InQuint";
    case MyEasing::Type::OutQuint:      return "OutQuint";
    case MyEasing::Type::InOutQuint:    return "InOutQuint";
    case MyEasing::Type::InExpo:        return "InExpo";
    case MyEasing::Type::OutExpo:       return "OutExpo";
    case MyEasing::Type::InOutExpo:     return "InOutExpo";
    case MyEasing::Type::InCirc:        return "InCirc";
    case MyEasing::Type::OutCirc:       return "OutCirc";
    case MyEasing::Type::InOutCirc:     return "InOutCirc";
    case MyEasing::Type::InBack:        return "InBack";
    case MyEasing::Type::OutBack:       return "OutBack";
    case MyEasing::Type::InOutBack:     return "InOutBack";
    case MyEasing::Type::InElastic:     return "InElastic";
    case MyEasing::Type::OutElastic:    return "OutElastic";
    case MyEasing::Type::InOutElastic:  return "InOutElastic";
    case MyEasing::Type::InBounce:      return "InBounce";
    case MyEasing::Type::OutBounce:     return "OutBounce";
    case MyEasing::Type::InOutBounce:   return "InOutBounce";
    default:                            return "Unknown";
    }
}

/**********************************************************
* @brief            : 数値型用の汎用イージング更新.
* @tparam   TOut    : 出力型（算術型）.
* @tparam   TStart  : 開始値型.
* @tparam   TEnd    : 終了値型.
* @param    Type    : イージングタイプ.
* @param    Time    : 現在時間.
* @param    MaxTime : 最大時間.
* @param    Start   : 開始値.
* @param    End     : 終了値.
* @param    Out     : 出力参照.
**********************************************************/
template<typename TOut, typename TStart, typename TEnd>
inline std::enable_if_t<std::is_arithmetic<TOut>::value, void>
UpdateEasing(Type Type, float Time, float MaxTime, TStart Start, TEnd End, TOut& Out)
{
    if (MaxTime <= 0.0f) { Out = static_cast<TOut>(Start); return; }
    float t = Time / MaxTime;
    t = std::min(1.0f, std::max(0.0f, t));
    float f = EaseFactor(Type, t);
    TOut s = static_cast<TOut>(Start);
    TOut e = static_cast<TOut>(End);
    Out = static_cast<TOut>(s + (e - s) * f);
}

template<typename T>
inline void UpdateEasing(Type Type, float Time, float MaxTime, T Start, T End, T& Out)
{
    UpdateEasing<T, T, T>(Type, Time, MaxTime, Start, End, Out);
}

inline void UpdateEasing(Type Type, float Time, float MaxTime, DirectX::XMFLOAT2 Start, DirectX::XMFLOAT2 End, DirectX::XMFLOAT2& Out)
{
    if (MaxTime <= 0.0f) { Out = Start; return; }
    float t = Time / MaxTime;
    t = std::min(1.0f, std::max(0.0f, t));
    float f = EaseFactor(Type, t);
    Out.x = Start.x + (End.x - Start.x) * f;
    Out.y = Start.y + (End.y - Start.y) * f;
}

inline void UpdateEasing(Type Type, float Time, float MaxTime, DirectX::XMFLOAT3 Start, DirectX::XMFLOAT3 End, DirectX::XMFLOAT3& Out)
{
    if (MaxTime <= 0.0f) { Out = Start; return; }
    float t = Time / MaxTime;
    t = std::min(1.0f, std::max(0.0f, t));
    float f = EaseFactor(Type, t);
    Out.x = Start.x + (End.x - Start.x) * f;
    Out.y = Start.y + (End.y - Start.y) * f;
    Out.z = Start.z + (End.z - Start.z) * f;
}

inline void UpdateEasing(Type Type, float Time, float MaxTime, DirectX::XMFLOAT4 Start, DirectX::XMFLOAT4 End, DirectX::XMFLOAT4& Out)
{
    if (MaxTime <= 0.0f) { Out = Start; return; }
    float t = Time / MaxTime;
    t = std::min(1.0f, std::max(0.0f, t));
    float f = EaseFactor(Type, t);
    Out.x = Start.x + (End.x - Start.x) * f;
    Out.y = Start.y + (End.y - Start.y) * f;
    Out.z = Start.z + (End.z - Start.z) * f;
    Out.w = Start.w + (End.w - Start.w) * f;
}

inline void UpdateEasing(Type Type, float Time, float MaxTime, DirectX::XMVECTOR Start, DirectX::XMVECTOR End, DirectX::XMVECTOR& Out)
{
    if (MaxTime <= 0.0f) { Out = Start; return; }
    float t = Time / MaxTime;
    t = std::min(1.0f, std::max(0.0f, t));
    float f = EaseFactor(Type, t);
    Out = DirectX::XMVectorLerp(Start, End, f);
}

} // namespace MyEasing

