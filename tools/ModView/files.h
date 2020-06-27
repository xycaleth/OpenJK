#pragma once

#include <cstdio>
#include <string>
#include <vector>

#define MAX_QPATH 64

#if defined(_WIN32)
#include <Windows.h>
#define MAX_OSPATH MAX_PATH
#else
#include <sys/syslimits.h>
#define MAX_OSPATH PATH_MAX
#endif

extern char qdir[];
extern char gamedir[];

std::vector<std::string>
Sys_ListFiles(const char* directory, const char* extension, bool wantsubs);

void SetQdirFromPath(const char* path);
void Filename_RemoveQUAKEBASE(std::string& string);
bool FileExists(const char* psFilename);
long FileLen(const char* psFilename);
char* Filename_WithoutPath(const char* psFilename);
char* Filename_WithoutExt(const char* psFilename);
char* Filename_PathOnly(const char* psFilename);
char* Filename_ExtOnly(const char* psFilename);

long filesize(FILE *handle);
int LoadFile (const char * psPathedFilename, void **bufferptr, int bReportErrors = true);
int SaveFile (const char * psPathedFilename, const void *pBuffer, int iSize);