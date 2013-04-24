// Filename:-	wintalk.h
//
// headers for inter-program communication
//

#ifndef WINTALK_H
#define WINTALK_H



bool WinTalk_HandleMessages(void);
bool WinTalk_IssueCommand(const char * psCommand, byte *pbData = NULL, int iDataSize = 0, const char * *ppsResultPassback = NULL, byte **ppbDataPassback = NULL, int *piDataSizePassback = NULL);



#endif	// #ifndef WINTALK_H

/////////////// eof /////////////

