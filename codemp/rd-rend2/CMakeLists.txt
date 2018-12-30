# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(MPRend2IncludeDirectories
	"${SharedDir}"
	"${MPDir}"
	"${MPDir}/rd-rend2"
	"${GSLIncludeDirectory}"
	)

set(MPRend2Files
	iqm.h
	glext.h
	qgl.h
	G2_API.cpp
	G2_bolts.cpp
	G2_bones.cpp
	G2_misc.cpp
	G2_surfaces.cpp
	tr_allocator.cpp
	tr_allocator.h
	tr_animation.cpp
	tr_backend.cpp
	tr_bsp.cpp
	tr_cache.cpp
	tr_cache.h
	tr_cmds.cpp
	tr_curve.cpp
	tr_decals.cpp
	tr_extensions.cpp
	tr_extramath.cpp
	tr_extramath.h
	tr_extratypes.h
	tr_fbo.cpp
	tr_fbo.h
	tr_flares.cpp
	tr_ghoul2.cpp
	tr_glsl.cpp
	tr_glsl_parse.cpp
	tr_image.cpp
	tr_init.cpp
	tr_light.cpp
	tr_local.h
	tr_main.cpp
	tr_marks.cpp
	tr_mesh.cpp
	tr_model.cpp
	tr_model_iqm.cpp
	tr_postprocess.cpp
	tr_postprocess.h
	tr_scene.cpp
	tr_shade.cpp
	tr_shade_calc.cpp
	tr_shader.cpp
	tr_shadows.cpp
	tr_skin.cpp
	tr_sky.cpp
	tr_subs.cpp
	tr_surface.cpp
	tr_vbo.cpp
	tr_world.cpp
	tr_weather.cpp
	tr_weather.h
		tr_steroids_scene.cpp
		tr_steroids_scene.h
	tr_steroids_render.cpp
	tr_steroids_render.h tr_steroids_cmd.h tr_steroids_cmd.cpp tr_steroids_frame.h tr_steroids_frame.cpp tr_steroids_backend.cpp tr_steroids_backend.h tr_steroids_math.h)
source_group("renderer" FILES ${MPRend2Files})

file(GLOB MPRend2GLSLFiles "${MPDir}/rd-rend2/glsl/*.glsl")
source_group("renderer\\glsl" FILES ${MPRend2GLSLFiles})
set(MPRend2Files ${MPRend2Files} ${MPRend2GLSLFiles})

set(MPRend2Ghoul2Files
	"${MPDir}/ghoul2/g2_local.h"
	"${MPDir}/ghoul2/ghoul2_shared.h"
	"${MPDir}/ghoul2/G2_gore.cpp"
	"${MPDir}/ghoul2/G2_gore.h")
source_group("ghoul2" FILES ${MPRend2Ghoul2Files})
set(MPRend2Files ${MPRend2Files} ${MPRend2Ghoul2Files})

set(MPRend2RdCommonFiles
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
source_group("rd-common" FILES ${MPRend2RdCommonFiles})
set(MPRend2Files ${MPRend2Files} ${MPRend2RdCommonFiles})

set(MPRend2CommonFiles
	"${MPDir}/qcommon/matcomp.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	"${SharedCommonFiles}")
source_group("common" FILES ${MPRend2CommonFiles})
set(MPRend2Files ${MPRend2Files} ${MPRend2CommonFiles})

set(MPRend2CommonSafeFiles
	${SharedCommonSafeFiles}
	)
source_group("common/safe" FILES ${MPRend2CommonSafeFiles})
set(MPRend2Files ${MPRend2Files} ${MPRend2CommonSafeFiles})

# Transparently use either bundled or system libjpeg.
list(APPEND MPRend2IncludeDirectories ${JPEG_INCLUDE_DIR})
list(APPEND MPRend2Libraries          ${JPEG_LIBRARIES})

# Transparently use either bundled or system libpng.  Order is important --
# libpng used zlib, so it must come before it on the linker command line.
list(APPEND MPRend2IncludeDirectories ${PNG_INCLUDE_DIRS})
list(APPEND MPRend2Libraries          ${PNG_LIBRARIES})

# Transparently use either bundled or system zlib.
list(APPEND MPRend2IncludeDirectories ${ZLIB_INCLUDE_DIR})
list(APPEND MPRend2Libraries          ${ZLIB_LIBRARIES})

# Transparently use our bundled minizip.
list(APPEND MPRend2IncludeDirectories ${MINIZIP_INCLUDE_DIRS})
list(APPEND MPRend2Libraries          ${MINIZIP_LIBRARIES})

find_package(OpenGL REQUIRED)
set(MPRend2IncludeDirectories ${MPRend2IncludeDirectories} ${OPENGL_INCLUDE_DIR})
set(MPRend2Libraries ${MPRend2Libraries} ${OPENGL_LIBRARIES})

source_group("renderer"
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.h
	${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)
set(MPRend2Files
	${MPRend2Files}
	${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.h
	${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)

set(MPRend2IncludeDirectories ${MPRend2IncludeDirectories} ${OpenJKLibDir})
add_library(${MPRend2} SHARED ${MPRend2Files})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPRend2} PROPERTIES PREFIX "")
endif()

if(WIN32)
	install(TARGETS ${MPRend2}
		RUNTIME
		DESTINATION ${JKAInstallDir}
		COMPONENT ${JKAMPClientComponent})
else(WIN32)
	if(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${MPRend2}
			LIBRARY
			DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/"
			COMPONENT ${JKAMPClientComponent})
	else()
		install(TARGETS ${MPRend2}
			LIBRARY
			DESTINATION ${JKAInstallDir}
			COMPONENT ${JKAMPClientComponent})
	endif()
endif()

set_target_properties(${MPRend2} PROPERTIES COMPILE_DEFINITIONS "${SharedDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPRend2} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})
set_target_properties(${MPRend2} PROPERTIES INCLUDE_DIRECTORIES "${MPRend2IncludeDirectories}")
set_target_properties(${MPRend2} PROPERTIES PROJECT_LABEL "MP Rend2 Renderer")
target_link_libraries(${MPRend2} ${MPRend2Libraries})
target_include_directories(${MPRend2} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# GLSL shader file generator
add_executable(compact_glsl
	${MPDir}/rd-rend2/glsl/compact.cpp
	${MPDir}/rd-rend2/tr_allocator.cpp
	${MPDir}/rd-rend2/tr_allocator.h
	${MPDir}/rd-rend2/tr_glsl_parse.cpp)
target_compile_definitions(compact_glsl PRIVATE "GLSL_BUILDTOOL")
if (NOT WIN32 AND NOT APPLE)
	target_compile_definitions(compact_glsl PRIVATE "ARCH_STRING=\"${Architecture}\"")
endif()
target_include_directories(compact_glsl PRIVATE "${MPRend2IncludeDirectories}")
add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp
		${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.h
	COMMAND
		compact_glsl
			${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp
			${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.h
			${MPRend2GLSLFiles}
	DEPENDS
		compact_glsl ${MPRend2GLSLFiles})
