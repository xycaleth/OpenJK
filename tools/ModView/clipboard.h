// Filename:-	clipboard.h
//

#ifndef CLIPBOARD_H
#define CLIPBOARD_H


bool Clipboard_SendString(const char * psString);
bool ClipBoard_SendDIB(void * pvData, int iBytes);

// other stuff that's not actually clipboard, but is called only in conjunction with it anyway...
//
bool ScreenShot(const char * psFilename = NULL, const char * psCopyrightMessage = NULL, int iWidth = g_iScreenWidth, int iHeight = g_iScreenHeight);
bool BMP_GetMemDIB(void *&pvAddress, int &iBytes);
void BMP_Free(void);

#endif

/////////////////// eof //////////////////

