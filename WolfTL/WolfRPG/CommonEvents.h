#pragma once

#include "Command.h"
#include "FileCoder.h"
#include "WolfRPGUtils.h"

#include <array>
#include <format>
#include <fstream>
#include <nlohmann\json.hpp>

class CommonEvent
{
public:
	using CommonEvents = std::vector<CommonEvent>;

public:
	CommonEvent()
	{
	}

	explicit CommonEvent(FileCoder& coder) :
		m_valid(false),
		m_unknown10Valid(false)
	{
		m_valid = init(coder);
	}

	void Dump(FileCoder& coder) const
	{
		coder.WriteByte(0x8E);
		coder.WriteInt(m_id);
		coder.WriteInt(m_unknown1);
		coder.Write(m_unknown2);
		coder.WriteString(m_name);
		coder.WriteInt(m_commands.size());

		for (const Command::CommandShPtr::Command& cmd : m_commands)
			cmd->Dump(coder);

		coder.WriteString(m_unknown11);
		coder.WriteString(m_description);
		coder.WriteByte(0x8F);
		coder.WriteInt(static_cast<uint32_t>(m_unknown3.size()));

		for (const tString& str : m_unknown3)
			coder.WriteString(str);

		coder.WriteInt(static_cast<uint32_t>(m_unknown4.size()));
		for (const uint8_t& byte : m_unknown4)
			coder.WriteByte(byte);

		coder.WriteInt(static_cast<uint32_t>(m_unknown5.size()));
		for (const tStrings& strs : m_unknown5)
		{
			coder.WriteInt(strs.size());
			for (const tString& str : strs)
				coder.WriteString(str);
		}

		coder.WriteInt(static_cast<uint32_t>(m_unknown6.size()));
		for (const uInts& uints : m_unknown6)
		{
			coder.WriteInt(uints.size());
			for (const uint32_t& uint : uints)
				coder.WriteInt(uint);
		}

		coder.Write(m_unknown7);
		for (const tString& str : m_unknown8)
			coder.WriteString(str);

		coder.WriteByte(0x91);
		coder.WriteString(m_unknown9);

		if (m_unknown10Valid)
		{
			coder.WriteByte(0x92);
			coder.WriteString(m_unknown10);
			coder.WriteInt(m_unknown12);
			coder.WriteByte(0x92);
		}
		else
			coder.WriteByte(0x91);
	}

	nlohmann::ordered_json ToJson() const
	{
		nlohmann::ordered_json j;

		j["id"]          = m_id;
		j["name"]        = ToUTF8(m_name);
		j["description"] = ToUTF8(m_description);
		j["commands"]    = nlohmann::ordered_json::array();

		for (std::size_t i = 0; i < m_commands.size(); i++)
		{
			nlohmann::ordered_json cmdJ = m_commands[i]->ToJson();

			if (!cmdJ.empty())
			{
				cmdJ["index"] = i;
				j["commands"].push_back(cmdJ);
			}
		}

		return j;
	}

	void Patch(const nlohmann::ordered_json& j)
	{
		if (!j.contains("id"))
			throw WolfRPGException(ERROR_TAG "Field 'id' not found in patch");

		const uint32_t id = j["id"].get<uint32_t>();
		if (id != m_id)
			throw WolfRPGException(ERROR_TAG "ID mismatch in patch (expected " + std::to_string(m_id) + ", got " + std::to_string(id) + ")");

		if (!j.contains("name"))
			throw WolfRPGException(ERROR_TAG "Field 'name' not found in patch");

		if (!j.contains("description"))
			throw WolfRPGException(ERROR_TAG "Field 'description' not found in patch");

		if (!j.contains("commands"))
			throw WolfRPGException(ERROR_TAG "Field 'commands' not found in patch");

		m_name        = ToUTF16(j["name"].get<std::string>());
		m_description = ToUTF16(j["description"].get<std::string>());

		for (const auto& cmdJ : j["commands"])
		{
			if (!cmdJ.contains("index"))
				throw WolfRPGException(ERROR_TAG "Field 'index' not found in patch");

			const uint32_t index = cmdJ["index"].get<uint32_t>();
			if (index >= m_commands.size())
				throw WolfRPGException(ERROR_TAG "Index out of range: " + std::to_string(index) + " >= " + std::to_string(m_commands.size()));

			m_commands[index]->Patch(cmdJ);
		}
	}

	const bool& IsValid() const
	{
		return m_valid;
	}

	const uint32_t& GetID() const
	{
		return m_id;
	}

	const tString& GetName() const
	{
		return m_name;
	}

	const Command::Commands& GetCommands() const
	{
		return m_commands;
	}

private:
	bool init(FileCoder& coder)
	{
		uint8_t indicator = coder.ReadByte();
		if (indicator != 0x8E)
			throw WolfRPGException(ERROR_TAG "CommonEvent header indicator not 0x8E (got 0x" + Dec2Hex(indicator) + ")");

		m_id = coder.ReadInt();

		m_unknown1 = coder.ReadInt();
		m_unknown2 = coder.Read(7);
		m_name     = coder.ReadString();

		uint32_t commandCnt = coder.ReadInt();
		for (uint32_t i = 0; i < commandCnt; i++)
		{
			Command::CommandShPtr::Command command = Command::Command::Init(coder);

			if (!command->Valid())
				throw WolfRPGException(ERROR_TAG "Command initialization failed");

			m_commands.push_back(command);
		}

		m_unknown11   = coder.ReadString();
		m_description = coder.ReadString();

		indicator = coder.ReadByte();
		if (indicator != 0x8F)
			throw WolfRPGException(ERROR_TAG "CommonEvent data indicator not 0x8F (got " + Dec2Hex(indicator) + ")");

		m_unknown3.resize(coder.ReadInt());
		for (tString& str : m_unknown3)
			str = coder.ReadString();

		m_unknown4.resize(coder.ReadInt());
		for (uint8_t& byte : m_unknown4)
			byte = coder.ReadByte();

		m_unknown5.resize(coder.ReadInt());
		for (tStrings& strs : m_unknown5)
		{
			strs = tStrings(coder.ReadInt());
			for (tString& str : strs)
				str = coder.ReadString();
		}

		m_unknown6.resize(coder.ReadInt());
		for (uInts& uints : m_unknown6)
		{
			uints = uInts(coder.ReadInt());
			for (uint32_t& uint : uints)
				uint = coder.ReadInt();
		}

		m_unknown7 = coder.Read(0x1D);
		for (tString& str : m_unknown8)
			str = coder.ReadString();

		indicator = coder.ReadByte();
		if (indicator != 0x91)
			throw WolfRPGException(ERROR_TAG "CommonEvent data indicator not 0x91 (got " + Dec2Hex(indicator) + ")");

		m_unknown9 = coder.ReadString();

		indicator = coder.ReadByte();
		if (indicator != 0x92)
		{
			if (indicator == 0x91) return true;

			throw WolfRPGException(ERROR_TAG "CommonEvent data indicator not 0x92 or 0x91 (got " + Dec2Hex(indicator) + ")");
		}

		m_unknown10Valid = true;
		m_unknown10      = coder.ReadString();
		m_unknown12      = coder.ReadInt();

		indicator = coder.ReadByte();
		if (indicator != 0x92)
			throw WolfRPGException(ERROR_TAG "CommonEvent data indicator not 0x92 (got " + Dec2Hex(indicator) + ")");

		return true;
	}

private:
	bool m_valid = false;

	uint32_t m_id                       = 0;
	uint32_t m_unknown1                 = 0;
	Bytes m_unknown2                    = {};
	tString m_name                      = TEXT("");
	Command::Commands m_commands        = {};
	tString m_unknown11                 = TEXT("");
	tString m_description               = TEXT("");
	std::vector<tString> m_unknown3     = {};
	std::vector<uint8_t> m_unknown4     = {};
	std::vector<tStrings> m_unknown5    = {};
	std::vector<uInts> m_unknown6       = {};
	Bytes m_unknown7                    = {};
	std::array<tString, 100> m_unknown8 = {};
	tString m_unknown9                  = TEXT("");
	tString m_unknown10                 = TEXT("");
	uint32_t m_unknown12                = 0;

	bool m_unknown10Valid = false;
};

class CommonEvents
{
public:
	CommonEvents() :
		m_valid(false)
	{
	}

	explicit CommonEvents(const tString& fileName) :
		m_valid(false),
		m_fileName(fileName)
	{
		m_valid = init(fileName);
	}

	void Dump(const tString& outputDir) const
	{
		tString outputFN = outputDir + L"/" + GetFileName(m_fileName);
		FileCoder coder(outputFN, FileCoder::Mode::WRITE, DAT_SEED_INDICES);

		coder.Write(MAGIC_NUMBER);

		coder.WriteByte(m_startIndicator);
		coder.WriteInt(m_events.size());
		for (const CommonEvent& ev : m_events)
			ev.Dump(coder);

		coder.WriteByte(m_terminator);
	}

	void ToJson(const tString& outputFolder) const
	{
		for (const CommonEvent& ev : m_events)
		{
			nlohmann::ordered_json j = ev.ToJson();

			// Get the file name without the extension
			const tString comEvName  = std::format(TEXT("{}_{}"), ev.GetID(), EscapePath(ev.GetName()));
			const tString outputFile = outputFolder + L"/" + comEvName + L".json";

			std::ofstream out(outputFile);
			out << j.dump(4);

			out.close();
		}
	}

	void Patch(const tString& patchFolder)
	{
		for (CommonEvent& ev : m_events)
		{
			const tString comEvName = std::format(TEXT("{}_{}"), ev.GetID(), EscapePath(ev.GetName()));
			const tString patchFile = patchFolder + L"/" + comEvName + L".json";

			if (!std::filesystem::exists(patchFile))
				throw WolfRPGException(ERROR_TAG L"Patch file not found: " + patchFile);

			std::ifstream in(patchFile);
			nlohmann::ordered_json j;
			in >> j;
			in.close();

			ev.Patch(j);
		}
	}

	const CommonEvent::CommonEvents& GetEvents() const
	{
		return m_events;
	}

	const bool& IsValid() const
	{
		return m_valid;
	}

private:
	bool init(const tString& fileName)
	{
		FileCoder coder(fileName, FileCoder::Mode::READ, DAT_SEED_INDICES);
		VERIFY_MAGIC(coder, MAGIC_NUMBER);
		m_startIndicator  = coder.ReadByte();
		uint32_t eventCnt = coder.ReadInt();
		m_events          = CommonEvent::CommonEvents(eventCnt);

		for (uint32_t i = 0; i < eventCnt; i++)
		{
			CommonEvent ev       = CommonEvent(coder);
			m_events[ev.GetID()] = ev;
		}

		m_terminator = coder.ReadByte();
		if (m_terminator < 0x89)
			throw WolfRPGException(ERROR_TAG "CommonEvent data terminator smaller than 0x89 (got " + Dec2Hex(m_terminator) + ")");

		if (!coder.IsEof())
			throw WolfRPGException(ERROR_TAG "CommonEvent has more data than expected");

		return true;
	}

private:
	bool m_valid = false;

	CommonEvent::CommonEvents m_events = {};

	tString m_fileName    = TEXT("");
	BYTE m_startIndicator = 0;
	BYTE m_terminator     = 0;

	inline static const uInts DAT_SEED_INDICES{ 0, 3, 9 };
	inline static const MagicNumber MAGIC_NUMBER = { { 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x00, 0x46, 0x43, 0x00 }, 5 };
};