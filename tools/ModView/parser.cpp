// Filename:-	parser.cpp
//
#include "stdafx.h"
#include "includes.h"
#include "stl.h"
//
#include "StringUtils.h"
#include "parser.h"


// Very simple parser, I just read the "alias" part of a raven-generic file, and store them into
//	a string map. 
//
// Example file:
//
/*
Alias
{
	"srcarples"	"boltpoint_righthand"
}

Alias
{
	"slcarples"	"boltpoint_lefthand"
}
*/
//  (possibly more than one per "Alias" brace? I'll code for it.
// 
// return = success / fail...
//
bool Parser_Load(const char * psFullPathedFilename, MappedString_t &ParsedAliases)
{
	bool bReturn = false;

	ParsedAliases.clear();

	FILE *fhHandle = fopen(psFullPathedFilename,"rt");
	if (fhHandle)
	{			
		bool bParsingBlock = false;
		bool bSkippingBlock= false;
		char sLine[1024];

		while (fgets(sLine,sizeof(sLine)-1,fhHandle)!=NULL)
		{
			sLine[sizeof(sLine)-1]='\0';

			// :-)
            std::string str(sLine);
            Trim (str);

			strcpy(sLine,str.c_str());

			if (!bSkippingBlock)
			{
				if (!bParsingBlock)
				{
					if (strlen(sLine))	// found any kind of header?
					{
						if (!Q_stricmp(sLine,"Alias"))
						{							
							bParsingBlock = true;						
						}
						else							
						{
							// not a recognised header, so...
							//
							bSkippingBlock = true;
						}
					}
					continue;
				}
				else
				{
					if (!Q_stricmp(sLine,"{"))
						continue;

					if (!Q_stricmp(sLine,"}"))
					{
						bParsingBlock = false;
						continue;
					}

					if (strlen(sLine))
					{
						// must be a value pair, so...
						//
						// first, find the whitespace that seperates them...
						//
                        std::string strPair(sLine);
						unsigned int iLoc = strPair.find_first_of(" \t");
						if (iLoc == std::string::npos)
						{
							assert(0);
							ErrorBox(va("Parser_Load(): Couldn't find whitespace-seperator in line:\n\n\"%s\"\n\n( File: \"%s\" )", strPair.c_str(), psFullPathedFilename));
							bReturn = false;
							break;
						}

						// stl & MFC rule!...
						//
                        std::string strArg_Left(strPair.substr(0, iLoc));	// real name
                        RightTrim(strArg_Left);
                        strArg_Left.erase (std::remove (strArg_Left.begin(), strArg_Left.end(), '"'), strArg_Left.end());

                        std::string strArg_Right(strPair.substr (iLoc));	// alias name
                        LeftTrim (strArg_Right);
                        strArg_Right.erase (std::remove (strArg_Right.begin(), strArg_Right.end(), '"'), strArg_Right.end());

						ParsedAliases[strArg_Left.c_str()] = strArg_Right.c_str();

						bReturn = true;
						continue;
					}
				}
			}
			else
			{
				// skip to close brace...
				//
				if (Q_stricmp(sLine,"}"))
					continue;

				bSkippingBlock = false;
			}
		}

		fclose(fhHandle);
	}
	// DT EDIT
	/*
	else
	{
		ErrorBox( va("Couldn't open file: %s\n", psFullPathedFilename));
		return false;
	}
	*/

	return bReturn;
}


/////////////// eof /////////////

