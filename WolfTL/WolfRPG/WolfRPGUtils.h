#pragma once

#include "Types.h"

#include <codecvt>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

#define ERROR_TAG "[" __FUNCTION__ "] "

#define VERIFY_MAGIC(CODER, MAGIC) \
	if (!CODER.Verify(MAGIC)) throw WolfRPGException(ERROR_TAG "MAGIC invalid");

namespace wolfRPGUtils
{
static bool g_skipBackup = false;
}

template<typename T>
static inline std::string Dec2Hex(T i)
{
	std::stringstream stream;
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2);
	// To counter act problems with 1-byte types
	stream << std::hex << (sizeof(T) == 1 ? static_cast<unsigned short>(i) : i);
	return stream.str();
}

template<typename T>
static inline std::wstring Dec2HexW(T i)
{
	std::wstringstream stream;
	stream << L"0x" << std::setfill(wchar_t('0')) << std::setw(sizeof(T) * 2);
	// To counter act problems with 1-byte types
	stream << std::hex << (sizeof(T) == 1 ? static_cast<unsigned short>(i) : i);
	return stream.str();
}

static inline const tString GetFileName(const tString& file)
{
	return fs::path(file).filename();
}

static inline const tString GetFileNameNoExt(const tString& file)
{
	return fs::path(file).stem().wstring();
}

static inline std::wstring ToUTF16(const std::string& utf8String)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(utf8String);
}

static inline std::string ToUTF8(const std::wstring& utf16String)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.to_bytes(utf16String);
}

static inline void CreateBackup(const tString& file)
{
	// If the skip backup flag is set, do not create a backup
	if (wolfRPGUtils::g_skipBackup) return;

	// If the file does not exist, do not create a backup
	if (!fs::exists(file)) return;

	// If the backup file already exists, do not create a new backup
	if (fs::exists(file + L".bak")) return;

	// Create a backup of the file
	fs::copy_file(file, file + L".bak");
}

static inline tString StrReplaceAll(tString str, const tString& from, const tString& to)
{
	size_t startPos = 0;
	if (from.empty() || str.empty()) return str;

	while ((startPos = str.find(from, startPos)) != std::string::npos)
	{
		str.replace(startPos, from.length(), to);
		startPos += to.length();
	}

	return str;
}

// Strip all leading / trailing whitespace, including fullwidth spaces
static inline tString FullStrip(tString str)
{
	StrReplaceAll(str, L" ", L"");
	str = std::regex_replace(str, std::wregex(L"^\\u3000*"), L"");
	str = std::regex_replace(str, std::wregex(L"\\u3000*$"), L"");

	return str;
}

static inline tString EscapePath(tString path)
{
	path = FullStrip(path);
	path = std::regex_replace(path, std::wregex(L"[\\/\\\\:\\*\\?\\\"<>\\|]"), L"_");

	return path;
}
