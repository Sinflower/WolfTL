#pragma once

#include <codecvt>
#include <locale>

inline std::wstring ToUTF16(const std::string& utf8String)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(utf8String);
}

inline std::string ToUTF8(const std::wstring& utf16String)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.to_bytes(utf16String);
}
