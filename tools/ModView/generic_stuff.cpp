// Filename:	generic_stuff.cpp
//

#include "stdafx.h"
#include "generic_stuff.h"

#include <QtWidgets/QMessageBox>
#include <algorithm>
#include <string>
#include "includes.h"
#include "StringUtils.h"


// used when sending output to a text file etc, keeps columns nice and neat...
//
char *String_EnsureMinLength(const char * psString, int iMinLength)
{
	static char	sString[8][1024];
	static int i=0;

	i=(i+1)&7;

	strcpy(sString[i],psString);

	// a bit slow and lazy, but who cares?...
	//
	while (strlen(sString[i])<(unsigned int)iMinLength)
		strcat(sString[i]," ");

	return sString[i];

}


// similar to strlwr, but (crucially) doesn't touch original...
//
char *String_ToLower(const char * psString)
{
	static char sString[MAX_QPATH];

	strcpy(sString,psString);
	
	return ToLower(sString);	

}

// similar to strupr, but (crucially) doesn't touch original...
//
char *String_ToUpper(const char * psString)
{
	static char sString[MAX_QPATH];

	strcpy(sString,psString);
	return ToUpper(sString);

}

char *String_ForwardSlash(const char * psString)
{
	static char sString[MAX_QPATH];

	strcpy(sString,psString);

	char *p;
	while ( (p=strchr(sString,'\\')) != NULL)
	{
		*p = '/';
	}
	
	return ToLower(sString);
}

char	*va(const char *format, ...)
{
	va_list		argptr;
	static char		string[16][16384];		// important to have a good depth to this. 16 is nice
	static int index = 0;

	index = (++index)&15;
	
	va_start (argptr, format);
//	vsprintf (string[index], format,argptr);
	vsnprintf(string[index], sizeof(string[0]), format, argptr);
	va_end (argptr);
	string[index][sizeof(string[0])-1] = '\0';

	return string[index];	
}


#ifdef _WIN32
// returns a path to somewhere writeable, without trailing backslash...
//
// (for extra speed now, only evaluates it on the first call, change this if you like)
//
char *scGetTempPath(void)
{
	static char sBuffer[MAX_QPATH];
	unsigned int dwReturnedSize;
	static int i=0;
	
	if (!i++)
	{
		dwReturnedSize = GetTempPath(sizeof(sBuffer),sBuffer);
		
		if (dwReturnedSize>sizeof(sBuffer))
		{
			// temp path too long to return, so forget it...
			//
			strcpy(sBuffer,"c:");	// "c:\\");	// should be writeable
		}
		
		// strip any trailing backslash...
		//
		if (sBuffer[strlen(sBuffer)-1]=='\\')
			sBuffer[strlen(sBuffer)-1]='\0';
	}// if (!i++)
	
	return sBuffer;
	
}
#endif

// these MUST all be MB_TASKMODAL boxes now!!
//
bool gbErrorBox_Inhibit = false;	// Keith wanted to be able to remote-deactivate these...
std::string strLastError;
const char * ModView_GetLastError()		// function for him to be able to interrogate if he's disabled the error boxes
{
	return strLastError.c_str();
}


extern bool Gallery_Active();
extern void Gallery_AddError	(const char * psText);
extern void Gallery_AddInfo		(const char * psText);
extern void Gallery_AddWarning	(const char * psText);

void ErrorBox(const char *sString)
{
	/*if (Gallery_Active())
	{
		Gallery_AddError(sString);
		Gallery_AddError("\n");
		return;
	}*/

	if (!gbErrorBox_Inhibit)
	{
		QMessageBox::critical (NULL, "Error", sString);
	}

	strLastError = sString;
}
void InfoBox(const char *sString)
{
	/*if (Gallery_Active())
	{
		Gallery_AddInfo(sString);
		Gallery_AddInfo("\n");
		return;
	}*/

	QMessageBox::information (NULL, "Information", sString);
}
void WarningBox(const char *sString)
{
	/*if (Gallery_Active())
	{
		Gallery_AddWarning(sString);
		Gallery_AddWarning("\n");
		return;
	}*/

    QMessageBox::warning (NULL, "Warning", sString);
}



bool GetYesNo(const char *psQuery)
{
	/*if (Gallery_Active())
	{
		Gallery_AddWarning("GetYesNo call (... to which I guessed 'NO' ) using this query...\n\n");
		Gallery_AddWarning(psQuery);
		Gallery_AddWarning("\n");
		return false;
	}*/

	return QMessageBox::question (NULL, "Query", psQuery) == QMessageBox::Yes;
}

void StatusMessage(const char * psString)	// param can be NULL to indicate fallback to "ready" or whatever you want
{
	/*CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if (pMainFrame)
	{
		pMainFrame->StatusMessage(psString);
	}*/
}



// this'll return a string of up to the first 4095 chars of a system error message...
//
const char * SystemErrorString(unsigned int dwError)
{
	static char sString[4096];
#ifdef _WIN32
	LPVOID lpMsgBuf=0;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);		

	ZEROMEM(sString);
	strncpy(sString,(const char *) lpMsgBuf,sizeof(sString)-1);

	LocalFree( lpMsgBuf ); 
#endif
	return sString;
}


void SystemErrorBox(unsigned int dwError)
{
	ErrorBox( SystemErrorString(dwError) );
}

bool SendFileToNotepad(const char * psFilename)
{
#ifdef _WIN32
	bool bReturn = false;

	char sExecString[MAX_QPATH];

	sprintf(sExecString,"notepad %s",psFilename);

	if (WinExec(sExecString,	//  const char * lpCmdLine,  // address of command line
				SW_SHOWNORMAL	//  UINT uCmdShow      // window style for new application
				)
		>31	// don't ask me, Windoze just uses >31 as OK in this call.
		)
	{
		// ok...
		//
		bReturn = true;
	}
	else
	{
		ErrorBox("Unable to locate/run NOTEPAD on this machine!\n\n(let me know about this -Ste)");		
	}

	return bReturn;
#else
	return true;
#endif
}

// creates as temp file, then spawns notepad with it...
//
bool SendStringToNotepad(const char * psWhatever, const char * psLocalFileName)
{
#ifdef _WIN32
	bool bReturn = false;

	const char * psOutputFileName = va("%s\\%s",scGetTempPath(),psLocalFileName);

	FILE *handle = fopen(psOutputFileName,"wt");
	if (handle)
	{
		fputs(psWhatever, handle);
		fclose(handle);

		bReturn = SendFileToNotepad(psOutputFileName);
	}
	else
	{
		ErrorBox(va("Unable to create file \"%s\" for notepad to use!",psOutputFileName));
	}

	return bReturn;
#else
	return true;
#endif
}



////////////////////////// eof ///////////////////////////
