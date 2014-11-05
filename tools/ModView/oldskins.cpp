//Filename:-	oldskins.cpp
//
// module containing code for old ID/EF1/CHC style skins...
//
#include "stdafx.h"

#include <algorithm>
#include <functional>
#include <string>
#include "includes.h"
#include "files.h"
#include "r_common.h"
//#include "ModViewTreeView.h"
//#include "glm_code.h"
//#include "R_Model.h"
//#include "R_Surface.h"
#include "StringUtils.h"
#include "textures.h"
//#include "TEXT.H"
//#include "sequence.h"
//#include "model.h"

////#include "GenericParser.h"
#include "oldskins.h"


// file format:  (dead simple)
//
//armor_chest,models/players/stormtrooper/torso_legs.tga
//armor_neck_augment,models/players/stormtrooper/torso_legs.tga
//body_torso,models/players/stormtrooper/torso_legs.tga

OldSkinSets_t OldSkinsFound;

// returns NULL for all ok, else error string...
//
const char * OldSkins_Parse(const char * psSkinName, const char * psText)
{
	std::string strText(psText);

    strText.erase (std::remove (strText.begin(), strText.end(), '\r'), strText.end());
    strText.erase (std::remove (strText.begin(), strText.end(), '\t'), strText.end());
    strText.erase (std::remove (strText.begin(), strText.end(), ' '), strText.end());

    std::transform (strText.begin(), strText.end(), strText.begin(), tolower);
	
	while (!strText.empty())
	{
		std::string strThisLine;
		
		std::size_t iLoc = strText.find('\n');

		if (iLoc == std::string::npos)
		{
			strThisLine = strText;
			strText.clear();
		}
		else
		{
			strThisLine = strText.substr (0, iLoc);
			strText = strText.substr (iLoc+1);
		}

		if (!strThisLine.empty())
		{
			iLoc = strThisLine.find (',');
			if (iLoc != std::string::npos)
			{	
				std::string strSurfaceName (strThisLine.substr (0, iLoc));						
				std::string strTGAName (strThisLine.substr (iLoc+1));

                Trim (strSurfaceName);
                Trim (strTGAName);

				//
				// new bit to cope with faulty ID skin files, where they have spurious lines like "tag_torso,"
				//
				// ("tag" is "*" in ghoul2)
				//
				if ( strTGAName.empty() || strSurfaceName[0] == '*' )
				{
					// crap line, so ignore it...
				}
				else
				{
					OldSkinsFound[psSkinName].push_back(std::make_pair (strSurfaceName, strTGAName));
				}
			}
			else
			{
				return va("Error parsing line \"%s\" in skin \"%s\"!",strThisLine.c_str(),psSkinName);
			}			
		}
	}

	return NULL;
}

// converts stuff like "<path>/stormtrooper_blue.skin" to "blue"...
//
std::string OldSkins_FilenameToSkinDescription(std::string strLocalSkinFileName)
{
	std::string strSkinName(Filename_WithoutPath(Filename_WithoutExt(strLocalSkinFileName.c_str())));

	ToLower (strSkinName);
	std::size_t iLoc = strSkinName.find('_');
	if (iLoc != std::string::npos)
	{
		strSkinName = strSkinName.substr(iLoc+1);
	}

	return strSkinName;
}


// returns true if at least one set of skin data was read, else false...
//
static bool OldSkins_Read(const char * psLocalFilename_GLM)
{
	const char * psError = NULL;

	OldSkinsFound.clear();
	
	const char * psSkinsPath = va("%s%s",gamedir,Filename_PathOnly(psLocalFilename_GLM));

	if (psSkinsPath)
	{
		std::string strSkinFileMustContainThisName( Filename_WithoutPath( Filename_WithoutExt( psLocalFilename_GLM )) );	
		strSkinFileMustContainThisName += "_";	// eg: "turret_canon_"
        
		// scan for skin files...
		//
        std::vector<std::string> skinFiles = Sys_ListFiles (psSkinsPath, ".skin", false);

		if ( skinFiles.empty() )
		{
			return false;
		}

		// load and parse skin files...
		//
		long iTotalBytesLoaded = 0;
		for ( std::size_t i=0; i<skinFiles.size() && !psError; i++ )
		{
			char sFileName[MAX_QPATH];

			std::string& strLocalSkinFileName(skinFiles[i]);

			// only look at skins that begin "modelname_skinvariation" for a given "modelname_"
			if (skinFiles[i].find (strSkinFileMustContainThisName) == 0)
			{
				Com_sprintf( sFileName, sizeof( sFileName ), "%s/%s", Filename_PathOnly(psLocalFilename_GLM), strLocalSkinFileName.c_str() );
				//ri.Printf( PRINT_ALL, "...loading '%s'\n", sFileName );

                char *buffer = NULL;
				iTotalBytesLoaded += ri.FS_ReadFile( sFileName, (void **)&buffer );

				if ( !buffer ) {
					ri.Error( ERR_DROP, "Couldn't load %s", sFileName );
				}

				psError = OldSkins_Parse( OldSkins_FilenameToSkinDescription(strLocalSkinFileName).c_str(), buffer);
				if (psError)
				{
					ErrorBox(va("Skins_Read(): Error reading file \"%s\"!\n\n( Skins will be ignored for this model )\n\nError was:\n\n%s",sFileName,psError));
					OldSkinsFound.clear();
				}
                
                ri.FS_FreeFile (buffer);
			}
		}
	}

	if (psError)
	{
		return false;
	}

	return !!(OldSkinsFound.size());
}



int DaysSinceCompile(void);
bool OldSkins_FilesExist(const char * psLocalFilename_GLM)
{		
	return OldSkins_Read(psLocalFilename_GLM);
}

bool OldSkins_Read(const char * psLocalFilename_GLM, ModelContainer_t *pContainer)
{
	if (OldSkins_Read(psLocalFilename_GLM))
	{
		pContainer->OldSkinSets = OldSkinsFound;	// huge nested stl-copy

/*		for (OldSkinSets_t::iterator it = OldSkinsFound.begin(); it != OldSkinsFound.end(); ++it)
		{
			string strFile = (*it).first;
			StringPairVector_t &blah = (*it).second;

			for (StringPairVector_t::iterator it2 = blah.begin(); it2 != blah.end(); ++it2)
			{
				string str1 = (*it2).first;
				string str2 = (*it2).second;

				OutputDebugString(va("Skin %s:  %s, %s\n",strFile.c_str(),str1.c_str(),str2.c_str()));
				OutputDebugString(__DATE__);
				OutputDebugString("\n");
			}
		}
*/
		return true;
	}

	return false;
}



// psSkinName = "blue", or "default" etc...
//
// this fills in a modelcontainer's "MaterialBinds" and "MaterialShaders" fields (registering textures etc)
//	based on the skinset pointed at by pContainer->OldSkinSets and strSkinFile
//
bool OldSkins_Apply( ModelContainer_t *pContainer, const char * psSkinName )
{
	bool bReturn = true;

	pContainer->strCurrentSkinFile	= psSkinName;

	pContainer->MaterialBinds.clear();
	pContainer->MaterialShaders.clear();

	for (int iSurface = 0; iSurface<pContainer->iNumSurfaces; iSurface++)
	{
		// when we're at this point we know it's GLM model, and that the shader name is in fact a material name...
		const char * psMaterialName = GLMModel_GetSurfaceShaderName( pContainer->hModel, iSurface );

		pContainer->MaterialShaders	[psMaterialName] = "";			// just insert the key for now, so the map<> is legit.
		pContainer->MaterialBinds	[psMaterialName] = (GLuint) 0;	// default to gl-white-notfound texture
	}

	OldSkinSets_t::iterator itOldSkins = pContainer->OldSkinSets.find(psSkinName);
	if (itOldSkins != pContainer->OldSkinSets.end())
	{
		StringPairVector_t &StringPairs = itOldSkins->second;

		for (std::size_t iSkinEntry = 0; iSkinEntry < StringPairs.size(); iSkinEntry++)
		{
			const char * psMaterialName = StringPairs[iSkinEntry].first.c_str();
			const char * psShaderName   = StringPairs[iSkinEntry].second.c_str();

			pContainer->MaterialShaders[psMaterialName] = psShaderName;

			if ( strcmp(psShaderName, "*off") == 0 )
			{
				pContainer->MaterialBinds[psMaterialName] = (GLuint)-1;
			}
			else
			{
				TextureHandle_t hTexture = Texture_Load(psShaderName);
				GLuint uiBind = Texture_GetGLBind( hTexture );
				pContainer->MaterialBinds[psMaterialName] = uiBind;
			}
		}
	}

	return bReturn;
}

#ifdef USE_MFC

bool OldSkins_ApplyToTree(HTREEITEM hTreeItem_Parent, ModelContainer_t *pContainer)
{
	bool bReturn = false;

	if (pContainer->OldSkinSets.size())
	{
		bReturn = true;

		TreeItemData_t	TreeItemData={0};
						TreeItemData.iItemType		= TREEITEMTYPE_OLDSKINSHEADER;
						TreeItemData.iModelHandle	= pContainer->hModel;

		HTREEITEM hTreeItem_SkinsHeader = ModelTree_InsertItem("Skins available", hTreeItem_Parent, TreeItemData.uiData);

		// skins...
		//
		int iSkinNumber = 0;
		for (OldSkinSets_t::iterator itSkins = pContainer->OldSkinSets.begin(); itSkins != pContainer->OldSkinSets.end(); ++itSkins, iSkinNumber++)
		{
			string strSkinName(itSkins->first);	// eg "blue"

			TreeItemData.iItemNumber	= iSkinNumber;
			TreeItemData.iItemType		= TREEITEMTYPE_OLDSKIN;
			
			HTREEITEM hTreeItem_ThisSkin = ModelTree_InsertItem(strSkinName.c_str(), hTreeItem_SkinsHeader, TreeItemData.uiData);
/*			
			// body parts...
			//
			StringPairVector_t &StringPairs = (*itSkins).second);
			int iMaterialNumber = 0;
			for (StringPairVector_t::iterator itMaterial = StringPairs.begin(); itMaterial != StringPairs.end(); ++itMaterial, iMaterialNumber++)
			{
				string strMaterialName((*itMaterial).first);	// eg "face"
				string strShaderName  ((*itMaterial).second);	// eg "face"

				TreeItemData.iItemNumber	= iMaterialNumber;
				TreeItemData.iItemType		= TREEITEMTYPE_SKINMATERIAL;

				HTREEITEM hTreeItem_ThisMaterial = ModelTree_InsertItem(strMaterialName.c_str(), hTreeItem_ThisEthnic, TreeItemData.uiData);

				// available shader variants for this material...
				//
				for (int iShaderVariantIndex=0; iShaderVariantIndex<(*itMaterial).second.size(); iShaderVariantIndex++)
				{
					string strShaderVariantName((*itMaterial).second[iShaderVariantIndex]);	// eg "models/characters/average_sleeves/face"

					TreeItemData.iItemNumber	= iShaderVariantIndex;
					TreeItemData.iItemType		= TREEITEMTYPE_SKINMATERIALSHADER;

					HTREEITEM hTreeItem_ThisShaderVariant = ModelTree_InsertItem(strShaderVariantName.c_str(), hTreeItem_ThisMaterial, TreeItemData.uiData);
				}
			}
*/
		}
	}

	return bReturn;
}
#endif


// sets up valid skin tables based on first entries loaded, also registers/binds appropriate textures...
//
void OldSkins_ApplyDefault(ModelContainer_t *pContainer)
{
	std::string strCurrentSkin;

	// look for one called "default" first...
	//
	OldSkinSets_t::iterator itSkins = pContainer->OldSkinSets.find("default");
	if (itSkins != pContainer->OldSkinSets.end())
	{
		strCurrentSkin = itSkins->first;
	}
	else
	{
		// just use the first one we have...
		//
        strCurrentSkin = pContainer->OldSkinSets.begin()->first;
	}

	// apply it, but don't barf if there wasn't one...
	//
	if (!strCurrentSkin.empty())
	{
		OldSkins_Apply(pContainer, strCurrentSkin.c_str());
	}
}


GLuint OldSkins_GetGLBind(ModelContainer_t *pContainer, const char * psSurfaceName)
{
	return pContainer->MaterialBinds[psSurfaceName];
}

extern bool g_bReportImageLoadErrors;
bool OldSkins_Validate( ModelContainer_t *pContainer, int iSkinNumber )
{
	bool bReturn = true;	
	bool bPREV_bReportImageLoadErrors = g_bReportImageLoadErrors;
										g_bReportImageLoadErrors = false;
	std::size_t iSurface_Other = 0;

	// build up a list of shaders used...
	//	
	StringSet_t UniqueSkinShaders;	
	//SkinFileMaterialsMissing_t SkinFileMaterialsMissing;
	int iThisSkinIndex = 0;

	std::string strSkinFileSurfaceDiscrepancies;
	
	for (OldSkinSets_t::iterator itOldSkins = pContainer->OldSkinSets.begin(); itOldSkins != pContainer->OldSkinSets.end(); ++itOldSkins, iThisSkinIndex++)
	{					
		std::string strSkinName				= itOldSkins->first;
		StringPairVector_t &StringPairs = itOldSkins->second;

		if (iSkinNumber == iThisSkinIndex || iSkinNumber == -1)
		{
			for (std::size_t iSurface = 0; iSurface < StringPairs.size(); iSurface++)
			{
				std::string strSurface(StringPairs[iSurface].first);
				std::string strTGAName(StringPairs[iSurface].second);

				UniqueSkinShaders.insert(UniqueSkinShaders.end(),strTGAName);

				if (iSkinNumber == -1)
				{
					// compare the current material against every other skin file, and report any that don't contain it...
					//					
					for (OldSkinSets_t::iterator itOldSkins_Other = pContainer->OldSkinSets.begin(); itOldSkins_Other != pContainer->OldSkinSets.end(); ++itOldSkins_Other)
					{
						std::string strSkinName_Other				= itOldSkins_Other->first;
						StringPairVector_t &StringPairs_Other	= itOldSkins_Other->second;

						for (iSurface_Other = 0; iSurface_Other < StringPairs_Other.size(); iSurface_Other++)
						{
							std::string strSurface_Other(StringPairs_Other[iSurface_Other].first);							

							if (strSurface_Other == strSurface)
							{
								break;
							}
						}
                        
						if (iSurface_Other == StringPairs_Other.size())
						{
							// surface not found in this file...
							//
							strSkinFileSurfaceDiscrepancies += va("Surface \"%s\" ( skin \"%s\" ) had no entry in skin \"%s\"\n",strSurface.c_str(),strSkinName.c_str(),strSkinName_Other.c_str());
						}
					}
				}

			}			
		}
	}

	// now process the unique list we've just built...
	//
	std::string strFoundList;
	std::string strNotFoundList;
	int iUniqueIndex = 0;
	for (StringSet_t::iterator it = UniqueSkinShaders.begin(); it != UniqueSkinShaders.end(); ++it, iUniqueIndex++)
	{			
		std::string strShader(*it);

		StatusMessage(va("Processing shader %d/%d: \"%s\"\n",iUniqueIndex,UniqueSkinShaders.size(),strShader.c_str()));

		OutputDebugString(va("Unique: \"%s\"... ",strShader.c_str()));

		int iTextureHandle = Texture_Load(strShader.c_str(), true);	// bInhibitStatus

		GLuint uiGLBind = Texture_GetGLBind( iTextureHandle );

		if (uiGLBind == 0)
		{
			OutputDebugString("NOT FOUND\n");
			
			strNotFoundList += strShader;
			strNotFoundList += "\n";
		}
		else
		{
			OutputDebugString("found\n");

			strFoundList += strShader;
			strFoundList += "\n";
		}
	}

	StatusMessage(NULL);

	
	// Now output results...

	// If too many lines to fit on screen (which is now happening), send 'em to notepad instead...
	//
	// ( tacky way of counting lines...)
	std::string strTackyCount(strNotFoundList.c_str());
	strTackyCount += strFoundList.c_str();			

	int iLines = std::count_if (strTackyCount.begin(), strTackyCount.end(), std::bind1st (std::equal_to<char>(), '\n'));	// :-)

	#define MAX_BOX_LINES_HERE 50

	// new popup before the other ones...
	//
	if (!strSkinFileSurfaceDiscrepancies.empty())
	{
		strSkinFileSurfaceDiscrepancies.insert (0,va("( \"%s\" )\n\nThe following skin file errors occured during cross-checking...\n\n",pContainer->sLocalPathName));

		if (GetYesNo(va("%s\n\nSend copy of report to Notepad?", strSkinFileSurfaceDiscrepancies.c_str())))
		{
			SendStringToNotepad( strSkinFileSurfaceDiscrepancies.c_str(), "skinfile_discrepancies.txt");
		}
	}

	if (strNotFoundList.empty())
	{
		if (iLines > MAX_BOX_LINES_HERE)
		{
			if (GetYesNo(va("All shaders found...    :-)\n\nList has > %d entries, send to Notepad?",MAX_BOX_LINES_HERE)))
			{
				SendStringToNotepad(va("All shaders found...    :-)\n\nList follows:\n\n%s",strFoundList.c_str()),"found_shaders.txt");
			}
		}
		else
		{
			InfoBox(va("All shaders found...    :-)\n\nList follows:\n\n%s",strFoundList.c_str()));
		}
	}
	else
	{
		if (iLines > MAX_BOX_LINES_HERE)
		{
			if (GetYesNo(va("Some missing shader, some found, but list is > %d entries, send to Notepad?",MAX_BOX_LINES_HERE)))
			{
				SendStringToNotepad(va("Missing shaders:\n\n%s\n\nFound shaders:\n\n%s",strNotFoundList.c_str(),strFoundList.c_str()),"found_shaders.txt");
			}
		}
		else
		{
			WarningBox(va("Missing shaders:\n\n%s\n\nFound shaders:\n\n%s",strNotFoundList.c_str(),strFoundList.c_str()));
		}
		bReturn = false;
	}


	g_bReportImageLoadErrors = bPREV_bReportImageLoadErrors;
	return bReturn;
}

////////////////// eof /////////////////


