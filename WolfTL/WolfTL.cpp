#include <Windows.h>
#include <tchar.h>

#include <filesystem>
#include <format>
#include <iostream>
#include <map>

#include "Context.h"
#include "WolfRPG\WolfRPG.h"

namespace fs = std::filesystem;

class Translation
{
public:
	Translation(const tString& patchFileName = L"", const tString& string = L"", const bool autogenerate = true) :
		m_patchFileName(patchFileName),
		m_string(string),
		m_autogenerate(autogenerate)
	{
	}

	const tString& to_s() const
	{
		return m_string;
	}

	const tString& getPatchFileName() const
	{
		return m_patchFileName;
	}

	const bool& isAutoGenereate() const
	{
		return m_autogenerate;
	}

private:
	tString m_patchFileName;
	tString m_string;
	bool m_autogenerate;
};

using Translations = std::vector<Translation>;

using StringPair = std::pair<tString, ContextsShPtr::Context>;

// Different @strings layout:
// One String associated with n Context|Translation combos

class WolfTL
{
	inline static const tString MAP_OUTPUT   = TEXT("dump/mps/");
	inline static const tString DB_OUTPUT    = TEXT("dump/db/");
	inline static const tString COM_OUTPUT   = TEXT("dump/common/");
	inline static const tString PATCHED_DATA = TEXT("patched/");

public:
	WolfTL(const tString& dataPath, const tString& outputPath) :
		m_dataPath(dataPath),
		m_outputPath(outputPath),
		m_wolf(dataPath)
	{
	}

	void ToJson() const
	{
		if (!m_wolf.Valid())
		{
			std::wcerr << ERROR_TAG << L"WolfRPG initialization failed" << std::endl;
			return;
		}

		maps2Json();
		databases2Json();
		commonEvents2Json();
	}

	void Patch()
	{
		if (!m_wolf.Valid())
		{
			std::wcerr << ERROR_TAG << L"WolfRPG initialization failed" << std::endl;
			return;
		}

		// Check if the patch folder exists
		if (!fs::exists(m_outputPath))
		{
			std::wcerr << ERROR_TAG << L"Patch folder does not exist" << std::endl;
			return;
		}

		patchMaps(m_outputPath);
		patchDatabases(m_outputPath);
		patchCommonEvents(m_outputPath);

		// Save the patched data
		m_wolf.Save2File(std::format(TEXT("{}/{}"), m_outputPath, PATCHED_DATA));
	}

private:
	void maps2Json() const
	{
		std::cout << "Writing Maps to JSON ... " << std::flush;

		const tString mapOutput = std::format(TEXT("{}/{}"), m_outputPath, MAP_OUTPUT);

		// Make sure the output folder exists
		fs::create_directories(mapOutput);

		for (const Map& map : m_wolf.GetMaps())
			map.ToJson(mapOutput);

		std::cout << "Done" << std::endl;
	}

	void databases2Json() const
	{
		std::cout << "Writing Databases to JSON ... " << std::flush;

		const tString dbOutput = std::format(TEXT("{}/{}"), m_outputPath, DB_OUTPUT);

		// Make sure the output folder exists
		fs::create_directories(dbOutput);

		for (const Database& db : m_wolf.GetDatabases())
			db.ToJson(dbOutput);

		std::cout << "Done" << std::endl;
	}

	void commonEvents2Json() const
	{
		std::cout << "Writing CommonEvents to JSON ... " << std::flush;

		const tString comOutput = std::format(TEXT("{}/{}"), m_outputPath, COM_OUTPUT);

		// Make sure the output folder exists
		fs::create_directories(comOutput);

		m_wolf.GetCommonEvents().ToJson(comOutput);

		std::cout << "Done" << std::endl;
	}

	void patchMaps(const tString& patchFolder)
	{
		std::cout << "Patching Maps ... " << std::flush;

		const tString mapPatch = std::format(TEXT("{}/{}"), patchFolder, MAP_OUTPUT);

		// Check if the patch folder exists
		if (!fs::exists(mapPatch))
		{
			std::wcerr << ERROR_TAG << L"Map patch folder does not exist" << std::endl;
			return;
		}

		for (Map& map : m_wolf.GetMaps())
			map.Patch(mapPatch);

		std::cout << "Done" << std::endl;
	}

	void patchDatabases(const tString& patchFolder)
	{
		std::cout << "Patching Databases ... " << std::flush;

		const tString dbPatch = std::format(TEXT("{}/{}"), patchFolder, DB_OUTPUT);

		// Check if the patch folder exists
		if (!fs::exists(dbPatch))
		{
			std::wcerr << ERROR_TAG << L"Database patch folder does not exist" << std::endl;
			return;
		}

		for (Database& db : m_wolf.GetDatabases())
			db.Patch(dbPatch);

		std::cout << "Done" << std::endl;
	}

	void patchCommonEvents(const tString& patchFolder)
	{
		std::cout << "Patching CommonEvents ... " << std::flush;

		const tString comPatch = std::format(TEXT("{}/{}"), patchFolder, COM_OUTPUT);

		// Check if the patch folder exists
		if (!fs::exists(comPatch))
		{
			std::wcerr << ERROR_TAG << L"Common event patch folder does not exist" << std::endl;
			return;
		}

		m_wolf.GetCommonEvents().Patch(comPatch);

		std::cout << "Done" << std::endl;
	}

private:
	tString m_dataPath;
	tString m_outputPath;
	WolfRPG m_wolf;
};

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "Usage: " << argv[0] << " <DATA-FOLDER> <OUTPUT-FOLDER> <MODE>" << std::endl;
		std::cout << "Modes:" << std::endl;
		std::cout << "  create - Create the Patch" << std::endl;
		std::cout << "  patch  - Apply the Patch" << std::endl;
		return 0;
	}

	LPWSTR* szArglist;
	int32_t nArgs;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist == nullptr)
	{
		std::cout << "CommandLineToArgvW failed" << std::endl;
		return -1;
	}

	const tString dataFolder = szArglist[1];
	const tString outputFolder = szArglist[2];
	tString mode = szArglist[3];

	// Convert to lowercase
	std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

	LocalFree(szArglist);

	try
	{
		WolfTL wolf(dataFolder, outputFolder);
		if (mode == TEXT("create"))
			wolf.ToJson();
		else if (mode == TEXT("patch"))
			wolf.Patch();
		else
			std::cerr << "Invalid mode" << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
