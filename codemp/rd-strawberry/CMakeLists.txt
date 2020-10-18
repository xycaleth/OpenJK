#============================================================================
# Copyright (C) 2019, OpenJK contributors
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

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

# Check for dependencies first
set(MPStrawberryRenderer rd-strawberry_${Architecture})

set(MPStrawberryRendererIncludeDirectories ${MPDir})
set(MPStrawberryRendererIncludeDirectories
	${MPStrawberryRendererIncludeDirectories}
	${SharedDir}
	${MPDir}
	"${MPDir}/rd-strawberry"
	"${GSLIncludeDirectory}"
	)

set(MPStrawberryRendererFiles
	glext.h
	qgl.h
	G2_API.cpp
	G2_bolts.cpp
	G2_bones.cpp
	G2_misc.cpp
	G2_surfaces.cpp
	tr_arb.cpp
	tr_backend.cpp
	tr_bsp.cpp
	tr_cmds.cpp
	tr_curve.cpp
	tr_decals.cpp
	tr_ghoul2.cpp
	tr_gpu.cpp
	tr_gpu.h
	tr_image.cpp
	tr_init.cpp
	tr_light.cpp
	tr_local.h
	tr_main.cpp
	tr_marks.cpp
	tr_mesh.cpp
	tr_model.cpp
	tr_quicksprite.cpp
	tr_quicksprite.h
	tr_scene.cpp
	tr_shade.cpp
	tr_shade_calc.cpp
	tr_shader.cpp
	tr_shadows.cpp
	tr_skin.cpp
	tr_sky.cpp
	tr_subs.cpp
	tr_surface.cpp
	tr_surfacesprites.cpp
    tr_vertex_formats.cpp
    tr_vertex_formats.h
	tr_world.cpp
	tr_WorldEffects.cpp
	tr_WorldEffects.h)
source_group("renderer" FILES ${MPStrawberryRendererFiles})

set(MPStrawberryRendererGhoul2Files
	"${MPDir}/ghoul2/g2_local.h"
	"${MPDir}/ghoul2/ghoul2_shared.h"
	"${MPDir}/ghoul2/G2_gore.cpp"
	"${MPDir}/ghoul2/G2_gore.h")
source_group("ghoul2" FILES ${MPStrawberryRendererGhoul2Files})
set(MPStrawberryRendererFiles ${MPStrawberryRendererFiles} ${MPStrawberryRendererGhoul2Files})

set(MPStrawberryRendererRdCommonFiles
	"${MPDir}/rd-common/mdx_format.h"
	"${MPDir}/rd-common/tr_common.h"
	"${MPDir}/rd-common/tr_font.cpp"
	"${MPDir}/rd-common/tr_font.h"
	"${MPDir}/rd-common/tr_image_load.cpp"
	"${MPDir}/rd-common/tr_image_jpg.cpp"
	"${MPDir}/rd-common/tr_image_tga.cpp"
	"${MPDir}/rd-common/tr_image_png.cpp"
	"${MPDir}/rd-common/tr_noise.cpp"
	"${MPDir}/rd-common/tr_public.h"
	"${MPDir}/rd-common/tr_types.h")
source_group("rd-common" FILES ${MPStrawberryRendererRdCommonFiles})
set(MPStrawberryRendererFiles ${MPStrawberryRendererFiles} ${MPStrawberryRendererRdCommonFiles})

set(MPStrawberryRendererCommonFiles
	"${MPDir}/qcommon/matcomp.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	
	${SharedCommonFiles})
source_group("common" FILES ${MPStrawberryRendererCommonFiles})
set(MPStrawberryRendererFiles ${MPStrawberryRendererFiles} ${MPStrawberryRendererCommonFiles})

set(MPStrawberryRendererCommonSafeFiles
	${SharedCommonSafeFiles}
	)
source_group("common/safe" FILES ${MPStrawberryRendererCommonSafeFiles})
set(MPStrawberryRendererFiles ${MPStrawberryRendererFiles} ${MPStrawberryRendererCommonSafeFiles})

# Transparently use either bundled or system libjpeg.
list(APPEND MPStrawberryRendererIncludeDirectories ${JPEG_INCLUDE_DIR})
list(APPEND MPStrawberryRendererLibraries          ${JPEG_LIBRARIES})

# Transparently use either bundled or system libpng.  Order is important --
# libpng used zlib, so it must come before it on the linker command line.
list(APPEND MPStrawberryRendererIncludeDirectories ${PNG_INCLUDE_DIRS})
list(APPEND MPStrawberryRendererLibraries          ${PNG_LIBRARIES})

# Transparently use either bundled or system zlib.
list(APPEND MPStrawberryRendererIncludeDirectories ${ZLIB_INCLUDE_DIR})
list(APPEND MPStrawberryRendererLibraries          ${ZLIB_LIBRARIES})

# Transparently use our bundled minizip.
list(APPEND MPStrawberryRendererIncludeDirectories ${MINIZIP_INCLUDE_DIRS})
list(APPEND MPStrawberryRendererLibraries          ${MINIZIP_LIBRARIES})

find_package(OpenGL REQUIRED)
set(MPStrawberryRendererIncludeDirectories ${MPStrawberryRendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
set(MPStrawberryRendererLibraries ${MPStrawberryRendererLibraries} ${OPENGL_LIBRARIES})

set(MPStrawberryRendererIncludeDirectories ${MPStrawberryRendererIncludeDirectories} ${OpenJKLibDir})

find_package(Vulkan REQUIRED)
add_library(${MPStrawberryRenderer} SHARED ${MPStrawberryRendererFiles})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPStrawberryRenderer} PROPERTIES PREFIX "")
endif()

target_link_libraries(${MPStrawberryRenderer} PRIVATE Vulkan::Vulkan ${MPStrawberryRendererLibraries})
target_compile_options(${MPStrawberryRenderer} PRIVATE ${OPENJK_VISIBILITY_FLAGS})
target_compile_definitions(${MPStrawberryRenderer} PRIVATE ${SharedDefines})
target_include_directories(${MPStrawberryRenderer} PRIVATE ${MPStrawberryRendererIncludeDirectories})

set_target_properties(${MPStrawberryRenderer} PROPERTIES PROJECT_LABEL "MP Strawberry Renderer")


if(WIN32)
	install(TARGETS ${MPStrawberryRenderer}
		RUNTIME
		DESTINATION ${JKAInstallDir}
		COMPONENT ${JKAMPClientComponent})
else()
	if(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${MPStrawberryRenderer}
			LIBRARY
			DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/"
			COMPONENT ${JKAMPClientComponent})
	else()
		install(TARGETS ${MPStrawberryRenderer}
			LIBRARY
			DESTINATION ${JKAInstallDir}
			COMPONENT ${JKAMPClientComponent})
	endif()
endif()
