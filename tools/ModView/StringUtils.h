#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

void ToLower ( std::string& s );
char *ToLower ( char *s );
char *ToUpper ( char *s );

std::string& LeftTrim ( std::string& s );
std::string& RightTrim ( std::string& s );
std::string& Trim ( std::string& s );
std::string Replace ( const std::string& s, const std::string& find, const std::string& replace );

int Q_stricmpn (const char *s1, const char *s2, int n);
int Q_stricmp (const char *s1, const char *s2);
int Q_strncmp (const char *s1, const char *s2, int n);

#endif