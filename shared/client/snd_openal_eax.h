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
#pragma once

extern bool s_bEAX;
extern bool s_bInWater;
extern int s_EnvironmentID;
extern bool s_bEALFileLoaded;

typedef struct channel_s channel_t;

void InitEAXManager();
void ReleaseEAXManager();
void EALFileInit(const char *level);
void UpdateEAXListener();
void UpdateEAXBuffer(channel_t *ch);

void S_ALEAX_SoundInfo_f();