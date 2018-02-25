#include "stdafx.h"

#include <algorithm>
#include <iostream>
#include <map>

#include "files.h"
#include "stl.h"

#include <QtCore/QDir>

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

namespace
{
#define	MAX_FOUND_FILES	0x1000

std::string joinPath(const std::string& s)
{
	return s;
}

template<typename... Ts>
std::string joinPath(const std::string& a, Ts... args)
{
	return a + QDir::separator().toLatin1() + joinPath(args...);
}

struct File
{
	std::string name;
};

struct FileSystem
{
	std::map<std::string, File> files;
};
}

bool InitFileSystem(const std::string& gameDataPath, const std::string& baseDir)
{
	const QDir gameDataDir(gameDataPath.c_str());
	if (!gameDataDir.exists())
	{
		return false;
	}

	const QDir basePath(joinPath(gameDataPath, baseDir).c_str());
	if (!basePath.exists())
	{
		return false;
	}

	const QString pk3Extension = "*.pk3";
	QStringList onlyPk3Extension;
	onlyPk3Extension.push_back(pk3Extension);

	const QStringList pk3Files = basePath.entryList(onlyPk3Extension, QDir::Files);
	for (const auto& file : pk3Files)
	{
		std::cout << file.toStdString() << '\n';
	}

	return true;
}

std::vector<std::string> Sys_ListFiles( const char *directory, const char *extension, bool wantsubs ) {
	int nfiles;
	std::vector<std::string> fileList;

	if (extension == nullptr) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == '\0') {
		extension = "";
	}

	// search
	nfiles = 0;

	QDir root (directory);
	if ( !root.exists() )
	{
		return fileList;
	}

	QDir::Filters filters = QDir::Files;
	if ( wantsubs )
	{
		filters |= QDir::AllDirs;
	}

	QStringList nameFilters;
	QString starExtension = "*";
	starExtension += extension;
	nameFilters.push_back (starExtension);

	QStringList files = root.entryList (nameFilters, filters);
	nfiles = std::min (files.size(), MAX_FOUND_FILES - 1);
	
	fileList.resize (nfiles);
	
	for ( int i = 0; i < nfiles; i++ )
	{
		fileList[i] = files[i].toLatin1().constData();
	}

	return fileList;
}
