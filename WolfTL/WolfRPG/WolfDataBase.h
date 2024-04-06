#pragma once

#include <format>
#include <fstream>
#include <nlohmann\json.hpp>

#include "FileCoder.h"
#include "Types.h"

class WolfDataBase
{
public:
	WolfDataBase(const tString& fileName, const MagicNumber& magic, const bool& isDB = false, const uInts& seedIndices = {}) :
		m_fileName(fileName),
		m_magic(magic),
		m_isDB(isDB),
		m_seedIndices(seedIndices)
	{
	}

	virtual ~WolfDataBase() = default;

	bool Load(const tString& fileName)
	{
		m_fileName = fileName;

		if (m_fileName.empty())
			throw WolfRPGException(ERROR_TAG + "Trying to load with empty filename");

		FileCoder coder(m_fileName, FileCoder::Mode::READ, m_isDB, m_seedIndices);

		if (coder.IsEncrypted())
			m_cryptHeader = coder.GetCryptHeader();
		else
			VERIFY_MAGIC(coder, m_magic);

		return load(coder);
	}

	void Dump(const tString& outputDir) const
	{
		tString outputFN = outputDir + L"/" + ::GetFileName(m_fileName);
		FileCoder coder(outputFN, FileCoder::Mode::WRITE, m_isDB, m_seedIndices);
		dump(coder);
	}

	virtual void ToJson(const tString& outputFolder) const
	{
		const tString outputFile = std::format(TEXT("{}/{}.json"), outputFolder, ::GetFileNameNoExt(m_fileName));

		std::ofstream out(outputFile);
		out << toJson().dump(4);

		out.close();
	}

	virtual void Patch(const tString& patchFolder)
	{
		const tString patchFile = patchFolder + L"/" + ::GetFileNameNoExt(m_fileName) + L".json";
		if (!fs::exists(patchFile))
			throw WolfRPGException(ERROR_TAGW + L"Patch file not found: " + patchFile);

		nlohmann::ordered_json j;
		std::ifstream in(patchFile);
		in >> j;
		in.close();

		patch(j);
	}

	const tString& FileName() const
	{
		return m_fileName;
	}

protected:
	virtual bool load(FileCoder& coder)                 = 0;
	virtual void dump(FileCoder& coder) const           = 0;
	virtual nlohmann::ordered_json toJson() const       = 0;
	virtual void patch(const nlohmann::ordered_json& j) = 0;

protected:
	tString m_fileName;
	MagicNumber m_magic;
	bool m_isDB;
	uInts m_seedIndices = {};

	Bytes m_cryptHeader = {};
};