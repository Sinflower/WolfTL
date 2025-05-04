﻿/*
 *  File: Wolf35Unprotect.hpp
 *  Copyright (c) 2025 Sinflower
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

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

// This depends on WolfRPG for the WolfFileType enum
#include "WolfRPG/NewWolfCrypt.h"
#include "WolfRPG/WolfRPG.h"

#include "WolfSha512.hpp"

namespace wolf::v3_5::unprotect
{
inline std::vector<uint8_t> file2Buffer(const std::filesystem::path &filePath)
{
	std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile)
		throw std::runtime_error("Failed to open file: " + filePath.string());

	std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	inFile.close();

	if (buffer.empty())
		throw std::runtime_error("File is empty: " + filePath.string());

	return buffer;
}

inline void buffer2File(const std::filesystem::path &filePath, const std::vector<uint8_t> &buffer)
{
	std::ofstream outFile(filePath, std::ios::binary);
	if (!outFile)
		throw std::runtime_error("Failed to open output file: " + filePath.string());
	outFile.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
	outFile.close();
}

struct ProMagic
{
	std::string staticSalt;
	std::vector<uint8_t> magicBytes;
};

static const std::map<WolfFileType, ProMagic> PRO_MAGIC = {
	{ WolfFileType::GameDat, { "basicD1", { 0x00, 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x00, 0x46, 0x4D, 0x55 } } },
	{ WolfFileType::CommonEvent, { "Commo2", { 0x00, 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x55, 0x46, 0x43, 0x00 } } },
	{ WolfFileType::DataBase, { "DBase4", { 0x00, 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x55, 0x46, 0x4D, 0x00 } } },
	{ WolfFileType::TileSetData, { "TilesetA", { 0x00, 0x57, 0x00, 0x00, 0x4F, 0x4C, 0x55, 0x46, 0x4D, 0x00 } } },
	{ WolfFileType::None, { "", {} } }
};

inline void decryptProV3P1(std::vector<uint8_t> &data, const std::array<uint8_t, 3> seedIdx)
{
	const uint32_t seed = (0xB << 24) | (data[seedIdx[0]] << 16) | (data[seedIdx[1]] << 8) | data[seedIdx[2]];
	int32_t rn          = xorshift32(seed);

	for (uint32_t i = 0xA; i < data.size(); i++)
	{
		int32_t v1 = (((rn << 0xF) ^ rn) >> 0x15) ^ (rn << 0xF) ^ rn;
		rn         = (v1 << 0x9) ^ v1;
		data[i] ^= rn % 0xF9;
	}
}

inline bool decryptProV3Dat(std::vector<uint8_t> &buffer, const WolfFileType &datType)
{
	constexpr uint32_t KEY_START_OFFSET = 12;
	constexpr uint32_t IV_START_OFFSET  = 73;
	constexpr uint32_t AES_DATA_OFFSET  = 20;
	constexpr uint32_t PRO_SPECIAL_SIZE = 143; // 15 byte header + 128 byte hash

	if (buffer.empty() || buffer.size() < PRO_SPECIAL_SIZE)
	{
		std::cerr << "Buffer is empty or too small" << std::endl;
		return false;
	}

	if (buffer[1] != 0x50 || buffer[5] < 0x57)
	{
		std::cout << "File is not protected or not a ProV3 file, skipping decryption" << std::endl;
		return false;
	}

	std::array<uint8_t, 3> seedIdx = { 0, 3, 9 }; // Default idx for everything except Game.dat

	if (datType == WolfFileType::GameDat)
		seedIdx = { 0, 8, 6 };

	decryptProV3P1(buffer, seedIdx);

	srand(buffer[12]);
	std::size_t aesSize = buffer.size() - AES_DATA_OFFSET;
	// ¯\_(ツ)_/¯ that's what to code says (probably) and it works ¯\_(ツ)_/¯
	if (aesSize >= rand() % 126 + 200)
		aesSize = rand() % 126 + 200;

	uint64_t nBuffer = 0;

	const ProMagic &proMagic = PRO_MAGIC.at(datType);

	sha512::s512DynSalt dynSalt = sha512::calcDynSalt(buffer);
	sha512::s512Pwd saltedPwd   = sha512::saltPassword("", dynSalt, proMagic.staticSalt);
	sha512::s512Input sInput    = sha512::preprocess(saltedPwd, nBuffer);
	sha512::s512Hash hashData   = sha512::process(sInput, nBuffer);
	std::string hashString      = sha512::digest(hashData);

	AesKey aesKey;
	AesIV aesIv;
	AesRoundKey roundKey;

	std::copy(hashString.begin() + KEY_START_OFFSET, hashString.begin() + KEY_START_OFFSET + AES_KEY_SIZE, aesKey.begin());
	std::copy(hashString.begin() + IV_START_OFFSET, hashString.begin() + IV_START_OFFSET + AES_IV_SIZE, aesIv.begin());

	keyExpansion(roundKey.data(), aesKey.data());
	std::copy(aesIv.begin(), aesIv.end(), roundKey.begin() + AES_KEY_EXP_SIZE);

	aesCtrXCrypt(buffer.data() + AES_DATA_OFFSET, roundKey.data(), aesSize);

	buffer.erase(buffer.begin(), buffer.begin() + PRO_SPECIAL_SIZE);
	buffer.insert(buffer.begin(), proMagic.magicBytes.begin(), proMagic.magicBytes.end());

	return true;
}

void unprotectProject(std::vector<uint8_t> &projData)
{
	// ¯\_(ツ)_/¯ So far it looks like this is how it is done
	srand(0);
	for (uint8_t &byte : projData)
		byte ^= static_cast<uint8_t>(rand());
}

} // namespace wolf::v3_5::unprotect
