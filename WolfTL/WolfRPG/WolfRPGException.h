#pragma once

#include <exception>
#include <string>

#include "WolfRPGUtils.h"

class WolfRPGException : public std::exception
{
public:
	explicit WolfRPGException(const char* pWhat) :
		m_what(pWhat)
	{
	}

	explicit WolfRPGException(const std::string& what) :
		m_what(what)
	{
	}

	explicit WolfRPGException(const std::wstring& what) :
		m_what(ToUTF8(what))
	{
	}

	virtual ~WolfRPGException() throw()
	{
	}

	virtual const char* what() const throw()
	{
		return m_what.c_str();
	}

private:
	std::string m_what;
};