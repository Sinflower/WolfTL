#pragma once

#include "FileCoder.h"
#include "WolfRPGUtils.h"

class RouteCommand
{
public:
	RouteCommand() = default;

	bool Init(FileCoder& coder)
	{
		m_id              = coder.ReadByte();
		uint32_t argCount = coder.ReadByte();

		for (uint32_t i = 0; i < argCount; i++)
			m_args.push_back(coder.ReadInt());

		VERIFY_MAGIC(coder, TERMINATOR);

		return true;
	}

	void Dump(FileCoder& coder) const
	{
		coder.WriteByte(m_id);
		coder.WriteByte((uint8_t)m_args.size());
		for (uint32_t arg : m_args)
			coder.WriteInt(arg);
		coder.Write(TERMINATOR);
	}

private:
	uint8_t m_id = 0;
	uInts m_args = {};

	inline static const Bytes TERMINATOR{ 0x01, 0x00 };
};

using RouteCommands = std::vector<RouteCommand>;
