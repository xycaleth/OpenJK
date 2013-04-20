#include "stdafx.h"
#include "StringUtils.h"

#include <algorithm>
#include <functional>

void ToLower ( std::string& s )
{
    std::transform (s.begin(), s.end(), s.begin(), tolower);
}

char *ToLower ( char *s )
{
    std::transform (s, s + strlen (s), s, tolower);
    return s;
}

char *ToUpper ( char *s )
{
    std::transform (s, s + strlen (s), s, toupper);
    return s;
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

int Q_strncmp (const char *s1, const char *s2, int n) {
    int     c1, c2;
    
    do {
        c1 = *s1++;
        c2 = *s2++;
        
        if (!n--) {
            return 0;       // strings are equal until end point
        }
        
        if (c1 != c2) {
            return c1 < c2 ? -1 : 1;
        }
    } while (c1);
    
    return 0;       // strings are equal
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
    int     c1, c2;
    
    // bk001129 - moved in 1.17 fix not in id codebase
    if ( s1 == NULL ) {
        if ( s2 == NULL )
            return 0;
        else
            return -1;
    }
    else if ( s2==NULL )
        return 1;
    
    
    
    do {
        c1 = *s1++;
        c2 = *s2++;
        
        if (!n--) {
            return 0;       // strings are equal until end point
        }
        
        if (c1 != c2) {
            if (c1 >= 'a' && c1 <= 'z') {
                c1 -= ('a' - 'A');
            }
            if (c2 >= 'a' && c2 <= 'z') {
                c2 -= ('a' - 'A');
            }
            if (c1 != c2) {
                return c1 < c2 ? -1 : 1;
            }
        }
    } while (c1);
    
    return 0;       // strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
    return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}