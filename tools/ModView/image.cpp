// Filename:-	image.cpp
//

#include "stdafx.h"
#include "includes.h"
//#include "modview.h"
#include "textures.h"
//
#include "image.h"



#define iDSTAMP_CELL_PIXDIM			16	// adjust this, higher = more secure but less stamps on picture
#define iDSTAMP_CELL_PIXELS			(iDSTAMP_CELL_PIXDIM * iDSTAMP_CELL_PIXDIM)
#define iDSTAMP_INSTANCE_CELLDIM	8	// leave this alone at all times!
#define iDSTAMP_INSTANCE_PIXDIM		(iDSTAMP_INSTANCE_CELLDIM * iDSTAMP_CELL_PIXDIM)
#define iDSTAMP_CHAR_BITS			7



#define iHIDDENBIT		1//32	// 1,2,4,8,16,32,64, or 128
#define iHIDDENBITSHIFT 0//5	// 0,1,2,3, 4  5  6       7

#pragma pack(push,1)
	typedef	struct
	{
		char	sHDR[3];		// "HDR", no trailing zero
		char	sDDMMYY[6];		// date/month/year, no trailing zero
		char	sText[14];		// text, no end trailing zero, but maybe early one
	} WatermarkData_t;
	typedef struct
	{
		WatermarkData_t Data;
		unsigned int	dwCRC;
	} Watermark_t;
	typedef struct
	{
		byte R,G,B,A;
	}Pixel_t;
#pragma pack(pop)

typedef struct
{
	byte *pPixels;
	int iWidth;
	int iHeight;
	int iPlanes;
} DStampImage_t;

typedef struct
{
	DStampImage_t *pImage;
	Watermark_t	*pWatermark;
} DStampData_t;

Watermark_t TheWatermark;

#define PixelSetR(p,r) p.R=r
#define PixelSetG(p,g) p.G=g
#define PixelSetB(p,b) p.B=b
#define PixelSetA(p,a) p.A=a

#define PixelGetR(p) p.R
#define PixelGetG(p) p.G
#define PixelGetB(p) p.B
#define PixelGetA(p) p.A

#define PixelInit(p) memset(&p,0,sizeof(p));	// later on I'll adapt this for initing default alpha etc, but for now



#define PROGRESS_INIT \
	CProgressCtrl *pProgress = NULL; \
	if (((CModViewApp*)AfxGetApp())->m_pMainWnd)	\
	{												\
		pProgress = new CProgressCtrl;				\
		bool bOK = !!pProgress->Create(	WS_CHILD|WS_VISIBLE|PBS_SMOOTH,				\
										CRect(100,100,200,200),						\
										((CModViewApp*)AfxGetApp())->m_pMainWnd,	\
										1											\
										);											\
		if (!bOK)						\
		{								\
			delete pProgress;			\
			pProgress = NULL;			\
		}								\
	}
	

#define PROGRESS_SETRANGE(range)		\
	if (pProgress)						\
	{									\
		pProgress->SetRange(0,range);	\
	}	

#define PROGRESS_SETPOS(position)	\
	if (pProgress)					\
	{								\
		pProgress->SetPos(position);\
	}
					
#define PROGRESS_CLOSE		\
	if (pProgress)			\
	{						\
		delete pProgress;	\
		pProgress = 0;		\
	}

// return NULL = legal, else string saying why not...
//
static const char * DStamp_TextIsLegal(const char * psText)
{
	int iLen = strlen(psText);
	if (iLen > sizeof(TheWatermark.Data.sText))
	{
		return va("\"%s\" is too long by %d chars",psText, sizeof(TheWatermark.Data.sText) - iLen);
	}

	for (int i=0; i<iLen; i++)
	{
		if (psText[i] > 127)
		{
			return va("\"%s\" contains hi-ascii char '%c', only 7-bit ascii allowed",psText, psText[i]);
		}
	}

	return NULL;
}


static void DStamp_EncryptData(void *pvData, int iByteLength)
{	
/*	don't use this method since we need to keep as 7-bit ascii
	byte *pData = (byte*) pvData;

	for (int i=0; i<iByteLength; i++)
	{
		*pData = *pData ^ ('#'+i);
		pData++;
	}
*/
}
// same as encrypt at the moment, because of XOR, but may change...
//
static void DStamp_DecryptData(void *pvData, int iByteLength)
{
	DStamp_EncryptData(pvData,iByteLength);
}

static unsigned int DStamp_UpdateCRC(unsigned int dwCRC, byte b)
{
	dwCRC += (unsigned int) b;

	if (dwCRC & (1<<31))
	{
		dwCRC<<=1;
		dwCRC+=2;	// add 1, plus carry
	}
	else
	{
		dwCRC<<=1;
		dwCRC++;	// add 1, no carry
	}

	return dwCRC;
}

static unsigned int DStamp_CalcBlockCRC(void *pvData, int iByteLength)
{
	byte *pData = (byte*) pvData;

	unsigned int dwCRC = 0;
	
	for (int i=0; i<iByteLength; i++)
	{
		dwCRC = DStamp_UpdateCRC(dwCRC, pData[i]);
	}

	dwCRC &= 0x7F7F7F7F;	// lose 7th bit from each byte
	return dwCRC;
}



static const char * DStamp_GetYear(void)
{
	static char sTemp[20];	
    time_t ltime;	

    time( &ltime );    
	
    struct tm *today = localtime( &ltime );    
	
	strftime( sTemp, sizeof(sTemp), "%d%m%y", today );

	return &sTemp[0];
}


static Watermark_t *DStamp_InitWatermark(const char * psText)
{
	Watermark_t *pWatermark = &TheWatermark;

	strncpy(pWatermark->Data.sHDR,		"HDR",				sizeof(pWatermark->Data.sHDR));
	strncpy(pWatermark->Data.sText,		psText,				sizeof(pWatermark->Data.sText));
	strncpy(pWatermark->Data.sDDMMYY,	DStamp_GetYear(),	sizeof(pWatermark->Data.sDDMMYY));

	DStamp_EncryptData(&pWatermark->Data, sizeof(pWatermark->Data));
	pWatermark->dwCRC = DStamp_CalcBlockCRC(&pWatermark->Data, sizeof(pWatermark->Data));

	return pWatermark;
}


static Pixel_t *DStamp_GetImagePixel(DStampImage_t *pImage, int iXpos, int iYpos)
{
	int iBPP = (pImage->iPlanes == 24)?3:4;

	byte *pPixels = pImage->pPixels + (iYpos * pImage->iWidth * iBPP) + (iXpos * iBPP);

	static Pixel_t Pixel;
	memcpy(&Pixel, pPixels, iBPP);
	if (iBPP == 3)
	{
		Pixel.A = 255;	// actually, this can be ignored
	}
	
	return &Pixel;
}

static void DStamp_SetImagePixel(DStampImage_t *pImage, int iXpos, int iYpos, Pixel_t *pPixel)
{
	int iBPP = (pImage->iPlanes == 24)?3:4;

	byte *pPixels = pImage->pPixels + (iYpos * pImage->iWidth * iBPP) + (iXpos * iBPP);

	memcpy(pPixels, pPixel, iBPP);
}

// returns either 0 or 1 for bits, or 2 for "finished"
//
static int DStamp_GetWaterMarkBit(int *piSourceIndex_Byte, int *piSourceIndex_Bit, void *pvBytes, int iBytesLen)
{
	byte *pBytes = (byte *) pvBytes;
//	if (*piSourceIndex_Byte==(int)strlen(psWatermarkString)+1)	// trying to get bits from beyond trailing zero?
//		return 2;
	if (*piSourceIndex_Byte==iBytesLen)	// trying to get bits from beyond trailing zero?
		return 2;

	char c = pBytes[*piSourceIndex_Byte];

	int iBitReturn = (c>>*piSourceIndex_Bit)&1;
	*piSourceIndex_Bit+=1;	// *not* ++!
	if (*piSourceIndex_Bit == iDSTAMP_CHAR_BITS)
	{
		*piSourceIndex_Bit = 0;
		*piSourceIndex_Byte+=1;	// *not* ++!
	}

	return iBitReturn;
}


static void DStamp_ReadCell(DStampData_t *pData, int iCellXpos, int iCellYpos, byte *b1, byte *b2, byte *b3)
{
	int iWBitR = 0,
		iWBitG = 0,
		iWBitB = 0;

	for (int iPixelY=0; iPixelY<iDSTAMP_CELL_PIXDIM; iPixelY++)
	{
		for (int iPixelX=0; iPixelX<iDSTAMP_CELL_PIXDIM; iPixelX++)
		{
			Pixel_t Pixel = *DStamp_GetImagePixel(pData->pImage, iPixelX + iCellXpos, iPixelY + iCellYpos);

			iWBitR += (PixelGetR(Pixel)&iHIDDENBIT)?1:0;
			iWBitG += (PixelGetG(Pixel)&iHIDDENBIT)?1:0;
			iWBitB += (PixelGetB(Pixel)&iHIDDENBIT)?1:0;
		}
	}

	// work out pixel consensuses... (consensi?)
	//
	// if > half the pixels are true, accept that as true...
	//
	*b1 = (iWBitR > iDSTAMP_CELL_PIXELS/2)?1:0;
	*b2 = (iWBitG > iDSTAMP_CELL_PIXELS/2)?1:0;
	*b3 = (iWBitB > iDSTAMP_CELL_PIXELS/2)?1:0;
}

static void DStamp_MarkCell(DStampData_t *pData, int iCellXpos, int iCellYpos, int *piTxtByte, int *piTxtBit)
{
	byte R = DStamp_GetWaterMarkBit(piTxtByte, piTxtBit, pData->pWatermark, sizeof(*pData->pWatermark));
	byte G = DStamp_GetWaterMarkBit(piTxtByte, piTxtBit, pData->pWatermark, sizeof(*pData->pWatermark));
	byte B = DStamp_GetWaterMarkBit(piTxtByte, piTxtBit, pData->pWatermark, sizeof(*pData->pWatermark));
	
	for (int iPixelY=0; iPixelY<iDSTAMP_CELL_PIXDIM; iPixelY++)
	{
		for (int iPixelX=0; iPixelX<iDSTAMP_CELL_PIXDIM; iPixelX++)
		{
			Pixel_t Pixel = *DStamp_GetImagePixel(pData->pImage, iPixelX + iCellXpos, iPixelY + iCellYpos);

			PixelSetR(Pixel,(  (PixelGetR(Pixel)&~iHIDDENBIT) | (((R==2)?0:R)<<iHIDDENBITSHIFT) ));
			PixelSetG(Pixel,(  (PixelGetG(Pixel)&~iHIDDENBIT) | (((G==2)?0:G)<<iHIDDENBITSHIFT) ));
			PixelSetB(Pixel,(  (PixelGetB(Pixel)&~iHIDDENBIT) | (((B==2)?0:B)<<iHIDDENBITSHIFT) ));

			DStamp_SetImagePixel(pData->pImage, iPixelX + iCellXpos, iPixelY + iCellYpos, &Pixel);
		}
	}

/*	byte R2,G2,B2;
	DStamp_ReadCell(pData, iCellXpos, iCellYpos, &R2,&G2,&B2);

	assert(	(R2==((R==2)?0:R)) && 
			(G2==((G==2)?0:G)) && 
			(B2==((B==2)?0:B))
			);
*/
}

// apply one watermark-instance to the image...
//
static void DStamp_MarkInstance(DStampData_t *pData, int iInstancePixelX, int iInstancePixelY)
{
	int iTxtByte = 0;
	int iTxtBit  = 0;

	for (int iYCell = 0; iYCell < iDSTAMP_INSTANCE_CELLDIM; iYCell++)
	{
		for (int iXCell = 0; iXCell < iDSTAMP_INSTANCE_CELLDIM; iXCell++)
		{
			int iCellXpos = iInstancePixelX + (iXCell * iDSTAMP_CELL_PIXDIM);
			int iCellYpos = iInstancePixelY + (iYCell * iDSTAMP_CELL_PIXDIM);
			DStamp_MarkCell(pData, iCellXpos, iCellYpos, &iTxtByte, &iTxtBit);
		}
	}
}


// see if we can read some watermark data, else return NULL...
//
static const char * DStamp_ReadInstance(DStampData_t *pData, int iInstancePixelX, int iInstancePixelY)
{
	const char * psMessage = NULL;

	int iTxtByte = 0;
	int iTxtBit  = 0;

	byte *pbOut = (byte*) pData->pWatermark;
	memset(pbOut,0,sizeof(*pData->pWatermark));

	for (int iYCell = 0; iYCell < iDSTAMP_INSTANCE_CELLDIM; iYCell++)
	{
		for (int iXCell = 0; iXCell < iDSTAMP_INSTANCE_CELLDIM; iXCell++)
		{
			int iCellXpos = iInstancePixelX + (iXCell * iDSTAMP_CELL_PIXDIM);
			int iCellYpos = iInstancePixelY + (iYCell * iDSTAMP_CELL_PIXDIM);

			byte b1,b2,b3;						
			DStamp_ReadCell(pData, iCellXpos, iCellYpos, &b1,&b2,&b3);

#define DECODEBIT(DestString,bit) \
				DestString[iTxtByte] |= bit<<iTxtBit++;	\
				if (iTxtBit==iDSTAMP_CHAR_BITS)			\
				{										\
					iTxtBit=0;							\
					iTxtByte++;							\
				}

			DECODEBIT(pbOut,b1);
			DECODEBIT(pbOut,b2);
			DECODEBIT(pbOut,b3);

			if (iTxtByte>=3)	// huge speed opt, check for header, if not found, give up on this cell...
			{
				if (strncmp((char*)pbOut,"HDR",3))
				{
					return NULL;
				}
			}
		}
	}

	if (!strncmp(pData->pWatermark->Data.sHDR,"HDR",3))
	{
		unsigned int dwCRC = DStamp_CalcBlockCRC(&pData->pWatermark->Data, sizeof(pData->pWatermark->Data));

		char sString[100];
		char sDate[100];
		strncpy(sString,pData->pWatermark->Data.sText,sizeof(pData->pWatermark->Data.sText));
		sString[sizeof(pData->pWatermark->Data.sText)] = '\0';
		strncpy(sDate,pData->pWatermark->Data.sDDMMYY,sizeof(pData->pWatermark->Data.sDDMMYY));
		sDate[sizeof(pData->pWatermark->Data.sDDMMYY)] = '\0';
		static char sOutput[1024];
		sprintf(sOutput,"SentTo: \"%s\",  Date(DD/MM/YY) = %s",sString,sDate);

		if (dwCRC == pData->pWatermark->dwCRC)
		{
			OutputDebugString(sOutput);
			psMessage = &sOutput[0];
		}
		else
		{
			OutputDebugString(va("Skipping non-CRC HDR-match: %s\n",sOutput));
		}
	}

	return psMessage;
}

////////////////// eof ////////////////
