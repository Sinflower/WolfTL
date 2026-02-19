/*
 *  File: WolfRPG.h
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

#include "CommonEvents.h"
#include "Database.h"
#include "GameDat.h"
#include "Map.h"
#include "Types.h"

#include <filesystem>

class WolfRPG
{
public:
	explicit WolfRPG(const tString& dataPath, const Paths& additionalMapPaths = {}, const bool& skipGD = false, const bool& saveUncompressed = false) :
		m_dataPath(dataPath),
		m_additionalMapPaths(additionalMapPaths),
		m_skipGD(skipGD),
		m_saveUncompressed(saveUncompressed)
	{
		try
		{
			WolfDataBase::SetUncompressedPath("uncompressed");
			loadGameDat();
			loadCommonEvents();
			loadDatabases();
			loadMaps();
			loadAdditionalMaps();

			m_valid = true;
		}
		catch (WolfRPGException& e)
		{
			std::wcerr << std::endl
					   << "Error while processing: " << g_activeFile << std::endl
					   << e.what() << std::endl;
		}
	}

	~WolfRPG()
	{
	}

	const bool& Valid() const
	{
		return m_valid;
	}

	void Save2File(const tString& outputPath) const
	{
		checkValid();

		tString basicDataDir = outputPath + L"/BasicData/";
		tString mapDataDir   = outputPath + L"/MapData/";

		checkAndCreateDir(basicDataDir);

		if (!m_maps.empty())
			checkAndCreateDir(mapDataDir);

		if (!m_skipGD)
		{
			std::cout << "Writing Game.dat to file ... ";
			m_gameDat.Dump(basicDataDir);
			std::cout << "Done" << std::endl;
		}

		std::cout << "Writing CommonEvents to file ... ";
		m_commonEvents.Dump(basicDataDir);
		std::cout << "Done" << std::endl;

		std::cout << "Writing Databases to file ... ";
		for (const Database& db : m_databases)
			db.Dump(basicDataDir);
		std::cout << "Done" << std::endl;

		std::cout << "Writing Maps to file ... ";
		for (const Map& map : m_maps)
			map.Dump(mapDataDir);
		std::cout << "Done" << std::endl;

		std::cout << "Writing additional Maps to file ... ";
		for (const auto& [mapPath, maps] : m_additionalMaps)
		{
			std::filesystem::path relativePath = std::filesystem::relative(mapPath, m_dataPath);

			tString additionalMapDir = outputPath + L"/" + relativePath.wstring();

			checkAndCreateDir(additionalMapDir);
			for (const Map& map : maps)
				map.Dump(additionalMapDir);
		}
		std::cout << "Done" << std::endl;
	}

	GameDat& GetGameDat()
	{
		checkValid();
		return m_gameDat;
	}

	const GameDat& GetGameDat() const
	{
		checkValid();
		return m_gameDat;
	}

	Maps& GetMaps()
	{
		checkValid();
		return m_maps;
	}

	const Maps& GetMaps() const
	{
		checkValid();
		return m_maps;
	}

	std::map<std::filesystem::path, Maps>& GetAdditionalMaps()
	{
		checkValid();
		return m_additionalMaps;
	}

	const std::map<std::filesystem::path, Maps>& GetAdditionalMaps() const
	{
		checkValid();
		return m_additionalMaps;
	}

	CommonEvents& GetCommonEvents()
	{
		checkValid();
		return m_commonEvents;
	}

	const CommonEvents& GetCommonEvents() const
	{
		checkValid();
		return m_commonEvents;
	}

	Databases& GetDatabases()
	{
		checkValid();
		return m_databases;
	}

	const Databases& GetDatabases() const
	{
		checkValid();
		return m_databases;
	}

private:
	void checkValid() const
	{
		if (!m_valid)
			throw WolfRPGException(ERROR_TAGW + L"Invalid WolfRPG object");
	}

	void loadGameDat()
	{
		if (m_skipGD) return;

		std::cout << "Loading Game.dat ... " << std::flush;

		m_gameDat = GameDat(m_dataPath + L"/BasicData/Game.dat", m_saveUncompressed);

		std::cout << "Done" << std::endl;
	}

	void loadMaps()
	{
		if (!std::filesystem::exists(m_dataPath + L"/MapData/"))
		{
			std::cout << "MapData directory not found. Skipping Maps ..." << std::endl;
			return;
		}

		std::cout << "Loading Maps ... " << std::flush;

		size_t prevLength = 0;
		for (std::filesystem::directory_entry p : std::filesystem::directory_iterator(m_dataPath + L"/MapData/"))
		{
			if (p.path().extension() == ".mps")
			{
				std::wcout << "\rLoading Map: " << p.path().filename() << std::setfill(TCHAR(' ')) << std::setw(prevLength) << "" << std::flush;
				prevLength = tString(p.path().filename()).length();
				tString file(p.path());
				m_maps.push_back(Map(file, m_saveUncompressed));
			}
		}

		std::cout << "\rLoading Maps ... Done" << std::setfill(' ') << std::setw(prevLength) << "" << std::endl;
	}

	void loadAdditionalMaps()
	{
		for (const std::filesystem::path& mapPath : m_additionalMapPaths)
		{
			std::string relativePath = std::filesystem::relative(mapPath, m_dataPath).string();

			if (!std::filesystem::exists(mapPath))
			{
				std::cerr << ERROR_TAG + "Additional map path does not exist: " << relativePath << std::endl;
				continue;
			}

			std::cout << "Loading additional Maps from path: " << relativePath << " ... " << std::flush;

			size_t prevLength = 0;
			for (std::filesystem::directory_entry p : std::filesystem::directory_iterator(mapPath))
			{
				if (p.path().extension() == ".mps")
				{
					std::wcout << "\rLoading Map: " << p.path().filename() << std::setfill(TCHAR(' ')) << std::setw(prevLength) << "" << std::flush;
					prevLength = tString(p.path().filename()).length();
					tString file(p.path());
					m_additionalMaps[mapPath].push_back(Map(file, m_saveUncompressed));
				}
			}

			std::cout << "\rLoading additional Maps from path: " << relativePath << " ... Done" << std::setfill(' ') << std::setw(prevLength) << "" << std::endl;
		}
	}

	void loadCommonEvents()
	{
		std::cout << "Loading CommonEvents ... " << std::flush;

		m_commonEvents = CommonEvents(m_dataPath + L"/BasicData/CommonEvent.dat", m_saveUncompressed);

		std::cout << "Done" << std::endl;
	}

	void loadDatabases()
	{
		std::cout << "Loading Databases ... " << std::flush;

		for (std::filesystem::directory_entry p : std::filesystem::directory_iterator(m_dataPath + L"/BasicData/"))
		{
			std::filesystem::path pp = p.path();
			if (pp.extension() == ".project" && pp.filename() != "SysDataBaseBasic.project")
			{
				tString projectFile(p.path());
				pp.replace_extension(".dat");
				tString datFile(pp);
				m_databases.push_back(Database(projectFile, datFile));
			}
		}

		std::cout << "Done" << std::endl;
	}

	void checkAndCreateDir(const tString& dir) const
	{
		if (!std::filesystem::exists(dir))
		{
			if (!std::filesystem::create_directories(dir))
			{
				if (!std::filesystem::exists(dir))
					throw WolfRPGException(ERROR_TAGW + L"Failed to create directory: " + dir);
			}
		}
	}

private:
	tString m_dataPath;
	Paths m_additionalMapPaths;
	bool m_skipGD;
	bool m_saveUncompressed;

	GameDat m_gameDat;
	Maps m_maps;
	std::map<std::filesystem::path, Maps> m_additionalMaps;
	CommonEvents m_commonEvents;
	Databases m_databases;

	bool m_valid = false;
};
