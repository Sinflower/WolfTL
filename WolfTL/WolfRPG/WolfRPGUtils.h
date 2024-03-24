#pragma once

#include "Types.h"

#include <codecvt>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

#define ERROR_TAG "[" __FUNCTION__ "] "

#define VERIFY_MAGIC(CODER, MAGIC) \
	if (!CODER.Verify(MAGIC)) throw WolfRPGException(ERROR_TAG "MAGIC invalid");

template<typename T>
static inline std::string Dec2Hex(T i)
{
	std::stringstream stream;
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2);
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
