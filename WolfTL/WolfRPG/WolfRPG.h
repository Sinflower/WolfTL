#pragma once

#include "CommonEvents.h"
#include "Database.h"
#include "GameDat.h"
#include "Map.h"
#include "Types.h"

#include <filesystem>

namespace fs = std::filesystem;

class WolfRPG
{
public:
	explicit WolfRPG(const tString& dataPath) :
		m_dataPath(dataPath)
	{
		try
		{
			loadGameDat();
			loadCommonEvents();
			loadDatabases();
			loadMaps();

			m_valid = true;
		}
		catch (WolfRPGException& e)
		{
			std::cerr << std::endl
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
		checkAndCreateDir(mapDataDir);

		std::cout << "Writing Game.dat to file ... ";
		m_gameDat.Dump(basicDataDir);
		std::cout << "Done" << std::endl;

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
			throw WolfRPGException(ERROR_TAG L"Invalid WolfRPG object");
	}

	void loadGameDat()
	{
		std::cout << "Loading Game.dat ... " << std::flush;

		m_gameDat = GameDat(m_dataPath + L"/BasicData/Game.dat");

		std::cout << "Done" << std::endl;
	}

	void loadMaps()
	{
		std::cout << "Loading Maps ... " << std::flush;

		size_t prevLength = 0;
		for (fs::directory_entry p : fs::directory_iterator(m_dataPath + L"/MapData/"))
		{
			if (p.path().extension() == ".mps")
			{
				std::wcout << "\rLoading Map: " << p.path().filename() << std::setfill(TCHAR(' ')) << std::setw(prevLength) << "" << std::flush;
				prevLength = tString(p.path().filename()).length();
				tString file(p.path());
				m_maps.push_back(Map(file));
			}
		}

		std::cout << "\rLoading Maps ... Done" << std::setfill(' ') << std::setw(prevLength) << "" << std::endl;
	}

	void loadCommonEvents()
	{
		std::cout << "Loading CommonEvents ... " << std::flush;

		m_commonEvents = CommonEvents(m_dataPath + L"/BasicData/CommonEvent.dat");

		std::cout << "Done" << std::endl;
	}

	void loadDatabases()
	{
		std::cout << "Loading Databases ... " << std::flush;

		for (fs::directory_entry p : fs::directory_iterator(m_dataPath + L"/BasicData/"))
		{
			fs::path pp = p.path();
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
		if (!fs::exists(dir))
		{
			if (!fs::create_directories(dir))
			{
				if (!fs::exists(dir))
					throw WolfRPGException(ERROR_TAG L"Failed to create directory: " + dir);
			}
		}
	}

private:
	tString m_dataPath;

	GameDat m_gameDat;
	Maps m_maps;
	CommonEvents m_commonEvents;
	Databases m_databases;

	bool m_valid = false;
};
