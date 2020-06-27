#pragma once

#include <string>

typedef unsigned char byte;

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

bool SendFileToNotepad(const char * psFilename);
bool SendStringToNotepad(const char * psWhatever, const char * psLocalFileName);

#ifndef WIN32
#define OutputDebugString(x) (void)(x)
#define APIENTRY
#endif