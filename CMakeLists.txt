#============================================================================
# Copyright (C) 2013 - 2015, OpenJK contributors
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

cmake_minimum_required(VERSION 3.1)

set(ProjectName "OpenJK" CACHE STRING "Project Name")
project(${ProjectName})

# Custom CMake Modules needed
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/CMakeModules")

include(BuildOptions)
include(PlatformDefinitions)

if (NOT CMAKE_BUILD_TYPE)
	message("No build type selected, default to RELEASE")
	set(CMAKE_BUILD_TYPE "RELEASE")
endif()

# Target names
set(SPEngine "openjk_sp.${Architecture}")
set(SPGame "jagame${Architecture}")
set(SPRDVanillaRenderer "rdsp-vanilla_${Architecture}")
set(MPEngine "openjk.${Architecture}")
set(MPVanillaRenderer "rd-vanilla_${Architecture}")
set(MPDed "openjkded.${Architecture}")
set(MPGame "jampgame${Architecture}")
set(MPCGame "cgame${Architecture}")
set(MPUI "ui${Architecture}")
set(JK2SPEngine "openjo_sp.${Architecture}")
set(JK2SPGame "jospgame${Architecture}")
set(JK2SPVanillaRenderer "rdjosp-vanilla_${Architecture}")
set(AssetsPk3 "openjk-${Architecture}.pk3")
set(MPBotLib "botlib")



# Paths
set(SPDir "${CMAKE_SOURCE_DIR}/code")
set(MPDir "${CMAKE_SOURCE_DIR}/codemp")
set(JK2SPDir "${CMAKE_SOURCE_DIR}/codeJK2")
set(SharedDir "${CMAKE_SOURCE_DIR}/shared")
set(OpenJKLibDir "${CMAKE_SOURCE_DIR}/lib")
set(GSLIncludeDirectory "${OpenJKLibDir}/gsl-lite/include")

include(InstallConfig)
include(CompilerFlags)
include(GlobalDefinitions)


# Files shared across all projects
set(SharedCommonFiles
	"${SharedDir}/qcommon/q_color.h"
	"${SharedDir}/qcommon/q_color.c"
	"${SharedDir}/qcommon/q_math.h"
	"${SharedDir}/qcommon/q_math.c"
	"${SharedDir}/qcommon/q_string.h"
	"${SharedDir}/qcommon/q_string.c"
	"${SharedDir}/qcommon/q_platform.h")
set(SharedCommonSafeFiles
	"${SharedDir}/qcommon/safe/gsl.h"
	"${SharedDir}/qcommon/safe/string.cpp"
	"${SharedDir}/qcommon/safe/string.h"
	"${SharedDir}/qcommon/safe/sscanf.h"
	"${SharedDir}/qcommon/safe/limited_vector.h")


if(UseInternalJPEG)
	add_subdirectory(lib/jpeg-9a)
else()
	find_package(JPEG REQUIRED)
	if(NOT TARGET JPEG::JPEG)
		add_library(JPEG::JPEG STATIC IMPORTED)
		set_property(TARGET JPEG::JPEG
			PROPERTY
				INTERFACE_LINK_LIBRARIES ${JPEG_LIBRARIES}
				INTERFACE_INCLUDE_DIRECTORIES ${JPEG_INCLUDE_DIR})
	endif()
endif()

if(UseInternalZlib)
	add_subdirectory(lib/zlib)
else()
	find_package(ZLIB REQUIRED)
	if(NOT TARGET ZLIB::ZLIB)
		add_library(ZLIB::ZLIB STATIC IMPORTED)
		set_property(TARGET ZLIB::ZLIB
			PROPERTY
				INTERFACE_LINK_LIBRARIES ${ZLIB_LIBRARIES}
				INTERFACE_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIR})
	endif()
endif()

if(UseInternalPNG)
	add_subdirectory(lib/libpng)
else()
	find_package(PNG REQUIRED)
	if(NOT TARGET PNG::PNG)
		add_library(PNG::PNG STATIC IMPORTED)
		set_property(TARGET PNG::PNG
			PROPERTY
				INTERFACE_LINK_LIBRARIES ${PNG_LIBRARIES}
				INTERFACE_INCLUDE_DIRECTORIES ${PNG_INCLUDE_DIR})
	endif()
endif()

add_subdirectory(lib/minizip)

include(CMakeParseArguments)
function(add_mp_game_module)
	set(oneValueArgs MODULE LABEL)
	set(multiValueArgs FILES DEFINES INCLUDES)

	cmake_parse_arguments(LOCAL "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_library(${LOCAL_MODULE} MODULE ${LOCAL_FILES})

	if(NOT MSVC)
		# remove "lib" prefix for .so/.dylib files
		set_target_properties(${LOCAL_MODULE} PROPERTIES PREFIX "")
	endif()

	if(APPLE)
		set_target_properties(${LOCAL_MODULE} PROPERTIES SUFFIX ".dylib")
	endif()

	set_target_properties(${LOCAL_MODULE}
		PROPERTIES PROJECT_LABEL ${LOCAL_LABEL})
	target_compile_options(${LOCAL_MODULE} PRIVATE ${OPENJK_VISIBILITY_FLAGS})
	target_compile_definitions(${LOCAL_MODULE} PRIVATE ${LOCAL_DEFINES})
	target_include_directories(${LOCAL_MODULE} PRIVATE ${LOCAL_INCLUDES})

	if(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${LOCAL_MODULE}
			LIBRARY
			DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/OpenJK"
			COMPONENT ${JKAMPCoreComponent})
		install(TARGETS ${LOCAL_MODULE}
			LIBRARY
			DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/base"
			COMPONENT ${JKAMPCoreComponent})
	elseif()
		install(TARGETS ${LOCAL_MODULE}
			RUNTIME
			DESTINATION "${JKAInstallDir}/OpenJK"
			COMPONENT ${JKAMPCoreComponent})
		if (WIN64)
			# Don't do this on 32-bit Windows to avoid overwriting
			# vanilla JKA's DLLs
			install(TARGETS ${LOCAL_MODULE}
				RUNTIME
				DESTINATION "${JKAInstallDir}/base"
				COMPONENT ${JKAMPCoreComponent})
		endif()
	else()
		install(TARGETS ${LOCAL_MODULE}
			LIBRARY
			DESTINATION "${JKAInstallDir}/OpenJK"
			COMPONENT ${JKAMPCoreComponent})
		install(TARGETS ${LOCAL_MODULE}
			LIBRARY
			DESTINATION "${JKAInstallDir}/base"
			COMPONENT ${JKAMPCoreComponent})
	endif()
endfunction()

# Add projects
if(BUILD_JASP)
	add_subdirectory(${SPDir})
	if(BuildJK2SPGame)
		add_subdirectory("${JK2SPDir}/game")
	endif()
endif()

if(BUILD_JAMP)
	add_subdirectory(${MPDir})
endif()

# Windows Symbol Server tools
if(BuildSymbolServer)
	add_subdirectory("tools/WinSymbol")
endif()

if(BuildTests)
	enable_testing()
	add_subdirectory("tests")
endif()
