#pragma once

#include "Types.h"
#include "WolfRPGUtils.h"
#include "WolfRPGException.h"

#include <array>
#include <iostream>

static const size_t CRYPT_HEADER_SIZE   = 10;
static const size_t DECRYPT_INTERVALS[] = { 1, 2, 5 };

#define CHECK_FILE_OPEN \
	if (m_pFile == nullptr) throw WolfRPGException(ERROR_TAG "Trying to read from unopened file.");

enum MODE
{
	READ,
	WRITE
};

class FileCoder
{
public:
	// Disable Copy/Move constructor
	DISABLE_COPY_MOVE(FileCoder)

	FileCoder(const tString& fileName, const MODE mode, const uInts& seedIndices = uInts(), const Bytes& cryptHeader = Bytes()) :
		m_fileName(fileName),
		m_cryptHeader(cryptHeader)
	{
		_wfopen_s(&m_pFile, fileName.c_str(), (mode == READ ? L"rb" : L"wb"));

		if (m_pFile == nullptr)
			throw WolfRPGException(ERROR_TAG L"Unable to open file: " + fileName);

		m_size = fs::file_size(fileName);

		if (mode == READ)
		{
			if (seedIndices.empty()) return;

			uint8_t indicator = ReadByte();

			if (indicator == 0x0) return;

			Bytes header(CRYPT_HEADER_SIZE);
			header[0] = indicator;

			for (int i = 1; i < CRYPT_HEADER_SIZE; i++)
				header[i] = ReadByte();

			Bytes seeds;
			for (size_t i = 0; i < seedIndices.size(); i++)
				seeds.push_back(header[seedIndices[i]]);

			m_cryptHeader = header;

			Bytes data = Read();
			crypt(data, seeds);
			// TODO: In order for reading from encrypted files to work more stuff is required
			// the unencrypted data should be inside "data" needs to be made accesible somehow
		}
		else if (mode == WRITE)
		{
			if (!seedIndices.empty() && !cryptHeader.empty())
			{
				Write(cryptHeader);
			}
			else
			{
				if (!seedIndices.empty())
					WriteByte(0);
			}
		}
	}

	~FileCoder()
	{
		fclose(m_pFile);
	}

	const Bytes& GetCryptHeader() const
	{
		return m_cryptHeader;
	}

	bool IsEncrypted() const
	{
		return !m_cryptHeader.empty();
	}

	uint32_t Tell() const
	{
		if (m_pFile == nullptr)
			return static_cast<uint32_t>(-1);

		if (IsEncrypted())
			return ftell(m_pFile) + CRYPT_HEADER_SIZE;
		else
			return ftell(m_pFile);
	}

	bool IsEof() const
	{
		return (feof(m_pFile) == 0);
	}

	Bytes Read(const size_t size = -1)
	{
		Bytes data;

		CHECK_FILE_OPEN;

		if (size != -1)
		{
			data.resize(size);
			fread(data.data(), sizeof(uint8_t), size, m_pFile);
		}
		else
		{
			long curPos = ftell(m_pFile);
			fseek(m_pFile, 0, SEEK_END);
			long endPos = ftell(m_pFile);
			fseek(m_pFile, curPos, SEEK_SET);

			long remainingSize = endPos - curPos;
			data.resize(remainingSize);

			fread(data.data(), sizeof(uint8_t), remainingSize, m_pFile);
		}

		return data;
	}

	uint8_t ReadByte()
	{
		CHECK_FILE_OPEN;

		uint8_t byte;
		fread(&byte, sizeof(uint8_t), 1, m_pFile);

		return byte;
	}

	uint32_t ReadInt()
	{
		CHECK_FILE_OPEN;

		uint32_t data;
		fread(&data, sizeof(uint32_t), 1, m_pFile);

		return data;
	}

	tString ReadString()
	{
		CHECK_FILE_OPEN;

		uint32_t size = ReadInt();
		if (size == 0)
			throw WolfRPGException(ERROR_TAG "Zero length string encountered.");

		Bytes data   = Read(size);
		tString wstr = sjis2utf8(data);
		return wstr;
	}

	Bytes ReadByteArray()
	{
		uint32_t size = ReadInt();
		Bytes data;

		for (uint32_t i = 0; i < size; i++)
			data.push_back(ReadByte());

		return data;
	}

	uInts ReadIntArray()
	{
		uint32_t size = ReadInt();
		uInts data;

		for (uint32_t i = 0; i < size; i++)
			data.push_back(ReadInt());

		return data;
	}

	tStrings ReadStringArray()
	{
		uint32_t size = ReadInt();
		tStrings data;

		for (uint32_t i = 0; i < size; i++)
			data.push_back(ReadString());

		return data;
	}

	bool Verify(const Bytes& vData)
	{
		Bytes data = Read(vData.size());
		if (std::equal(vData.begin(), vData.end(), data.begin()))
			return true;

		return false;
	}

	void Skip(const long size)
	{
		CHECK_FILE_OPEN;
		fseek(m_pFile, size, SEEK_CUR);
	}

	void Write(Bytes data)
	{
		CHECK_FILE_OPEN;

		fwrite(data.data(), sizeof(uint8_t), data.size(), m_pFile);
	}

	void WriteByte(uint8_t data)
	{
		CHECK_FILE_OPEN;

		fwrite(&data, sizeof(uint8_t), 1, m_pFile);
	}

#ifdef BIT_64
	// For 64 bit an additional method is required to properly handle size_t inputs
	void WriteInt(size_t data)
	{
		WriteInt(static_cast<uint32_t>(data));
	}
#endif

	void WriteInt(uint32_t data)
	{
		CHECK_FILE_OPEN;

		fwrite(&data, sizeof(uint32_t), 1, m_pFile);
	}

	void WriteString(const tString& wstr)
	{
		CHECK_FILE_OPEN;

		Bytes str = utf82sjis(wstr);
		WriteInt(static_cast<uint32_t>(str.size()));
		Write(str);
	}

	void WriteByteArray(Bytes data)
	{
		CHECK_FILE_OPEN;

		WriteInt(static_cast<uint32_t>(data.size()));
		for (uint8_t byte : data)
			WriteByte(byte);
	}

	void WriteIntArray(uInts data)
	{
		CHECK_FILE_OPEN;

		WriteInt(static_cast<uint32_t>(data.size()));
		for (uint32_t uint : data)
			WriteInt(uint);
	}

	void WriteStringArray(tStrings strs)
	{
		CHECK_FILE_OPEN;

		WriteInt(static_cast<uint32_t>(strs.size()));
		for (tString str : strs)
			WriteString(str);
	}

private:
	void crypt(Bytes& data, const Bytes& seeds)
	{
		for (size_t i = 0; i < seeds.size(); i++)
		{
			unsigned int seed = seeds.at(i);
			for (size_t j = 0; j < data.size(); j += DECRYPT_INTERVALS[i])
			{
				seed = (seed * 0x343FD + 0x269EC3) & 0xFFFFFFFF;
				data[j] ^= (seed >> 28) & 7;
			}
		}
	}

	const tString sjis2utf8(const Bytes& sjis)
	{
		const LPCCH pSJIS = reinterpret_cast<const LPCCH>(sjis.data());
		int sjisSize      = MultiByteToWideChar(932, 0, pSJIS, -1, NULL, 0);

		WCHAR* pUTF8 = new WCHAR[sjisSize + 1]();
		MultiByteToWideChar(932, 0, pSJIS, -1, pUTF8, sjisSize);
		tString utf8(pUTF8);
		delete[] pUTF8;
		return utf8;
	}

	const Bytes utf82sjis(const tString& utf8)
	{
		// Empty strings are length 1 with terminating 0
		if (utf8.empty()) return Bytes(1, 0);

		int utf8Size = WideCharToMultiByte(932, 0, &utf8[0], (int)utf8.size(), NULL, 0, NULL, NULL);
		Bytes sjis(utf8Size + 1, 0);
		WideCharToMultiByte(932, 0, &utf8[0], (int)utf8.size(), reinterpret_cast<const LPSTR>(sjis.data()), utf8Size, NULL, NULL);
		return sjis;
	}

private:
	tString m_fileName;
	FILE* m_pFile = nullptr;
	Bytes m_cryptHeader;
	std::uintmax_t m_size = 0;
};
