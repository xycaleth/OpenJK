#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

void ToLower ( std::string& s );
std::string& LeftTrim ( std::string& s );
std::string& RightTrim ( std::string& s );
std::string& Trim ( std::string& s );
std::string Replace ( const std::string& s, const std::string& find, const std::string& replace );

#endif