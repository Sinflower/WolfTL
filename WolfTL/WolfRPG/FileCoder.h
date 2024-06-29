/*
 *  File: FileCoder.h
 *  Copyright (c) 2024 Sinflower
 *
 *  MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#pragma once

#include "FileAccess.h"
#include "NewWolfCrypt.h"
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
		bool isProject       = false;
		bool isMap           = false;
		const bool isGameDat = (fs::path(fileName).filename() == "Game.dat");

		// If the file extension is .project change the flag
		if (fs::path(fileName).extension() == ".project")
			isProject = true;
		else if (fs::path(fileName).extension() == ".mps")
			isMap = true;

		if (mode == Mode::READ)
		{
			m_reader.Open(fileName);

			if (!isProject)
			{
				if (seedIndices.empty() && !isMap) return;

				if (m_reader.At(1) == 0x50)
				{
					Bytes data = Read();
					cryptDatV2(data);

					m_cryptHeader = Bytes(data.begin(), data.begin() + 143);

					m_reader.InitData(data);
					m_reader.Skip(143);

					s_projKey = m_cryptHeader[0x14];
				}
				else if (isMap)
				{
					if (m_reader.At(20) != 0x65) return;

					const Bytes header = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x4F, 0x4C, 0x46, 0x4D, 0x00, 0x55, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x66 };

					const uint32_t headerSize = static_cast<uint32_t>(header.size());

					m_reader.Seek(headerSize);
					uint32_t decDataSize = m_reader.ReadUInt32();
					uint32_t encDataSize = m_reader.ReadUInt32();

					Bytes decData(decDataSize + header.size(), 0);

					lz4Unpack(m_reader.Get(), &decData[headerSize], encDataSize);

					std::memcpy(decData.data(), header.data(), headerSize); // Copy header

					m_reader.InitData(decData);
				}
				else
				{
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
					cryptDatV1(data, seeds);

					m_reader.InitData(data);

					if (isGameDat) return;

					m_reader.Skip(5);
					uint32_t keySize = m_reader.ReadUInt32();
					int8_t projKey   = m_reader.ReadInt8();

					if (s_projKey == -1)
						s_projKey = projKey;

					m_reader.Skip(keySize - 1);
				}
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

	const DWORD& GetSize() const
	{
		return m_reader.GetSize();
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
			throw WolfRPGException(ERROR_TAG + "Zero length string encountered.");

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

	void SetUTF8(const bool& isUTF8)
	{
		s_isUTF8 = isUTF8;
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
	void WriteInt(const std::size_t& data)
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

	static std::size_t CalcStringSize(const tString& str)
	{
		if (s_isUTF8)
			return ToUTF8(str).size() + 1;
		else
			return utf82sjis(str).size();
	}

private:
	void cryptDatV1(Bytes& data, const Bytes& seeds)
	{
		for (std::size_t i = 0; i < seeds.size(); i++)
		{
			srand(seeds[i]);

			for (std::size_t j = 0; j < data.size(); j += DECRYPT_INTERVALS[i])
				data[j] ^= static_cast<uint8_t>(rand() >> 12);
		}
	}

	void cryptDatV2(Bytes& data)
	{
		CryptData cd = decryptV2File(data);
		data.assign(cd.gameDatBytes.begin(), cd.gameDatBytes.end());
	}

	void cryptProj(Bytes& data)
	{
		srand(s_projKey);

		for (uint8_t& byte : data)
			byte ^= static_cast<uint8_t>(rand());
	}

	static tString sjis2utf8(const Bytes& sjis)
	{
		const LPCCH pSJIS = reinterpret_cast<const LPCCH>(sjis.data());
		int sjisSize      = MultiByteToWideChar(932, 0, pSJIS, -1, NULL, 0);

		WCHAR* pUTF8 = new WCHAR[sjisSize + 1]();
		MultiByteToWideChar(932, 0, pSJIS, -1, pUTF8, sjisSize);
		tString utf8(pUTF8);
		delete[] pUTF8;
		return utf8;
	}

	static Bytes utf82sjis(const tString& utf8)
	{
		// Empty strings are length 1 with terminating 0
		if (utf8.empty()) return Bytes(1, 0);

		int utf8Size = WideCharToMultiByte(932, 0, &utf8[0], (int)utf8.size(), NULL, 0, NULL, NULL);
		Bytes sjis(utf8Size + 1, 0);
		WideCharToMultiByte(932, 0, &utf8[0], (int)utf8.size(), reinterpret_cast<const LPSTR>(sjis.data()), utf8Size, NULL, NULL);
		return sjis;
	}

	static void lz4Unpack(const uint8_t* pPacked, uint8_t* pUnpacked, const uint32_t& packSize)
	{
		uint32_t upOff = 0;
		uint32_t pcOff = 0;

		if (pPacked[0] == 0)
			return;

		while (pcOff < packSize)
		{
			uint32_t token = pPacked[pcOff++];
			uint32_t len   = token >> 4;

			if (len == 0xF)
			{
				while (pPacked[pcOff] == 0xFF)
				{
					len += 0xFF;
					pcOff++;
				}

				len += pPacked[pcOff++];
			}

			for (uint32_t i = 0; i < len; ++i)
				pUnpacked[upOff++] = pPacked[pcOff++];

			if (pcOff == packSize)
				break;

			uint32_t mOff = *reinterpret_cast<const uint16_t*>(&pPacked[pcOff]);
			pcOff += 2;

			len = (token & 0x0F) + 4;

			if (len == (0xF + 4))
			{
				while (pPacked[pcOff] == 0xFF)
				{
					len += 0xFF;
					pcOff++;
				}

				len += pPacked[pcOff++];
			}

			for (uint32_t i = 0; i < len; i++)
			{
				pUnpacked[upOff] = pUnpacked[upOff - mOff];
				upOff++;
			}
		}
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
