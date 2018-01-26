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

set(MPSharedDefines ${SharedDefines})

if(BuildMPGame)
	add_subdirectory(game)
endif()

if(BuildMPCGame)
	add_subdirectory(cgame)
endif()

if(BuildMPUI)
	add_subdirectory(ui)
endif()

if(BuildMPRdVanilla)
	add_subdirectory(rd-vanilla)
endif(BuildMPRdVanilla)

# Common things between Engine and Dedicated Server
if(BuildMPEngine OR BuildMPDed)
	add_subdirectory(botlib)

	# Common files/libraries/defines of both Engine and Dedicated Server
	set(MPEngineAndDedLibraries ${MPBotLib})
	# Platform-specific libraries
	if(WIN32)
		set(MPEngineAndDedLibraries ${MPEngineAndDedLibraries} "winmm" "wsock32")
	endif()

	# Include directories
	# codemp folder, since includes are not always relative in the files
	set(MPEngineAndDedIncludeDirectories
		${MPDir}
		${SharedDir}
		${GSLIncludeDirectory})

	# Transparently use our bundled minizip.
	list(APPEND MPEngineAndDedIncludeDirectories ${MINIZIP_INCLUDE_DIRS})
	list(APPEND MPEngineAndDedLibraries          ${MINIZIP_LIBRARIES})

	# Transparently use either bundled or system zlib.
	list(APPEND MPEngineAndDedIncludeDirectories ${ZLIB_INCLUDE_DIR})
	list(APPEND MPEngineAndDedLibraries          ${ZLIB_LIBRARIES})

	set(MPEngineAndDedCgameFiles
		"${MPDir}/cgame/cg_public.h")
	source_group("cgame" FILES ${MPEngineAndDedCgameFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedCgameFiles})

	set(MPEngineAndDedClientFiles
		"${MPDir}/client/client.h"
		"${MPDir}/client/keys.h"
		"${MPDir}/client/snd_public.h")
	source_group("client" FILES ${MPEngineAndDedClientFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedClientFiles})

	set(MPEngineAndDedGameFiles
		"${MPDir}/game/anims.h"
		"${MPDir}/game/bg_public.h"
		"${MPDir}/game/bg_weapons.h"
		"${MPDir}/botlib/botlib.h"
		"${MPDir}/game/g_public.h"
		"${MPDir}/game/surfaceflags.h")
	source_group("game" FILES ${MPEngineAndDedGameFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedGameFiles})

	set(MPEngineAndDedCommonFiles
		"${MPDir}/qcommon/q_shared.h"
		"${MPDir}/qcommon/cm_load.cpp"
		"${MPDir}/qcommon/cm_local.h"
		"${MPDir}/qcommon/cm_patch.cpp"
		"${MPDir}/qcommon/cm_patch.h"
		"${MPDir}/qcommon/cm_polylib.cpp"
		"${MPDir}/qcommon/cm_polylib.h"
		"${MPDir}/qcommon/cm_public.h"
		"${MPDir}/qcommon/cm_test.cpp"
		"${MPDir}/qcommon/cm_trace.cpp"
		"${MPDir}/qcommon/cmd.cpp"
		"${MPDir}/qcommon/common.cpp"
		"${MPDir}/qcommon/cvar.cpp"
		"${MPDir}/qcommon/disablewarnings.h"
		"${MPDir}/qcommon/files.cpp"
		"${MPDir}/qcommon/game_version.h"
		"${MPDir}/qcommon/GenericParser2.cpp"
		"${MPDir}/qcommon/GenericParser2.h"
		"${MPDir}/qcommon/huffman.cpp"
		"${MPDir}/qcommon/md4.cpp"
		"${MPDir}/qcommon/md5.cpp"
		"${MPDir}/qcommon/md5.h"
		"${MPDir}/qcommon/MiniHeap.h"
		"${MPDir}/qcommon/msg.cpp"
		"${MPDir}/qcommon/matcomp.h"
		"${MPDir}/qcommon/matcomp.cpp"
		"${MPDir}/qcommon/net_chan.cpp"
		"${MPDir}/qcommon/net_ip.cpp"
		"${MPDir}/qcommon/persistence.cpp"
		"${MPDir}/qcommon/q_shared.cpp"
		"${MPDir}/qcommon/qcommon.h"
		"${MPDir}/qcommon/qfiles.h"
		"${MPDir}/qcommon/RoffSystem.cpp"
		"${MPDir}/qcommon/RoffSystem.h"
		"${MPDir}/qcommon/sstring.h"
		"${MPDir}/qcommon/stringed_ingame.cpp"
		"${MPDir}/qcommon/stringed_ingame.h"
		"${MPDir}/qcommon/stringed_interface.cpp"
		"${MPDir}/qcommon/stringed_interface.h"
		"${MPDir}/qcommon/tags.h"
		"${MPDir}/qcommon/timing.h"
		"${MPDir}/qcommon/vm.cpp"
		"${MPDir}/qcommon/z_memman_pc.cpp"
		${SharedCommonFiles})
	source_group("common" FILES ${MPEngineAndDedCommonFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedCommonFiles})
	
	set(MPEngineAndDedCommonSafeFiles
		"${SharedDir}/qcommon/safe/files.cpp"
		"${SharedDir}/qcommon/safe/files.h"
		"${SharedDir}/qcommon/safe/gsl.h")
	source_group("common/safe" FILES ${MPEngineAndDedCommonSafeFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedCommonSafeFiles})

	set(MPEngineAndDedG2Files
		"${MPDir}/ghoul2/G2.h"
		"${MPDir}/ghoul2/G2_gore.h"
		"${MPDir}/ghoul2/ghoul2_shared.h"
		"${MPDir}/ghoul2/g2_local.h")
	source_group("ghoul2" FILES ${MPEngineAndDedG2Files})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedG2Files})

	set(MPEngineAndDedIcarusFiles
		"${MPDir}/icarus/BlockStream.cpp"
		"${MPDir}/icarus/blockstream.h"
		"${MPDir}/icarus/GameInterface.cpp"
		"${MPDir}/icarus/GameInterface.h"
		"${MPDir}/icarus/icarus.h"
		"${MPDir}/icarus/Instance.cpp"
		"${MPDir}/icarus/instance.h"
		"${MPDir}/icarus/Interface.cpp"
		"${MPDir}/icarus/interface.h"
		"${MPDir}/icarus/interpreter.h"
		"${MPDir}/icarus/Memory.cpp"
		"${MPDir}/icarus/Q3_Interface.cpp"
		"${MPDir}/icarus/Q3_Interface.h"
		"${MPDir}/icarus/Q3_Registers.cpp"
		"${MPDir}/icarus/Q3_Registers.h"
		"${MPDir}/icarus/Sequence.cpp"
		"${MPDir}/icarus/sequence.h"
		"${MPDir}/icarus/Sequencer.cpp"
		"${MPDir}/icarus/sequencer.h"
		"${MPDir}/icarus/TaskManager.cpp"
		"${MPDir}/icarus/taskmanager.h"
		"${MPDir}/icarus/tokenizer.h")
	source_group("icarus" FILES ${MPEngineAndDedIcarusFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedIcarusFiles})

	set(MPEngineAndDedServerFiles
		"${MPDir}/server/NPCNav/navigator.cpp"
		"${MPDir}/server/NPCNav/navigator.h"
		"${MPDir}/server/server.h"
		"${MPDir}/server/sv_bot.cpp"
		"${MPDir}/server/sv_ccmds.cpp"
		"${MPDir}/server/sv_challenge.cpp"
		"${MPDir}/server/sv_client.cpp"
		"${MPDir}/server/sv_game.cpp"
		"${MPDir}/server/sv_init.cpp"
		"${MPDir}/server/sv_main.cpp"
		"${MPDir}/server/sv_net_chan.cpp"
		"${MPDir}/server/sv_snapshot.cpp"
		"${MPDir}/server/sv_world.cpp"
		"${MPDir}/server/sv_gameapi.cpp"
		"${MPDir}/server/sv_gameapi.h")
	source_group("server" FILES ${MPEngineAndDedServerFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedServerFiles})

	set(MPEngineAndDedUiFiles
		"${MPDir}/ui/keycodes.h"
		"${MPDir}/ui/ui_public.h")
	source_group("ui" FILES ${MPEngineAndDedUiFiles})
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedUiFiles})

	set(MPEngineAndDedSysFiles
		"${SharedDir}/sys/snapvector.cpp")
	set(MPEngineAndDedFiles ${MPEngineAndDedFiles} ${MPEngineAndDedSysFiles})
	source_group("sys" FILES ${MPEngineAndDedSysFiles})
endif()

# Engine/Executable (jamp.exe)
if(BuildMPEngine)
	set(MPEngineLibraries ${MPEngineAndDedLibraries})
	set(MPEngineIncludeDirectories ${MPEngineAndDedIncludeDirectories})
	set(MPEngineFiles ${MPEngineAndDedFiles})
	set(MPEngineDefines ${MPSharedDefines})

	# Dependencies

	if(NOT WIN32)
		# OpenGL
		find_package(OpenGL REQUIRED)
		set(MPEngineIncludeDirectories ${MPEngineIncludeDirectories} ${OPENGL_INCLUDE_DIR})
		set(MPEngineLibraries ${MPEngineLibraries} ${OPENGL_LIBRARIES})
	endif()

	# OpenAL (is optionally included for Windows)
	if(MSVC AND NOT WIN64)
		if(UseInternalOpenAL)
			# There's a libs/OpenAL32.lib
			# (mrw) TODO Is this actually required on windows?
			# It is loaded at runtime, should not be linked?
			find_library(OpenALLibrary NAMES OpenAL32 PATHS ${OpenJKLibDir})
			if(NOT OpenALLibrary)
				message(FATAL_ERROR "UseInternalOpenAL enabled, but lib/OpenAL32.lib not found!")
			endif(NOT OpenALLibrary)
			set(MPEngineLibraries ${MPEngineLibraries} ${OpenALLibrary})
		else()
			find_package(OpenAL REQUIRED)
			set(MPEngineIncludeDirectories ${MPEngineIncludeDirectories} ${OPENAL_INCLUDE_DIR})
			set(MPEngineLibraries ${MPEngineLibraries} ${OPENAL_LIBRARY})
		endif()
	endif()

	if(UseInternalSDL2)
		if(NOT BUILD_64BIT)
			set(MPEngineLibraries
				${MPEngineLibraries}
				${OpenJKLibDir}/SDL2/lib/x86/SDL2main.lib
				${OpenJKLibDir}/SDL2/lib/x86/SDL2.lib
				)
		else()
			set(MPEngineLibraries
				${MPEngineLibraries}
				${OpenJKLibDir}/SDL2/lib/x64/SDL2main.lib
				${OpenJKLibDir}/SDL2/lib/x64/SDL2.lib
				)
		endif()

		set(MPEngineIncludeDirectories
			${MPEngineIncludeDirectories}
			${OpenJKLibDir}/SDL2/include)
	else()
		find_package(SDL2 REQUIRED)
		set(MPEngineIncludeDirectories ${MPEngineIncludeDirectories} ${SDL2_INCLUDE_DIR})
		set(MPEngineLibraries ${MPEngineLibraries} ${SDL2_LIBRARY})
	endif()

	# EAX is Windows-Only (right?)
	if(MSVC)
		set(MPEngineEAXFiles
			"${MPDir}/client/eax/eax.h"
			"${MPDir}/client/eax/EaxMan.h")
		source_group("eax" FILES ${MPEngineEAXFiles})
		set(MPEngineFiles ${MPEngineFiles} ${MPEngineEAXFiles})
	endif()

	# Source Files
	set(MPEngineClientFiles
		"${MPDir}/client/cl_avi.cpp"
		"${MPDir}/client/cl_cgame.cpp"
		"${MPDir}/client/cl_cgameapi.cpp"
		"${MPDir}/client/cl_cgameapi.h"
		"${MPDir}/client/cl_cin.cpp"
		"${MPDir}/client/cl_console.cpp"
		"${MPDir}/client/cl_input.cpp"
		"${MPDir}/client/cl_keys.cpp"
		"${MPDir}/client/cl_lan.cpp"
		"${MPDir}/client/cl_lan.h"
		"${MPDir}/client/cl_main.cpp"
		"${MPDir}/client/cl_net_chan.cpp"
		"${MPDir}/client/cl_parse.cpp"
		"${MPDir}/client/cl_scrn.cpp"
		"${MPDir}/client/cl_ui.cpp"
		"${MPDir}/client/cl_uiapi.cpp"
		"${MPDir}/client/cl_uiapi.h"
		"${MPDir}/client/FXExport.cpp"
		"${MPDir}/client/FXExport.h"
		"${MPDir}/client/FxPrimitives.cpp"
		"${MPDir}/client/FxPrimitives.h"
		"${MPDir}/client/FxScheduler.cpp"
		"${MPDir}/client/FxScheduler.h"
		"${MPDir}/client/FxSystem.cpp"
		"${MPDir}/client/FxSystem.h"
		"${MPDir}/client/FxTemplate.cpp"
		"${MPDir}/client/FxUtil.cpp"
		"${MPDir}/client/FxUtil.h"
		"${MPDir}/client/snd_ambient.cpp"
		"${MPDir}/client/snd_ambient.h"
		"${MPDir}/client/snd_dma.cpp"
		"${MPDir}/client/snd_local.h"
		"${MPDir}/client/snd_mem.cpp"
		"${MPDir}/client/snd_mix.cpp"
		"${MPDir}/client/snd_mp3.cpp"
		"${MPDir}/client/snd_mp3.h"
		"${MPDir}/client/snd_music.cpp"
		"${MPDir}/client/snd_music.h")
	source_group("client" FILES ${MPEngineClientFiles})
	set(MPEngineFiles ${MPEngineFiles} ${MPEngineClientFiles})

	set(MPEngineRatlFiles
		"${MPDir}/Ratl/bits_vs.h"
		"${MPDir}/Ratl/ratl_common.h"
		"${MPDir}/Ratl/vector_vs.h")
	source_group("ratl" FILES ${MPEngineRatlFiles})
	set(MPEngineFiles ${MPEngineFiles} ${MPEngineRatlFiles})

	set(MPEngineRavlFiles
		"${MPDir}/Ravl/CVec.h")
	source_group("ravl" FILES ${MPEngineRavlFiles})
	set(MPEngineFiles ${MPEngineFiles} ${MPEngineRavlFiles})

	set(MPEngineMp3Files
		"${MPDir}/mp3code/cdct.c"
		"${MPDir}/mp3code/config.h"
		"${MPDir}/mp3code/csbt.c"
		"${MPDir}/mp3code/csbtb.c"
		"${MPDir}/mp3code/csbtl3.c"
		"${MPDir}/mp3code/cup.c"
		"${MPDir}/mp3code/cupini.c"
		"${MPDir}/mp3code/cupl1.c"
		"${MPDir}/mp3code/cupl3.c"
		"${MPDir}/mp3code/cwin.c"
		"${MPDir}/mp3code/cwinb.c"
		"${MPDir}/mp3code/cwinm.c"
		"${MPDir}/mp3code/htable.h"
		"${MPDir}/mp3code/hwin.c"
		"${MPDir}/mp3code/jdw.h"
		"${MPDir}/mp3code/l3.h"
		"${MPDir}/mp3code/l3dq.c"
		"${MPDir}/mp3code/l3init.c"
		"${MPDir}/mp3code/mdct.c"
		"${MPDir}/mp3code/mhead.c"
		"${MPDir}/mp3code/mhead.h"
		"${MPDir}/mp3code/mp3struct.h"
		"${MPDir}/mp3code/msis.c"
		"${MPDir}/mp3code/port.h"
		"${MPDir}/mp3code/small_header.h"
		"${MPDir}/mp3code/tableawd.h"
		"${MPDir}/mp3code/towave.c"
		"${MPDir}/mp3code/uph.c"
		"${MPDir}/mp3code/upsf.c"
		"${MPDir}/mp3code/wavep.c")
	source_group("mp3code" FILES ${MPEngineMp3Files})
	set(MPEngineFiles ${MPEngineFiles} ${MPEngineMp3Files})

	set(MPEngineSysFiles
		"${SharedDir}/sdl/sdl_window.cpp"
		"${SharedDir}/sdl/sdl_icon.h"
		"${SharedDir}/sdl/sdl_input.cpp"
		"${SharedDir}/sdl/sdl_sound.cpp"
		"${SharedDir}/sys/sys_loadlib.h"
		"${SharedDir}/sys/sys_local.h"
		"${SharedDir}/sys/sys_main.cpp"
		"${SharedDir}/sys/sys_event.cpp"
		"${SharedDir}/sys/sys_public.h"
		"${SharedDir}/sys/con_local.h"
		"${SharedDir}/sys/con_log.cpp")

	if(WIN32)
		set(MPEngineWin32Files
			"${MPDir}/win32/icon.ico"
			"${MPDir}/win32/resource.h"
			"${MPDir}/win32/openjk.rc")

		set(MPEngineFiles ${MPEngineFiles} ${MPEngineWin32Files})
		source_group("win32" FILES ${MPEngineWin32Files})

		set(MPEngineSysFiles
			${MPEngineSysFiles}
			"${SharedDir}/sys/sys_win32.cpp"
			"${SharedDir}/sys/con_passive.cpp")
	else()
		set(MPEngineSysFiles
			${MPEngineSysFiles}
			"${SharedDir}/sys/sys_unix.cpp"
			"${SharedDir}/sys/con_tty.cpp"
			)
	endif()

	set(MPEngineFiles ${MPEngineFiles} ${MPEngineSysFiles})
	source_group("sys" FILES ${MPEngineSysFiles})

	if(MakeApplicationBundles)
		set(MACOSX_BUNDLE_BUNDLE_NAME ${MPEngine})
		set(MACOSX_BUNDLE_DISPLAY_NAME ${MPEngine})
		set(MACOSX_BUNDLE_ICON_FILE OpenJK.icns)

		set(MPEngineFiles
			${MPEngineFiles}
			${MPDir}/macosx/OpenJK.icns)
		set_source_files_properties(
			${MPDir}/macosx/OpenJK.icns
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

		set(MPEngineFiles
			${MPEngineFiles}
			${MPDir}/macosx/English.lproj/InfoPlist.strings)
		set_source_files_properties(
			${MPDir}/macosx/English.lproj/InfoPlist.strings
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj)
	endif()

	if(WIN32)
		add_executable(${MPEngine} WIN32 ${MPEngineFiles})
		if(UseInternalSDL2)
			if(NOT BUILD_64BIT)
				set(MPEngineExtraInstallFiles
					${OpenJKLibDir}/SDL2/bin/x86/SDL2.dll)
			else()
				set(MPEngineExtraInstallFiles
					${OpenJKLibDir}/SDL2/bin/x64/SDL2.dll)
			endif()
		endif()
	else()
		if(MakeApplicationBundles)
			add_executable(${MPEngine} MACOSX_BUNDLE ${MPEngineFiles})
			set_target_properties(
				${MPEngine}
				PROPERTIES
					MACOSX_BUNDLE_INFO_PLIST "${MPDir}/macosx/OpenJKInfo.plist")
		else()
			add_executable(${MPEngine} ${MPEngineFiles})
		endif()
	endif()

	target_link_libraries(${MPEngine} ${MPEngineLibraries})
	set_target_properties(${MPEngine} PROPERTIES PROJECT_LABEL "MP Client")
	target_compile_options(${MPEngine} PUBLIC ${OPENJK_VISIBILITY_FLAGS})
	target_compile_definitions(${MPEngine} PUBLIC ${MPEngineDefines})
	target_include_directories(${MPEngine} PUBLIC ${MPEngineIncludeDirectories})

	if(MakeApplicationBundles)
		install(TARGETS ${MPEngine}
			BUNDLE
			DESTINATION ${JKAInstallDir}
			COMPONENT ${JKAMPClientComponent})
	else()
		install(TARGETS ${MPEngine}
			RUNTIME
			DESTINATION ${JKAInstallDir}
			COMPONENT ${JKAMPClientComponent})
		if(MPEngineExtraInstallFiles)
			install(FILES ${MPEngineExtraInstallFiles}
				DESTINATION ${JKAInstallDir}
				COMPONENT ${JKAMPClientComponent})
		endif()
	endif()
endif()

# Dedicated Server (Engine) (jampded.exe)

if(BuildMPDed)
	set(MPDedLibraries ${MPEngineAndDedLibraries} ${CMAKE_DL_LIBS})
	set(MPDedIncludeDirectories ${MPEngineAndDedIncludeDirectories})
	set(MPDedFiles ${MPEngineAndDedFiles})
	set(MPDedDefines ${MPSharedDefines} "_CONSOLE" "DEDICATED")

	# Dedicated renderer is compiled with the server.
	set(MPDedicatedRendererFiles
		"${MPDir}/ghoul2/G2_gore.cpp"
		"${MPDir}/rd-common/mdx_format.h"
		"${MPDir}/rd-common/tr_public.h"
		"${MPDir}/rd-dedicated/tr_local.h"
		"${MPDir}/rd-dedicated/G2_API.cpp"
		"${MPDir}/rd-dedicated/G2_bolts.cpp"
		"${MPDir}/rd-dedicated/G2_bones.cpp"
		"${MPDir}/rd-dedicated/G2_misc.cpp"
		"${MPDir}/rd-dedicated/G2_surfaces.cpp"
		"${MPDir}/rd-dedicated/tr_backend.cpp"
		"${MPDir}/rd-dedicated/tr_ghoul2.cpp"
		"${MPDir}/rd-dedicated/tr_init.cpp"
		"${MPDir}/rd-dedicated/tr_main.cpp"
		"${MPDir}/rd-dedicated/tr_mesh.cpp"
		"${MPDir}/rd-dedicated/tr_model.cpp"
		"${MPDir}/rd-dedicated/tr_shader.cpp"
		"${MPDir}/rd-dedicated/tr_skin.cpp")
	source_group("renderer" FILES ${MPDedicatedRendererFiles})
	set(MPDedFiles ${MPDedFiles} ${MPDedicatedRendererFiles})

	set(MPDedNullFiles
		"${MPDir}/null/null_client.cpp"
		"${MPDir}/null/null_input.cpp"
		"${MPDir}/null/null_renderer.cpp"
		"${MPDir}/null/null_snddma.cpp")
	source_group("null" FILES ${MPDedNullFiles})
	set(MPDedFiles ${MPDedFiles} ${MPDedNullFiles})

	if(WIN32)
		set(MPDedWin32Files
			"${MPDir}/win32/openjkded.rc")
		source_group("win32" FILES ${MPDedWin32Files})
		set(MPDedFiles ${MPDedFiles} ${MPDedWin32Files})

		set(MPDedSysFiles
			"${SharedDir}/sys/sys_loadlib.h"
			"${SharedDir}/sys/sys_local.h"
			"${SharedDir}/sys/sys_main.cpp"
			"${SharedDir}/sys/sys_event.cpp"
			"${SharedDir}/sys/sys_public.h"
			"${SharedDir}/sys/sys_win32.cpp"
			"${SharedDir}/sys/con_local.h"
			"${SharedDir}/sys/con_log.cpp"
			"${SharedDir}/sys/con_win32.cpp")
	else()
		set(MPDedSysFiles
			"${SharedDir}/sys/sys_loadlib.h"
			"${SharedDir}/sys/sys_local.h"
			"${SharedDir}/sys/sys_main.cpp"
			"${SharedDir}/sys/sys_event.cpp"
			"${SharedDir}/sys/sys_public.h"
			"${SharedDir}/sys/sys_unix.cpp"
			"${SharedDir}/sys/con_local.h"
			"${SharedDir}/sys/con_log.cpp"
			"${SharedDir}/sys/con_tty.cpp")
	endif()
	source_group("sys" FILES ${MPDedSysFiles})
	set(MPDedFiles ${MPDedFiles} ${MPDedSysFiles})

	add_executable(${MPDed} ${MPDedFiles})

	set_target_properties(${MPDed} PROPERTIES PROJECT_LABEL "MP Dedicated Server")
	target_link_libraries(${MPDed} ${MPDedLibraries})
	target_compile_options(${MPDed} PUBLIC ${OPENJK_VISIBILITY_FLAGS})
	target_compile_definitions(${MPDed} PUBLIC ${MPDedDefines})
	target_include_directories(${MPDed} PUBLIC ${MPDedIncludeDirectories})

	install(TARGETS ${MPDed}
		RUNTIME
		DESTINATION ${JKAInstallDir}
		COMPONENT ${JKAMPServerComponent})
endif()

set(GameLibsBuilt)
if(BuildMPGame)
	set(GameLibsBuilt ${GameLibsBuilt} ${MPGame})
endif()

if(BuildMPCGame)
	set(GameLibsBuilt ${GameLibsBuilt} ${MPCGame})
endif()

if(BuildMPUI)
	set(GameLibsBuilt ${GameLibsBuilt} ${MPUI})
endif()

if(WIN32)
	set(GameLibFullPaths)
	if(MSVC)
		foreach(GameLib ${GameLibsBuilt})
			set(GameLibFullPaths
				${GameLibFullPaths}
				${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${GameLib}${CMAKE_SHARED_LIBRARY_SUFFIX})
		endforeach(GameLib)
	else()
		foreach(GameLib ${GameLibsBuilt})
			set(GameLibFullPaths
				${GameLibFullPaths}
				${CMAKE_BINARY_DIR}/${GameLib}${CMAKE_SHARED_LIBRARY_SUFFIX})
		endforeach(GameLib)
	endif()

	include(InstallZIP)
	add_zip_command(${AssetsPk3}
		FILES ${GameLibFullPaths}
		DEPENDS "${GameLibsBuilt}")
	add_custom_target(Assets
		ALL
		DEPENDS ${AssetsPk3})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${AssetsPk3}
		DESTINATION "${JKAInstallDir}/OpenJK"
		COMPONENT ${JKAMPCoreComponent})
endif()
