#include "stdafx.h"
#include "StringUtils.h"

#include <algorithm>
#include <functional>

void ToLower ( std::string& s )
{
    std::transform (s.begin(), s.end(), s.begin(), tolower);
}

std::string& LeftTrim ( std::string& s )
{
    s.erase (s.begin(), std::find_if (s.begin(), s.end(), std::not1 (std::ptr_fun<int, int>(isspace))));
    return s;
}

std::string& RightTrim ( std::string& s )
{
    s.erase (std::find_if (s.rbegin(), s.rend(), std::not1 (std::ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}

std::string& Trim ( std::string& s )
{
    return LeftTrim (RightTrim (s));
}

std::string Replace ( const std::string& s, const std::string& find, const std::string& replace )
{
    std::string newString (s);
    std::size_t pos = newString.find (find);
    while ( pos != std::string::npos )
    {
        newString = newString.replace (pos, find.length(), replace);
        pos = newString.find (find, pos + find.length());
    }

    return newString;
}