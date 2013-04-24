// Filename:-	generic_stuff.h
//

#ifndef GENERIC_STUFF_H
#define GENERIC_STUFF_H

#include <string>

typedef unsigned char byte;

extern char qdir[];
extern char	gamedir[];

void SetQdirFromPath( const char *path );
void Filename_RemoveQUAKEBASE(std::string& string);
bool FileExists (const char * psFilename);
long FileLen( const char * psFilename);
char *Filename_WithoutPath(const char * psFilename);
char *Filename_WithoutExt(const char * psFilename);
char *Filename_PathOnly(const char * psFilename);
char *Filename_ExtOnly(const char * psFilename);
char *String_EnsureMinLength(const char * psString, int iMinLength);
char *String_ToLower(const char * psString);
char *String_ToUpper(const char * psString);
char *String_ForwardSlash(const char * psString);
char *va(const char *format, ...);
char *scGetTempPath(void);

extern bool gbErrorBox_Inhibit;
const char * ModView_GetLastError();
void ErrorBox(const char *sString);
void InfoBox(const char *sString);
void WarningBox(const char *sString);
void StatusMessage(const char * psString);

void  SystemErrorBox(unsigned int dwError);
const char * SystemErrorString(unsigned int dwError);

//#define GetYesNo(psQuery)	(!!(MessageBox(AppVars.hWnd,psQuery,"Query",MB_YESNO|MB_ICONWARNING|MB_TASKMODAL)==IDYES))
bool GetYesNo(const char *psQuery);

long filesize(FILE *handle);
int LoadFile (const char * psPathedFilename, void **bufferptr, int bReportErrors = true);
int SaveFile (const char * psPathedFilename, const void *pBuffer, int iSize);

bool SendFileToNotepad(const char * psFilename);
bool SendStringToNotepad(const char * psWhatever, const char * psLocalFileName);

#ifndef WIN32
#define OutputDebugString(x) (void)(x)
#define APIENTRY
#endif

#endif	// #ifndef GENERIC_STUFF_H


////////////// eof /////////////

