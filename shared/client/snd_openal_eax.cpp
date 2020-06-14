/*
===========================================================================
Copyright (C) 2020, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
#include "snd_openal_eax.h"

#include <OpenAL/al.h>
#include <Windows.h>
#include "eax/EaxMan.h"
#include "eax/eax.h"

#include "snd_local.h"

//////////////////////////////////////////////////////////////////////////////
//
//  Creative EAX support
//
//////////////////////////////////////////////////////////////////////////////
#define ENV_UPDATE_RATE 100 // Environmental audio update rate (in ms)

// Displays the closest env. zones (including the one the listener is in)
//#define DISPLAY_CLOSEST_ENVS

typedef struct ENVTABLE_s {
	ALuint ulNumApertures;
	ALint lFXSlotID;
	ALboolean bUsed;
	struct
	{
		ALfloat vPos1[3];
		ALfloat vPos2[3];
		ALfloat vCenter[3];
	} Aperture[64];
} ENVTABLE, *LPENVTABLE;

typedef struct REVERBDATA_s {
	long lEnvID;
	long lApertureNum;
	float flDist;
} REVERBDATA, *LPREVERBDATA;

typedef struct FXSLOTINFO_s {
	GUID FXSlotGuid;
	ALint lEnvID;
} FXSLOTINFO, *LPFXSLOTINFO;

LPEAXMANAGER s_lpEAXManager;		  // Pointer to EAXManager object
HINSTANCE s_hEAXManInst;		  // Handle of EAXManager DLL
EAXSet s_eaxSet;			  // EAXSet() function
EAXGet s_eaxGet;			  // EAXGet() function
EAXREVERBPROPERTIES s_eaxLPCur;		  // Current EAX Parameters
LPENVTABLE s_lpEnvTable = NULL;		  // Stores information about each environment zone
long s_lLastEnvUpdate;			  // Time of last EAX update
long s_lNumEnvironments;		  // Number of environment zones
long s_NumFXSlots;			  // Number of EAX 4.0 FX Slots
FXSLOTINFO s_FXSlotInfo[EAX_MAX_FXSLOTS]; // Stores information about the EAX 4.0 FX Slots

bool s_bEAX;      // Is EAX 4.0 support available
bool s_bInWater;	      // Underwater effect currently active
int s_EnvironmentID;   // EAGLE ID of current environment
bool s_bEALFileLoaded; // Has an .eal file been loaded for the current level

static bool LoadEALFile(char *szEALFilename);
static void UnloadEALFile();
static float CalcDistance(EMPOINT A, EMPOINT B);

static void Normalize(EAXVECTOR *v) {
	float flMagnitude;

	flMagnitude = (float)sqrt(v->x * v->x + v->y * v->y + v->z * v->z);

	v->x = v->x / flMagnitude;
	v->y = v->y / flMagnitude;
	v->z = v->z / flMagnitude;
}

// EAX 4.0 GUIDS ... confidential information ...

const GUID EAXPROPERTYID_EAX40_FXSlot0 = {0xc4d79f1e, 0xf1ac, 0x436b, {0xa8, 0x1d, 0xa7, 0x38, 0xe7, 0x4, 0x54, 0x69}};

const GUID EAXPROPERTYID_EAX40_FXSlot1 = {0x8c00e96, 0x74be, 0x4491, {0x93, 0xaa, 0xe8, 0xad, 0x35, 0xa4, 0x91, 0x17}};

const GUID EAXPROPERTYID_EAX40_FXSlot2 = {0x1d433b88, 0xf0f6, 0x4637, {0x91, 0x9f, 0x60, 0xe7, 0xe0, 0x6b, 0x5e, 0xdd}};

const GUID EAXPROPERTYID_EAX40_FXSlot3 = {0xefff08ea, 0xc7d8, 0x44ab, {0x93, 0xad, 0x6d, 0xbd, 0x5f, 0x91, 0x0, 0x64}};

const GUID EAXPROPERTYID_EAX40_Context = {0x1d4870ad, 0xdef, 0x43c0, {0xa4, 0xc, 0x52, 0x36, 0x32, 0x29, 0x63, 0x42}};

const GUID EAXPROPERTYID_EAX40_Source = {0x1b86b823, 0x22df, 0x4eae, {0x8b, 0x3c, 0x12, 0x78, 0xce, 0x54, 0x42, 0x27}};

const GUID EAX_NULL_GUID = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

const GUID EAX_PrimaryFXSlotID = {0xf317866d, 0x924c, 0x450c, {0x86, 0x1b, 0xe6, 0xda, 0xa2, 0x5e, 0x7c, 0x20}};

const GUID EAX_REVERB_EFFECT = {0xcf95c8f, 0xa3cc, 0x4849, {0xb0, 0xb6, 0x83, 0x2e, 0xcc, 0x18, 0x22, 0xdf}};

void S_ALEAX_SoundInfo_f()
{
	Com_Printf("EAX 4.0 %s supported\n", s_bEAX ? "is" : "not");
	Com_Printf("Eal file %s loaded\n", s_bEALFileLoaded ? "is" : "not");
	Com_Printf("s_EnvironmentID = %d\n", s_EnvironmentID);
	Com_Printf("s_bInWater = %s\n", s_bInWater ? "true" : "false");
}

void EALFileInit(const char *level) {
	// If an EAL File is already unloaded, remove it
	if (s_bEALFileLoaded) {
		UnloadEALFile();
	}

	// Reset variables
	s_bInWater = false;

	// Try and load an EAL file for the new level
	char name[MAX_QPATH];
	char szEALFilename[MAX_QPATH];
	COM_StripExtension(level, name, sizeof(name));
	Com_sprintf(szEALFilename, MAX_QPATH, "eagle/%s.eal", name);

	s_bEALFileLoaded = LoadEALFile(szEALFilename);

	if (!s_bEALFileLoaded) {
		Com_sprintf(szEALFilename, MAX_QPATH, "base/eagle/%s.eal", name);
		s_bEALFileLoaded = LoadEALFile(szEALFilename);
	}

	if (s_bEALFileLoaded) {
		s_lLastEnvUpdate = timeGetTime();
	} else {
		// Mute reverbs if no EAL file is found
		if ((s_bEAX) && (s_eaxSet)) {
			long lRoom = -10000;
			for (int i = 0; i < s_NumFXSlots; i++) {
				s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_ROOM, NULL, &lRoom, sizeof(long));
			}
		}
	}
}

void InitEAXManager() {
	LPEAXMANAGERCREATE lpEAXManagerCreateFn;
	EAXFXSLOTPROPERTIES FXSlotProp;
	GUID Effect;
	GUID FXSlotGuids[4];
	int i;

	s_bEALFileLoaded = false;

	// Check for EAX 4.0 support
	s_bEAX = alIsExtensionPresent("EAX4.0") != 0;

	if (s_bEAX) {
		Com_Printf("Found EAX 4.0 native support\n");
	} else {
		// Support for EAXUnified (automatic translation of EAX 4.0 calls into EAX 3.0)
		if ((alIsExtensionPresent("EAX3.0")) && (alIsExtensionPresent("EAX4.0Emulated"))) {
			s_bEAX = AL_TRUE;
			Com_Printf("Found EAX 4.0 EMULATION support\n");
		}
	}

	if (s_bEAX) {
		s_eaxSet = (EAXSet)alGetProcAddress("EAXSet");
		if (s_eaxSet == NULL)
			s_bEAX = false;
		s_eaxGet = (EAXGet)alGetProcAddress("EAXGet");
		if (s_eaxGet == NULL)
			s_bEAX = false;
	}

	// If we have detected EAX support, then try and load the EAX Manager DLL
	if (s_bEAX) {
		s_hEAXManInst = LoadLibrary("EAXMan.dll");
		if (s_hEAXManInst) {
			lpEAXManagerCreateFn = (LPEAXMANAGERCREATE)GetProcAddress(s_hEAXManInst, "EaxManagerCreate");
			if (lpEAXManagerCreateFn) {
				if (lpEAXManagerCreateFn(&s_lpEAXManager) == EM_OK) {
					// Configure our EAX 4.0 Effect Slots

					s_NumFXSlots = 0;
					for (i = 0; i < EAX_MAX_FXSLOTS; i++) {
						s_FXSlotInfo[i].FXSlotGuid = EAX_NULL_GUID;
						s_FXSlotInfo[i].lEnvID = -1;
					}

					FXSlotGuids[0] = EAXPROPERTYID_EAX40_FXSlot0;
					FXSlotGuids[1] = EAXPROPERTYID_EAX40_FXSlot1;
					FXSlotGuids[2] = EAXPROPERTYID_EAX40_FXSlot2;
					FXSlotGuids[3] = EAXPROPERTYID_EAX40_FXSlot3;

					// For each effect slot, try and load a reverb and lock the slot
					FXSlotProp.guidLoadEffect = EAX_REVERB_EFFECT;
					FXSlotProp.lVolume = 0;
					FXSlotProp.lLock = EAXFXSLOT_LOCKED;
					FXSlotProp.ulFlags = EAXFXSLOTFLAGS_ENVIRONMENT;

					for (i = 0; i < EAX_MAX_FXSLOTS; i++) {
						if (s_eaxSet(&FXSlotGuids[i], EAXFXSLOT_ALLPARAMETERS, NULL, &FXSlotProp, sizeof(EAXFXSLOTPROPERTIES)) == AL_NO_ERROR) {
							// We can use this slot
							s_FXSlotInfo[s_NumFXSlots].FXSlotGuid = FXSlotGuids[i];
							s_NumFXSlots++;
						} else {
							// If this slot already contains a reverb, then we will use it anyway (Slot 0 will
							// be in this category).  (It probably means that Slot 0 is locked)
							if (s_eaxGet(&FXSlotGuids[i], EAXFXSLOT_LOADEFFECT, NULL, &Effect, sizeof(GUID)) == AL_NO_ERROR) {
								if (Effect == EAX_REVERB_EFFECT) {
									// We can use this slot
									// Make sure the environment flag is on
									s_eaxSet(&FXSlotGuids[i], EAXFXSLOT_FLAGS, NULL, &FXSlotProp.ulFlags, sizeof(unsigned long));
									s_FXSlotInfo[s_NumFXSlots].FXSlotGuid = FXSlotGuids[i];
									s_NumFXSlots++;
								}
							}
						}
					}

					return;
				}
			}
		}
	}

	// If the EAXManager library was loaded (and there was a problem), then unload it
	if (s_hEAXManInst) {
		FreeLibrary(s_hEAXManInst);
		s_hEAXManInst = NULL;
	}

	s_lpEAXManager = NULL;
	s_bEAX = false;

	return;
}

void ReleaseEAXManager() {
	s_bEAX = false;

	UnloadEALFile();

	if (s_lpEAXManager) {
		s_lpEAXManager->Release();
		s_lpEAXManager = NULL;
	}
	if (s_hEAXManInst) {
		FreeLibrary(s_hEAXManInst);
		s_hEAXManInst = NULL;
	}
}

static bool LoadEALFile(char *szEALFilename) {
	char *ealData = NULL;
	HRESULT hr;
	long i, j, lID, lEnvID;
	EMPOINT EMPoint;
	char szAperture[128];
	char szFullEALFilename[MAX_QPATH];
	long lNumInst, lNumInstA, lNumInstB;
	bool bLoaded = false;
	bool bValid = true;
	int result;
	char szString[256];

	if ((!s_lpEAXManager) || (!s_bEAX))
		return false;

	if (strstr(szEALFilename, "nomap"))
		return false;

	s_EnvironmentID = 0xFFFFFFFF;

	// Assume there is no aperture information in the .eal file
	s_lpEnvTable = NULL;

	// Load EAL file from PAK file
	result = FS_ReadFile(szEALFilename, (void **)&ealData);

	if ((ealData) && (result != -1)) {
		hr = s_lpEAXManager->LoadDataSet(ealData, EMFLAG_LOADFROMMEMORY);

		// Unload EAL file
		FS_FreeFile(ealData);

		if (hr == EM_OK) {
			Com_DPrintf("Loaded %s by Quake loader\n", szEALFilename);
			bLoaded = true;
		}
	} else {
		// Failed to load via Quake loader, try manually
		Com_sprintf(szFullEALFilename, MAX_QPATH, "base/%s", szEALFilename);
		if (SUCCEEDED(s_lpEAXManager->LoadDataSet(szFullEALFilename, 0))) {
			Com_DPrintf("Loaded %s by EAXManager\n", szEALFilename);
			bLoaded = true;
		}
	}

	if (bLoaded) {
		// For a valid eal file ... need to find 'Center' tag, record num of instances,  and then find
		// the right number of instances of 'Aperture0a' and 'Aperture0b'.

		if (s_lpEAXManager->GetSourceID("Center", &lID) == EM_OK) {
			if (s_lpEAXManager->GetSourceNumInstances(lID, &s_lNumEnvironments) == EM_OK) {
				if (s_lpEAXManager->GetSourceID("Aperture0a", &lID) == EM_OK) {
					if (s_lpEAXManager->GetSourceNumInstances(lID, &lNumInst) == EM_OK) {
						if (lNumInst == s_lNumEnvironments) {
							if (s_lpEAXManager->GetSourceID("Aperture0b", &lID) == EM_OK) {
								if (s_lpEAXManager->GetSourceNumInstances(lID, &lNumInst) == EM_OK) {
									if (lNumInst == s_lNumEnvironments) {
										// Check equal numbers of ApertureXa and ApertureXb
										i = 1;
										while (true) {
											lNumInstA = lNumInstB = 0;

											sprintf(szAperture, "Aperture%da", i);
											if ((s_lpEAXManager->GetSourceID(szAperture, &lID) == EM_OK) && (s_lpEAXManager->GetSourceNumInstances(lID, &lNumInstA) == EM_OK)) {
												sprintf(szAperture, "Aperture%db", i);
												s_lpEAXManager->GetSourceID(szAperture, &lID);
												s_lpEAXManager->GetSourceNumInstances(lID, &lNumInstB);

												if (lNumInstA != lNumInstB) {
													Com_DPrintf(S_COLOR_YELLOW "Invalid EAL file - %d Aperture%da tags, and %d Aperture%db tags\n", lNumInstA, i, lNumInstB, i);
													bValid = false;
												}
											} else {
												break;
											}

											i++;
										}

										if (bValid) {
											s_lpEnvTable = (LPENVTABLE)Z_Malloc(s_lNumEnvironments * sizeof(ENVTABLE), TAG_GENERAL, qtrue);
										}
									} else
										Com_DPrintf(S_COLOR_YELLOW "Invalid EAL File - expected %d instances of Aperture0b, found %d\n", s_lNumEnvironments, lNumInst);
								} else
									Com_DPrintf(S_COLOR_YELLOW "EAXManager- failed GetSourceNumInstances()\n");
							} else
								Com_DPrintf(S_COLOR_YELLOW "Invalid EAL File - no instances of 'Aperture0b' source-tag\n");
						} else
							Com_DPrintf(S_COLOR_YELLOW "Invalid EAL File - found %d instances of the 'Center' tag, but only %d instances of 'Aperture0a'\n", s_lNumEnvironments, lNumInst);
					} else
						Com_DPrintf(S_COLOR_YELLOW "EAXManager- failed GetSourceNumInstances()\n");
				} else
					Com_DPrintf(S_COLOR_YELLOW "Invalid EAL File - no instances of 'Aperture0a' source-tag\n");
			} else
				Com_DPrintf(S_COLOR_YELLOW "EAXManager- failed GetSourceNumInstances()\n");
		} else
			Com_DPrintf(S_COLOR_YELLOW "Invalid EAL File - no instances of 'Center' source-tag\n");

		if (s_lpEnvTable) {
			i = 0;
			while (true) {
				sprintf(szAperture, "Aperture%da", i);
				if (s_lpEAXManager->GetSourceID(szAperture, &lID) == EM_OK) {
					if (s_lpEAXManager->GetSourceNumInstances(lID, &lNumInst) == EM_OK) {
						for (j = 0; j < s_lNumEnvironments; j++) {
							s_lpEnvTable[j].bUsed = false;
						}

						for (j = 0; j < lNumInst; j++) {
							if (s_lpEAXManager->GetSourceInstancePos(lID, j, &EMPoint) == EM_OK) {
								if (s_lpEAXManager->GetListenerDynamicAttributes(0, &EMPoint, &lEnvID, 0) == EM_OK) {
									if ((lEnvID >= 0) && (lEnvID < s_lNumEnvironments)) {
										if (!s_lpEnvTable[lEnvID].bUsed) {
											s_lpEnvTable[lEnvID].bUsed = true;
											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[0] = EMPoint.fX;
											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[1] = EMPoint.fY;
											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[2] = EMPoint.fZ;
										} else {
											s_lpEAXManager->GetEnvironmentName(lEnvID, szString, 256);
											Com_DPrintf(S_COLOR_YELLOW "Found more than one occurance of Aperture%da in %s sub-space\n", i, szString);
											Com_DPrintf(S_COLOR_YELLOW "One tag at %.3f,%.3f,%.3f, other at %.3f,%.3f,%.3f\n", EMPoint.fX, EMPoint.fY, EMPoint.fZ, s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[0], s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[1], s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[2]);
											bValid = false;
										}
									} else {
										if (lEnvID == -1)
											Com_DPrintf(S_COLOR_YELLOW "%s (%.3f,%.3f,%.3f) in Default Environment - please remove\n", szAperture, EMPoint.fX, EMPoint.fY, EMPoint.fZ);
										else
											Com_DPrintf(S_COLOR_YELLOW "Detected more reverb presets than zones - please delete unused presets\n");
										bValid = false;
									}
								}
							}
						}
					}
				} else {
					break;
				}

				if (bValid) {
					sprintf(szAperture, "Aperture%db", i);
					if (s_lpEAXManager->GetSourceID(szAperture, &lID) == EM_OK) {
						if (s_lpEAXManager->GetSourceNumInstances(lID, &lNumInst) == EM_OK) {
							for (j = 0; j < s_lNumEnvironments; j++) {
								s_lpEnvTable[j].bUsed = false;
							}

							for (j = 0; j < lNumInst; j++) {
								if (s_lpEAXManager->GetSourceInstancePos(lID, j, &EMPoint) == EM_OK) {
									if (s_lpEAXManager->GetListenerDynamicAttributes(0, &EMPoint, &lEnvID, 0) == EM_OK) {
										if ((lEnvID >= 0) && (lEnvID < s_lNumEnvironments)) {
											if (!s_lpEnvTable[lEnvID].bUsed) {
												s_lpEnvTable[lEnvID].bUsed = true;
												s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[0] = EMPoint.fX;
												s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[1] = EMPoint.fY;
												s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[2] = EMPoint.fZ;
											} else {
												s_lpEAXManager->GetEnvironmentName(lEnvID, szString, 256);
												Com_DPrintf(S_COLOR_YELLOW "Found more than one occurance of Aperture%db in %s sub-space\n", i, szString);
												bValid = false;
											}

											// Calculate center position of aperture (average of 2 points)

											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vCenter[0] =
												(s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[0] +
												 s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[0]) /
												2;

											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vCenter[1] =
												(s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[1] +
												 s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[1]) /
												2;

											s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vCenter[2] =
												(s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos1[2] +
												 s_lpEnvTable[lEnvID].Aperture[s_lpEnvTable[lEnvID].ulNumApertures].vPos2[2]) /
												2;

											s_lpEnvTable[lEnvID].ulNumApertures++;
										} else {
											if (lEnvID == -1)
												Com_DPrintf(S_COLOR_YELLOW "%s (%.3f,%.3f,%.3f) in Default Environment - please remove\n", szAperture, EMPoint.fX, EMPoint.fY, EMPoint.fZ);
											else
												Com_DPrintf(S_COLOR_YELLOW "Detected more reverb presets than zones - please delete unused presets\n");
											bValid = false;
										}
									}
								}
							}
						}
					}
				}

				if (!bValid) {
					// Found a problem
					Com_DPrintf(S_COLOR_YELLOW "EAX legacy behaviour invoked (one reverb)\n");

					Z_Free(s_lpEnvTable);
					s_lpEnvTable = NULL;
					break;
				}

				i++;
			}
		} else {
			Com_DPrintf(S_COLOR_YELLOW "EAX legacy behaviour invoked (one reverb)\n");
		}

		return true;
	}

	Com_DPrintf(S_COLOR_YELLOW "Failed to load %s\n", szEALFilename);
	return false;
}

static void UnloadEALFile() {
	HRESULT hr;

	if ((!s_lpEAXManager) || (!s_bEAX))
		return;

	hr = s_lpEAXManager->FreeDataSet(0);
	s_bEALFileLoaded = false;

	if (s_lpEnvTable) {
		Z_Free(s_lpEnvTable);
		s_lpEnvTable = NULL;
	}

	return;
}

/*
	Updates the current EAX Reverb setting, based on the location of the listener
*/
void UpdateEAXListener() {
	EMPOINT ListPos, ListOri;
	EMPOINT EMAperture;
	EMPOINT EMSourcePoint;
	long lID, lSourceID, lApertureNum;
	int i, j, k;
	float flDistance, flNearest;
	EAXREVERBPROPERTIES Reverb;
	bool bFound;
	long lVolume;
	long lCurTime;
	channel_t *ch;
	EAXVECTOR LR, LP1, LP2, Pan;
	REVERBDATA ReverbData[3]; // Hardcoded to three (maximum no of reverbs)
#ifdef DISPLAY_CLOSEST_ENVS
	char szEnvName[256];
#endif

	if ((!s_lpEAXManager) || (!s_bEAX))
		return;

	lCurTime = timeGetTime();

	if ((s_lLastEnvUpdate + ENV_UPDATE_RATE) < lCurTime) {
		// Update closest reverbs
		s_lLastEnvUpdate = lCurTime;

		// No panning information in .eal file, or we only have 1 FX Slot to use, revert to legacy
		// behaviour (i.e only one reverb)
		if ((!s_lpEnvTable) || (s_NumFXSlots == 1)) {
			// Convert Listener co-ordinate to left-handed system
			ListPos.fX = listener_pos[0];
			ListPos.fY = listener_pos[1];
			ListPos.fZ = -listener_pos[2];

			if (SUCCEEDED(s_lpEAXManager->GetListenerDynamicAttributes(0, &ListPos, &lID, EMFLAG_LOCKPOSITION))) {
				if (lID != s_EnvironmentID) {
#ifdef DISPLAY_CLOSEST_ENVS
					if (SUCCEEDED(s_lpEAXManager->GetEnvironmentName(lID, szEnvName, 256)))
						Com_Printf("Changing to '%s' zone !\n", szEnvName);
#endif
					// Get EAX Preset info.
					if (SUCCEEDED(s_lpEAXManager->GetEnvironmentAttributes(lID, &s_eaxLPCur))) {
						// Override
						s_eaxLPCur.flAirAbsorptionHF = 0.0f;

						// Set Environment
						s_eaxSet(&EAXPROPERTYID_EAX40_FXSlot0, EAXREVERB_ALLPARAMETERS, NULL, &s_eaxLPCur, sizeof(EAXREVERBPROPERTIES));

						s_EnvironmentID = lID;
					}
				}
			}

			return;
		}

		// Convert Listener position and orientation to left-handed system
		ListPos.fX = listener_pos[0];
		ListPos.fY = listener_pos[1];
		ListPos.fZ = -listener_pos[2];

		ListOri.fX = listener_ori[0];
		ListOri.fY = listener_ori[1];
		ListOri.fZ = -listener_ori[2];

		// Need to find closest s_NumFXSlots (including the Listener's slot)

		if (s_lpEAXManager->GetListenerDynamicAttributes(0, &ListPos, &lID, EMFLAG_LOCKPOSITION) == EM_OK) {
			if (lID == -1) {
				// Found default environment
				//				Com_Printf( S_COLOR_YELLOW "Listener in default environment - ignoring zone !\n");
				return;
			}

			ReverbData[0].lEnvID = -1;
			ReverbData[0].lApertureNum = -1;
			ReverbData[0].flDist = FLT_MAX;

			ReverbData[1].lEnvID = -1;
			ReverbData[1].lApertureNum = -1;
			ReverbData[1].flDist = FLT_MAX;

			ReverbData[2].lEnvID = lID;
			ReverbData[2].lApertureNum = -1;
			ReverbData[2].flDist = 0.0f;

			for (i = 0; i < s_lNumEnvironments; i++) {
				// Ignore Environment id lID as this one will always be used
				if (i != lID) {
					flNearest = FLT_MAX;
					lApertureNum = 0; //shut up compile warning

					for (j = 0; j < s_lpEnvTable[i].ulNumApertures; j++) {
						EMAperture.fX = s_lpEnvTable[i].Aperture[j].vCenter[0];
						EMAperture.fY = s_lpEnvTable[i].Aperture[j].vCenter[1];
						EMAperture.fZ = s_lpEnvTable[i].Aperture[j].vCenter[2];

						flDistance = CalcDistance(EMAperture, ListPos);

						if (flDistance < flNearest) {
							flNearest = flDistance;
							lApertureNum = j;
						}
					}

					// Now have closest point for this Environment - see if this is closer than any others

					if (flNearest < ReverbData[1].flDist) {
						if (flNearest < ReverbData[0].flDist) {
							ReverbData[1] = ReverbData[0];
							ReverbData[0].flDist = flNearest;
							ReverbData[0].lApertureNum = lApertureNum;
							ReverbData[0].lEnvID = i;
						} else {
							ReverbData[1].flDist = flNearest;
							ReverbData[1].lApertureNum = lApertureNum;
							ReverbData[1].lEnvID = i;
						}
					}
				}
			}
		}

#ifdef DISPLAY_CLOSEST_ENVS
		char szEnvName1[256] = {0};
		char szEnvName2[256] = {0};
		char szEnvName3[256] = {0};

		s_lpEAXManager->GetEnvironmentName(ReverbData[0].lEnvID, szEnvName1, 256);
		s_lpEAXManager->GetEnvironmentName(ReverbData[1].lEnvID, szEnvName2, 256);
		s_lpEAXManager->GetEnvironmentName(ReverbData[2].lEnvID, szEnvName3, 256);

		Com_Printf("Closest zones are %s, %s (Listener in %s)\n", szEnvName1, szEnvName2, szEnvName3);
#endif

		// Mute any reverbs no longer required ...

		for (i = 0; i < s_NumFXSlots; i++) {
			if ((s_FXSlotInfo[i].lEnvID != -1) && (s_FXSlotInfo[i].lEnvID != ReverbData[0].lEnvID) && (s_FXSlotInfo[i].lEnvID != ReverbData[1].lEnvID) && (s_FXSlotInfo[i].lEnvID != ReverbData[2].lEnvID)) {
				// This environment is no longer needed

				// Mute it
				lVolume = -10000;
				if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXFXSLOT_VOLUME, NULL, &lVolume, sizeof(long)) != AL_NO_ERROR)
					Com_OPrintf("Failed to Mute FX Slot\n");

				// If any source is sending to this Slot ID then we need to stop them sending to the slot
				for (j = 1; j < s_numChannels; j++) {
					if (s_channels[j].lSlotID == i) {
						if (s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_ACTIVEFXSLOTID, s_channels[j].alSource, (void *)&EAX_NULL_GUID, sizeof(GUID)) != AL_NO_ERROR) {
							Com_OPrintf("Failed to set Source ActiveFXSlotID to NULL\n");
						}

						s_channels[j].lSlotID = -1;
					}
				}

				assert(s_FXSlotInfo[i].lEnvID < s_lNumEnvironments && s_FXSlotInfo[i].lEnvID >= 0);
				if (s_FXSlotInfo[i].lEnvID < s_lNumEnvironments && s_FXSlotInfo[i].lEnvID >= 0) {
					s_lpEnvTable[s_FXSlotInfo[i].lEnvID].lFXSlotID = -1;
				}
				s_FXSlotInfo[i].lEnvID = -1;
			}
		}

		// Make sure all the reverbs we want are being rendered, if not, find an empty slot
		// and apply appropriate reverb settings
		for (j = 0; j < 3; j++) {
			bFound = false;

			for (i = 0; i < s_NumFXSlots; i++) {
				if (s_FXSlotInfo[i].lEnvID == ReverbData[j].lEnvID) {
					bFound = true;
					break;
				}
			}

			if (!bFound) {
				// Find the first available slot and use that one
				for (i = 0; i < s_NumFXSlots; i++) {
					if (s_FXSlotInfo[i].lEnvID == -1) {
						// Found slot

						// load reverb here

						// Retrieve reverb properties from EAX Manager
						if (s_lpEAXManager->GetEnvironmentAttributes(ReverbData[j].lEnvID, &Reverb) == EM_OK) {
							// Override Air Absorption HF
							Reverb.flAirAbsorptionHF = 0.0f;

							s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_ALLPARAMETERS, NULL, &Reverb, sizeof(EAXREVERBPROPERTIES));

							// See if any Sources are in this environment, if they are, enable their sends
							ch = s_channels + 1;
							for (k = 1; k < s_numChannels; k++, ch++) {
								if (ch->fixed_origin) {
									// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
									EMSourcePoint.fX = ch->origin[0];
									EMSourcePoint.fY = ch->origin[2];
									EMSourcePoint.fZ = ch->origin[1];
								} else {
									if (ch->entnum == listener_number) {
										// Source at same position as listener
										// Probably won't be any Occlusion / Obstruction effect -- unless the listener is underwater
										// Converting from Open AL -> DS3D (for EAGLE) ... invert Z
										EMSourcePoint.fX = listener_pos[0];
										EMSourcePoint.fY = listener_pos[1];
										EMSourcePoint.fZ = -listener_pos[2];
									} else {
										// Get position of Entity
										// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
										EMSourcePoint.fX = loopSounds[ch->entnum].origin[0];
										EMSourcePoint.fY = loopSounds[ch->entnum].origin[2];
										EMSourcePoint.fZ = loopSounds[ch->entnum].origin[1];
									}
								}

								// Get Source Environment point
								if (s_lpEAXManager->GetListenerDynamicAttributes(0, &EMSourcePoint, &lSourceID, 0) != EM_OK)
									Com_OPrintf("Failed to get environment zone for Source\n");

								if (lSourceID == i) {
									if (s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_ACTIVEFXSLOTID, ch->alSource, (void *)&(s_FXSlotInfo[i].FXSlotGuid), sizeof(GUID)) != AL_NO_ERROR) {
										Com_OPrintf("Failed to set Source ActiveFXSlotID to new environment\n");
									}

									ch->lSlotID = i;
								}
							}

							assert(ReverbData[j].lEnvID < s_lNumEnvironments && ReverbData[j].lEnvID >= 0);
							if (ReverbData[j].lEnvID < s_lNumEnvironments && ReverbData[j].lEnvID >= 0) {
								s_FXSlotInfo[i].lEnvID = ReverbData[j].lEnvID;
								s_lpEnvTable[ReverbData[j].lEnvID].lFXSlotID = i;
							}
							break;
						}
					}
				}
			}
		}

		// Make sure Primary FX Slot ID is set correctly
		if (s_EnvironmentID != ReverbData[2].lEnvID) {
			s_eaxSet(&EAXPROPERTYID_EAX40_Context, EAXCONTEXT_PRIMARYFXSLOTID, NULL, &(s_FXSlotInfo[s_lpEnvTable[ReverbData[2].lEnvID].lFXSlotID].FXSlotGuid), sizeof(GUID));
			s_EnvironmentID = ReverbData[2].lEnvID;
		}

		// Have right reverbs loaded ... now to pan them and adjust volume

		// We need to rotate the vector from the Listener to the reverb Aperture by minus the listener
		// orientation

		// Need dot product of Listener Orientation and the straight ahead vector (0, 0, 1)

		// Since both vectors are already normalized, and two terms cancel out (0's), the angle
		// is the arc cosine of the z component of the Listener Orientation

		float flTheta = (float)acos(ListOri.fZ);

		// If the Listener Orientation is to the left of straight ahead, then invert the angle
		if (ListOri.fX < 0)
			flTheta = -flTheta;

		float flSin = (float)sin(-flTheta);
		float flCos = (float)cos(-flTheta);

		for (i = 0; i < Q_min(s_NumFXSlots, s_lNumEnvironments); i++) {
			if (s_FXSlotInfo[i].lEnvID == s_EnvironmentID) {
				// Listener's environment

				// Find the closest Aperture in *this* environment

				flNearest = FLT_MAX;
				lApertureNum = 0; //shut up compile warning

				for (j = 0; j < s_lpEnvTable[s_EnvironmentID].ulNumApertures; j++) {
					EMAperture.fX = s_lpEnvTable[s_EnvironmentID].Aperture[j].vCenter[0];
					EMAperture.fY = s_lpEnvTable[s_EnvironmentID].Aperture[j].vCenter[1];
					EMAperture.fZ = s_lpEnvTable[s_EnvironmentID].Aperture[j].vCenter[2];

					flDistance = CalcDistance(EMAperture, ListPos);

					if (flDistance < flNearest) {
						flNearest = flDistance;
						lApertureNum = j;
					}
				}

				// Have closest environment, work out pan vector direction

				LR.x = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vCenter[0] - ListPos.fX;
				LR.y = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vCenter[1] - ListPos.fY;
				LR.z = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vCenter[2] - ListPos.fZ;

				Pan.x = (LR.x * flCos) + (LR.z * flSin);
				Pan.y = 0.0f;
				Pan.z = (LR.x * -flSin) + (LR.z * flCos);

				Normalize(&Pan);

				// Adjust magnitude ...

				// Magnitude is based on the angle subtended by the aperture, so compute the angle between
				// the vector from the Listener to Pos1 of the aperture, and the vector from the
				// Listener to Pos2 of the aperture.

				LP1.x = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos1[0] - ListPos.fX;
				LP1.y = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos1[1] - ListPos.fY;
				LP1.z = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos1[2] - ListPos.fZ;

				LP2.x = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos2[0] - ListPos.fX;
				LP2.y = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos2[1] - ListPos.fY;
				LP2.z = s_lpEnvTable[s_EnvironmentID].Aperture[lApertureNum].vPos2[2] - ListPos.fZ;

				Normalize(&LP1);
				Normalize(&LP2);

				float flGamma = acos((LP1.x * LP2.x) + (LP1.y * LP2.y) + (LP1.z * LP2.z));

				// We want opposite magnitude (because we are 'in' this environment)
				float flMagnitude = 1.0f - ((2.0f * (float)sin(flGamma / 2.0f)) / flGamma);

				// Negative (because pan should be 180 degrees)
				Pan.x *= -flMagnitude;
				Pan.y *= -flMagnitude;
				Pan.z *= -flMagnitude;

				if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_REVERBPAN, NULL, &Pan, sizeof(EAXVECTOR)) != AL_NO_ERROR)
					Com_OPrintf("Failed to set Listener Reverb Pan\n");

				if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_REFLECTIONSPAN, NULL, &Pan, sizeof(EAXVECTOR)) != AL_NO_ERROR)
					Com_OPrintf("Failed to set Listener Reflections Pan\n");
			} else {
				// Find out which Reverb this is
				if (ReverbData[0].lEnvID == s_FXSlotInfo[i].lEnvID)
					k = 0;
				else
					k = 1;

				LR.x = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vCenter[0] - ListPos.fX;
				LR.y = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vCenter[1] - ListPos.fY;
				LR.z = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vCenter[2] - ListPos.fZ;

				// Rotate the vector

				Pan.x = (LR.x * flCos) + (LR.z * flSin);
				Pan.y = 0.0f;
				Pan.z = (LR.x * -flSin) + (LR.z * flCos);

				Normalize(&Pan);

				// Adjust magnitude ...

				// Magnitude is based on the angle subtended by the aperture, so compute the angle between
				// the vector from the Listener to Pos1 of the aperture, and the vector from the
				// Listener to Pos2 of the aperture.

				LP1.x = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos1[0] - ListPos.fX;
				LP1.y = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos1[1] - ListPos.fY;
				LP1.z = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos1[2] - ListPos.fZ;

				LP2.x = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos2[0] - ListPos.fX;
				LP2.y = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos2[1] - ListPos.fY;
				LP2.z = s_lpEnvTable[ReverbData[k].lEnvID].Aperture[ReverbData[k].lApertureNum].vPos2[2] - ListPos.fZ;

				Normalize(&LP1);
				Normalize(&LP2);

				float flGamma = acos((LP1.x * LP2.x) + (LP1.y * LP2.y) + (LP1.z * LP2.z));
				float flMagnitude = (2.0f * (float)sin(flGamma / 2.0f)) / flGamma;

				Pan.x *= flMagnitude;
				Pan.y *= flMagnitude;
				Pan.z *= flMagnitude;

				if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_REVERBPAN, NULL, &Pan, sizeof(EAXVECTOR)) != AL_NO_ERROR)
					Com_OPrintf("Failed to set Reverb Pan\n");

				if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXREVERB_REFLECTIONSPAN, NULL, &Pan, sizeof(EAXVECTOR)) != AL_NO_ERROR)
					Com_OPrintf("Failed to set Reflections Pan\n");
			}
		}

		lVolume = 0;
		for (i = 0; i < s_NumFXSlots; i++) {
			if (s_eaxSet(&s_FXSlotInfo[i].FXSlotGuid, EAXFXSLOT_VOLUME, NULL, &lVolume, sizeof(long)) != AL_NO_ERROR)
				Com_OPrintf("Failed to set FX Slot Volume to 0\n");
		}
	}

	return;
}

/*
	Updates the EAX Buffer related effects on the given Source
*/
void UpdateEAXBuffer(channel_t *ch) {
	HRESULT hr;
	EMPOINT EMSourcePoint;
	EMPOINT EMVirtualSourcePoint;
	EAXOBSTRUCTIONPROPERTIES eaxOBProp;
	EAXOCCLUSIONPROPERTIES eaxOCProp;
	int i;
	long lSourceID;

	// If EAX Manager is not initialized, or there is no EAX support, or the listener
	// is underwater, return
	if ((!s_lpEAXManager) || (!s_bEAX) || (s_bInWater))
		return;

	// Set Occlusion Direct Ratio to the default value (it won't get set by the current version of
	// EAX Manager)
	eaxOCProp.flOcclusionDirectRatio = EAXSOURCE_DEFAULTOCCLUSIONDIRECTRATIO;

	// Convert Source co-ordinate to left-handed system
	if (ch->fixed_origin) {
		// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
		EMSourcePoint.fX = ch->origin[0];
		EMSourcePoint.fY = ch->origin[2];
		EMSourcePoint.fZ = ch->origin[1];
	} else {
		if (ch->entnum == listener_number) {
			// Source at same position as listener
			// Probably won't be any Occlusion / Obstruction effect -- unless the listener is underwater
			// Converting from Open AL -> DS3D (for EAGLE) ... invert Z
			EMSourcePoint.fX = listener_pos[0];
			EMSourcePoint.fY = listener_pos[1];
			EMSourcePoint.fZ = -listener_pos[2];
		} else {
			// Get position of Entity
			// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
			if (ch->bLooping) {
				EMSourcePoint.fX = loopSounds[ch->entnum].origin[0];
				EMSourcePoint.fY = loopSounds[ch->entnum].origin[2];
				EMSourcePoint.fZ = loopSounds[ch->entnum].origin[1];
			} else {
				EMSourcePoint.fX = s_entityPosition[ch->entnum][0];
				EMSourcePoint.fY = s_entityPosition[ch->entnum][2];
				EMSourcePoint.fZ = s_entityPosition[ch->entnum][1];
			}
		}
	}

	long lExclusion;

	// Just determine what environment the source is in
	if (s_lpEAXManager->GetListenerDynamicAttributes(0, &EMSourcePoint, &lSourceID, 0) == EM_OK) {
		// See if a Slot is rendering this environment
		for (i = 0; i < s_NumFXSlots; i++) {
			if (s_FXSlotInfo[i].lEnvID == lSourceID) {
				// If the Source is not sending to this slot, then enable the send now
				if (ch->lSlotID != i) {
					// Set this
					if (s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_ACTIVEFXSLOTID, ch->alSource, &s_FXSlotInfo[i].FXSlotGuid, sizeof(GUID)) != AL_NO_ERROR)
						Com_OPrintf("UpdateEAXBuffer = failed to set ActiveFXSlotID\n");

					ch->lSlotID = i;
				}

				break;
			}
		}
	} else {
		Com_OPrintf("UpdateEAXBuffer::Failed to get Source environment zone\n");
	}

	// Add some Exclusion to sounds that are not located in the Listener's environment
	if (s_FXSlotInfo[ch->lSlotID].lEnvID == s_EnvironmentID) {
		lExclusion = 0;
		if (s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_EXCLUSION, ch->alSource, &lExclusion, sizeof(long)) != AL_NO_ERROR)
			Com_OPrintf("UpdateEAXBuffer : Failed to set exclusion to 0\n");
	} else {
		lExclusion = -1000;
		if (s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_EXCLUSION, ch->alSource, &lExclusion, sizeof(long)) != AL_NO_ERROR)
			Com_OPrintf("UpdateEAXBuffer : Failed to set exclusion to -1000\n");
	}

	if ((ch->entchannel == CHAN_VOICE) || (ch->entchannel == CHAN_VOICE_ATTEN) || (ch->entchannel == CHAN_VOICE_GLOBAL)) {
		// Remove any Occlusion + Obstruction
		eaxOBProp.lObstruction = EAXSOURCE_DEFAULTOBSTRUCTION;
		eaxOBProp.flObstructionLFRatio = EAXSOURCE_DEFAULTOBSTRUCTIONLFRATIO;

		eaxOCProp.lOcclusion = EAXSOURCE_DEFAULTOCCLUSION;
		eaxOCProp.flOcclusionLFRatio = EAXSOURCE_DEFAULTOCCLUSIONLFRATIO;
		eaxOCProp.flOcclusionRoomRatio = EAXSOURCE_DEFAULTOCCLUSIONROOMRATIO;
		eaxOCProp.flOcclusionDirectRatio = EAXSOURCE_DEFAULTOCCLUSIONDIRECTRATIO;

		s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_OBSTRUCTIONPARAMETERS, ch->alSource, &eaxOBProp, sizeof(EAXOBSTRUCTIONPROPERTIES));

		s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_OCCLUSIONPARAMETERS, ch->alSource, &eaxOCProp, sizeof(EAXOCCLUSIONPROPERTIES));
	} else {
		// Check for Occlusion + Obstruction
		hr = s_lpEAXManager->GetSourceDynamicAttributes(0, &EMSourcePoint, &eaxOBProp.lObstruction, &eaxOBProp.flObstructionLFRatio, &eaxOCProp.lOcclusion, &eaxOCProp.flOcclusionLFRatio, &eaxOCProp.flOcclusionRoomRatio, &EMVirtualSourcePoint, 0);
		if (hr == EM_OK) {
			// Set EAX effect !
			s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_OBSTRUCTIONPARAMETERS, ch->alSource, &eaxOBProp, sizeof(EAXOBSTRUCTIONPROPERTIES));

			s_eaxSet(&EAXPROPERTYID_EAX40_Source, EAXSOURCE_OCCLUSIONPARAMETERS, ch->alSource, &eaxOCProp, sizeof(EAXOCCLUSIONPROPERTIES));
		}
	}

	return;
}

static float CalcDistance(EMPOINT A, EMPOINT B) {
	const float dx = A.fX - B.fX;
	const float dy = A.fY - B.fY;
	const float dz = A.fZ - B.fZ;
	return (float)sqrt(dx * dx + dy * dy + dz * dz);
}
#endif