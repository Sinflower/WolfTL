#pragma once

#include "Utils.h"
#include "WolfRPG\WolfRPG.h"

#include <iostream>
#include <memory>
#include <regex>
#include <sstream>

class Context
{
public:
	virtual const tString to_s() const
	{
		return L"";
	}

	static std::shared_ptr<Context> fromString(const tString& string);
};

namespace Contexts
{
class MapEvent : public Context
{
public:
	MapEvent(const tString& mapName, const uint32_t& eventNum, const uint32_t& pageNum, const uint32_t& lineNum, const tString& commandName) :
		m_mapName(mapName),
		m_eventNum(eventNum),
		m_pageNum(pageNum),
		m_lineNum(lineNum),
		m_commandName(commandName)
	{
	}

	const tString to_s() const
	{
		std::wostringstream oss;
		oss << "MPS:" << m_mapName << "/events/" << m_eventNum << "/pages/" << m_pageNum << "/" << m_lineNum << "/" << m_commandName;
		return oss.str();
	}

	static std::shared_ptr<MapEvent> fromData(const tString& mapName, const Event& event, const Page& page, const uint32_t& cmdIndex, const Command::CommandShPtr::Command& command)
	{
		return std::make_shared<MapEvent>(mapName, event.GetID(), page.GetID() + 1, cmdIndex + 1, command->GetClassString());
	}

	static std::shared_ptr<MapEvent> fromString(const tStrings& path)
	{
		if (path.size() != 7)
			throw WolfRPGException(ERROR_TAG "Invalid path specified for MPS context line");

		if (path.at(1) != L"events" || path.at(3) != L"pages")
			throw WolfRPGException(ERROR_TAG "Unexpected path element in MPS context line");

		return std::make_shared<MapEvent>(path.at(0), std::stoi(path.at(2)), std::stoi(path.at(4)), std::stoi(path.at(5)), path.at(6));
	}

private:
	tString m_mapName;
	uint32_t m_eventNum;
	uint32_t m_pageNum;
	uint32_t m_lineNum;
	tString m_commandName;
};

class CommonEvent : public Context
{
public:
	CommonEvent(const uint32_t& eventNum, const uint32_t& lineNum, const tString& commandName) :
		m_eventNum(eventNum),
		m_lineNum(lineNum),
		m_commandName(commandName)
	{
	}

	const tString to_s() const
	{
		std::wostringstream oss;
		oss << "COMMONEVENT:" << m_eventNum << "/" << m_lineNum << "/" << m_commandName;
		return oss.str();
	}

	static std::shared_ptr<CommonEvent> fromData(const ::CommonEvent& ev, const uint32_t& cmdIndex, const Command::CommandShPtr::Command& command)
	{
		return std::make_shared<CommonEvent>(ev.GetID(), cmdIndex + 1, command->GetClassString());
	}

	static std::shared_ptr<CommonEvent> fromString(const tStrings& path)
	{
		if (path.size() != 3)
			throw WolfRPGException(ERROR_TAG "Invalid path specified for COMMONEVENT context line");

		return std::make_shared<CommonEvent>(std::stoi(path.at(0)), std::stoi(path.at(1)), path.at(2));
	}

private:
	uint32_t m_eventNum;
	uint32_t m_lineNum;
	tString m_commandName;
};

class GameDat : public Context
{
public:
	explicit GameDat(const tString& name) :
		m_name(name)
	{
	}

	const tString to_s() const
	{
		return tString(TEXT("GAMEDAT:") + m_name);
 	}

	static std::shared_ptr<GameDat> fromData(const tString& name)
	{
		return std::make_shared<GameDat>(name);
	}

	static std::shared_ptr<GameDat> fromString(const tStrings& path)
	{
		if (path.size() != 1)
			throw WolfRPGException(ERROR_TAG "Invalid path specified for GAMEDAT context line");

		return std::make_shared<GameDat>(path.at(0));
	}

private:
	tString m_name;
};

class Database : public Context
{
public:
	Database(const tString& dbName, const uint32_t& typeIndex, const tString& typeName, const uint32_t& datumIndex, const tString& datumName, const uint32_t& fieldIndex, const tString& fieldName) :
		m_dbName(dbName),
		m_typeIndex(typeIndex),
		m_typeName(typeName),
		m_datumIndex(datumIndex),
		m_datumName(datumName),
		m_fieldIndex(fieldIndex),
		m_fieldName(fieldName)
	{
	}

	const tString to_s() const
	{
		std::wostringstream oss;
		oss << "DB:" << m_dbName << "/[" << m_typeIndex << "]" << m_typeName << "/[" << m_datumIndex << "]" << m_datumName << "/[" << m_fieldIndex << "]" << m_fieldName;
		return oss.str();
	}

	static std::shared_ptr<Database> fromString(const tStrings& path)
	{
		if (path.size() != 4)
			throw WolfRPGException(ERROR_TAG "Invalid path specified for DB context line");

		tStrings localPath = path;

		std::wregex rx(L"\\[(\\d+)\\]");
		std::wsmatch sm;
		uInts indices;
		for (size_t i = 1; i < localPath.size(); i++)
		{
			std::regex_search(localPath.at(i), sm, rx);
			indices.push_back(std::stoi(sm[1]));
			localPath[i] = std::regex_replace(localPath.at(i), rx, L"");
		}

		return std::make_shared<Database>(localPath.at(0), indices.at(0), localPath.at(1), indices.at(1), localPath.at(2), indices.at(2), localPath.at(3));
	}

	/*
	  def self.from_data(db_name, type_index, type, datum_index, datum, field)
		Database.new(db_name,
					 type_index, type.name.gsub('/', '_'),
					 datum_index, datum.name.gsub('/', '_'),
					 field.index, field.name.gsub('/', '_'))
	  end

	*/

	static std::shared_ptr<Database> fromData(const tString& dbName, const uint32_t& typeIndex, const Type& type, const uint32_t& datumIndex, const Data& datum, const Field& field)
	{
		tString typeName  = std::regex_replace(type.GetName(), std::wregex(L"//"), L"_");
		tString datumName = std::regex_replace(datum.GetName(), std::wregex(L"//"), L"_");
		tString fieldName = std::regex_replace(field.GetName(), std::wregex(L"//"), L"_");

		return std::make_shared<Database>(dbName, typeIndex, typeName, datumIndex, datumName, field.Index(), fieldName);
	}

private:
	tString m_dbName;
	uint32_t m_typeIndex;
	tString m_typeName;
	uint32_t m_datumIndex;
	tString m_datumName;
	uint32_t m_fieldIndex;
	tString m_fieldName;
};
} // namespace Contexts

namespace ContextsShPtr
{
using Context     = std::shared_ptr<Context>;
using MapEvent    = std::shared_ptr<Contexts::MapEvent>;
using GameDat     = std::shared_ptr<Contexts::GameDat>;
using Database    = std::shared_ptr<Contexts::Database>;
using CommonEvent = std::shared_ptr<Contexts::CommonEvent>;
} // namespace ContextsShPtr

ContextsShPtr::Context Context::fromString(const tString& string)
{
	tString type;
	tStrings path;

	size_t pos = string.find(L":");

	if (pos == std::wstring::npos)
		throw WolfRPGException(ERROR_TAG "Malformated context line");

	type = string.substr(0, pos);
	path = splitString(string.substr(pos + 1), L'/');

	if (type == L"MPS")
		return Contexts::MapEvent::fromString(path);
	else if (type == L"GAMEDAT")
		return Contexts::GameDat::fromString(path);
	else if (type == L"DB")
		return Contexts::Database::fromString(path);
	else if (type == L"COMMONEVENT")
		return Contexts::CommonEvent::fromString(path);

	throw WolfRPGException(ERROR_TAG L"Unrecongnized context type " + type);
}
