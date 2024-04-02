#pragma once

#include "FileCoder.h"
#include "RouteCommand.h"
#include "WolfRPGUtils.h"

#include <algorithm>
#include <memory>
#include <nlohmann\json.hpp>

namespace Command
{
enum class PictureType
{
	file,
	fileString,
	text,
	windowFile,
	windowString,
	invalid
};

enum class CommandType
{
	Blank              = 0,
	Checkpoint         = 99,
	Message            = 101,
	Choices            = 102,
	Comment            = 103,
	ForceStopMessage   = 105,
	DebugMessage       = 106,
	ClearDebugText     = 107,
	VariableCondition  = 111,
	StringCondition    = 112,
	SetVariable        = 121,
	SetString          = 122,
	InputKey           = 123,
	SetVariableEx      = 124,
	AutoInput          = 125,
	BanInput           = 126,
	Teleport           = 130,
	Sound              = 140,
	Picture            = 150,
	ChangeColor        = 151,
	SetTransition      = 160,
	PrepareTransition  = 161,
	ExecuteTransition  = 162,
	StartLoop          = 170,
	BreakLoop          = 171,
	BreakEvent         = 172,
	EraseEvent         = 173,
	ReturnToTitle      = 174,
	EndGame            = 175,
	StartLoop2         = 176,
	StopNonPic         = 177,
	ResumeNonPic       = 178,
	LoopTimes          = 179,
	Wait               = 180,
	Move               = 201,
	WaitForMove        = 202,
	CommonEvent        = 210,
	CommonEventReserve = 211,
	SetLabel           = 212,
	JumpLabel          = 213,
	SaveLoad           = 220,
	LoadGame           = 221,
	SaveGame           = 222,
	MoveDuringEventOn  = 230,
	MoveDuringEventOff = 231,
	Chip               = 240,
	ChipSet            = 241,
	Database           = 250,
	ImportDatabase     = 251,
	Party              = 270,
	MapEffect          = 280,
	ScrollScreen       = 281,
	Effect             = 290,
	CommonEventByName  = 300,
	ChoiceCase         = 401,
	SpecialChoiceCase  = 402,
	ElseCase           = 420,
	CancelCase         = 421,
	LoopEnd            = 498,
	BranchEnd          = 499,
	Default            = 999,
	Invalid            = -1
};

class Command
{
public:
	Command(const CommandType& cid = CommandType::Default, const uInts& args = uInts(), const tStrings& stringArgs = tStrings(), const uint8_t& indent = -1) :
		m_cid(cid),
		m_args(args),
		m_stringArgs(stringArgs),
		m_indent(indent)
	{
	}

	bool Valid() const
	{
		return (m_cid != CommandType::Invalid);
	}

	static std::shared_ptr<Command> Init(FileCoder& coder);

	void DumpData(FileCoder& coder) const
	{
		coder.WriteByte((uint8_t)m_args.size() + 1);
		coder.WriteInt(static_cast<uint32_t>(m_cid));
		for (uint32_t arg : m_args)
			coder.WriteInt(arg);
		coder.WriteByte(m_indent);
		coder.WriteByte((uint8_t)m_stringArgs.size());
		for (tString arg : m_stringArgs)
			coder.WriteString(arg);
	}

	virtual void Dump(FileCoder& coder) const
	{
		DumpData(coder);
		DumpTerminator(coder);
	}

	virtual nlohmann::ordered_json ToJson() const
	{
		if (m_stringArgs.empty() && m_args.empty())
			return nlohmann::ordered_json();

		nlohmann::ordered_json json;
		json["code"]    = static_cast<int32_t>(m_cid);
		json["codeStr"] = ToUTF8(GetClassString());

		if (!m_stringArgs.empty())
		{
			json["stringArgs"] = nlohmann::ordered_json::array();

			for (const tString& arg : m_stringArgs)
				json["stringArgs"].push_back(ToUTF8(arg));
		}

		if (!m_args.empty())
		{
			json["intArgs"] = nlohmann::ordered_json::array();

			for (const uint32_t& arg : m_args)
				json["intArgs"].push_back(arg);
		}

		return json;
	}

	virtual void Patch(const nlohmann::ordered_json& j)
	{
		CHECK_JSON_KEY(j, "code", "command");

		if (j.contains("stringArgs"))
		{
			m_stringArgs.clear();
			for (const auto& arg : j["stringArgs"])
				m_stringArgs.push_back(ToUTF16(arg));
		}

		if (j.contains("intArgs"))
		{
			m_args.clear();
			for (const auto& arg : j["intArgs"])
				m_args.push_back(arg);
		}
	}

	bool IsUpdatable() const
	{
		return (!m_stringArgs.empty());
	}

	const CommandType GetType() const
	{
		return m_cid;
	}

	const tString GetClassString() const
	{
		switch (m_cid)
		{
			case CommandType::Blank:
				return L"Blank";
			case CommandType::Checkpoint:
				return L"Checkpoint";
			case CommandType::Message:
				return L"Message";
			case CommandType::Choices:
				return L"Choices";
			case CommandType::Comment:
				return L"Comment";
			case CommandType::ForceStopMessage:
				return L"ForceStopMessage";
			case CommandType::DebugMessage:
				return L"DebugMessage";
			case CommandType::ClearDebugText:
				return L"ClearDebugText";
			case CommandType::VariableCondition:
				return L"VariableCondition";
			case CommandType::StringCondition:
				return L"StringCondition";
			case CommandType::SetVariable:
				return L"SetVariable";
			case CommandType::SetString:
				return L"SetString";
			case CommandType::InputKey:
				return L"InputKey";
			case CommandType::SetVariableEx:
				return L"SetVariableEx";
			case CommandType::AutoInput:
				return L"AutoInput";
			case CommandType::BanInput:
				return L"BanInput";
			case CommandType::Teleport:
				return L"Teleport";
			case CommandType::Sound:
				return L"Sound";
			case CommandType::Picture:
				return L"Picture";
			case CommandType::ChangeColor:
				return L"ChangeColor";
			case CommandType::SetTransition:
				return L"SetTransition";
			case CommandType::PrepareTransition:
				return L"PrepareTransition";
			case CommandType::ExecuteTransition:
				return L"ExecuteTransition";
			case CommandType::StartLoop:
				return L"StartLoop";
			case CommandType::BreakLoop:
				return L"BreakLoop";
			case CommandType::BreakEvent:
				return L"BreakEvent";
			case CommandType::EraseEvent:
				return L"EraseEvent";
			case CommandType::ReturnToTitle:
				return L"ReturnToTitle";
			case CommandType::EndGame:
				return L"EndGame";
			case CommandType::StartLoop2:
				return L"StartLoop";
			case CommandType::StopNonPic:
				return L"StopNonPic";
			case CommandType::ResumeNonPic:
				return L"ResumeNonPic";
			case CommandType::LoopTimes:
				return L"LoopTimes";
			case CommandType::Wait:
				return L"Wait";
			case CommandType::Move:
				return L"Move";
			case CommandType::WaitForMove:
				return L"WaitForMove";
			case CommandType::CommonEvent:
				return L"CommonEvent";
			case CommandType::CommonEventReserve:
				return L"CommonEventReserve";
			case CommandType::SetLabel:
				return L"SetLabel";
			case CommandType::JumpLabel:
				return L"JumpLabel";
			case CommandType::SaveLoad:
				return L"SaveLoad";
			case CommandType::LoadGame:
				return L"LoadGame";
			case CommandType::SaveGame:
				return L"SaveGame";
			case CommandType::MoveDuringEventOn:
				return L"MoveDuringEventOn";
			case CommandType::MoveDuringEventOff:
				return L"MoveDuringEventOff";
			case CommandType::Chip:
				return L"Chip";
			case CommandType::ChipSet:
				return L"ChipSet";
			case CommandType::Database:
				return L"Database";
			case CommandType::ImportDatabase:
				return L"ImportDatabase";
			case CommandType::Party:
				return L"Party";
			case CommandType::MapEffect:
				return L"MapEffect";
			case CommandType::ScrollScreen:
				return L"ScrollScreen";
			case CommandType::Effect:
				return L"Effect";
			case CommandType::CommonEventByName:
				return L"CommonEventByName";
			case CommandType::ChoiceCase:
				return L"ChoiceCase";
			case CommandType::SpecialChoiceCase:
				return L"SpecialChoiceCase";
			case CommandType::ElseCase:
				return L"ElseCase";
			case CommandType::CancelCase:
				return L"CancelCase";
			case CommandType::LoopEnd:
				return L"LoopEnd";
			case CommandType::BranchEnd:
				return L"BranchEnd";
			case CommandType::Default:
				return L"Command";
			default:
				return L"Command";
		}
	}

	virtual const tString Text() const
	{
		if (m_stringArgs.empty()) return L"";
		return m_stringArgs.at(0);
	}

	virtual void SetText(const tString& value, const uint32_t& index)
	{
		if (m_stringArgs.size() <= index)
			throw WolfRPGException(ERROR_TAGW + L"setText(" + value + L", " + std::to_wstring(index) + L") out of range");

		m_stringArgs[index] = value;
	}

	virtual const tStrings& Texts() const
	{
		return m_stringArgs;
	}

	virtual const PictureType Type() const
	{
		return PictureType::invalid;
	}

	virtual const uint8_t Num() const
	{
		return 0xFF;
	}

	virtual const tString Filename() const
	{
		return L"";
	}

	virtual void SetFilename([[maybe_unused]] const tString& value)
	{
	}

	virtual void DumpTerminator(FileCoder& coder) const
	{
		coder.WriteByte(TERMINATOR);
	}

protected:
	uInts m_args;
	CommandType m_cid;
	tStrings m_stringArgs;
	uint8_t m_indent;

	static const uint8_t TERMINATOR = 0x0;
};

namespace CommandSpecialClasses
{
class Picture : public Command
{
public:
	Picture(const CommandType& cid, const uInts& args, const tStrings& stringArgs, const uint8_t& indent) :
		Command(cid, args, stringArgs, indent)
	{
	}

	virtual const PictureType Type() const
	{
		uint8_t t = (m_args[0] >> 4) & 0x07;
		switch (t)
		{
			case 0:
				return PictureType::file;
			case 1:
				return PictureType::fileString;
			case 2:
				return PictureType::text;
			case 3:
				return PictureType::windowFile;
			case 4:
				return PictureType::windowString;
			default:
				return PictureType::invalid;
		}
	}

	virtual const uint8_t Num() const
	{
		return static_cast<uint8_t>(m_args[1]);
	}

	virtual const tString Text() const
	{
		if (Type() != PictureType::text)
			throw WolfRPGException(ERROR_TAG + "Picture type \"" + std::to_string(static_cast<int32_t>(Type())) + "\" has no text");

		if (m_stringArgs.empty())
			return L"";

		return m_stringArgs.at(0);
	}

	virtual void SetText(const tString& value)
	{
		if (Type() != PictureType::text)
			throw WolfRPGException(ERROR_TAG + "Picture type \"" + std::to_string(static_cast<int32_t>(Type())) + "\" has no text");

		if (m_stringArgs.empty())
			m_stringArgs.push_back(value);
		else
			m_stringArgs[0] = value;
	}

	virtual const tString Filename() const
	{
		if (Type() != PictureType::file && Type() != PictureType::windowFile)
			throw WolfRPGException(ERROR_TAG + "Picture type \"" + std::to_string(static_cast<int32_t>(Type())) + "\" has no text");

		return m_stringArgs[0];
	}

	virtual void SetFilename(const tString& value)
	{
		if (Type() != PictureType::file && Type() != PictureType::windowFile)
			throw WolfRPGException(ERROR_TAG + "Picture type \"" + std::to_string(static_cast<int32_t>(Type())) + "\" has no text");

		m_stringArgs[0] = value;
	}
};

class Move : public Command
{
public:
	Move(const CommandType& cid, const uInts& args, const tStrings& stringArgs, const uint8_t& indent, FileCoder& coder) :
		Command(cid, args, stringArgs, indent)
	{
		// Read unknown data
		for (uint32_t i = 0; i < 5; i++)
			m_unknown.push_back(coder.ReadByte());
		// Read known data
		m_flags = coder.ReadByte();

		// Read route
		uint32_t routeCount = coder.ReadInt();
		for (uint32_t i = 0; i < routeCount; i++)
		{
			RouteCommand rc;
			if (!rc.Init(coder))
				throw WolfRPGException(ERROR_TAG + "RouteCommand initialization failed");

			m_route.push_back(rc);
		}
	}

	virtual void Dump(FileCoder& coder) const
	{
		DumpData(coder);
		DumpTerminator(coder);
	}

	virtual void DumpTerminator(FileCoder& coder) const
	{
		coder.WriteByte(1);
		for (uint8_t byte : m_unknown)
			coder.WriteByte(byte);

		coder.WriteByte(m_flags);
		coder.WriteInt(static_cast<uint32_t>(m_route.size()));
		for (RouteCommand cmd : m_route)
			cmd.Dump(coder);
	}

private:
	Bytes m_unknown       = {};
	uint8_t m_flags       = 0;
	RouteCommands m_route = {};
};
}; // namespace CommandSpecialClasses

namespace CommandShPtr
{
using Command = std::shared_ptr<Command>;
using Move    = std::shared_ptr<CommandSpecialClasses::Move>;
using Picture = std::shared_ptr<CommandSpecialClasses::Picture>;
} // namespace CommandShPtr

inline CommandShPtr::Command Command::Command::Init(FileCoder& coder)
{
	uint8_t argsCount = coder.ReadByte() - 1;
	CommandType cid   = static_cast<CommandType>(coder.ReadInt());
	uInts args;

	for (uint8_t i = 0; i < argsCount; i++)
		args.push_back(coder.ReadInt());

	uint8_t indent = coder.ReadByte();
	argsCount      = coder.ReadByte();

	tStrings stringArgs;

	for (uint8_t i = 0; i < argsCount; i++)
		stringArgs.push_back(coder.ReadString());

	uint8_t terminator = coder.ReadByte();
	if (terminator == 0x01)
		return std::make_shared<CommandSpecialClasses::Move>(cid, args, stringArgs, indent, coder);
	else if (terminator != TERMINATOR)
		throw WolfRPGException(ERROR_TAG + "Unexpected command terminator: " + std::to_string(terminator));

	switch (cid)
	{
		case CommandType::Picture:
			return std::make_shared<CommandSpecialClasses::Picture>(cid, args, stringArgs, indent);
		case CommandType::Move:
			return std::make_shared<CommandSpecialClasses::Move>(cid, args, stringArgs, indent, coder);
		default:
			return std::make_shared<Command>(cid, args, stringArgs, indent);
	}
}

static const tStrings stringsOfCommand(const CommandShPtr::Command& command)
{
	tStrings strs = tStrings();
	if (!command->Valid()) return strs;

	switch (command->GetType())
	{
		case CommandType::Message:
		case CommandType::SetString:
		case CommandType::Database:
			strs.push_back(command->Text());
			break;
		case CommandType::Choices:
		case CommandType::StringCondition:
			return command->Texts();
		case CommandType::Picture:
			if (command->Type() == PictureType::text)
				strs.push_back(command->Text());
			break;
		case CommandType::CommonEventByName:
			for (size_t i = 1; i <= 3; i++)
				strs.push_back(command->Texts().at(i));
			break;
		default:
			break;
	}

	return strs;
}

using Commands = std::vector<CommandShPtr::Command>;
} // namespace Command
