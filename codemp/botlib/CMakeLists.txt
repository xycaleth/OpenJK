#============================================================================
# Copyright (C) 2013 - 2018, OpenJK contributors
#
# This file is part of the OpenJK source code.
#
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

# Botlib
set(MPBotlibDefines ${SharedDefines} "BOTLIB")
set(MPBotlibIncludeDirectories
	${SharedDir}
	${MPDir}
	${GSLIncludeDirectory})
set(MPBotlibFiles
	aasfile.h
	be_aas.h
	be_aas_bsp.h
	be_aas_cluster.h
	be_aas_debug.h
	be_aas_def.h
	be_aas_entity.h
	be_aas_file.h
	be_aas_funcs.h
	be_aas_main.h
	be_aas_move.h
	be_aas_optimize.h
	be_aas_reach.h
	be_aas_route.h
	be_aas_routealt.h
	be_aas_sample.h
	be_ai_char.h
	be_ai_chat.h
	be_ai_gen.h
	be_ai_goal.h
	be_ai_move.h
	be_ai_weap.h
	be_ai_weight.h
	be_ea.h
	be_interface.h
	botlib.h
	l_crc.h
	l_libvar.h
	l_log.h
	l_memory.h
	l_precomp.h
	l_script.h
	l_struct.h
	l_utils.h

	be_aas_bspq3.cpp
	be_aas_cluster.cpp
	be_aas_debug.cpp
	be_aas_entity.cpp
	be_aas_file.cpp
	be_aas_main.cpp
	be_aas_move.cpp
	be_aas_optimize.cpp
	be_aas_reach.cpp
	be_aas_route.cpp
	be_aas_routealt.cpp
	be_aas_sample.cpp
	be_ai_char.cpp
	be_ai_chat.cpp
	be_ai_gen.cpp
	be_ai_goal.cpp
	be_ai_move.cpp
	be_ai_weap.cpp
	be_ai_weight.cpp
	be_ea.cpp
	be_interface.cpp
	l_crc.cpp
	l_libvar.cpp
	l_log.cpp
	l_memory.cpp
	l_precomp.cpp
	l_script.cpp
	l_struct.cpp)
source_group("botlib" FILES ${MPBotlibFiles})
set(MPBotlibFiles ${MPBotlibFiles} ${MPBotlibFiles})

set(MPBotlibGameFiles
	"${MPDir}/game/surfaceflags.h")
source_group("game" FILES ${MPBotlibGameFiles})
set(MPBotlibFiles ${MPBotlibFiles} ${MPBotlibGameFiles})

set(MPBotlibCommonFiles
	"${MPDir}/qcommon/cm_public.h"
	"${MPDir}/qcommon/disablewarnings.h"
	"${MPDir}/qcommon/q_shared.h"
	"${MPDir}/qcommon/qcommon.h"
	"${MPDir}/qcommon/qfiles.h"
	"${MPDir}/qcommon/tags.h" 
	${SharedCommonFiles})
source_group("common" FILES ${MPBotlibCommonFiles})
set(MPBotlibFiles ${MPBotlibFiles} ${MPBotlibCommonFiles})

set(MPBotlibCommonSafeFiles
	"${SharedDir}/qcommon/safe/gsl.h"
	"${SharedDir}/qcommon/safe/string.cpp"
	"${SharedDir}/qcommon/safe/string.h"
	"${SharedDir}/qcommon/safe/sscanf.h")
source_group("common/safe" FILES ${MPBotlibCommonSafeFiles})
set(MPBotlibFiles ${MPBotlibFiles} ${MPBotlibCommonSafeFiles})

set(MPBotlibServerFiles
	"${MPDir}/server/server.h")
source_group("server" FILES ${MPBotlibServerFiles})
set(MPBotlibFiles ${MPBotlibFiles} ${MPBotlibServerFiles})

add_library(${MPBotLib} STATIC ${MPBotlibFiles})
set_target_properties(${MPBotLib} PROPERTIES PROJECT_LABEL "Bot Library")
target_compile_options(${MPBotLib} PUBLIC ${OPENJK_VISIBILITY_FLAGS})
target_compile_definitions(${MPBotLib} PUBLIC ${MPBotlibDefines})
target_include_directories(${MPBotLib} PUBLIC ${MPBotlibIncludeDirectories})
