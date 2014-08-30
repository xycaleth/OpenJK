#include "stdafx.h"

#include <algorithm>
#include "files.h"
#include "stl.h"

#include <QtCore/QDir>

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

#define	MAX_FOUND_FILES	0x1000

std::vector<std::string> Sys_ListFiles( const char *directory, const char *extension, bool wantsubs ) {
	int nfiles;
	std::vector<std::string> fileList;

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
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
