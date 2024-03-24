/*
 *  File: JsonDumper.h
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

#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <vector>

class JsonDumper
{
	struct Section
	{
		Section() = default;
		Section(const std::string& name) :
			name(name)
		{
		}

		std::string name            = "";
		nlohmann::ordered_json json = nlohmann::ordered_json();
		uint64_t varCnt             = 0;

		std::map<std::string, uint64_t> cstmVarCnt = std::map<std::string, uint64_t>();
		std::map<std::string, uint32_t> sectionCnt = std::map<std::string, uint32_t>();
	};

public:
	enum Flags
	{
		NON          = 0,
		DO_NOT_TOUCH = 1 << 0,
		COUNTER      = 1 << 1,
		SKIP         = 1 << 2,
		MAGIC_NUMBER = 1 << 3
	};

public:
	JsonDumper(const tString& filePath) :
		m_filePath(filePath)
	{
	}

	void EnterSection(const std::string& section)
	{
		// m_sectionCnt.try_emplace(section, 0);
		// m_sectionCnt[section]++;
		if (curSection())
		{
			curSection()->sectionCnt.try_emplace(section, 0);
			curSection()->sectionCnt[section]++;
		}

		m_sections.push_back(Section(section));
	}

	void LeaveSection()
	{
		const std::string secName            = buildSecName(curSection()->name);
		const nlohmann::ordered_json secJson = curSection()->json;

		m_sections.pop_back();

		// If the current section is a sub-section, add it to the parent section
		if (m_sections.empty())
			m_json[secName] = secJson;
		else
			m_sections.back().json[secName] = secJson;
	}

	template<typename T>
	void Dump(const T& data, const std::string& name, const uint32_t& flags = Flags::NON)
	{
		nlohmann::ordered_json j = buildObj(data);

		if (flags != Flags::NON)
			j["flags"] = flags2Strings(flags);

		addObj(j, name);
	}

	template<typename T>
	void Dump(const T& data, const uint32_t& flags = Flags::NON)
	{
		Dump(data, "", flags);
	}

	template<typename T>
	void Dump(const std::vector<T>& vec, const std::string& name, const uint32_t& flags = Flags::NON)
	{
		nlohmann::ordered_json j = buildVecObj(vec);

		if (flags != Flags::NON)
			j["flags"] = flags2Strings(flags);

		addObj(j, name);
	}
	
	template<typename T>
	void Dump(const std::vector<T>& vec, const uint32_t& flags = Flags::NON)
	{
		Dump(vec, "", flags);
	}

	template<typename T, size_t U>
	void Dump(const std::array<T, U>& vec, const std::string& name, const uint32_t& flags = Flags::NON)
	{
		nlohmann::ordered_json j = buildArrObj(vec);

		if (flags != Flags::NON)
			j["flags"] = flags2Strings(flags);

		addObj(j, name);
	}
	
	template<typename T, size_t U>
	void Dump(const std::array<T, U>& vec, const uint32_t& flags = Flags::NON)
	{
		Dump(vec, "", flags);
	}


	void Write2File()
	{
		// Check if the given file path is valid
		if (m_filePath.empty())
		{
			std::cout << "Invalid file path" << std::endl;
			return;
		}

		if (m_json.empty())
		{
			std::cout << "No data to write" << std::endl;
			return;
		}

		std::ofstream file(m_filePath);
		file << m_json.dump(4);
		file.close();
	}

private:
	Section* curSection()
	{
		if (m_sections.empty()) return nullptr;

		return &m_sections.back();
	}

	const Section* parentSection() const
	{
		if (m_sections.size() < 2) return nullptr;

		return &m_sections[m_sections.size() - 2];
	}

	void addObj(const nlohmann::ordered_json& j, const std::string& name = "")
	{
		curSection()->json[buildObjName(name)] = j;
	}

	std::string buildObjName(const std::string& name = "")
	{
		std::string objName = "";
		if (name.empty())
			objName = "var_#" + std::to_string(curSection()->varCnt++);
		else
		{
			curSection()->cstmVarCnt.try_emplace(name, 0);
			if (curSection()->cstmVarCnt[name] == 0)
				objName = name;
			else
				objName = name + "_#" + std::to_string(curSection()->cstmVarCnt[name]);

			curSection()->cstmVarCnt[name]++;
		}

		return objName;
	}

	std::string buildSecName(const std::string& secName) const
	{
		std::string name = secName;

		if (parentSection() != nullptr)
			name += "_#" + std::to_string(parentSection()->sectionCnt.at(secName));

		return name;
	}

	template<typename T>
	nlohmann::ordered_json buildObj(const T& data) const
	{
		nlohmann::ordered_json j;
		j["value"]    = data;
		j["typeSize"] = sizeof(T);

		return j;
	}

	template<typename T>
	nlohmann::ordered_json buildVecObj(const std::vector<T>& vec) const
	{
		if (vec.empty()) return nlohmann::ordered_json();

		nlohmann::ordered_json j;
		j["value"]    = vec;
		j["typeSize"] = sizeof(T);

		return j;
	}

	template<typename T, std::size_t U>
	nlohmann::ordered_json buildArrObj(const std::array<T, U>& arr) const
	{
		if (U == 0) return nlohmann::ordered_json();

		nlohmann::ordered_json j;
		j["value"]    = arr;
		j["typeSize"] = sizeof(T);
		j["arrCount"] = U;

		return j;
	}

	std::vector<std::string> flags2Strings(const uint32_t& flags) const
	{
		std::vector<std::string> strs;

		if (flags & Flags::DO_NOT_TOUCH)
			strs.push_back("DO_NOT_TOUCH");
		if (flags & Flags::COUNTER)
			strs.push_back("COUNTER");
		if (flags & Flags::SKIP)
			strs.push_back("SKIP");
		if (flags & Flags::MAGIC_NUMBER)
			strs.push_back("MAGIC_NUMBER");

		return strs;
	}

private:
	tString m_filePath = _T("");
	std::vector<Section> m_sections;
	// std::map<std::string, uint32_t> m_sectionCnt;
	nlohmann::ordered_json m_json;
};
