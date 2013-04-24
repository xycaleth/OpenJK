// Filename:-	text.h
//
//

#ifndef TEXT_H
#define TEXT_H

#define TEXT_WIDTH 8
#define TEXT_DEPTH 8

//void Text_Create(void);	// called automatically internally
void Text_Destroy(void);	// should be called at program shutdown
void Text_Display(const char * psString, vec3_t v3DPos, unsigned char r, unsigned char g, unsigned char b);
int  Text_DisplayFlat(const char * psString, int x, int y, unsigned char r, unsigned char g, unsigned char b, bool bResizeStringIfNecessary = false);

extern int g_iScreenWidth;
extern int g_iScreenHeight;
extern bool gbTextInhibit;


#endif	// #ifndef TEXT_H

////////////////////////// eof ///////////////////////////



