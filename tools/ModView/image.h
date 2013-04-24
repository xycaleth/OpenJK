// Filename:-	image.h
//


#ifndef IMAGE_H
#define IMAGE_H

#include "textures.h"

bool	DStamp_MarkImage(Texture_t *pTexture, const char * psText);	// this-app-specific
const char *	DStamp_MarkImage(byte *pPixels, int iWidth, int iHeight, int iPlanes, const char * psText);	// generic
const char *	DStamp_ReadImage(byte *pPixels, int iWidth, int iHeight, int iPlanes);

void DStamp_AnalyseImage(byte *pPixels, int iWidth, int iHeight, int iPlanes);
void DStamp_AnalyseImage(Texture_t *pTexture);


#endif	// #ifndef IMAGE_H

//////////////// eof ////////////

