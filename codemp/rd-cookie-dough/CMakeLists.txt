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

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(MPCookieDoughRendererIncludeDirectories ${MPDir})
set(MPCookieDoughRendererIncludeDirectories
	${MPCookieDoughRendererIncludeDirectories}
	${SharedDir}
	${MPDir}
	"${MPDir}/rd-cookie-dough"
	"${GSLIncludeDirectory}"
	)

set(MPCookieDoughRendererFiles
	"${MPDir}/rd-cookie-dough/glad.c"
	"${MPDir}/rd-cookie-dough/glad.h"
	"${MPDir}/rd-cookie-dough/khrplatform.h"
	"${MPDir}/rd-cookie-dough/G2_API.cpp"
	"${MPDir}/rd-cookie-dough/G2_bolts.cpp"
	"${MPDir}/rd-cookie-dough/G2_bones.cpp"
	"${MPDir}/rd-cookie-dough/G2_misc.cpp"
	"${MPDir}/rd-cookie-dough/G2_surfaces.cpp"
	"${MPDir}/rd-cookie-dough/tr_arb.cpp"
	"${MPDir}/rd-cookie-dough/tr_backend.cpp"
	"${MPDir}/rd-cookie-dough/tr_bsp.cpp"
	"${MPDir}/rd-cookie-dough/tr_cmds.cpp"
	"${MPDir}/rd-cookie-dough/tr_curve.cpp"
	"${MPDir}/rd-cookie-dough/tr_decals.cpp"
	"${MPDir}/rd-cookie-dough/tr_ghoul2.cpp"
	"${MPDir}/rd-cookie-dough/tr_glsl.cpp"
	"${MPDir}/rd-cookie-dough/tr_glsl.h"
	"${MPDir}/rd-cookie-dough/tr_image.cpp"
	"${MPDir}/rd-cookie-dough/tr_init.cpp"
	"${MPDir}/rd-cookie-dough/tr_light.cpp"
	"${MPDir}/rd-cookie-dough/tr_local.h"
	"${MPDir}/rd-cookie-dough/tr_main.cpp"
	"${MPDir}/rd-cookie-dough/tr_marks.cpp"
	"${MPDir}/rd-cookie-dough/tr_mesh.cpp"
	"${MPDir}/rd-cookie-dough/tr_model.cpp"
	"${MPDir}/rd-cookie-dough/tr_quicksprite.cpp"
	"${MPDir}/rd-cookie-dough/tr_quicksprite.h"
	"${MPDir}/rd-cookie-dough/tr_scene.cpp"
	"${MPDir}/rd-cookie-dough/tr_shade.cpp"
	"${MPDir}/rd-cookie-dough/tr_shade_calc.cpp"
	"${MPDir}/rd-cookie-dough/tr_shader.cpp"
	"${MPDir}/rd-cookie-dough/tr_shadows.cpp"
	"${MPDir}/rd-cookie-dough/tr_skin.cpp"
	"${MPDir}/rd-cookie-dough/tr_sky.cpp"
	"${MPDir}/rd-cookie-dough/tr_subs.cpp"
	"${MPDir}/rd-cookie-dough/tr_surface.cpp"
	"${MPDir}/rd-cookie-dough/tr_surfacesprites.cpp"
	"${MPDir}/rd-cookie-dough/tr_world.cpp"
	"${MPDir}/rd-cookie-dough/tr_WorldEffects.cpp"
	"${MPDir}/rd-cookie-dough/tr_WorldEffects.h"
	)
source_group("renderer" FILES ${MPCookieDoughRendererFiles})

set(MPCookieDoughRendererGhoul2Files
	"${MPDir}/ghoul2/g2_local.h"
	"${MPDir}/ghoul2/ghoul2_shared.h"
	"${MPDir}/ghoul2/G2_gore.cpp"
	"${MPDir}/ghoul2/G2_gore.h")
source_group("ghoul2" FILES ${MPCookieDoughRendererGhoul2Files})
set(MPCookieDoughRendererFiles ${MPCookieDoughRendererFiles} ${MPCookieDoughRendererGhoul2Files})

set(MPCookieDoughRendererRdCommonFiles
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
source_group("rd-common" FILES ${MPCookieDoughRendererRdCommonFiles})
set(MPCookieDoughRendererFiles ${MPCookieDoughRendererFiles} ${MPCookieDoughRendererRdCommonFiles})

set(MPCookieDoughRendererCommonFiles
	"${MPDir}/qcommon/matcomp.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	
	${SharedCommonFiles})
source_group("common" FILES ${MPCookieDoughRendererCommonFiles})
set(MPCookieDoughRendererFiles ${MPCookieDoughRendererFiles} ${MPCookieDoughRendererCommonFiles})

set(MPCookieDoughRendererCommonSafeFiles
	${SharedCommonSafeFiles}
	)
source_group("common/safe" FILES ${MPCookieDoughRendererCommonSafeFiles})
set(MPCookieDoughRendererFiles ${MPCookieDoughRendererFiles} ${MPCookieDoughRendererCommonSafeFiles})

# Transparently use either bundled or system libjpeg.
list(APPEND MPCookieDoughRendererIncludeDirectories ${JPEG_INCLUDE_DIR})
list(APPEND MPCookieDoughRendererLibraries          ${JPEG_LIBRARIES})

# Transparently use either bundled or system libpng.  Order is important --
# libpng used zlib, so it must come before it on the linker command line.
list(APPEND MPCookieDoughRendererIncludeDirectories ${PNG_INCLUDE_DIRS})
list(APPEND MPCookieDoughRendererLibraries          ${PNG_LIBRARIES})

# Transparently use either bundled or system zlib.
list(APPEND MPCookieDoughRendererIncludeDirectories ${ZLIB_INCLUDE_DIR})
list(APPEND MPCookieDoughRendererLibraries          ${ZLIB_LIBRARIES})

# Transparently use our bundled minizip.
list(APPEND MPCookieDoughRendererIncludeDirectories ${MINIZIP_INCLUDE_DIRS})
list(APPEND MPCookieDoughRendererLibraries          ${MINIZIP_LIBRARIES})

find_package(OpenGL REQUIRED)
set(MPCookieDoughRendererIncludeDirectories ${MPCookieDoughRendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
set(MPCookieDoughRendererLibraries ${MPCookieDoughRendererLibraries} ${OPENGL_LIBRARIES})

set(MPCookieDoughRendererIncludeDirectories ${MPCookieDoughRendererIncludeDirectories} ${OpenJKLibDir})
add_library(${MPCookieDoughRenderer} SHARED ${MPCookieDoughRendererFiles})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPCookieDoughRenderer} PROPERTIES PREFIX "")
endif()

if(WIN32)
	install(TARGETS ${MPCookieDoughRenderer}
		RUNTIME
		DESTINATION ${JKAInstallDir}
		COMPONENT ${JKAMPClientComponent})
else(WIN32)
	if(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${MPCookieDoughRenderer}
			LIBRARY
			DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/"
			COMPONENT ${JKAMPClientComponent})
	else()
		install(TARGETS ${MPCookieDoughRenderer}
			LIBRARY
			DESTINATION ${JKAInstallDir}
			COMPONENT ${JKAMPClientComponent})
	endif()
endif()

set_target_properties(${MPCookieDoughRenderer} PROPERTIES COMPILE_DEFINITIONS "${SharedDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPCookieDoughRenderer} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

set_target_properties(${MPCookieDoughRenderer} PROPERTIES INCLUDE_DIRECTORIES "${MPCookieDoughRendererIncludeDirectories}")
set_target_properties(${MPCookieDoughRenderer} PROPERTIES PROJECT_LABEL "MP Cookie Dough Renderer")
target_link_libraries(${MPCookieDoughRenderer} ${MPCookieDoughRendererLibraries})
