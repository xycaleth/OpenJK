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


include(CMakeParseArguments)
function(find_package_and_import)
	set(oneValueArgs PACKAGE TARGET)
	set(multiValueArgs LIBS INCLUDES)

	cmake_parse_arguments(LOCAL "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	find_package(${LOCAL_PACKAGE} REQUIRED)
	if(NOT TARGET ${LOCAL_TARGET})
		add_library(${LOCAL_TARGET} STATIC IMPORTED)
		set_property(TARGET ${LOCAL_TARGET}
			PROPERTY
				INTERFACE_LINK_LIBRARIES ${LOCAL_LIBS}
				INTERFACE_INCLUDE_DIRECTORIES ${LOCAL_INCLUDES})
	endif()
endfunction()

if(USE_INTERNAL_JPEG)
	add_subdirectory(lib/jpeg-9a)
else()
	find_package_and_import(
		PACKAGE JPEG
		TARGET JPEG::JPEG
		LIBS ${JPEG_LIBRARIES}
		INCLUDES ${JPEG_INCLUDE_DIR})
endif()

if(USE_INTERNAL_ZLIB)
	add_subdirectory(lib/zlib)
else()
	find_package_and_import(
		PACKAGE ZLIB
		TARGET ZLIB::ZLIB
		LIBS ${ZLIB_LIBRARIES}
		INCLUDES ${ZLIB_INCLUDE_DIR})
endif()

if(USE_INTERNAL_PNG)
	add_subdirectory(lib/libpng)
else()
	find_package_and_import(
		PACKAGE PNG
		TARGET PNG::PNG
		LIBS ${PNG_LIBRARIES}
		INCLUDES ${PNG_INCLUDE_DIR})
endif()

add_subdirectory(lib/minizip)

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

function(add_game_application)
	set(oneValueArgs PROJECT LABEL DIRNAME INSTALL_DIR COMPONENT)
	set(multiValueArgs FILES DEFINES INCLUDES LIBS)

	cmake_parse_arguments(LOCAL "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(MakeApplicationBundles)
		set(MACOSX_BUNDLE_BUNDLE_NAME ${LOCAL_PROJECT})
		set(MACOSX_BUNDLE_DISPLAY_NAME ${LOCAL_PROJECT})
		set(MACOSX_BUNDLE_ICON_FILE OpenJK.icns)

		set(LOCAL_FILES
			${LOCAL_FILES}
			${LOCAL_DIRNAME}/macosx/OpenJK.icns)
		set_source_files_properties(
			${LOCAL_DIRNAME}/macosx/OpenJK.icns
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

		set(LOCAL_FILES
			${LOCAL_FILES}
			${LOCAL_DIRNAME}/macosx/English.lproj/InfoPlist.strings)
		set_source_files_properties(
			${LOCAL_DIRNAME}/macosx/English.lproj/InfoPlist.strings
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj)
	endif()

	if(WIN32)
		add_executable(${LOCAL_PROJECT} WIN32 ${LOCAL_FILES})
		if(USE_INTERNAL_SDL2)
			if(BUILD_64BIT)
				set(EngineExtraInstallFiles
					${OpenJKLibDir}/SDL2/bin/x64/SDL2.dll)
			else()
				set(EngineExtraInstallFiles
					${OpenJKLibDir}/SDL2/bin/x86/SDL2.dll)
			endif()
		endif()
	else()
		if(MakeApplicationBundles)
			add_executable(${LOCAL_PROJECT} MACOSX_BUNDLE ${LOCAL_FILES})
			set_target_properties(
				${LOCAL_PROJECT}
				PROPERTIES
					MACOSX_BUNDLE_INFO_PLIST "${LOCAL_DIRNAME}/macosx/OpenJKInfo.plist")
		else()
			add_executable(${LOCAL_PROJECT} ${LOCAL_FILES})
		endif()
	endif()

	set_target_properties(${LOCAL_PROJECT} PROPERTIES PROJECT_LABEL ${LOCAL_LABEL})
	target_link_libraries(${LOCAL_PROJECT} PRIVATE ${LOCAL_LIBS})
	target_compile_options(${LOCAL_PROJECT} PRIVATE ${OPENJK_VISIBILITY_FLAGS})
	target_compile_definitions(${LOCAL_PROJECT} PRIVATE ${LOCAL_DEFINES})
	target_include_directories(${LOCAL_PROJECT} PRIVATE ${LOCAL_INCLUDES})

	if(MakeApplicationBundles)
		install(CODE "
			include(BundleUtilities)
			set(BU_CHMOD_BUNDLE_ITEMS ON)
			fixup_bundle(\"${CMAKE_CURRENT_BINARY_DIR}/${LOCAL_PROJECT}.app\" \"\" \"\")
			"
			COMPONENT Runtime)
		install(TARGETS ${LOCAL_PROJECT}
			BUNDLE
			DESTINATION ${LOCAL_INSTALL_DIR}
			COMPONENT ${LOCAL_COMPONENT})
	else()
		install(TARGETS ${LOCAL_PROJECT}
			RUNTIME
			DESTINATION ${LOCAL_INSTALL_DIR}
			COMPONENT ${LOCAL_COMPONENT})
		if(SPEngineExtraInstallFiles)
			install(FILES ${EngineExtraInstallFiles}
				DESTINATION ${LOCAL_INSTALL_DIR}
				COMPONENT ${LOCAL_COMPONENT})
		endif()
	endif()
endfunction()

# Add projects
if(BUILD_JASP OR BUILD_JOSP)
	add_subdirectory(${SPDir})
	if(BuildJK2SPGame)
		add_subdirectory("${JK2SPDir}/game")
	endif()
endif()

if(BUILD_JAMP)
	add_subdirectory(${MPDir})
endif()

if(BUILD_SYMBOL_SERVER)
	add_subdirectory(tools/WinSymbol)
endif()

if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
