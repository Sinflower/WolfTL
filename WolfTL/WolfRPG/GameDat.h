#pragma once

#include "FileCoder.h"
#include "WolfRPGUtils.h"

#include <iostream>
#include <nlohmann\json.hpp>

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
			throw WolfRPGException(ERROR_TAG + "Trying to load with empty filename");

		FileCoder coder(fileName, FileCoder::Mode::READ, false, SEED_INDICES);
		if (coder.IsEncrypted())
			m_cryptHeader = coder.GetCryptHeader();
		else
			VERIFY_MAGIC(coder, MAGIC_NUMBER)

		m_oldSize = coder.GetSize() + static_cast<uint32_t>(m_cryptHeader.size()) - 1;

		m_unknown1    = coder.ReadByteArray();
		m_stringCount = coder.ReadInt();

		// String 0
		m_title = coder.ReadString();

		// String 1
		m_magicString = coder.ReadString();

		if (m_magicString != MAGIC_STRING)
			throw WolfRPGException(ERROR_TAGW + L"Invalid magic string: \"" + m_magicString + L"\" expected: \"" + MAGIC_STRING + L"\"");

		// String 2
		m_decryptKey = coder.ReadByteArray();
		// String 3
		m_font = coder.ReadString();

		// Strings 4-6
		for (uint32_t i = 0; i < 3; i++)
			m_subFonts.push_back(coder.ReadString());

		// String 7
		m_defaultPCGraphic = coder.ReadString();

		// String 8
		if (m_stringCount >= 9)
			m_titlePlus = coder.ReadString();

		// Strings 9-13
		if (m_stringCount > 9)
		{
			m_roadImg = coder.ReadString();
			m_gaugeImg = coder.ReadString();
			m_startUpMsg = coder.ReadString();
			m_titleMsg = coder.ReadString();
		}

		m_fileSize = coder.ReadInt();

		if (m_fileSize != m_oldSize)
			throw WolfRPGException(ERROR_TAG + std::format("Game.dat has different size than expected - {} vs {}", m_fileSize, m_oldSize));

		m_unknown2 = coder.Read();

		if (!coder.IsEof())
			throw WolfRPGException(ERROR_TAG + "Game.dat has more data than expected");

		return true;
	}

	void Dump(const tString& outputDir) const
	{
		tString outputFN = outputDir + L"/" + GetFileName(m_fileName);
		FileCoder coder(outputFN, FileCoder::Mode::WRITE, false, SEED_INDICES);

		coder.Write(MAGIC_NUMBER);

		coder.WriteByteArray(m_unknown1);
		coder.WriteInt(m_stringCount);
		coder.WriteString(m_title);
		coder.WriteString(MAGIC_STRING);
		coder.WriteByteArray(m_decryptKey);
		coder.WriteString(m_font);
		for (const tString& font : m_subFonts)
			coder.WriteString(font);
		coder.WriteString(m_defaultPCGraphic);
		if (m_stringCount >= 9) coder.WriteString(m_titlePlus);

		if (m_stringCount > 9)
		{
			coder.WriteString(m_roadImg);
			coder.WriteString(m_gaugeImg);
			coder.WriteString(m_startUpMsg);
			coder.WriteString(m_titleMsg);
		}

		coder.WriteInt(calcNewSize());
		coder.Write(m_unknown2);
	}

	void ToJson(const tString& outputFolder) const
	{
		nlohmann::ordered_json j;

		j["Title"] = ToUTF8(m_title);
		j["TitlePlus"] = ToUTF8(m_titlePlus);

		if (m_stringCount > 9)
		{
			j["StartUpMsg"] = ToUTF8(m_startUpMsg);
			j["TitleMsg"] = ToUTF8(m_titleMsg);
		}

		const tString outputFile = outputFolder + L"/" + ::GetFileNameNoExt(m_fileName) + L".json";

		std::ofstream out(outputFile);
		out << j.dump(4);

		out.close();
	}

	void Patch(const tString& patchFolder)
	{
		const tString patchFile = patchFolder + L"/" + ::GetFileNameNoExt(m_fileName) + L".json";
		if (!fs::exists(patchFile))
		{
			std::wcerr << ERROR_TAGW << L"Patch file not found: " << patchFile << std::endl;
			return;
		}

		nlohmann::ordered_json j;
		std::ifstream in(patchFile);
		in >> j;
		in.close();

		m_title = ToUTF16(j["Title"]);
		m_titlePlus = ToUTF16(j["TitlePlus"]);

		if (m_stringCount > 9)
		{
			m_startUpMsg = ToUTF16(j["StartUpMsg"]);
			m_titleMsg = ToUTF16(j["TitleMsg"]);
		}
	}

	const tString& GetTitle() const
	{
		return m_title;
	}

	const tString& GetVersion() const
	{
		return m_titlePlus;
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
	std::size_t calcNewSize() const
	{
		std::size_t size = 0;
		size += MAGIC_NUMBER.Size();
		size += m_unknown1.size() + 4;
		size += sizeof(m_stringCount);
		size += FileCoder::CalcStringSize(m_title) + 4;
		size += FileCoder::CalcStringSize(MAGIC_STRING) + 4;
		size += m_decryptKey.size() + 4;
		size += FileCoder::CalcStringSize(m_font) + 4;

		for (const tString& font : m_subFonts)
			size += FileCoder::CalcStringSize(font) + 4;

		size += FileCoder::CalcStringSize(m_defaultPCGraphic) + 4;

		if (m_stringCount >= 9) size += FileCoder::CalcStringSize(m_titlePlus) + 4;

		if (m_stringCount > 9)
		{
			size += FileCoder::CalcStringSize(m_roadImg) + 4;
			size += FileCoder::CalcStringSize(m_gaugeImg) + 4;
			size += FileCoder::CalcStringSize(m_startUpMsg) + 4;
			size += FileCoder::CalcStringSize(m_titleMsg) + 4;
		}

		size += sizeof(m_fileSize);

		size += m_unknown2.size();

		return size;
	}

private:
	tString m_fileName;

	Bytes m_cryptHeader = {};

	Bytes m_unknown1           = {};
	uint32_t m_stringCount     = 0;
	tString m_title            = TEXT("");
	tString m_magicString      = TEXT("");
	Bytes m_decryptKey         = {};
	tString m_font             = TEXT("");
	tStrings m_subFonts        = {};
	tString m_defaultPCGraphic = TEXT("");
	tString m_titlePlus        = TEXT("");
	tString m_roadImg          = TEXT("");
	tString m_gaugeImg         = TEXT("");
	tString m_startUpMsg	   = TEXT("");
	tString m_titleMsg         = TEXT("");
	uint32_t m_fileSize        = 0;
	Bytes m_unknown2           = {};

	uint32_t m_oldSize = 0;

	inline static const uInts SEED_INDICES{ 0, 8, 6 };
	inline static const MagicNumber MAGIC_NUMBER{ { 0x57, 0x00, 0x00, 0x4f, 0x4c, 0x00, 0x46, 0x4d, 0x00 }, 8 };
	inline static const tString MAGIC_STRING = L"0000-0000";
};