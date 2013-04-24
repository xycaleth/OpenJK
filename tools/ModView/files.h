// Filename:-	files.h
//

#ifndef FILES_H
#define FILES_H

#include <string>
#include <vector>


char*	Com_StringContains(char *str1, char *str2, int casesensitive);
int		Com_Filter(char *filter, char *name, int casesensitive);
int		Com_FilterPath(char *filter, char *name, int casesensitive);

char	*CopyString( const char *in );

std::vector<std::string> Sys_ListFiles ( const char *directory, const char *extension, bool wantsubs );

#endif	// #ifndef FILES_H


/////////////////////// eof ////////////////////////

