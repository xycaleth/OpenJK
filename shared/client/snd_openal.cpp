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
#include "client/snd_local.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#if defined(USE_EAX)
#include "snd_openal_eax.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  Globals
//
//////////////////////////////////////////////////////////////////////////////
#define DEFAULT_REF_DISTANCE 300.0f	   // Default reference distance
#define DEFAULT_VOICE_REF_DISTANCE 1500.0f // Default voice reference distance

static cvar_t *s_UseOpenAL;

static ALfloat listener_pos[3]; // Listener Position
static ALfloat listener_ori[6]; // Listener Orientation

static short s_rawdata[MAX_RAW_SAMPLES * 2]; // Used for Raw Samples (Music etc...)
static int s_numChannels;		     // Number of AL Sources == Num of Channels

static bool CheckChannelStomp(int chan1, int chan2);
static int MP3PreProcessLipSync(channel_t *ch, short *data);
static void PreProcessLipSync(sfx_t *sfx);
static void SetLipSyncs();
static void UpdateLoopingSounds();
static void UpdateRawSamples();
static void UpdateSingleShotSounds();

//////////////////////////////////////////////////////////////////////////////
//
//  BADBADFIXMEFIXME: Globals from snd_dma.cpp
//
//////////////////////////////////////////////////////////////////////////////
extern int s_soundStarted;
extern int listener_number;

//////////////////////////////////////////////////////////////////////////////
//
//  OpenAL sound mixer API
//
//////////////////////////////////////////////////////////////////////////////
void S_AL_InitCvars() {
	s_UseOpenAL = Cvar_Get("s_UseOpenAL", "0", CVAR_ARCHIVE | CVAR_LATCH, "Use OpenAL sound mixer");
}

qboolean S_AL_Init() {
	const ALCchar *audioDevice = nullptr;
#if defined(_WIN32)
	audioDevice = "DirectSound3D";
#endif

	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
		const ALCchar *defaultDevice =
			alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		Com_Printf("Default audio device: %s\n", defaultDevice);

		const ALCchar *availableDevices =
			alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
		const char *end = nullptr;

		Com_Printf("Available audio devices:\n");
		for (const char *device = availableDevices; *device != '\0';
		     device = end + 1) {
			Com_Printf("...%s\n", device);
			end = strchr(device, '\0');
		}
	}

	ALCdevice *ALCDevice = alcOpenDevice(audioDevice);
	if (!ALCDevice)
		return qfalse;

	ALCcontext *ALCContext = alcCreateContext(ALCDevice, NULL);
	if (!ALCContext)
		return qfalse;

	alcMakeContextCurrent(ALCContext);
	if (alcGetError(ALCDevice) != ALC_NO_ERROR)
		return qfalse;

	// Set Listener attributes
	ALfloat listenerPos[] = {0.0, 0.0, 0.0};
	ALfloat listenerVel[] = {0.0, 0.0, 0.0};
	ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);

#if defined(USE_EAX)
	InitEAXManager();
#endif

	memset(s_channels, 0, sizeof(s_channels));
	s_numChannels = 0;

	// Create as many AL Sources (up to Max) as possible
	for (int i = 0; i < MAX_CHANNELS; i++) {
		channel_t *ch = s_channels + i;
		alGenSources(1, &ch->alSource);
		if (alGetError() != AL_NO_ERROR) {
			// Reached limit of sources
			break;
		}

		alSourcef(
			ch->alSource,
			AL_REFERENCE_DISTANCE,
			DEFAULT_REF_DISTANCE);
		if (alGetError() != AL_NO_ERROR) {
			break;
		}

#if defined(USE_EAX)
		// Sources / Channels are not sending to any Slots (other than the
		// Listener / Primary FX Slot)
		ch->lSlotID = -1;

		if (s_bEAX) {
			// Remove the RoomAuto flag from each Source (to remove Reverb
			// Engine Statistical model that is assuming units are in
			// metres) Without this call reverb sends from the sources will
			// attenuate too quickly with distance, especially for the
			// non-primary reverb zones.
			unsigned long ulFlags = 0;

			s_eaxSet(
				&EAXPROPERTYID_EAX40_Source,
				EAXSOURCE_FLAGS,
				ch->alSource,
				&ulFlags,
				sizeof(ulFlags));
		}
#endif

		s_numChannels++;
	}

	// Generate AL Buffers for streaming audio playback (used for MP3s)
	channel_t *ch = s_channels + 1;
	for (int i = 1; i < s_numChannels; i++, ch++) {
		for (int j = 0; j < NUM_STREAMING_BUFFERS; j++) {
			alGenBuffers(1, &(ch->buffers[j].BufferID));
			ch->buffers[j].Status = UNQUEUED;
			ch->buffers[j].Data =
				(char *)Z_Malloc(STREAMING_BUFFER_SIZE, TAG_SND_RAWDATA, qfalse);
		}
	}

	// These aren't really relevant for Open AL, but for completeness ...
	dma.speed = 22050;
	dma.channels = 2;
	dma.samplebits = 16;
	dma.samples = 0;
	dma.submission_chunk = 0;
	dma.buffer = NULL;

#if defined(USE_EAX)
	// s_init could be called in game, if so there may be an .eal file
	// for this level
	const char *mapname = Cvar_VariableString("mapname");
	EALFileInit(mapname);
#endif

	return qtrue;
}

void S_AL_OnLoadSound(sfx_t *sfx) {
	if (!S_AL_IsEnabled()) {
		return;
	}
	if ((strstr(sfx->sSoundName, "chars")) ||
	    (strstr(sfx->sSoundName, "CHARS"))) {
		sfx->lipSyncData = (char *)Z_Malloc(
			(sfx->iSoundLengthInSamples / 1000) + 1, TAG_SND_RAWDATA, qfalse);
		PreProcessLipSync(sfx);
	} else
		sfx->lipSyncData = NULL;

	// Clear Open AL Error State
	alGetError();

	// Generate AL Buffer
	ALuint Buffer;
	alGenBuffers(1, &Buffer);
	if (alGetError() == AL_NO_ERROR) {
		// Copy audio data to AL Buffer
		alBufferData(
			Buffer,
			AL_FORMAT_MONO16,
			sfx->pSoundData,
			sfx->iSoundLengthInSamples * 2,
			22050);
		if (alGetError() == AL_NO_ERROR) {
			// Store AL Buffer in sfx struct, and release sample data
			sfx->Buffer = Buffer;
			Z_Free(sfx->pSoundData);
			sfx->pSoundData = NULL;
		}
	}
}

void S_AL_OnRegistration() {
#if defined(USE_EAX)
	if (!S_AL_IsEnabled()) {
		return;
	}

	// Find name of level so we can load in the appropriate EAL file
	const char *mapname = Cvar_VariableString("mapname");
	EALFileInit(mapname);
	// clear carry crap from previous map
	for (int i = 0; i < EAX_MAX_FXSLOTS; i++) {
		s_FXSlotInfo[i].lEnvID = -1;
	}
#endif
}

void S_AL_Shutdown() {
	int i, j;
	// Release all the AL Sources (including Music channel (Source 0))
	for (i = 0; i < s_numChannels; i++) {
		alDeleteSources(1, &s_channels[i].alSource);
	}

	// Release Streaming AL Buffers
	channel_t *ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		for (j = 0; j < NUM_STREAMING_BUFFERS; j++) {
			alDeleteBuffers(1, &(ch->buffers[j].BufferID));
			ch->buffers[j].BufferID = 0;
			ch->buffers[j].Status = UNQUEUED;
			if (ch->buffers[j].Data) {
				Z_Free(ch->buffers[j].Data);
				ch->buffers[j].Data = NULL;
			}
		}
	}

	ALCcontext *ALCContext = alcGetCurrentContext();
	ALCdevice *ALCDevice = alcGetContextsDevice(ALCContext);
	alcDestroyContext(ALCContext);
	alcCloseDevice(ALCDevice);

#if defined(USE_EAX)
	ReleaseEAXManager();
#endif

	s_numChannels = 0;
}

bool S_AL_IsEnabled() {
	return s_UseOpenAL->integer != 0;
}

void S_AL_SoundInfo_f() {
#if defined(USE_EAX)
    S_ALEAX_SoundInfo_f();
#endif
}

void S_AL_MuteAllSounds(qboolean bMute) {
	if (!s_soundStarted)
		return;

	if (!S_AL_IsEnabled())
		return;

	alListenerf(AL_GAIN, bMute ? 0.0f : 1.0f);
}

void S_AL_OnStartSound(int entnum, int entchannel) {
	if (!S_AL_IsEnabled()) {
		return;
	}

	channel_t *ch = s_channels + 1;
	if (entchannel == CHAN_WEAPON) {
		// Check if we are playing a 'charging' sound, if so, stop it now ..
		for (int i = 1; i < s_numChannels; i++, ch++) {
			if (ch->entnum == entnum &&
			    ch->entchannel == CHAN_WEAPON &&
			    ch->thesfx != nullptr &&
			    strstr(ch->thesfx->sSoundName, "altcharge") != nullptr) {
				// Stop this sound
				alSourceStop(ch->alSource);
				alSourcei(ch->alSource, AL_BUFFER, 0);
				ch->bPlaying = false;
				ch->thesfx = NULL;
				break;
			}
		}
	} else {
		for (int i = 1; i < s_numChannels; i++, ch++) {
			if (ch->entnum == entnum &&
			    ch->thesfx != nullptr &&
			    strstr(ch->thesfx->sSoundName, "falling") != nullptr) {
				// Stop this sound
				alSourceStop(ch->alSource);
				alSourcei(ch->alSource, AL_BUFFER, 0);
				ch->bPlaying = false;
				ch->thesfx = NULL;
				break;
			}
		}
	}
}

// Allows more than one sound of the same type to emanate from the same entity - sounds much better
// on hardware this way esp. rapid fire modes of weapons!
channel_t *S_AL_PickChannel(int entnum, int entchannel) {
	int ch_idx;
	channel_t *ch, *ch_firstToDie;
	bool foundChan = false;
	float source_pos[3];

	if (entchannel < 0) {
		Com_Error(ERR_DROP, "S_PickChannel: entchannel<0");
	}

	// Check for replacement sound, or find the best one to replace

	ch_firstToDie = s_channels + 1; // channel 0 is reserved for Music

	for (ch_idx = 1, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++) {
		if (ch->entnum == entnum && CheckChannelStomp(ch->entchannel, entchannel)) {
			// always override sound from same entity
			if (s_show->integer == 1 && ch->thesfx) {
				Com_Printf(S_COLOR_YELLOW "...overrides %s\n", ch->thesfx->sSoundName);
				ch->thesfx = 0; //just to clear the next error msg
			}
			ch_firstToDie = ch;
			foundChan = true;
			break;
		}
	}

	if (!foundChan)
		for (ch_idx = 1, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++) {
			// See if the channel is free
			if (!ch->thesfx) {
				ch_firstToDie = ch;
				foundChan = true;
				break;
			}
		}

	if (!foundChan) {
		for (ch_idx = 1, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++) {
			if ((ch->entnum == entnum) && (ch->entchannel == entchannel) && (ch->entchannel != CHAN_AMBIENT) && (ch->entnum != listener_number)) {
				// Same entity and same type of sound effect (entchannel)
				ch_firstToDie = ch;
				foundChan = true;
				break;
			}
		}
	}

	int longestDist;
	int dist;

	if (!foundChan) {
		// Find sound effect furthest from listener
		ch = s_channels + 1;

		if (ch->fixed_origin) {
			// Convert to Open AL co-ordinates
			source_pos[0] = ch->origin[0];
			source_pos[1] = ch->origin[2];
			source_pos[2] = -ch->origin[1];

			longestDist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
				      ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
				      ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
		} else {
			if (ch->entnum == listener_number)
				longestDist = 0;
			else {
				if (ch->bLooping) {
					// Convert to Open AL co-ordinates
					source_pos[0] = loopSounds[ch->entnum].origin[0];
					source_pos[1] = loopSounds[ch->entnum].origin[2];
					source_pos[2] = -loopSounds[ch->entnum].origin[1];
				} else {
					// Convert to Open AL co-ordinates
					source_pos[0] = s_entityPosition[ch->entnum][0];
					source_pos[1] = s_entityPosition[ch->entnum][2];
					source_pos[2] = -s_entityPosition[ch->entnum][1];
				}

				longestDist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
					      ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
					      ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
			}
		}

		for (ch_idx = 2, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++) {
			if (ch->fixed_origin) {
				// Convert to Open AL co-ordinates
				source_pos[0] = ch->origin[0];
				source_pos[1] = ch->origin[2];
				source_pos[2] = -ch->origin[1];

				dist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
				       ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
				       ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
			} else {
				if (ch->entnum == listener_number)
					dist = 0;
				else {
					if (ch->bLooping) {
						// Convert to Open AL co-ordinates
						source_pos[0] = loopSounds[ch->entnum].origin[0];
						source_pos[1] = loopSounds[ch->entnum].origin[2];
						source_pos[2] = -loopSounds[ch->entnum].origin[1];
					} else {
						// Convert to Open AL co-ordinates
						source_pos[0] = s_entityPosition[ch->entnum][0];
						source_pos[1] = s_entityPosition[ch->entnum][2];
						source_pos[2] = -s_entityPosition[ch->entnum][1];
					}

					dist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
					       ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
					       ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
				}
			}

			if (dist > longestDist) {
				longestDist = dist;
				ch_firstToDie = ch;
			}
		}
	}

	if (ch_firstToDie->bPlaying) {
		if (s_show->integer == 1 && ch_firstToDie->thesfx) {
			Com_Printf(S_COLOR_RED "***kicking %s\n", ch_firstToDie->thesfx->sSoundName);
		}

		// Stop sound
		alSourceStop(ch_firstToDie->alSource);
		ch_firstToDie->bPlaying = false;
	}

	// Reset channel variables
	memset(&ch_firstToDie->MP3StreamHeader, 0, sizeof(MP3STREAM));
	ch_firstToDie->bLooping = false;
	ch_firstToDie->bProcessed = false;
	ch_firstToDie->bStreaming = false;

	return ch_firstToDie;
}

void S_AL_ClearSoundBuffer() {
	s_paintedtime = 0;
	s_soundtime = 0;
}

void S_AL_ClearChannel(channel_t *channel) {
	if (!S_AL_IsEnabled()) {
		return;
	}

	alSourceStop(channel->alSource);
}

void S_AL_StopSounds() {
	channel_t *ch = s_channels;
	for (int i = 0; i < s_numChannels; i++, ch++) {
		alSourceStop(s_channels[i].alSource);
		alSourcei(s_channels[i].alSource, AL_BUFFER, 0);
		ch->thesfx = NULL;
		memset(&ch->MP3StreamHeader, 0, sizeof(MP3STREAM));
		ch->bLooping = false;
		ch->bProcessed = false;
		ch->bPlaying = false;
		ch->bStreaming = false;
	}
}

void S_AL_OnClearLoopingSounds() {
	if (!S_AL_IsEnabled()) {
		return;
	}

	for (int i = 0; i < MAX_LOOP_SOUNDS; i++)
		loopSounds[i].bProcessed = false;
}

void S_AL_OnUpdateEntityPosition(int entnum, const vec3_t position) {
	if (!S_AL_IsEnabled()) {
		return;
	}

	if (entnum == 0)
		return;

	channel_t *ch = s_channels + 1;
	for (int i = 1; i < s_numChannels; i++, ch++) {
		if ((s_channels[i].bPlaying) && (s_channels[i].entnum == entnum) && (!s_channels[i].bLooping)) {
			// Ignore position updates for CHAN_VOICE_GLOBAL
			if (ch->entchannel != CHAN_VOICE_GLOBAL && ch->entchannel != CHAN_ANNOUNCER) {
				ALfloat pos[3];
				pos[0] = position[0];
				pos[1] = position[2];
				pos[2] = -position[1];
				alSourcefv(s_channels[i].alSource, AL_POSITION, pos);

#if defined(USE_EAX)
				UpdateEAXBuffer(ch);
#endif
			}

			/*				pos[0] = origin[0];
            pos[1] = origin[2];
            pos[2] = -origin[1];
            alSourcefv(s_channels[i].alSource, AL_POSITION, pos);

            if ((s_bEALFileLoaded) && !( ch->entchannel == CHAN_VOICE || ch->entchannel == CHAN_VOICE_ATTEN || ch->entchannel == CHAN_VOICE_GLOBAL ) )
            {
                UpdateEAXBuffer(ch);
            }
*/
		}
	}
}

void S_AL_Respatialize(int entityNum, const vec3_t head, matrix3_t axis, int inwater) {
	listener_pos[0] = head[0];
	listener_pos[1] = head[2];
	listener_pos[2] = -head[1];
	alListenerfv(AL_POSITION, listener_pos);

	listener_ori[0] = axis[0][0];
	listener_ori[1] = axis[0][2];
	listener_ori[2] = -axis[0][1];
	listener_ori[3] = axis[2][0];
	listener_ori[4] = axis[2][2];
	listener_ori[5] = -axis[2][1];
	alListenerfv(AL_ORIENTATION, listener_ori);

#if defined(USE_EAX)
	// Update EAX effects here
	if (s_bEALFileLoaded) {
		// Check if the Listener is underwater
		if (inwater) {
			// Check if we have already applied Underwater effect
			if (!s_bInWater) {
				// New underwater fix
				for (i = 0; i < EAX_MAX_FXSLOTS; i++) {
					s_FXSlotInfo[i].lEnvID = -1;
				}

				// Load underwater reverb effect into FX Slot 0, and set this as
				// the Primary FX Slot
				unsigned int ulEnvironment = EAX_ENVIRONMENT_UNDERWATER;
				s_eaxSet(
					&EAXPROPERTYID_EAX40_FXSlot0,
					EAXREVERB_ENVIRONMENT,
					NULL,
					&ulEnvironment,
					sizeof(unsigned int));
				s_EnvironmentID = 999;

				s_eaxSet(
					&EAXPROPERTYID_EAX40_Context,
					EAXCONTEXT_PRIMARYFXSLOTID,
					NULL,
					(ALvoid *)&EAXPROPERTYID_EAX40_FXSlot0,
					sizeof(GUID));

				// Occlude all sounds into this environment, and mute all their
				// sends to other reverbs
				EAXOCCLUSIONPROPERTIES eaxOCProp;
				eaxOCProp.lOcclusion = -3000;
				eaxOCProp.flOcclusionLFRatio = 0.0f;
				eaxOCProp.flOcclusionRoomRatio = 1.37f;
				eaxOCProp.flOcclusionDirectRatio = 1.0f;

				EAXACTIVEFXSLOTS eaxActiveSlots;
				eaxActiveSlots.guidActiveFXSlots[0] = EAX_NULL_GUID;
				eaxActiveSlots.guidActiveFXSlots[1] = EAX_PrimaryFXSlotID;

				ch = s_channels + 1;
				for (i = 1; i < s_numChannels; i++, ch++) {
					// New underwater fix
					s_channels[i].lSlotID = -1;

					s_eaxSet(
						&EAXPROPERTYID_EAX40_Source,
						EAXSOURCE_OCCLUSIONPARAMETERS,
						ch->alSource,
						&eaxOCProp,
						sizeof(EAXOCCLUSIONPROPERTIES));

					s_eaxSet(
						&EAXPROPERTYID_EAX40_Source,
						EAXSOURCE_ACTIVEFXSLOTID,
						ch->alSource,
						&eaxActiveSlots,
						2 * sizeof(GUID));
				}

				s_bInWater = true;
			}
		} else {
			// Not underwater ... check if the underwater effect is still
			// present
			if (s_bInWater) {
				s_bInWater = false;

				// Remove underwater Reverb effect, and reset Occlusion /
				// Obstruction amount on all Sources
				UpdateEAXListener();

				ch = s_channels + 1;
				for (i = 1; i < s_numChannels; i++, ch++) {
					UpdateEAXBuffer(ch);
				}
			} else {
				UpdateEAXListener();
			}
		}
	}
#endif
}

void S_AL_Update() {
	UpdateSingleShotSounds();

	channel_t *ch = s_channels + 1;
	for (int i = 1; i < MAX_CHANNELS; i++, ch++) {
		if (!ch->thesfx || ch->bPlaying)
			continue;

		int source = ch - s_channels;

		if (ch->entchannel == CHAN_VOICE_GLOBAL ||
		    ch->entchannel == CHAN_ANNOUNCER) {
			// Always play these sounds at 0,0,-1 (in front of listener)
			float pos[3];
			pos[0] = 0.0f;
			pos[1] = 0.0f;
			pos[2] = -1.0f;

			alSourcefv(s_channels[source].alSource, AL_POSITION, pos);
			alSourcei(s_channels[source].alSource, AL_LOOPING, AL_FALSE);
			alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_TRUE);
			if (ch->entchannel == CHAN_ANNOUNCER) {
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.0f);
			} else {
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volumeVoice->value) / 255.0f);
			}
		} else {
			float pos[3];

			// Get position of source
			if (ch->fixed_origin) {
				pos[0] = ch->origin[0];
				pos[1] = ch->origin[2];
				pos[2] = -ch->origin[1];
				alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_FALSE);
			} else {
				if (ch->entnum == listener_number) {
					pos[0] = 0.0f;
					pos[1] = 0.0f;
					pos[2] = 0.0f;
					alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_TRUE);
				} else {
					// Get position of Entity
					if (ch->bLooping) {
						pos[0] = loopSounds[ch->entnum].origin[0];
						pos[1] = loopSounds[ch->entnum].origin[2];
						pos[2] = -loopSounds[ch->entnum].origin[1];
					} else {
						pos[0] = s_entityPosition[ch->entnum][0];
						pos[1] = s_entityPosition[ch->entnum][2];
						pos[2] = -s_entityPosition[ch->entnum][1];
					}
					alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_FALSE);
				}
			}

			alSourcefv(s_channels[source].alSource, AL_POSITION, pos);
			alSourcei(s_channels[source].alSource, AL_LOOPING, AL_FALSE);

			if (ch->entchannel == CHAN_VOICE) {
				// Reduced fall-off (Large Reference Distance), affected by
				// Voice Volume
				alSourcef(s_channels[source].alSource, AL_REFERENCE_DISTANCE, DEFAULT_VOICE_REF_DISTANCE);
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volumeVoice->value) / 255.0f);
			} else if (ch->entchannel == CHAN_VOICE_ATTEN) {
				// Normal fall-off, affected by Voice Volume
				alSourcef(s_channels[source].alSource, AL_REFERENCE_DISTANCE, DEFAULT_REF_DISTANCE);
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volumeVoice->value) / 255.0f);
			} else if (ch->entchannel == CHAN_LESS_ATTEN) {
				// Reduced fall-off, affected by Sound Effect Volume
				alSourcef(s_channels[source].alSource, AL_REFERENCE_DISTANCE, DEFAULT_VOICE_REF_DISTANCE);
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.f);
			} else {
				// Normal fall-off, affect by Sound Effect Volume
				alSourcef(s_channels[source].alSource, AL_REFERENCE_DISTANCE, DEFAULT_REF_DISTANCE);
				alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.f);
			}
		}

#if defined(USE_EAX)
		if (s_bEALFileLoaded)
			UpdateEAXBuffer(ch);
#endif

		if (ch->thesfx->pMP3StreamHeader) {
			memcpy(&ch->MP3StreamHeader, ch->thesfx->pMP3StreamHeader, sizeof(ch->MP3StreamHeader));
			ch->iMP3SlidingDecodeWritePos = 0;
			ch->iMP3SlidingDecodeWindowPos = 0;

			// Reset streaming buffers status's
			for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
				ch->buffers[i].Status = UNQUEUED;

			// Decode (STREAMING_BUFFER_SIZE / 1152) MP3 frames for each of the
			// NUM_STREAMING_BUFFERS AL Buffers
			for (i = 0; i < NUM_STREAMING_BUFFERS; i++) {
				int nTotalBytesDecoded = 0;

				for (int j = 0; j < (STREAMING_BUFFER_SIZE / 1152); j++) {
					int nBytesDecoded = C_MP3Stream_Decode(&ch->MP3StreamHeader, 0);
					memcpy(ch->buffers[i].Data + nTotalBytesDecoded, ch->MP3StreamHeader.bDecodeBuffer, nBytesDecoded);
					if (ch->entchannel == CHAN_VOICE ||
					    ch->entchannel == CHAN_VOICE_ATTEN ||
					    ch->entchannel == CHAN_VOICE_GLOBAL) {
						if (ch->thesfx->lipSyncData) {
							ch->thesfx->lipSyncData[(i * NUM_STREAMING_BUFFERS) + j] = MP3PreProcessLipSync(ch, (short *)(ch->MP3StreamHeader.bDecodeBuffer));
						} else {
#ifdef _DEBUG
							Com_OPrintf("Missing lip-sync info. for %s\n", ch->thesfx->sSoundName);
#endif
						}
					}
					nTotalBytesDecoded += nBytesDecoded;
				}

				if (nTotalBytesDecoded != STREAMING_BUFFER_SIZE) {
					memset(ch->buffers[i].Data + nTotalBytesDecoded, 0, (STREAMING_BUFFER_SIZE - nTotalBytesDecoded));
					break;
				}
			}

			int nBuffersToAdd = 0;
			if (i >= NUM_STREAMING_BUFFERS)
				nBuffersToAdd = NUM_STREAMING_BUFFERS;
			else
				nBuffersToAdd = i + 1;

			// Make sure queue is empty first
			alSourcei(s_channels[source].alSource, AL_BUFFER, 0);

			for (i = 0; i < nBuffersToAdd; i++) {
				// Copy decoded data to AL Buffer
				alBufferData(ch->buffers[i].BufferID, AL_FORMAT_MONO16, ch->buffers[i].Data, STREAMING_BUFFER_SIZE, 22050);

				// Queue AL Buffer on Source
				alSourceQueueBuffers(s_channels[source].alSource, 1, &(ch->buffers[i].BufferID));
				if (alGetError() == AL_NO_ERROR) {
					ch->buffers[i].Status = QUEUED;
				}
			}

			// Clear error state, and check for successful Play call
			alGetError();
			alSourcePlay(s_channels[source].alSource);
			if (alGetError() == AL_NO_ERROR) {
				s_channels[source].bPlaying = true;
			}

			ch->bStreaming = true;

			if (ch->entchannel == CHAN_VOICE ||
			    ch->entchannel == CHAN_VOICE_ATTEN ||
			    ch->entchannel == CHAN_VOICE_GLOBAL) {
				if (ch->thesfx->lipSyncData) {
					// Record start time for Lip-syncing
					s_channels[source].iStartTime = Sys_Milliseconds();

					// Prepare lipsync value(s)
					s_entityWavVol[ch->entnum] = ch->thesfx->lipSyncData[0];
				} else {
#ifdef _DEBUG
					Com_OPrintf(
						"Missing lip-sync info. for %s\n",
						ch->thesfx->sSoundName);
#endif
				}
			}

			return;
		} else {
			// Attach buffer to source
			alSourcei(s_channels[source].alSource, AL_BUFFER, ch->thesfx->Buffer);

			ch->bStreaming = false;

			// Clear error state, and check for successful Play call
			alGetError();
			alSourcePlay(s_channels[source].alSource);
			if (alGetError() == AL_NO_ERROR) {
				s_channels[source].bPlaying = true;
			}

			if (ch->entchannel == CHAN_VOICE ||
			    ch->entchannel == CHAN_VOICE_ATTEN ||
			    ch->entchannel == CHAN_VOICE_GLOBAL) {
				if (ch->thesfx->lipSyncData) {
					// Record start time for Lip-syncing
					s_channels[source].iStartTime = Sys_Milliseconds();

					// Prepare lipsync value(s)
					s_entityWavVol[ch->entnum] = ch->thesfx->lipSyncData[0];
				} else {
#ifdef _DEBUG
					Com_OPrintf(
						"Missing lip-sync info. for %s\n",
						ch->thesfx->sSoundName);
#endif
				}
			}
		}
	}

	SetLipSyncs();

	UpdateLoopingSounds();

	UpdateRawSamples();
}

/*
	Precalculate the lipsync values for the whole sample
*/
static void PreProcessLipSync(sfx_t *sfx) {
	int i, j;
	int sample;
	int sampleTotal = 0;

	j = 0;
	for (i = 0; i < sfx->iSoundLengthInSamples; i += 100) {
		sample = LittleShort(sfx->pSoundData[i]);

		sample = sample >> 8;
		sampleTotal += sample * sample;
		if (((i + 100) % 1000) == 0) {
			sampleTotal /= 10;

			if (sampleTotal < sfx->fVolRange * s_lip_threshold_1->value) {
				// tell the scripts that are relying on this that we are still going, but actually silent right now.
				sample = -1;
			} else if (sampleTotal < sfx->fVolRange * s_lip_threshold_2->value)
				sample = 1;
			else if (sampleTotal < sfx->fVolRange * s_lip_threshold_3->value)
				sample = 2;
			else if (sampleTotal < sfx->fVolRange * s_lip_threshold_4->value)
				sample = 3;
			else
				sample = 4;

			sfx->lipSyncData[j] = sample;
			j++;

			sampleTotal = 0;
		}
	}

	if ((i % 1000) == 0)
		return;

	i -= 100;
	i = i % 1000;
	i = i / 100;
	// Process last < 1000 samples
	if (i != 0)
		sampleTotal /= i;
	else
		sampleTotal = 0;

	if (sampleTotal < sfx->fVolRange * s_lip_threshold_1->value) {
		// tell the scripts that are relying on this that we are still going, but actually silent right now.
		sample = -1;
	} else if (sampleTotal < sfx->fVolRange * s_lip_threshold_2->value)
		sample = 1;
	else if (sampleTotal < sfx->fVolRange * s_lip_threshold_3->value)
		sample = 2;
	else if (sampleTotal < sfx->fVolRange * s_lip_threshold_4->value)
		sample = 3;
	else
		sample = 4;

	sfx->lipSyncData[j] = sample;
}

static bool CheckChannelStomp(int chan1, int chan2) {
	return (
		(chan1 == CHAN_VOICE || chan1 == CHAN_VOICE_ATTEN ||
		 chan1 == CHAN_VOICE_GLOBAL) &&
		(chan2 == CHAN_VOICE || chan2 == CHAN_VOICE_ATTEN ||
		 chan2 == CHAN_VOICE_GLOBAL));
}

static void SetLipSyncs() {
	int i;
	unsigned int samples;
	int currentTime, timePlayed;
	channel_t *ch;

	currentTime = Sys_Milliseconds();

	memset(s_entityWavVol, 0, sizeof(s_entityWavVol));

	ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		if (!ch->thesfx || !ch->bPlaying)
			continue;

		if (ch->entchannel != CHAN_VOICE && ch->entchannel != CHAN_VOICE_ATTEN && ch->entchannel != CHAN_VOICE_GLOBAL) {
			continue;
		}

		// Calculate how much time has passed since the sample was started
		timePlayed = currentTime - ch->iStartTime;

		if (ch->thesfx->eSoundCompressionMethod == ct_16) {
			// There is a new computed lip-sync value every 1000 samples - so find out how many samples
			// have been played and lookup the value in the lip-sync table
			samples = (timePlayed * 22050) / 1000;

			if (ch->thesfx->lipSyncData == NULL) {
#ifdef _DEBUG
				Com_OPrintf("Missing lip-sync info. for %s\n", ch->thesfx->sSoundName);
#endif
			}

			if ((ch->thesfx->lipSyncData) && ((int)samples < ch->thesfx->iSoundLengthInSamples)) {
				s_entityWavVol[ch->entnum] = ch->thesfx->lipSyncData[samples / 1000];
				if (s_show->integer == 3) {
					Com_Printf("(%i)%i %s vol = %i\n", ch->entnum, i, ch->thesfx->sSoundName, s_entityWavVol[ch->entnum]);
				}
			}
		} else {
			// MP3

			// There is a new computed lip-sync value every 576 samples - so find out how many samples
			// have been played and lookup the value in the lip-sync table
			samples = (timePlayed * 22050) / 1000;

			if (ch->thesfx->lipSyncData == NULL) {
#ifdef _DEBUG
				Com_OPrintf("Missing lip-sync info. for %s\n", ch->thesfx->sSoundName);
#endif
			}

			if ((ch->thesfx->lipSyncData) && (samples < (unsigned)ch->thesfx->iSoundLengthInSamples)) {
				s_entityWavVol[ch->entnum] = ch->thesfx->lipSyncData[(samples / 576) % 16];

				if (s_show->integer == 3) {
					Com_Printf("(%i)%i %s vol = %i\n", ch->entnum, i, ch->thesfx->sSoundName, s_entityWavVol[ch->entnum]);
				}
			}
		}
	}
}

static void UpdateSingleShotSounds() {
	int i, j, k;
	ALint state;
	ALint processed;
	channel_t *ch;

	// Firstly, check if any single-shot sounds have completed, or if they need more data (for streaming Sources),
	// and/or if any of the currently playing (non-Ambient) looping sounds need to be stopped
	ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		ch->bProcessed = false;
		if (!s_channels[i].bPlaying || ch->bLooping) {
			continue;
		}

		// Single-shot
		if (!s_channels[i].bStreaming) {
			alGetSourcei(s_channels[i].alSource, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED) {
				s_channels[i].thesfx = NULL;
				s_channels[i].bPlaying = false;
			}

			continue;
		}

		// Process streaming sample

		// Procedure :-
		// if more data to play
		//		if any UNQUEUED Buffers
		//			fill them with data
		//		(else ?)
		//			get number of buffers processed
		//			fill them with data
		//		restart playback if it has stopped (buffer underrun)
		// else
		//		free channel

		if (ch->thesfx->pMP3StreamHeader == nullptr) {
			continue;
		}

		if (ch->MP3StreamHeader.iSourceBytesRemaining == 0) {
			// Finished decoding data - if the source has finished playing then we're done
			alGetSourcei(ch->alSource, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED) {
				// Attach NULL buffer to Source to remove any buffers left in the queue
				alSourcei(ch->alSource, AL_BUFFER, 0);
				ch->thesfx = NULL;
				ch->bPlaying = false;
			}
			// Move on to next channel ...
			continue;
		}

		// Check to see if any Buffers have been processed
		alGetSourcei(ch->alSource, AL_BUFFERS_PROCESSED, &processed);

		while (processed--) {
			ALuint buffer;
			alSourceUnqueueBuffers(ch->alSource, 1, &buffer);
			for (j = 0; j < NUM_STREAMING_BUFFERS; j++) {
				if (ch->buffers[j].BufferID == buffer) {
					ch->buffers[j].Status = UNQUEUED;
					break;
				}
			}
		}

		for (j = 0; j < NUM_STREAMING_BUFFERS; j++) {
			if (ch->buffers[j].Status != UNQUEUED || ch->MP3StreamHeader.iSourceBytesRemaining <= 0) {
				continue;
			}

			int nTotalBytesDecoded = 0;

			for (k = 0; k < (STREAMING_BUFFER_SIZE / 1152); k++) {
				int nBytesDecoded = C_MP3Stream_Decode(&ch->MP3StreamHeader, 0);
				if (nBytesDecoded > 0) {
					memcpy(ch->buffers[j].Data + nTotalBytesDecoded, ch->MP3StreamHeader.bDecodeBuffer, nBytesDecoded);

					if (ch->entchannel == CHAN_VOICE || ch->entchannel == CHAN_VOICE_ATTEN || ch->entchannel == CHAN_VOICE_GLOBAL) {
						if (ch->thesfx->lipSyncData) {
							ch->thesfx->lipSyncData[(j * 4) + k] = MP3PreProcessLipSync(ch, (short *)(ch->buffers[j].Data));
						} else {
#ifdef _DEBUG
							Com_OPrintf("Missing lip-sync info. for %s\n", ch->thesfx->sSoundName);
#endif
						}
					}
					nTotalBytesDecoded += nBytesDecoded;
				} else {
					// Make sure that iSourceBytesRemaining is 0
					if (ch->MP3StreamHeader.iSourceBytesRemaining != 0) {
						ch->MP3StreamHeader.iSourceBytesRemaining = 0;
						break;
					}
				}
			}

			if (nTotalBytesDecoded != STREAMING_BUFFER_SIZE) {
				memset(ch->buffers[j].Data + nTotalBytesDecoded, 0, (STREAMING_BUFFER_SIZE - nTotalBytesDecoded));

				alBufferData(ch->buffers[j].BufferID, AL_FORMAT_MONO16, ch->buffers[j].Data, STREAMING_BUFFER_SIZE, 22050);
				alSourceQueueBuffers(ch->alSource, 1, &(ch->buffers[j].BufferID));
				ch->buffers[j].Status = QUEUED;

				break;
			} else {
				alBufferData(ch->buffers[j].BufferID, AL_FORMAT_MONO16, ch->buffers[j].Data, STREAMING_BUFFER_SIZE, 22050);
				alSourceQueueBuffers(ch->alSource, 1, &(ch->buffers[j].BufferID));
				ch->buffers[j].Status = QUEUED;
			}
		}

		alGetSourcei(ch->alSource, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {
			alSourcePlay(ch->alSource);
#ifdef _DEBUG
			Com_OPrintf("[%d] Restarting playback of single-shot streaming MP3 sample - still have %d bytes to decode\n", i, ch->MP3StreamHeader.iSourceBytesRemaining);
#endif
		}
	}
}

static int MP3PreProcessLipSync(channel_t *ch, short *data) {
	int i;
	int sample;
	int sampleTotal = 0;

	for (i = 0; i < 576; i += 100) {
		sample = LittleShort(data[i]);
		sample = sample >> 8;
		sampleTotal += sample * sample;
	}

	sampleTotal /= 6;

	if (sampleTotal < ch->thesfx->fVolRange * s_lip_threshold_1->value)
		sample = -1;
	else if (sampleTotal < ch->thesfx->fVolRange * s_lip_threshold_2->value)
		sample = 1;
	else if (sampleTotal < ch->thesfx->fVolRange * s_lip_threshold_3->value)
		sample = 2;
	else if (sampleTotal < ch->thesfx->fVolRange * s_lip_threshold_4->value)
		sample = 3;
	else
		sample = 4;

	return sample;
}

static void UpdateLoopingSounds() {
	int i, j;
	ALuint source;
	channel_t *ch;
	loopSound_t *loop;
	float pos[3];

	// First check to see if any of the looping sounds are already playing at the correct positions
	ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		if (!ch->bLooping || !s_channels[i].bPlaying) {
			continue;
		}

		for (j = 0; j < numLoopSounds; j++) {
			loop = &loopSounds[j];

			// If this channel is playing the right sound effect at the right position then mark this channel and looping sound
			// as processed
			if (loop->bProcessed || ch->thesfx != loop->sfx) {
				continue;
			}

			if (loop->origin[0] == listener_pos[0] &&
			    loop->origin[1] == -listener_pos[2] &&
			    loop->origin[2] == listener_pos[1]) {
				// Assume that this sound is head relative
				if (!loop->bRelative) {
					// Set position to 0,0,0 and turn on Head Relative Mode
					const float pos[3] = {0.0f, 0.0f, 0.0f};
					alSourcefv(s_channels[i].alSource, AL_POSITION, pos);
					alSourcei(s_channels[i].alSource, AL_SOURCE_RELATIVE, AL_TRUE);
					loop->bRelative = true;
				}

				// Make sure Gain is set correctly
				if (ch->master_vol != loop->volume) {
					ch->master_vol = loop->volume;
					alSourcef(s_channels[i].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.f);
				}

				ch->bProcessed = true;
				loop->bProcessed = true;
			} else if (!loop->bProcessed && ch->thesfx == loop->sfx && !memcmp(ch->origin, loop->origin, sizeof(ch->origin))) {
				// Match !
				ch->bProcessed = true;
				loop->bProcessed = true;

				// Make sure Gain is set correctly
				if (ch->master_vol != loop->volume) {
					ch->master_vol = loop->volume;
					alSourcef(s_channels[i].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.f);
				}

				break;
			}
		}
	}

	// Next check if the correct looping sound is playing, but at the wrong position
	ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		if (!ch->bLooping || ch->bProcessed || !s_channels[i].bPlaying) {
			continue;
		}

		for (j = 0; j < numLoopSounds; j++) {
			loop = &loopSounds[j];

			if (!loop->bProcessed && ch->thesfx == loop->sfx) {
				// Same sound - wrong position
				ch->origin[0] = loop->origin[0];
				ch->origin[1] = loop->origin[1];
				ch->origin[2] = loop->origin[2];

				pos[0] = loop->origin[0];
				pos[1] = loop->origin[2];
				pos[2] = -loop->origin[1];
				alSourcefv(s_channels[i].alSource, AL_POSITION, pos);

				ch->master_vol = loop->volume;
				alSourcef(s_channels[i].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.f);

#if defined(USE_EAX)
				if (s_bEALFileLoaded)
					UpdateEAXBuffer(ch);
#endif

				ch->bProcessed = true;
				loop->bProcessed = true;
				break;
			}
		}
	}

	// If any non-procesed looping sounds are still playing on a channel, they can be removed as they are no longer
	// required
	ch = s_channels + 1;
	for (i = 1; i < s_numChannels; i++, ch++) {
		if (!s_channels[i].bPlaying || !ch->bLooping || ch->bProcessed) {
			continue;
		}

		// Sound no longer needed
		alSourceStop(s_channels[i].alSource);
		ch->thesfx = NULL;
		ch->bPlaying = false;
	}

#ifdef _DEBUG
	alGetError();
#endif
	// Finally if there are any non-processed sounds left, we need to try and play them
	for (j = 0; j < numLoopSounds; j++) {
		loop = &loopSounds[j];
		if (loop->bProcessed) {
			continue;
		}

		ch = S_PickChannel(0, 0);
		ch->master_vol = loop->volume;
		ch->entnum = loop->entnum;
		ch->entchannel = CHAN_AMBIENT; // Make sure this gets set to something
		ch->thesfx = loop->sfx;
		ch->bLooping = true;

		// Check if the Source is positioned at exactly the same location as the listener
		if (loop->origin[0] == listener_pos[0] &&
		    loop->origin[1] == -listener_pos[2] &&
		    loop->origin[2] == listener_pos[1]) {
			// Assume that this sound is head relative
			loop->bRelative = true;
			ch->origin[0] = 0.f;
			ch->origin[1] = 0.f;
			ch->origin[2] = 0.f;
		} else {
			ch->origin[0] = loop->origin[0];
			ch->origin[1] = loop->origin[1];
			ch->origin[2] = loop->origin[2];
			loop->bRelative = false;
		}

		ch->fixed_origin = (qboolean)loop->bRelative;
		pos[0] = ch->origin[0];
		pos[1] = ch->origin[2];
		pos[2] = -ch->origin[1];

		source = ch - s_channels;
		alSourcei(s_channels[source].alSource, AL_BUFFER, ch->thesfx->Buffer);
		alSourcefv(s_channels[source].alSource, AL_POSITION, pos);
		alSourcei(s_channels[source].alSource, AL_LOOPING, AL_TRUE);
		alSourcef(s_channels[source].alSource, AL_REFERENCE_DISTANCE, DEFAULT_REF_DISTANCE);
		alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.0f);
		alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, ch->fixed_origin ? AL_TRUE : AL_FALSE);

#if defined(USE_EAX)
		if (s_bEALFileLoaded)
			UpdateEAXBuffer(ch);
#endif

		alGetError();
		alSourcePlay(s_channels[source].alSource);
		if (alGetError() == AL_NO_ERROR)
			s_channels[source].bPlaying = true;
	}
}

static void UpdateRawSamples() {
	ALuint buffer;
	ALint size;
	ALint processed;
	ALint state;
	int i, j, src;

#ifdef _DEBUG
	// Clear Open AL Error
	alGetError();
#endif

	S_UpdateBackgroundTrack();

	channel_t *musicChannel = s_channels;
	// Find out how many buffers have been processed (played) by the music channel source
	alGetSourcei(musicChannel->alSource, AL_BUFFERS_PROCESSED, &processed);
	while (processed--) {
		// Unqueue each buffer, determine the length of the buffer, and then delete it
		alSourceUnqueueBuffers(musicChannel->alSource, 1, &buffer);
		alGetBufferi(buffer, AL_SIZE, &size);
		alDeleteBuffers(1, &buffer);

		//Com_Printf(S_COLOR_RED "Unqueued buffer=%d size=%d\n", buffer, size);

		// Update sg.soundtime (+= number of samples played (number of bytes / 4))
		s_soundtime += (size >> 2);
	}

	//Com_Printf(S_COLOR_YELLOW "rawend=%d paintedtime=%d\n", s_rawend, s_paintedtime);
	// Add new data to a new Buffer and queue it on the Source
	if (s_rawend > s_paintedtime) {
		size = (s_rawend - s_paintedtime) << 2;
		if (size > (MAX_RAW_SAMPLES << 2)) {
			Com_OPrintf("UpdateRawSamples :- Raw Sample buffer has overflowed !!!\n");
			size = MAX_RAW_SAMPLES << 2;
			s_paintedtime = s_rawend - MAX_RAW_SAMPLES;
		}

		// Copy samples from RawSamples to audio buffer (sg.rawdata)
		for (i = s_paintedtime, j = 0; i < s_rawend; i++, j += 2) {
			src = i & (MAX_RAW_SAMPLES - 1);
			s_rawdata[j] = (short)(s_rawsamples[src].left >> 8);
			s_rawdata[j + 1] = (short)(s_rawsamples[src].right >> 8);
		}

		// Need to generate more than 1 buffer for music playback
		// iterations = 0;
		// largestBufferSize = (MAX_RAW_SAMPLES / 4) * 4
		// while (size)
		//	generate a buffer
		//	if size > largestBufferSize
		//		copy sg.rawdata + ((iterations * largestBufferSize)>>1) to buffer
		//		size -= largestBufferSize
		//	else
		//		copy remainder
		//		size = 0
		//	queue the buffer
		//  iterations++;

		int iterations = 0;
		int largestBufferSize = MAX_RAW_SAMPLES; // in bytes (== quarter of Raw Samples data)
		while (size) {
			alGenBuffers(1, &buffer);

			if (size > largestBufferSize) {
				//Com_Printf(S_COLOR_GREEN "Enqueueing buffer %d with %d bytes of data\n", buffer, largestBufferSize);
				alBufferData(buffer, AL_FORMAT_STEREO16, (char *)(s_rawdata + ((iterations * largestBufferSize) >> 1)), largestBufferSize, 22050);
				size -= largestBufferSize;
			} else {
				//Com_Printf(S_COLOR_GREEN "Enqueueing buffer %d with %d bytes of data\n", buffer, size);
				alBufferData(buffer, AL_FORMAT_STEREO16, (char *)(s_rawdata + ((iterations * largestBufferSize) >> 1)), size, 22050);
				size = 0;
			}

			alSourceQueueBuffers(musicChannel->alSource, 1, &buffer);
			iterations++;
		}

		// Update paintedtime
		s_paintedtime = s_rawend;

		// Check that the Source is actually playing
		alGetSourcei(musicChannel->alSource, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {
#if 0
			// Stopped playing ... due to buffer underrun
			// Unqueue any buffers still on the Source (they will be PROCESSED), and restart playback
			alGetSourcei(musicChannel->alSource, AL_BUFFERS_PROCESSED, &processed);
			while (processed--)
			{
				alSourceUnqueueBuffers(musicChannel->alSource, 1, &buffer);
				alGetBufferi(buffer, AL_SIZE, &size);
				alDeleteBuffers(1, &buffer);

                //Com_Printf(S_COLOR_RED "Unqueued buffer=%d size=%d for underrun\n", buffer, size);

				// Update sg.soundtime (+= number of samples played (number of bytes / 4))
				s_soundtime += (size >> 2);
			}
#endif

#ifdef _DEBUG
			Com_OPrintf("Restarting / Starting playback of Raw Samples\n");
#endif
			alSourcePlay(musicChannel->alSource);
		}
	}

#ifdef _DEBUG
	if (alGetError() != AL_NO_ERROR)
		Com_OPrintf("OAL Error : UpdateRawSamples\n");
#endif
}

int S_AL_OnFreeSfxMemory(sfx_t *sfx) {
	if (!S_AL_IsEnabled()) {
		return 0;
	}

	alGetError();
	if (sfx->Buffer) {
		alDeleteBuffers(1, &(sfx->Buffer));
#ifdef _DEBUG
		if (alGetError() != AL_NO_ERROR) {
			Com_OPrintf(
				"Failed to delete AL Buffer (%s) ... !\n", sfx->sSoundName);
		}
#endif
		sfx->Buffer = 0;
	}

	int bytesFreed = 0;
	if (sfx->lipSyncData) {
		bytesFreed += Z_Size(sfx->lipSyncData);
		Z_Free(sfx->lipSyncData);
		sfx->lipSyncData = NULL;
	}
	return bytesFreed;
}