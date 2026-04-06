#include "String.h"
#include <Windows.h>

namespace MyString 
{
	// テンプレート実装はリンク問題回避のためヘッダに移動済み.

	// 特定の行の値を取り出す.
	std::string ExtractAmount(const std::string& str)
	{
		std::istringstream iss(str);
		std::string valueStr;
		std::string typeStr;

		// ','までの値部分とその後の型部分を分割.
		if (std::getline(iss, valueStr, ',') && std::getline(iss, typeStr, ';')) {
			// 末尾の改行やスペースを除去する.
			while (!typeStr.empty() && (typeStr.back() == '\n' || typeStr.back() == '\r' || typeStr.back() == ' '))
				typeStr.pop_back();

			// 型がfloatの場合、小数点第一位までを文字列として返す.
			if (typeStr == "float") {
				char* end;
				float value = std::strtof(valueStr.c_str(), &end);

				std::ostringstream oss;
				oss << std::fixed << std::setprecision(1) << value;
				return oss.str();
			}

			// 型がboolの場合真偽値を文字列として返す.
			if (typeStr == "bool") {
				if (valueStr == "true" || valueStr == "1") {
					return "true";
				}
				else {
					return "false";
				}
			}
			// 値部分を返す.
			return valueStr;
		}

		return "";
	}

	// 特定の行を取り出す.
	std::string ExtractLine(const std::string& str, int Line)
	{
		std::istringstream iss(str);
		std::string line;
		for (int i = 0; i <= Line; ++i)
		{
			if (!std::getline(iss, line))
			{
				return "";
			}
		}
		return line;
	}

	// 文字列をfloatへ変換.
	float Stof(std::string str)
	{
		try {
			return std::stof(str);
		}
		catch (const std::exception& e) {
			std::cerr << "Stof conversion failed: " << e.what() << std::endl;
		}
		return 0.0f;
	}

	// 文字列をboolへ変換.
	bool Stob(std::string str)
	{
		if (str == "true" || str == "1") return true;
		return false;
	}

	std::string UTF16ToUTF8(const std::u16string& UTF16)
	{
		if (UTF16.empty()) return std::string();
		// char16_tからwchar_tバッファを経由してUTF-16からUTF-8へ変換する.
		int wideCount = MultiByteToWideChar(CP_UTF8, 0, "", 0, nullptr, 0); // 未使用のダミー呼び出し.
		// char16_t*からwchar_t*へのreinterpret_castは非移植的なためwstringを経由する.
		std::wstring tmp;
		tmp.resize(UTF16.size());
		for (size_t i = 0; i < UTF16.size(); ++i) tmp[i] = static_cast<wchar_t>(UTF16[i]);
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, tmp.c_str(), static_cast<int>(tmp.size()), nullptr, 0, nullptr, nullptr);
		if (sizeNeeded == 0) return std::string();
		std::string out(sizeNeeded, '\0');
		WideCharToMultiByte(CP_UTF8, 0, tmp.c_str(), static_cast<int>(tmp.size()), &out[0], sizeNeeded, nullptr, nullptr);
		return out;
	}

	std::u16string UTF8ToUTF16(const std::string& UTF8)
	{
		if (UTF8.empty()) return std::u16string();
		int wideSize = MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), static_cast<int>(UTF8.size()), nullptr, 0);
		if (wideSize == 0) return std::u16string();
		std::wstring tmp(wideSize, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, UTF8.c_str(), static_cast<int>(UTF8.size()), &tmp[0], wideSize);
		std::u16string out(tmp.size(), u'\0');
		for (size_t i = 0; i < tmp.size(); ++i) out[i] = static_cast<char16_t>(tmp[i]);
		return out;
	}

	std::wstring CovertToWString(const std::string& str, UINT codePage)
	{
		if (str.empty()) return std::wstring();
		int wlen = MultiByteToWideChar(codePage, 0, str.c_str(), -1, nullptr, 0);
		if (wlen == 0) { return L""; }

		std::wstring wstr(wlen, 0);
		MultiByteToWideChar(codePage, 0, str.c_str(), -1, &wstr[0], wlen);

		// MultiByteToWideCharのカウントにはヌル終端が含まれるためstd::wstringの正当性のために除去する.
		if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
		return wstr;
	}

	std::string ConvertFromWString(const std::wstring& wstr, UINT codePage)
	{
		if (wstr.empty()) return std::string();
		int len = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (len == 0) { return std::string(); }

		std::string output(len, 0);
		WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &output[0], len, nullptr, nullptr);
		if (!output.empty() && output.back() == '\0') output.pop_back();
		return output;
	}

	std::string ConvertEncodeing(const std::string& str, UINT fromCodePage, UINT toCodePage)
	{
		return ConvertFromWString(CovertToWString(str, fromCodePage), toCodePage);
	}

	std::wstring ConvertEncodeing(const std::wstring& wstr, UINT toCodePage)
	{
		return CovertToWString(ConvertFromWString(wstr, toCodePage), CP_UTF8);
	}

	const std::wstring StringToWString(const std::string& str)
	{
		return CovertToWString(str, CP_ACP);
	}

	const std::string WStringToString(const std::wstring& wstr)
	{
		return ConvertFromWString(wstr, CP_UTF8);
	}

}
