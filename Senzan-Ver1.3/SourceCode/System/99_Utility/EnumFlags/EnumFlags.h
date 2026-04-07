#pragma once
#include <type_traits> // std::underlying_type_t, std::enable_if_t, std::true_type, std::false_type.
#include <cstdint>     // 標準の整数型.

// トレイト構造体: EnumType がビットフラグとして有効化されているかを示すマーカー.
template <typename T>
struct is_scoped_enum_flag : std::false_type {};

// 基底型の別名定義.
template <typename T>
using underlying_type = std::underlying_type_t<T>;

// テンプレート関数を有効化するためのエイリアス.
template <typename T> using enable_if_flag = std::enable_if_t<is_scoped_enum_flag<T>::value, T>;
template <typename T> using enable_if_flag_ref = std::enable_if_t<is_scoped_enum_flag<T>::value, T&>;

// | (OR).
template <typename T> inline enable_if_flag<T> operator|(T a, T b) noexcept { return static_cast<T>(static_cast<underlying_type<T>>(a) | static_cast<underlying_type<T>>(b)); }
// & (AND).
template <typename T> inline enable_if_flag<T> operator&(T a, T b) noexcept { return static_cast<T>(static_cast<underlying_type<T>>(a) & static_cast<underlying_type<T>>(b)); }
// ^ (XOR).
template <typename T> inline enable_if_flag<T> operator^(T a, T b) noexcept { return static_cast<T>(static_cast<underlying_type<T>>(a) ^ static_cast<underlying_type<T>>(b)); }
// ~ (NOT/反転).
template <typename T> inline enable_if_flag<T> operator~(T a) noexcept { return static_cast<T>(~static_cast<underlying_type<T>>(a)); }
// |= (OR 代入).
template <typename T> inline enable_if_flag_ref<T> operator|=(T& a, T b) noexcept { a = a | b; return a; }
// &= (AND 代入).
template <typename T> inline enable_if_flag_ref<T> operator&=(T& a, T b) noexcept { a = a & b; return a; }
// ^= (XOR 代入).
template <typename T> inline enable_if_flag_ref<T> operator^=(T& a, T b) noexcept { a = a ^ b; return a; }

// EnumType をビットフラグとして有効化する.
#define ENABLE_ENUM_FLAG_OPERATORS(EnumType) \
    template <> struct is_scoped_enum_flag<EnumType> : std::true_type {};
