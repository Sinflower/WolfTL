#pragma once

#include "WolfRPG/Types.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

#define CAT(A, B) A##B
#define WSTR(STR) CAT(L, #STR)

#define VECTOR_CONTAINS(VEC, OBJ) (std::find(VEC.begin(), VEC.end(), OBJ) != VEC.end())

static inline tStrings splitString(const tString& str, const WCHAR& delimiter)
{
	tStrings strings;
	std::wstringstream f(str);
	tString s;
	while (std::getline(f, s, delimiter))
		strings.push_back(s);

	return strings;
}

static inline bool translatable(const tString& str)
{
	if (str.empty() || str == L"\u25A0")
		return false;

	return true;
}

static inline tString strReplaceAll(tString str, const tString& from, const tString& to)
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

// trim from start (in place)
static inline void ltrim(tString& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
}

// trim from end (in place)
static inline void rtrim(tString& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(),
			s.end());
}

// Strip all leading / trailing whitespace, including fullwidth spaces
static inline tString fullStrip(tString str)
{
	strReplaceAll(str, L" ", L"");
	str = std::regex_replace(str, std::wregex(L"^\\u3000*"), L"");
	str = std::regex_replace(str, std::wregex(L"\\u3000*$"), L"");

	return str;
}

static inline tString escapePath(tString path)
{
	path = fullStrip(path);
	path = std::regex_replace(path, std::wregex(L"[\\/\\\\:\\*\\?\\\"<>\\|]"), L"_");

	return path;
}
