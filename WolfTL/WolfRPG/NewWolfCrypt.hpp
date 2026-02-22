/*
 *  File: NewWolfCrypt.hpp
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

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <vector>

inline uint32_t xorshift32(const uint32_t &seed = 0)
{
	static uint32_t state = 0;

	if (seed != 0)
		state = seed;

	state ^= state << 0xB;
	state ^= state >> 0x13;
	state ^= state << 0x7;
	return state;
}

inline constexpr uint8_t sbox[256] = {
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15, 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B,
	0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF, 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73, 0x60, 0x81, 0x4F, 0xDC,
	0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08, 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1,
	0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF, 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

inline constexpr uint8_t Rcon[11] = { 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36 };

#define Nk 4
#define Nb 4
#define Nr 10

#define AES_KEY_EXP_SIZE 176
#define AES_KEY_SIZE     16
#define AES_IV_SIZE      16
#define AES_BLOCKLEN     16

#define AES_ROUND_KEY_SIZE AES_KEY_EXP_SIZE + AES_IV_SIZE

#define PW_SIZE 15

using AesRoundKey = std::array<uint8_t, AES_ROUND_KEY_SIZE>;
using AesKey      = std::array<uint8_t, AES_KEY_SIZE>;
using AesIV       = std::array<uint8_t, AES_IV_SIZE>;

// Init the AES RoundKey
inline void keyExpansion(uint8_t *pRoundKey, const uint8_t *pKey)
{
	uint8_t tempa[4] = { 0 };

	// The first round key is the key itself.
	for (uint32_t i = 0; i < Nk; i++)
	{
		pRoundKey[(i * 4) + 0] = pKey[(i * 4) + 0];
		pRoundKey[(i * 4) + 1] = pKey[(i * 4) + 1];
		pRoundKey[(i * 4) + 2] = pKey[(i * 4) + 2];
		pRoundKey[(i * 4) + 3] = pKey[(i * 4) + 3];
	}

	for (uint32_t i = Nk; i < Nb * (Nr + 1); i++)
	{
		uint32_t k = (i - 1) * 4;

		tempa[0] = pRoundKey[k + 0];
		tempa[1] = pRoundKey[k + 1];
		tempa[2] = pRoundKey[k + 2];
		tempa[3] = pRoundKey[k + 3];

		if ((i % Nk) == 0)
		{
			const uint8_t u8tmp = tempa[0];
			tempa[0]            = tempa[1];
			tempa[1]            = tempa[2];
			tempa[2]            = tempa[3];
			tempa[3]            = u8tmp;

			// This differs between the original and the WolfRPG version
			tempa[0] = sbox[tempa[0]] ^ Rcon[i / Nk];
			tempa[1] = sbox[tempa[1]] >> 4;
			tempa[2] = ~sbox[tempa[2]];
			tempa[3] = std::rotr(sbox[tempa[3]], 7);
		}

		const uint32_t j = i * 4;

		k = (i - Nk) * 4;

		pRoundKey[j + 0] = pRoundKey[k + 0] ^ tempa[0];
		pRoundKey[j + 1] = pRoundKey[k + 1] ^ tempa[1];
		pRoundKey[j + 2] = pRoundKey[k + 2] ^ tempa[2];
		pRoundKey[j + 3] = pRoundKey[k + 3] ^ tempa[3];
	}
}

/////////////////////////////////
////// AES CTR Crypt -- based on: https://github.com/kokke/tiny-AES-c
// While this code should be a normal AES CTR implementation, the keyExpansion implementation above is not standard AES
// it contains minor changes when the tempa values are modified using the sbox values

inline void addRoundKey(uint8_t *pState, const uint8_t &round, const uint8_t *pRoundKey)
{
	for (uint32_t i = 0; i < AES_KEY_SIZE; i++)
		pState[i] ^= pRoundKey[(round * AES_KEY_SIZE) + i];
}

inline void subBytes(uint8_t *pState)
{
	for (uint32_t i = 0; i < AES_KEY_SIZE; i++)
		pState[i] = sbox[pState[i]];
}

inline void shiftRows(uint8_t *pState)
{
	uint8_t temp;

	temp       = pState[1];
	pState[1]  = pState[5];
	pState[5]  = pState[9];
	pState[9]  = pState[13];
	pState[13] = temp;

	temp       = pState[2];
	pState[2]  = pState[10];
	pState[10] = temp;

	temp       = pState[6];
	pState[6]  = pState[14];
	pState[14] = temp;

	temp       = pState[3];
	pState[3]  = pState[15];
	pState[15] = pState[11];
	pState[11] = pState[7];
	pState[7]  = temp;
}

inline uint8_t xtime(const uint8_t &x)
{
	return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

inline void mixColumns(uint8_t *pState)
{
	uint8_t tmp;
	uint8_t t;

	for (uint32_t i = 0; i < 4; i++)
	{
		t   = pState[0];
		tmp = pState[1] ^ pState[0] ^ pState[2] ^ pState[3];

		pState[0] ^= tmp ^ xtime(pState[1] ^ pState[0]);
		pState[1] ^= tmp ^ xtime(pState[2] ^ pState[1]);
		pState[2] ^= tmp ^ xtime(pState[2] ^ pState[3]);
		pState[3] ^= tmp ^ xtime(pState[3] ^ t);

		pState += 4;
	}
}

// AES Cipher
inline void cipher(uint8_t *pState, const uint8_t *pRoundKey)
{
	addRoundKey(pState, 0, pRoundKey);

	for (uint32_t round = 1; round < Nr; round++)
	{
		subBytes(pState);
		shiftRows(pState);
		mixColumns(pState);
		addRoundKey(pState, round, pRoundKey);
	}

	subBytes(pState);
	shiftRows(pState);
	addRoundKey(pState, Nr, pRoundKey);
}

// AES_CTR_xcrypt
inline void aesCtrXCrypt(uint8_t *pData, uint8_t *pKey, const std::size_t &size)
{
	uint8_t state[AES_BLOCKLEN];
	uint8_t *pIv = pKey + AES_KEY_EXP_SIZE;
	int32_t bi   = AES_BLOCKLEN;

	for (std::size_t i = 0; i < size; i++, bi++)
	{
		if (bi == AES_BLOCKLEN)
		{
			std::memcpy(state, pIv, AES_BLOCKLEN);

			cipher(state, pKey);

			for (bi = AES_BLOCKLEN - 1; bi >= 0; bi--)
			{
				if (pIv[bi] == 0xFF)
				{
					pIv[bi] = 0;
					continue;
				}
				pIv[bi]++;
				break;
			}
			bi = 0;
		}

		pData[i] ^= state[bi];
	}
}

////// AES CTR Crypt
/////////////////////////////////

struct CryptData
{
	std::array<uint8_t, 4> keyBytes  = { 0 };
	std::array<uint8_t, 4> seedBytes = { 0 };

	std::vector<uint8_t> gameDatBytes = {};

	uint32_t dataSize = 0;

	uint32_t seed1 = 0;
	uint32_t seed2 = 0;
};

struct RngData
{
	static constexpr uint32_t OUTER_VEC_LEN = 0x20;
	static constexpr uint32_t INNER_VEC_LEN = 0x100;
	static constexpr uint32_t DATA_VEC_LEN  = 0x30;

	uint32_t seed1   = 0;
	uint32_t seed2   = 0;
	uint32_t counter = 0;

	std::vector<std::vector<uint32_t>> data = std::vector<std::vector<uint32_t>>(OUTER_VEC_LEN, std::vector<uint32_t>(INNER_VEC_LEN, 0));

	void Reset()
	{
		seed1   = 0;
		seed2   = 0;
		counter = 0;

		data = std::vector<std::vector<uint32_t>>(OUTER_VEC_LEN, std::vector<uint32_t>(INNER_VEC_LEN, 0));
	}
};

inline uint32_t customRng1(RngData &rd)
{
	uint32_t state;
	uint32_t stateMod;

	const uint32_t seedP1 = (rd.seed1 ^ (((rd.seed1 << 11) ^ rd.seed1) >> 8));
	const uint32_t seed   = (rd.seed1 << 11) ^ seedP1;

	state = 1664525 * seed + 1013904223;

	if (((13 * seedP1 + 95) & 1) == 0)
		stateMod = state / 8;
	else
		stateMod = state * 4;

	state ^= stateMod;

	if ((state & 0x400) != 0)
	{
		state ^= state << 21;
		stateMod = state >> 9;
	}
	else
	{
		state ^= state * 4;
		stateMod = state >> 22;
	}

	state ^= stateMod;

	if ((state & 0xFFFFF) == 0)
		state += 256;

	rd.seed1 = state;
	return state;
}

inline uint32_t customRng2(RngData &rd)
{
	uint32_t stateMod;
	uint32_t state;

	uint32_t seed = rd.seed1;

	state    = 1664525 * seed + 1013904223;
	stateMod = (seed & 7) + 1;

	if (state % 3)
	{
		if (state % 3 == 1)
			state ^= (state >> stateMod);
		else
			state = ~state + (state << stateMod);
	}
	else
		state ^= (state << stateMod);

	if (state)
	{
		if (!static_cast<uint16_t>(state))
			state ^= 0x55AA55AA;
	}
	else
		state = 0x173BEF;

	rd.seed1 = state;
	return state;
}

inline uint32_t customRng3(RngData &rd)
{
	uint32_t state;
	uint32_t seed = rd.seed2;

	state = (1566083941 * rd.seed2) ^ (292331520 * rd.seed2);
	state ^= (state >> 17) ^ (32 * (state ^ (state >> 17)));
	state = 69069 * (state ^ (state ^ (state >> 11)) & 0x3FFFFFFF);

	if (state)
	{
		if (!static_cast<uint16_t>(state))
			state ^= 0x59A6F141;

		if ((state & 0xFFFFF) == 0)
			state += 256;
	}
	else
		state = 1566083941;

	rd.seed2 = state;
	return state;
}

inline void rngChain(RngData &rd, std::vector<uint32_t> &data)
{
	uint32_t i = 0;
	for (uint32_t &d : data)
	{
		uint32_t rn = customRng2(rd);

		d = rn ^ customRng3(rd);

		if ((++rd.counter & 1) == 0)
			d += customRng3(rd);

		if (!(rd.counter % 3))
			d ^= customRng1(rd) + 3;

		if (!(rd.counter % 7))
			d += customRng3(rd) + 1;

		if ((rd.counter & 7) == 0)
			d *= customRng1(rd);

		if (!((i + rd.seed1) % 5))
			d ^= customRng1(rd);

		if (!(rd.counter % 9))
			d += customRng2(rd) + 4;

		if (!(rd.counter % 0x18))
			d += customRng2(rd) + 7;

		if (!(rd.counter % 0x1F))
			d += 3 * customRng3(rd);

		if (!(rd.counter % 0x3D))
			d += customRng3(rd) + 1;

		if (!(rd.counter % 0xA1))
			d += customRng2(rd);

		if (static_cast<uint16_t>(rn) == 256)
			d += 3 * customRng3(rd);

		i++;
	}
}

inline void runCrypt(RngData &rd, const uint32_t &seed1, const uint32_t &seed2)
{
	rd.seed1   = seed1;
	rd.seed2   = seed2;
	rd.counter = 0;

	srand(seed1);

	for (uint32_t i = 0; i < rd.data.size(); i++)
		rngChain(rd, rd.data[i]);
}

inline void aLotOfRngStuff(RngData &rd, uint32_t a2, uint32_t a3, const uint32_t &idx, std::vector<uint8_t> &cryptData)
{
	uint32_t itrs = 20;

	for (uint32_t i = 0; i < itrs; i++)
	{
		uint32_t idx1 = (a2 ^ customRng1(rd)) & 0x1F;
		uint32_t idx2 = (a3 ^ customRng2(rd)) & 0xFF;
		a3            = rd.data[idx1][idx2];

		switch ((a2 + rd.counter) % 0x14u)
		{
			case 1:
				rngChain(rd, rd.data[(a2 + 5) & 0x1F]);
				break;
			case 2:
				a3 ^= customRng1(rd);
				break;
			case 5:
				if ((a2 & 0xFFFFF) == 0)
					cryptData[idx] ^= customRng3(rd);
				break;
			case 9:
			case 0xE:
				cryptData[customRng2(rd) % 0x30] += a3;
				break;
			case 0xB:
				cryptData[idx] ^= customRng1(rd);
				break;
			case 0x11:
				itrs++;
				break;
			case 0x13:
				if (static_cast<uint16_t>(a2) == 0)
					cryptData[idx] ^= customRng2(rd);
				break;
			default:
				break;
		}

		a2 += customRng3(rd);

		if (itrs > 50)
			itrs = 50;
	}

	cryptData[idx] += a3;
}

inline void aesKeyGen(CryptData &cd, RngData &rd, std::array<uint8_t, AES_KEY_SIZE> &aesKey, std::array<uint8_t, AES_IV_SIZE> &aesIv)
{
	runCrypt(rd, cd.seedBytes[0], cd.seedBytes[1]);

	std::vector<uint8_t> cryptData(RngData::DATA_VEC_LEN, 0);

	for (uint32_t i = 0; i < RngData::DATA_VEC_LEN; i++)
		aLotOfRngStuff(rd, i + cd.seedBytes[3], cd.seedBytes[2] - i, i, cryptData);

	uint8_t seed = cd.seedBytes[1] ^ cd.seedBytes[2];

	std::vector<uint8_t> indexes(RngData::DATA_VEC_LEN, 0);
	std::vector<uint8_t> resData(RngData::DATA_VEC_LEN, 0);
	std::iota(indexes.begin(), indexes.end(), 0);

	srand(seed);

	for (uint32_t i = 0; i < RngData::DATA_VEC_LEN; i++)
	{
		uint32_t rn = rand();
		uint8_t old = indexes[i];
		indexes[i]  = indexes[rn % RngData::DATA_VEC_LEN];

		indexes[rn % RngData::DATA_VEC_LEN] = old;
	}

	for (uint32_t i = 0; i < RngData::DATA_VEC_LEN; i++)
		resData[i] = cryptData[indexes[i]];

	const auto ivBegin = resData.begin() + AES_KEY_SIZE;

	std::copy(resData.begin(), resData.begin() + AES_KEY_SIZE, aesKey.begin());
	std::copy(ivBegin, ivBegin + AES_IV_SIZE, aesIv.begin());
}

inline uint32_t genMTSeed(const std::array<uint8_t, 3> &seeds)
{
	uint32_t x = (seeds[0] << 16) | (seeds[1] << 8) | seeds[2];
	uint32_t y = (x << 13) ^ x;
	uint32_t z = (y >> 17) ^ y;

	return z ^ (z << 5);
}

inline void decrpytProV2P1(std::vector<uint8_t> &data, const uint32_t &seed)
{
	const uint32_t NUM_RNDS = 128;

	std::mt19937 gen;
	gen.seed(seed);

	std::array<uint32_t, NUM_RNDS> rnds;

	for (uint32_t &rnd : rnds)
		rnd = gen();

	for (uint32_t i = 0xA; i < data.size(); i++)
		data[i] ^= rnds[i % NUM_RNDS];
}

inline void initCryptProt(CryptData &cd)
{
	uint32_t fileSize = static_cast<uint32_t>(cd.gameDatBytes.size());

	if (fileSize - 20 < 326)
		cd.dataSize = fileSize - 20;
	else
		cd.dataSize = 326;

	decrpytProV2P1(cd.gameDatBytes, genMTSeed({ cd.gameDatBytes[0], cd.gameDatBytes[3], cd.gameDatBytes[9] }));

	std::copy(cd.gameDatBytes.begin() + 0xB, cd.gameDatBytes.begin() + 0xF, cd.keyBytes.begin());

	cd.seedBytes[0] = cd.gameDatBytes[7] + 3 * cd.keyBytes[0];
	cd.seedBytes[1] = cd.keyBytes[1] ^ cd.keyBytes[2];
	cd.seedBytes[2] = cd.keyBytes[3] ^ cd.gameDatBytes[7];
	cd.seedBytes[3] = cd.keyBytes[2] + cd.gameDatBytes[7] - cd.keyBytes[0];

	cd.seed1 = cd.keyBytes[1] ^ cd.keyBytes[2];
	cd.seed2 = cd.keyBytes[1] ^ cd.keyBytes[2];
}

inline CryptData decryptV2File(const std::vector<uint8_t> &bytes)
{
	CryptData cd;
	RngData rd;

	cd.gameDatBytes = bytes;
	initCryptProt(cd);

	runCrypt(rd, cd.seed1, cd.seed2);

	AesKey aesKey;
	AesIV aesIv;

	aesKeyGen(cd, rd, aesKey, aesIv);

	AesRoundKey roundKey;

	keyExpansion(roundKey.data(), aesKey.data());
	std::copy(aesIv.begin(), aesIv.end(), roundKey.begin() + AES_KEY_EXP_SIZE);

	aesCtrXCrypt(cd.gameDatBytes.data() + 20, roundKey.data(), cd.dataSize);

	return cd;
}
