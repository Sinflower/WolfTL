#pragma once
#include "Command.h"
#include "FileCoder.h"
#include "RouteCommand.h"
#include "WolfRPGUtils.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

class Page
{
public:
	Page() = default;

	bool Init(FileCoder& coder, uint32_t id)
	{
		m_id       = id;
		m_unknown1 = coder.ReadInt();

		m_graphicName       = coder.ReadString();
		m_graphicDirection  = coder.ReadByte();
		m_graphicFrame      = coder.ReadByte();
		m_graphicOpacity    = coder.ReadByte();
		m_graphicRenderMode = coder.ReadByte();

		m_conditions = coder.Read(1 + 4 + 4 * 4 + 4 * 4);
		m_movement   = coder.Read(4);

		m_flags = coder.ReadByte();

		m_routeFlags = coder.ReadByte();

		uint32_t routeCount = coder.ReadInt();
		for (uint32_t i = 0; i < routeCount; i++)
		{
			RouteCommand rc;
			if (!rc.Init(coder))
				throw WolfRPGException(ERROR_TAG + "RouteCommand initialization failed");

			m_route.push_back(rc);
		}

		uint32_t commandCount = coder.ReadInt();
		for (uint32_t i = 0; i < commandCount; i++)
		{
			Command::CommandShPtr::Command command = Command::Command::Init(coder);
			if (!command->Valid())
				throw WolfRPGException(ERROR_TAG + "Command initialization failed");

			m_commands.push_back(command);
		}

		VERIFY_MAGIC(coder, COMMANDS_TERMINATOR);

		m_shadowGraphicNum = coder.ReadByte();
		m_collisionWidth   = coder.ReadByte();
		m_collisionHeight  = coder.ReadByte();

		uint8_t terminator = coder.ReadByte();
		if (terminator != 0x7A)
			throw WolfRPGException(ERROR_TAG + "Page terminator not 0x7A (found: 0x" + Dec2Hex(terminator) + ")");

		return true;
	}

	void Dump(FileCoder& coder) const
	{
		coder.WriteInt(m_unknown1);
		coder.WriteString(m_graphicName);
		coder.WriteByte(m_graphicDirection);
		coder.WriteByte(m_graphicFrame);
		coder.WriteByte(m_graphicOpacity);
		coder.WriteByte(m_graphicRenderMode);
		coder.Write(m_conditions);
		coder.Write(m_movement);
		coder.WriteByte(m_flags);
		coder.WriteByte(m_routeFlags);
		coder.WriteInt(static_cast<uint32_t>(m_route.size()));
		for (RouteCommand cmd : m_route)
			cmd.Dump(coder);
		coder.WriteInt(static_cast<uint32_t>(m_commands.size()));
		for (const Command::CommandShPtr::Command& cmd : m_commands)
			cmd->Dump(coder);
		coder.Write(COMMANDS_TERMINATOR);
		coder.WriteByte(m_shadowGraphicNum);
		coder.WriteByte(m_collisionWidth);
		coder.WriteByte(m_collisionHeight);
		coder.WriteByte(0x7A);
	}

	nlohmann::ordered_json ToJson() const
	{
		nlohmann::ordered_json j;
		j["id"]   = m_id;
		j["list"] = nlohmann::ordered_json::array();

		for (std::size_t i = 0; i < m_commands.size(); i++)
		{
			nlohmann::ordered_json cmdJ = m_commands[i]->ToJson();

			if (!cmdJ.empty())
			{
				cmdJ["index"] = i;
				j["list"].push_back(cmdJ);
			}
		}

		return j;
	}

	void Patch(const nlohmann::ordered_json& j)
	{
		CHECK_JSON_KEY(j, "list", "pages");
		CHECK_JSON_KEY(j, "id", "pages");

		const uint32_t id = j["id"].get<uint32_t>();

		if (id != m_id)
			throw WolfRPGException(ERROR_TAG + "Page ID mismatch: " + std::to_string(m_id) + " != " + std::to_string(id));

		uint32_t cmdIdx = 0;

		for (const auto& cmdJ : j["list"])
		{
			CHECK_JSON_KEY(cmdJ, "index", std::format("pages::list[{}]", cmdIdx));
			cmdIdx++;

			const uint32_t index = cmdJ["index"].get<uint32_t>();
			if (index >= m_commands.size())
				throw WolfRPGException(ERROR_TAG + "Index out of range: " + std::to_string(index) + " >= " + std::to_string(m_commands.size()));

			m_commands[index]->Patch(cmdJ);
		}
	}

	const uint32_t& GetID() const
	{
		return m_id;
	}

	const tString& GetGraphicName() const
	{
		return m_graphicName;
	}

	const uint8_t& GetGraphicDirection() const
	{
		return m_graphicDirection;
	}

	const uint8_t& GetGraphicFrame() const
	{
		return m_graphicFrame;
	}

	const uint8_t& GetGraphicOpacity() const
	{
		return m_graphicOpacity;
	}

	const uint8_t& GetGraphicRenderMode() const
	{
		return m_graphicRenderMode;
	}

	const Bytes& GetConditions() const
	{
		return m_conditions;
	}

	const Bytes& GetMovement() const
	{
		return m_movement;
	}

	const uint8_t& GetFlags() const
	{
		return m_flags;
	}

	const uint8_t& GetRouteFlags() const
	{
		return m_routeFlags;
	}

	const RouteCommands& GetRouteCommands() const
	{
		return m_route;
	}

	const Command::Commands& GetCommands() const
	{
		return m_commands;
	}

	const uint8_t& GetShadowGraphicNum() const
	{
		return m_shadowGraphicNum;
	}

	const uint8_t& GetCollisionWidth() const
	{
		return m_collisionWidth;
	}

	const uint8_t& GetCollisionHeight() const
	{
		return m_collisionHeight;
	}

private:
	uint32_t m_id                = 0;
	uint32_t m_unknown1          = 0;
	tString m_graphicName        = TEXT("");
	uint8_t m_graphicDirection   = 0;
	uint8_t m_graphicFrame       = 0;
	uint8_t m_graphicOpacity     = 0;
	uint8_t m_graphicRenderMode  = 0;
	Bytes m_conditions           = {};
	Bytes m_movement             = {};
	uint8_t m_flags              = 0;
	uint8_t m_routeFlags         = 0;
	RouteCommands m_route        = {};
	Command::Commands m_commands = {};
	uint8_t m_shadowGraphicNum   = 0;
	uint8_t m_collisionWidth     = 0;
	uint8_t m_collisionHeight    = 0;

	const Bytes COMMANDS_TERMINATOR{ 0x03, 0x00, 0x00, 0x00 };
};

using Pages = std::vector<Page>;

class Event
{
public:
	Event() = default;

	bool Init(FileCoder& coder)
	{
		VERIFY_MAGIC(coder, MAGIC_NUMBER1);

		m_id               = coder.ReadInt();
		m_name             = coder.ReadString();
		m_x                = coder.ReadInt();
		m_y                = coder.ReadInt();
		uint32_t pageCount = coder.ReadInt();

		VERIFY_MAGIC(coder, MAGIC_NUMBER2);

		uint8_t indicator = 0x0;
		uint32_t pageID   = 0;
		while ((indicator = coder.ReadByte()) == 0x79)
		{
			Page page;
			if (!page.Init(coder, pageID))
				throw WolfRPGException(ERROR_TAG + "Page initialization failed");

			m_pages.push_back(page);
			pageID++;
		}

		if (m_pages.size() != pageCount)
			throw WolfRPGException(ERROR_TAG + "Expected " + std::to_string(pageCount) + " Pages, but read: " + std::to_string(m_pages.size()) + " Pages");

		if (indicator != 0x70)
			throw WolfRPGException(ERROR_TAG + "Unexpected event indicator: " + Dec2Hex(indicator) + " expected 0x70");

		m_valid = true;

		return m_valid;
	}

	void Dump(FileCoder& coder) const
	{
		coder.Write(MAGIC_NUMBER1);
		coder.WriteInt(m_id);
		coder.WriteString(m_name);
		coder.WriteInt(m_x);
		coder.WriteInt(m_y);
		coder.WriteInt(static_cast<uint32_t>(m_pages.size()));
		coder.Write(MAGIC_NUMBER2);

		for (Page page : m_pages)
		{
			coder.WriteByte(0x79);
			page.Dump(coder);
		}

		coder.WriteByte(0x70);
	}

	nlohmann::ordered_json ToJson() const
	{
		nlohmann::ordered_json j;
		j["id"]    = m_id;
		j["name"]  = ToUTF8(m_name);
		j["pages"] = nlohmann::ordered_json::array();

		for (const Page& page : m_pages)
			j["pages"].push_back(page.ToJson());

		return j;
	}

	void Patch(const nlohmann::ordered_json& j)
	{
		CHECK_JSON_KEY(j, "pages", "events");
		CHECK_JSON_KEY(j, "id", "events");

		const uint32_t id = j["id"].get<uint32_t>();

		if (id != m_id)
			throw WolfRPGException(ERROR_TAG + "Event ID mismatch: " + std::to_string(m_id) + " != " + std::to_string(id));

		for (std::size_t i = 0; i < m_pages.size(); i++)
			m_pages[i].Patch(j["pages"][i]);
	}

	const uint32_t& GetID() const
	{
		return m_id;
	}

	const tString& GetName() const
	{
		return m_name;
	}

	const uint32_t& GetX() const
	{
		return m_x;
	}

	const uint32_t& GetY() const
	{
		return m_y;
	}

	const Pages& GetPages() const
	{
		return m_pages;
	}

	const bool& IsValid() const
	{
		return m_valid;
	}

private:
	uint32_t m_id  = 0;
	tString m_name = TEXT("");
	uint32_t m_x   = 0;
	uint32_t m_y   = 0;
	Pages m_pages  = {};
	bool m_valid   = false;

	const Bytes MAGIC_NUMBER1{ 0x39, 0x30, 0x00, 0x00 };
	const Bytes MAGIC_NUMBER2{ 0x00, 0x00, 0x00, 0x00 };
};

using Events = std::vector<Event>;

class Map
{
public:
	explicit Map(const tString& fileName = L"") :
		m_fileName(fileName)
	{
		if (!fileName.empty())
			Load(fileName);
	}

	bool Load(const tString& fileName)
	{
		m_fileName = fileName;

		if (m_fileName.empty())
			throw WolfRPGException(ERROR_TAG + "Trying to load map with empty filename");

		FileCoder coder(fileName, FileCoder::Mode::READ);
		VERIFY_MAGIC(coder, MAGIC_NUMBER);

		m_unknown1 = coder.ReadInt();
		m_unknown2 = coder.ReadByte();
		m_unknown3 = coder.ReadString();

		m_tilesetID = coder.ReadInt();
		m_width     = coder.ReadInt();
		m_height    = coder.ReadInt();

		uint32_t eventCount = coder.ReadInt();

		bool readTiles = true;

		if (FileCoder::IsUTF8())
		{
			int32_t v = coder.ReadInt();
			if (v == -1)
				readTiles = false;
			else
				coder.Seek(-4);
		}

		if (readTiles)
			m_tiles = coder.Read(m_width * m_height * 3 * 4);

		uint8_t indicator = 0x0;
		while ((indicator = coder.ReadByte()) == 0x6F)
		{
			Event ev;
			if (!ev.Init(coder))
				throw WolfRPGException(ERROR_TAG + "Event initialization failed");

			m_events.push_back(ev);
		}

		if (m_events.size() != eventCount)
			throw WolfRPGException(ERROR_TAG + "Expected " + std::to_string(eventCount) + " Events, but read: " + std::to_string(m_events.size()) + " Events");

		if (indicator != 0x66)
			throw WolfRPGException(ERROR_TAG + "Unexpected event indicator: " + Dec2Hex(indicator) + " expected 0x66");

		if (!coder.IsEof())
			throw WolfRPGException(ERROR_TAGW + L"Map [" + fileName + L"] has more data than expected");

		return true;
	}

	const tString& FileName() const
	{
		return m_fileName;
	}

	void Dump(const tString& outputDir) const
	{
		tString outputFN = outputDir + L"/" + ::GetFileName(m_fileName);
		FileCoder coder(outputFN, FileCoder::Mode::WRITE);
		coder.Write(MAGIC_NUMBER);

		coder.WriteInt(m_unknown1);
		coder.WriteByte(m_unknown2);
		coder.WriteString(m_unknown3);

		coder.WriteInt(m_tilesetID);
		coder.WriteInt(m_width);
		coder.WriteInt(m_height);
		coder.WriteInt(static_cast<uint32_t>(m_events.size()));

		if (FileCoder::IsUTF8() && m_tiles.empty())
			coder.WriteInt(0xFFFFFFFF);
		else
			coder.Write(m_tiles);

		for (Event event : m_events)
		{
			coder.WriteByte(0x6F);
			event.Dump(coder);
		}
		coder.WriteByte(0x66);
	}

	void ToJson(const tString& outputFolder) const
	{
		nlohmann::ordered_json j;
		j["events"] = nlohmann::ordered_json::array();

		for (const Event& ev : m_events)
			j["events"].push_back(ev.ToJson());

		const tString outputFile = outputFolder + L"/" + ::GetFileNameNoExt(m_fileName) + L".json";

		std::ofstream out(outputFile);
		out << j.dump(4);

		out.close();
	}

	void Patch(const tString& patchFolder)
	{
		const tString patchFile = patchFolder + L"/" + ::GetFileNameNoExt(m_fileName) + L".json";
		if (!fs::exists(patchFile))
			throw WolfRPGException(ERROR_TAGW + L"Patch file not found: " + patchFile);

		nlohmann::ordered_json j;
		std::ifstream in(patchFile);
		in >> j;
		in.close();

		for (std::size_t i = 0; i < m_events.size(); i++)
			m_events[i].Patch(j["events"][i]);
	}

	const Events& GetEvents() const
	{
		return m_events;
	}

private:
	tString m_fileName;

	uint32_t m_unknown1 = 0;
	uint8_t m_unknown2  = 0;
	tString m_unknown3  = TEXT("");

	uint32_t m_tilesetID = 0;
	uint32_t m_width     = 0;
	uint32_t m_height    = 0;
	Bytes m_tiles        = {};
	Events m_events      = {};

	inline static const MagicNumber MAGIC_NUMBER{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
													0x57, 0x4F, 0x4C, 0x46, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00 },
												  16 };
};

using Maps = std::vector<Map>;