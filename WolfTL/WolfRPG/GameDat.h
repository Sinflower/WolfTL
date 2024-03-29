#pragma once

#include "FileCoder.h"
#include "WolfRPGUtils.h"
#include <iostream>

class GameDat
{
public:
	explicit GameDat(const tString& fileName = L"") :
		m_fileName(fileName)
	{
		if (!fileName.empty())
			Load(fileName);
	}

	bool Load(const tString& fileName)
	{
		m_fileName = fileName;

		if (m_fileName.empty())
			throw WolfRPGException(ERROR_TAG "Trying to load with empty filename");

		FileCoder coder(fileName, FileCoder::Mode::READ, SEED_INDICES);
		if (coder.IsEncrypted())
			m_cryptHeader = coder.GetCryptHeader();
		else
			VERIFY_MAGIC(coder, MAGIC_NUMBER)

		m_unknown1    = coder.ReadByteArray();
		m_fileVersion = coder.ReadInt();

		m_title = coder.ReadString();

		m_magicString = coder.ReadString();

		if (m_magicString != MAGIC_STRING)
			throw WolfRPGException(ERROR_TAG L"Invalid magic string: \"" + m_magicString + L"\" expected: \"" + MAGIC_STRING + L"\"");

		m_unknown2 = coder.ReadByteArray();
		m_font     = coder.ReadString();

		m_subFonts;

		for (int i = 0; i < 3; i++)
			m_subFonts.push_back(coder.ReadString());

		m_defaultPCGraphic = coder.ReadString();

		if (m_fileVersion >= 9)
			m_version = coder.ReadString();

		m_unknown3 = coder.ReadInt();

		m_unknown4 = coder.Read();

		if (!coder.IsEof())
			throw WolfRPGException(ERROR_TAG L"GameDat has more data than expected");

		return true;
	}

	void DumpConsole() const
	{
		std::wcout << "Filename: " << m_fileName << std::endl;
		std::wcout << "File Version: " << m_fileVersion << std::endl;
		std::wcout << "Title: " << m_title << std::endl;
		std::wcout << "Magic String: " << m_magicString << std::endl;
		std::wcout << "Font: " << m_font << std::endl;

		for (size_t i = 0; i < m_subFonts.size(); i++)
			std::wcout << "SubFont[" << i << "]: " << m_subFonts.at(i) << std::endl;

		std::wcout << "Default PC Graphic: " << m_defaultPCGraphic << std::endl;
		std::wcout << "Version: " << m_version << std::endl;
	}

	void Dump(const tString& outputDir) const
	{
		tString outputFN = outputDir + L"/" + GetFileName(m_fileName);
		FileCoder coder(outputFN, FileCoder::Mode::WRITE, SEED_INDICES, m_cryptHeader);

		if (m_cryptHeader.empty())
			coder.Write(MAGIC_NUMBER);

		coder.WriteByteArray(m_unknown1);
		coder.WriteInt(m_fileVersion);
		coder.WriteString(m_title);
		coder.WriteString(MAGIC_STRING);
		coder.WriteByteArray(m_unknown2);
		coder.WriteString(m_font);
		for (tString font : m_subFonts)
			coder.WriteString(font);
		coder.WriteString(m_defaultPCGraphic);
		if (m_fileVersion >= 9) coder.WriteString(m_version);
		coder.WriteInt(m_unknown3);
		coder.Write(m_unknown4);
	}

	const tString& GetTitle() const
	{
		return m_title;
	}

	const tString& GetVersion() const
	{
		return m_version;
	}

	const tString& GetFont() const
	{
		return m_font;
	}

	const tStrings& GetSubFonts() const
	{
		return m_subFonts;
	}

private:
	tString m_fileName;

	Bytes m_cryptHeader = {};

	Bytes m_unknown1           = {};
	uint32_t m_fileVersion     = 0;
	tString m_title            = TEXT("");
	tString m_magicString      = TEXT("");
	Bytes m_unknown2           = {};
	tString m_font             = TEXT("");
	tStrings m_subFonts        = {};
	tString m_defaultPCGraphic = TEXT("");
	tString m_version          = TEXT("");
	uint32_t m_unknown3        = 0;
	Bytes m_unknown4           = {};

	inline static const uInts SEED_INDICES{ 0, 8, 6 };
	inline static const MagicNumber MAGIC_NUMBER{ { 0x57, 0x00, 0x00, 0x4f, 0x4c, 0x00, 0x46, 0x4d, 0x00 }, 8 };
	inline static const tString MAGIC_STRING = L"0000-0000";
};