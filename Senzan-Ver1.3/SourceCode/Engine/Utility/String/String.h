#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <stdexcept>
#include <unordered_set>
#include <locale>
#include <codecvt>
#include <Windows.h>

namespace MyString {
	// 値を文字列に変換する.
	template<typename T>
	std::string ToString(const T& value);
	// 文字列から値を戻す.
	template<typename T>
	T FromString(const std::string& str);

	// 特定の行の値を取り出す.
	std::string ExtractAmount(const std::string& str);

	// 特定の行を取り出す.
	std::string ExtractLine(const std::string& str, int Line);

	// 文字列をfloatへ変換.
	float Stof(std::string str);
	// 文字列をboolへ変換.
	bool Stob(std::string str);

	// ワイド文字をマルチバイトに変換.
	inline std::string WStringToString(const std::wstring& WideStr)
	{
		const int size = WideCharToMultiByte(CP_THREAD_ACP, 0, WideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1)
		{
			return std::string();
		}

		std::string str_buf(static_cast<size_t>(size - 1), '\0');
		WideCharToMultiByte(CP_THREAD_ACP, 0, WideStr.c_str(), -1, &str_buf[0], size, nullptr, nullptr);
		return str_buf;
	}

	// マルチバイトをワイド文字に変換.
	inline std::wstring StringToWString(const std::string& Str)
	{
		const int size = MultiByteToWideChar(CP_THREAD_ACP, 0, Str.c_str(), -1, nullptr, 0);
		if (size <= 1)
		{
			return std::wstring();
		}

		std::wstring wide_str_buf(static_cast<size_t>(size - 1), L'\0');
		MultiByteToWideChar(CP_THREAD_ACP, 0, Str.c_str(), -1, &wide_str_buf[0], size);
		return wide_str_buf;
	}

	// UTF-16からUTF-8へ変換.
	std::string UTF16ToUTF8(const std::u16string& utf16);

	// UTF-8からUTF-16へ変換.
	std::u16string UTF8ToUTF16(const std::string& utf8);
}