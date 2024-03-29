#pragma once

#include "FileAccess.h"
#include "Types.h"
#include "WolfRPGException.h"
#include "WolfRPGUtils.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

class MagicNumber
{
public:
	MagicNumber(const Bytes& data, const int32_t& utf8Idx = -1) :
		m_data(data),
		m_utf8Idx(utf8Idx)
	{
	}

	bool operator==(const Bytes& check) const
	{
		if (std::equal(m_data.begin(), m_data.end(), check.begin()))
			return true;

		if (m_utf8Idx != -1)
		{
			Bytes utf8Data      = m_data;
			utf8Data[m_utf8Idx] = 0x55;
			return std::equal(utf8Data.begin(), utf8Data.end(), check.begin());
		}
		else
			return false;
	}

	const Bytes& GetData() const
	{
		return m_data;
	}

	Bytes GetUTF8Data() const
	{
		Bytes utf8Data      = m_data;
		utf8Data[m_utf8Idx] = 0x55;
		return utf8Data;
	}

	std::size_t Size() const
	{
		return m_data.size();
	}

	bool IsUTF8(const Bytes& data) const
	{
		if (m_utf8Idx == -1) return false;

		return (data[m_utf8Idx] == 0x55);
	}

private:
	Bytes m_data;
	int32_t m_utf8Idx = -1;
};

class FileCoder
{
	static constexpr std::size_t CRYPT_HEADER_SIZE   = 10;
	static constexpr std::size_t DECRYPT_INTERVALS[] = { 1, 2, 5 };

public:
	enum class Mode
	{
		READ,
		WRITE
	};

public:
	// Disable Copy/Move constructor
	DISABLE_COPY_MOVE(FileCoder)

	FileCoder(const tString& fileName, const Mode& mode, const bool& isDB = false, const uInts& seedIndices = uInts(), const Bytes& cryptHeader = Bytes()) :
		m_cryptHeader(cryptHeader),
		m_mode(mode)
	{
		bool isProject = false;

		// If the file extension is .project change the flag
		if (fs::path(fileName).extension() == ".project")
			isProject = true;

		if (mode == Mode::READ)
		{
			m_reader.Open(fileName);

			if (!isProject)
			{
				if (seedIndices.empty()) return;

				uint8_t indicator = ReadByte();

				if (isDB)
				{
					if (m_reader.At(1) != 0x50 || m_reader.At(5) != 0x54 || m_reader.At(7) != 0x4B)
						return;
				}
				else
				{
					if (indicator == 0x0)
						return;
				}

				Bytes header(CRYPT_HEADER_SIZE);
				header[0] = indicator;

				for (int i = 1; i < CRYPT_HEADER_SIZE; i++)
					header[i] = ReadByte();

				Bytes seeds;
				for (size_t i = 0; i < seedIndices.size(); i++)
					seeds.push_back(header[seedIndices[i]]);

				m_cryptHeader = header;

				Bytes data = Read();
				cryptDat(data, seeds);

				s_isUTF8 = true;

				m_reader.InitData(data);
				m_reader.Skip(5);
				uint32_t keySize = m_reader.ReadUInt32();
				int8_t projKey   = m_reader.ReadInt8();

				if (s_projKey == -1)
					s_projKey = projKey;

				m_reader.Skip(keySize - 1);
			}
			else if (s_projKey != -1)
			{
				Bytes data = Read();

				cryptProj(data);

				m_reader.InitData(data);
			}
		}
		else if (mode == Mode::WRITE)
		{
			CreateBackup(fileName);
			m_writer.Open(fileName);

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
	}

	const Bytes& GetCryptHeader() const
	{
		return m_cryptHeader;
	}

	bool IsEncrypted() const
	{
		return !m_cryptHeader.empty();
	}

	void Seek(const int32_t& pos)
	{
		if (m_mode == Mode::READ)
		{
			DWORD o = m_reader.GetOffset() + pos;
			m_reader.Seek(o);
		}
	}

	bool IsEof() const
	{
		if (m_mode == Mode::READ)
			return m_reader.IsEoF();

		return false;
	}

	Bytes Read(const size_t& size = -1)
	{
		Bytes data;

		if (size != -1)
			data.resize(size);
		else
		{
			DWORD remainingSize = m_reader.GetSize() - m_reader.GetOffset();
			data.resize(remainingSize);
		}

		m_reader.ReadBytesVec(data);

		return data;
	}

	uint8_t ReadByte()
	{
		return m_reader.ReadUInt8();
	}

	uint32_t ReadInt()
	{
		return m_reader.ReadUInt32();
	}

	tString ReadString()
	{
		uint32_t size = ReadInt();

		if (size == 0)
			throw WolfRPGException(ERROR_TAG "Zero length string encountered.");

		Bytes data = Read(size);

		if (s_isUTF8)
		{
			std::string str = std::string(reinterpret_cast<const char*>(data.data()), data.size() - ((data.back() == 0x0) ? 1 : 0));
			return ToUTF16(str);
		}
		else
			return sjis2utf8(data);
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

	bool Verify(const MagicNumber& magicNumber)
	{
		Bytes data = Read(magicNumber.Size());
		if (magicNumber == data)
		{
			s_isUTF8 = magicNumber.IsUTF8(data);
			return true;
		}

		return false;
	}

	void Skip(const DWORD& size)
	{
		m_reader.Skip(size);
	}

	void Write(const Bytes& data)
	{
		m_writer.WriteBytesVec(data);
	}

	void Write(const MagicNumber& mn)
	{
		if (s_isUTF8)
			Write(mn.GetUTF8Data());
		else
			Write(mn.GetData());
	}

	void WriteByte(const uint8_t& data)
	{
		m_writer.Write(data);
	}

#ifdef BIT_64
	// For 64 bit an additional method is required to properly handle size_t inputs
	void WriteInt(const size_t& data)
	{
		WriteInt(static_cast<uint32_t>(data));
	}
#endif

	void WriteInt(const uint32_t& data)
	{
		m_writer.Write(data);
	}

	void WriteString(const tString& wstr)
	{
		Bytes str;

		if (s_isUTF8)
		{
			std::string s = ToUTF8(wstr);
			str           = Bytes(s.begin(), s.end());
			str.push_back(0x0);
		}
		else
			str = utf82sjis(wstr);

		WriteInt(static_cast<uint32_t>(str.size()));
		Write(str);
	}

	void WriteByteArray(const Bytes& data)
	{
		WriteInt(static_cast<uint32_t>(data.size()));
		for (uint8_t byte : data)
			WriteByte(byte);
	}

	void WriteIntArray(const uInts& data)
	{
		WriteInt(static_cast<uint32_t>(data.size()));
		for (uint32_t uint : data)
			WriteInt(uint);
	}

	void WriteStringArray(const tStrings& strs)
	{
		WriteInt(static_cast<uint32_t>(strs.size()));
		for (tString str : strs)
			WriteString(str);
	}

	static bool IsUTF8()
	{
		return s_isUTF8;
	}

private:
	void cryptDat(Bytes& data, const Bytes& seeds)
	{
		for (std::size_t i = 0; i < seeds.size(); i++)
		{
			srand(seeds[i]);

			for (std::size_t j = 0; j < data.size(); j += DECRYPT_INTERVALS[i])
				data[j] ^= static_cast<uint8_t>(rand() >> 12);
		}
	}

	void cryptProj(Bytes& data)
	{
		srand(s_projKey);

		for (uint8_t& byte : data)
			byte ^= static_cast<uint8_t>(rand());
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
	Bytes m_cryptHeader;
	Mode m_mode;

	FileReader m_reader = {};
	FileWriter m_writer = {};

	static bool s_isUTF8;
	static uint32_t s_projKey;
};

bool FileCoder::s_isUTF8      = false;
uint32_t FileCoder::s_projKey = -1;
