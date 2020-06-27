#pragma once

#include <ctime>

typedef struct
{
    char sName[MAX_QPATH]; // game-relative path (without extension)
    char sExt[20];   // ".tga" etc (extra space for stuff like ".shader" maybe?)
    bool bIsDefault; // false in all but one cases
    byte* pPixels;   // actual texture bitmap
    int iWidth, iHeight; // size of TextureData
    GLuint gluiBind; // id the texture is actually bound to (may be zero if file
                     // was missing)
    GLuint gluiDesiredBind; // id the texture would be bound to if present (used
                            // for refresh purposes)
    int iUsageCount;
    time_t ft;
    bool bFTValid;
} Texture_t;

void Texture_SetFilter(void);
void TextureList_SetFilter(void);
void TextureList_OnceOnlyInit(void);
void TextureList_DeleteAll(void);
int TextureList_GetMip(void);
void TextureList_ReMip(int iMIPLevel);
void TextureList_Refresh(void);
TextureHandle_t TextureHandle_ForName(const char* psLocalTexturePath);
GLuint Texture_GetGLBind(TextureHandle_t thHandle);
Texture_t* Texture_GetTextureData(TextureHandle_t thHandle);
int Texture_Load(const char* psLocalTexturePath, bool bInhibitStatus = false);

void FakeCvars_Shutdown(void);
void FakeCvars_OnceOnlyInit(void);

void OnceOnly_GLVarsInit(void);