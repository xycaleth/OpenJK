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

set(SPVanillaRendererIncludeDirectories ${SPDir})
set(SPVanillaRendererIncludeDirectories
	${SharedDir}
	${SPVanillaRendererIncludeDirectories}
	"${SPDir}/rd-common"
	"${SPDir}/rd-vanilla"
	"${GSLIncludeDirectory}")

# Defines
set(SPVanillaDefines ${SharedDefines} "RENDERER" "_JK2EXE")

# GHOUL 2
set(SPVanillaG2Files
	"${SPDir}/ghoul2/G2.h"
	"${SPDir}/ghoul2/ghoul2_gore.h"
	"${SPDir}/rd-vanilla/G2_API.cpp"
	"${SPDir}/rd-vanilla/G2_bolts.cpp"
	"${SPDir}/rd-vanilla/G2_bones.cpp"
	"${SPDir}/rd-vanilla/G2_misc.cpp"
	"${SPDir}/rd-vanilla/G2_surfaces.cpp")
source_group("ghoul2" FILES ${SPVanillaG2Files})
set(SPVanillaFiles ${SPVanillaFiles} ${SPVanillaG2Files})


# Misc files
set(SPVanillaCommonFiles
	"${SPDir}/qcommon/matcomp.cpp"
	"${SPDir}/qcommon/q_shared.cpp"
	"${SPDir}/qcommon/q_shared.h"
	"${SPDir}/qcommon/ojk_i_saved_game.h"
	"${SPDir}/qcommon/ojk_saved_game_class_archivers.h"
	"${SPDir}/qcommon/ojk_saved_game_helper.h"
	"${SPDir}/qcommon/ojk_saved_game_helper_fwd.h"
	"${SPDir}/qcommon/ojk_scope_guard.h"
	${SharedCommonFiles}
	)
source_group("common" FILES ${SPVanillaCommonFiles})
set(SPVanillaFiles ${SPVanillaFiles} ${SPVanillaCommonFiles})

# Misc files
set(SPVanillaCommonSafeFiles
	${SharedCommonSafeFiles}
	)
source_group("common/safe" FILES ${SPVanillaCommonSafeFiles})
set(SPVanillaFiles ${SPVanillaFiles} ${SPVanillaCommonSafeFiles})

# main files
set(SPVanillaMainFiles
	"${SPDir}/rd-vanilla/glext.h"
	"${SPDir}/rd-vanilla/qgl.h"
	"${SPDir}/rd-vanilla/tr_arb.cpp"
	"${SPDir}/rd-vanilla/tr_backend.cpp"
	"${SPDir}/rd-vanilla/tr_bsp.cpp"
	"${SPDir}/rd-vanilla/tr_cmds.cpp"
	"${SPDir}/rd-vanilla/tr_curve.cpp"
	"${SPDir}/rd-vanilla/tr_draw.cpp"
	"${SPDir}/rd-vanilla/tr_ghoul2.cpp"
	"${SPDir}/rd-vanilla/tr_image.cpp"
	"${SPDir}/rd-vanilla/tr_init.cpp"
	"${SPDir}/rd-vanilla/tr_light.cpp"
	"${SPDir}/rd-vanilla/tr_local.h"
	"${SPDir}/rd-vanilla/tr_main.cpp"
	"${SPDir}/rd-vanilla/tr_marks.cpp"
	"${SPDir}/rd-vanilla/tr_mesh.cpp"
	"${SPDir}/rd-vanilla/tr_model.cpp"
	"${SPDir}/rd-vanilla/tr_quicksprite.cpp"
	"${SPDir}/rd-vanilla/tr_quicksprite.h"
	"${SPDir}/rd-vanilla/tr_scene.cpp"
	"${SPDir}/rd-vanilla/tr_shade.cpp"
	"${SPDir}/rd-vanilla/tr_shade_calc.cpp"
	"${SPDir}/rd-vanilla/tr_shader.cpp"
	"${SPDir}/rd-vanilla/tr_shadows.cpp"
	"${SPDir}/rd-vanilla/tr_skin.cpp"
	"${SPDir}/rd-vanilla/tr_sky.cpp"
	"${SPDir}/rd-vanilla/tr_stl.cpp"
	"${SPDir}/rd-vanilla/tr_stl.h"
	"${SPDir}/rd-vanilla/tr_subs.cpp"
	"${SPDir}/rd-vanilla/tr_surface.cpp"
	"${SPDir}/rd-vanilla/tr_surfacesprites.cpp"
	"${SPDir}/rd-vanilla/tr_world.cpp"
	"${SPDir}/rd-vanilla/tr_WorldEffects.cpp"
	"${SPDir}/rd-vanilla/tr_WorldEffects.h"
	)
source_group("renderer" FILES ${SPVanillaMainFiles})
set(SPVanillaFiles ${SPVanillaFiles} ${SPVanillaMainFiles})

set(SPVanillaRdCommonFiles
	"${SPDir}/rd-common/mdx_format.h"
	"${SPDir}/rd-common/tr_common.h"
	"${SPDir}/rd-common/tr_font.cpp"
	"${SPDir}/rd-common/tr_font.h"
	"${SPDir}/rd-common/tr_image_load.cpp"
	"${SPDir}/rd-common/tr_image_jpg.cpp"
	"${SPDir}/rd-common/tr_image_tga.cpp"
	"${SPDir}/rd-common/tr_image_png.cpp"
	"${SPDir}/rd-common/tr_noise.cpp"
	"${SPDir}/rd-common/tr_public.h"
	"${SPDir}/rd-common/tr_types.h")
source_group("rd-common" FILES ${SPVanillaRdCommonFiles})
set(SPVanillaFiles ${SPVanillaFiles} ${SPVanillaRdCommonFiles})

find_package(OpenGL REQUIRED)
list(APPEND SPVanillaRendererLibraries ${OPENGL_LIBRARIES})
list(APPEND SPVanillaRendererIncludeDirectories ${OPENGL_INCLUDE_DIR})

list(APPEND SPVanillaRendererLibraries ${JPEG_LIBRARIES})
list(APPEND SPVanillaRendererIncludeDirectories ${JPEGL_INCLUDE_DIR})

list(APPEND SPVanillaRendererLibraries ${PNG_LIBRARIES})
list(APPEND SPVanillaRendererIncludeDirectories ${PNG_INCLUDE_DIRS})

list(APPEND SPVanillaRendererIncludeDirectories ${OpenJKLibDir})

function(add_sp_renderer_project ProjectName Label EngineName InstallDir Component)
	add_library(${ProjectName} MODULE ${SPVanillaFiles})

	set_target_properties(${ProjectName} PROPERTIES PROJECT_LABEL ${Label})
	target_link_libraries(${ProjectName} PRIVATE ${SPVanillaRendererLibraries})
	target_compile_options(${ProjectName} PRIVATE ${OPENJK_VISIBILITY_FLAGS})
	target_compile_definitions(${ProjectName} PRIVATE ${SPVanillaDefines})
	target_include_directories(${ProjectName} PRIVATE ${SPVanillaRendererIncludeDirectories})

	if(NOT MSVC)
		set_target_properties(${ProjectName} PROPERTIES PREFIX "")
	endif()

	if(APPLE)
		set_target_properties(${ProjectName} PROPERTIES SUFFIX ".dylib")
	endif()

	if(WIN32)
		install(TARGETS ${ProjectName}
			RUNTIME
			DESTINATION ${InstallDir}
			COMPONENT ${Component})
	else()
		if(BUILD_APP_BUNDLES AND (BUILD_JASP_CLIENT OR BUILD_JOSP_CLIENT))
			install(TARGETS ${ProjectName}
				LIBRARY
				DESTINATION "${InstallDir}/${EngineName}.app/Contents/MacOS/"
				COMPONENT ${Component})
		else()
			install(TARGETS ${ProjectName}
				LIBRARY
				DESTINATION ${InstallDir}
				COMPONENT ${Component})
		endif()
	endif()

endfunction()

if(BUILD_JASP_RENDERER)
	add_sp_renderer_project(
		${SPVanillaRenderer}
		"SP Vanilla Renderer"
		${SPEngine}
		${JKAInstallDir}
		${JKASPClientComponent})
endif()

if(BUILD_JOSP_RENDERER)
	set(SPVanillaDefines ${SPVanillaDefines} "JK2_MODE")
	add_sp_renderer_project(
		${JK2SPVanillaRenderer}
		"JK2 SP Vanilla Renderer"
		${JK2SPEngine}
		${JK2InstallDir}
		${JK2SPClientComponent})
endif()
