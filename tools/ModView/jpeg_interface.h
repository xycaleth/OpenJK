// Filename:-	jpeg_interface.h
//


#ifndef JPEG_INTERFACE_H
#define JPEG_INTERFACE_H


#ifdef __cplusplus
extern "C"
{
#endif


void LoadJPG( const char *filename, unsigned char **pic, int *width, int *height );

void JPG_ErrorThrow(const char * message);
void JPG_MessageOut(const char * message);
#define ERROR_STRING_NO_RETURN(message) JPG_ErrorThrow(message)
#define MESSAGE_STRING(message)			JPG_MessageOut(message)


void *JPG_Malloc( int iSize );
void JPG_Free( void *pvObject);


#ifdef __cplusplus
};
#endif



#endif	// #ifndef JPEG_INTERFACE_H


////////////////// eof //////////////////

