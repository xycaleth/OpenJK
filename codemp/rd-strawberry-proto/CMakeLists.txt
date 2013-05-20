cmake_minimum_required(VERSION 2.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(MPStrawberryProtoRendererIncludeDirectories ${MPDir})
set(MPStrawberryProtoRendererIncludeDirectories ${MPStrawberryProtoRendererIncludeDirectories} "${MPDir}/rd-strawberry-proto")

set(MPStrawberryProtoRendererFiles
	"${MPDir}/rd-strawberry-proto/glext.h"
	"${MPDir}/rd-strawberry-proto/qgl.h"
	"${MPDir}/rd-strawberry-proto/G2_API.cpp"
	"${MPDir}/rd-strawberry-proto/G2_bolts.cpp"
	"${MPDir}/rd-strawberry-proto/G2_bones.cpp"
	"${MPDir}/rd-strawberry-proto/G2_local.h"
	"${MPDir}/rd-strawberry-proto/G2_misc.cpp"
	"${MPDir}/rd-strawberry-proto/G2_surfaces.cpp"
	"${MPDir}/rd-strawberry-proto/tr_animation.cpp"
	"${MPDir}/rd-strawberry-proto/tr_arioche.cpp"
	"${MPDir}/rd-strawberry-proto/tr_backend.cpp"
	"${MPDir}/rd-strawberry-proto/tr_bsp.cpp"
	"${MPDir}/rd-strawberry-proto/tr_cmds.cpp"
	"${MPDir}/rd-strawberry-proto/tr_curve.cpp"
	"${MPDir}/rd-strawberry-proto/tr_font.cpp"
	"${MPDir}/rd-strawberry-proto/tr_ghoul2.cpp"
	"${MPDir}/rd-strawberry-proto/tr_image.cpp"
	"${MPDir}/rd-strawberry-proto/tr_init.cpp"
	"${MPDir}/rd-strawberry-proto/tr_landscape.h"
	"${MPDir}/rd-strawberry-proto/tr_light.cpp"
	"${MPDir}/rd-strawberry-proto/tr_main.cpp"
	"${MPDir}/rd-strawberry-proto/tr_marks.cpp"
	"${MPDir}/rd-strawberry-proto/tr_mesh.cpp"
	"${MPDir}/rd-strawberry-proto/tr_model.cpp"
	"${MPDir}/rd-strawberry-proto/tr_noise.cpp"
	"${MPDir}/rd-strawberry-proto/tr_quicksprite.cpp"
	"${MPDir}/rd-strawberry-proto/tr_quicksprite.h"
	"${MPDir}/rd-strawberry-proto/tr_scene.cpp"
	"${MPDir}/rd-strawberry-proto/tr_shade.cpp"
	"${MPDir}/rd-strawberry-proto/tr_shade_calc.cpp"
	"${MPDir}/rd-strawberry-proto/tr_shader.cpp"
	"${MPDir}/rd-strawberry-proto/tr_shadows.cpp"
	"${MPDir}/rd-strawberry-proto/tr_sky.cpp"
	"${MPDir}/rd-strawberry-proto/tr_subs.cpp"
	"${MPDir}/rd-strawberry-proto/tr_surface.cpp"
	"${MPDir}/rd-strawberry-proto/tr_surfacesprites.cpp"
	"${MPDir}/rd-strawberry-proto/tr_terrain.cpp"
	"${MPDir}/rd-strawberry-proto/tr_world.cpp"
	"${MPDir}/rd-strawberry-proto/tr_WorldEffects.cpp"
	"${MPDir}/rd-strawberry-proto/tr_WorldEffects.h"
	)
source_group("renderer" FILES ${MPStrawberryProtoRendererFiles})

set(MPStrawberryProtoRendererQcommonFiles
	"${MPDir}/qcommon/GenericParser2.cpp"
	"${MPDir}/qcommon/matcomp.cpp"
	"${MPDir}/qcommon/q_math.cpp"
	"${MPDir}/qcommon/q_shared.cpp")
source_group("qcommon" FILES ${MPStrawberryProtoRendererQcommonFiles})
set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererQcommonFiles})

file(GLOB_RECURSE MPStrawberryProtoRendererJpegFiles "${MPDir}/jpeg-8c/*.c" "${MPDir}/jpeg-8c/*.h")
source_group("jpeg-8c" FILES ${MPStrawberryProtoRendererJpegFiles})
set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererJpegFiles})

set(MPStrawberryProtoRendererPngFiles
	"${MPDir}/png/rpng.cpp")
source_group("png" FILES ${MPStrawberryProtoRendererPngFiles})
set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererPngFiles})

set(MPStrawberryProtoRendererLibPngFiles
	"${MPDir}/libpng/png.c"
	"${MPDir}/libpng/pngerror.c"
	"${MPDir}/libpng/pngget.c"
	"${MPDir}/libpng/pngmem.c"
	"${MPDir}/libpng/pngpread.c"
	"${MPDir}/libpng/pngread.c"
	"${MPDir}/libpng/pngrio.c"
	"${MPDir}/libpng/pngrtran.c"
	"${MPDir}/libpng/pngrutil.c"
	"${MPDir}/libpng/pngset.c"
	"${MPDir}/libpng/pngtrans.c"
	"${MPDir}/libpng/pngwio.c"
	"${MPDir}/libpng/pngwrite.c"
	"${MPDir}/libpng/pngwtran.c"
	"${MPDir}/libpng/pngwutil.c")
source_group("libpng" FILES ${MPStrawberryProtoRendererLibPngFiles})
set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererLibPngFiles})

if(UseInternalZlib)
	set(MPStrawberryProtoRendererIncludeDirectories ${MPStrawberryProtoRendererIncludeDirectories} "${MPDir}/zlib")
	# OpenAL32.lib is included for windows
	find_library(ZlibLibrary NAMES zlib PATHS "${MPDir}/libs/win32")
	if(NOT ZlibLibrary)
		message(FATAL_ERROR "UseInternalOpenAL set, but libs/OpenAL32 not found!")
	endif(NOT ZlibLibrary)
	set(MPStrawberryProtoRendererLibraries ${MPStrawberryProtoRendererLibraries} ${ZlibLibrary})
else(UseInternalZlib)
	find_package(ZLIB REQUIRED)
	set(MPStrawberryProtoRendererIncludeDirectories ${MPStrawberryProtoRendererIncludeDirectories} ${ZLIB_INCLUDE_DIR})
	set(MPStrawberryProtoRendererLibraries ${MPStrawberryProtoRendererLibraries} ${ZLIB_LIBRARY})
endif(UseInternalZlib)

if(NOT WIN32)
	find_package(OpenGL REQUIRED)
	set(MPStrawberryProtoRendererIncludeDirectories ${MPStrawberryProtoRendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
	set(MPStrawberryProtoRendererLibraries ${MPStrawberryProtoRendererLibraries} ${OPENGL_LIBRARIES})

	find_package(SDL2 REQUIRED)
	set(MPStrawberryProtoRendererIncludeDirectories ${MPStrawberryProtoRendererIncludeDirectories} ${SDL2_INCLUDE_DIR})
	set(MPStrawberryProtoRendererLibraries ${MPStrawberryProtoRendererLibraries} ${SDL2_LIBRARY})
endif(NOT WIN32)


if(WIN32)
	set(MPStrawberryProtoRendererWin32Files
		"${MPDir}/win32/win_gamma.cpp"
		"${MPDir}/win32/win_glimp.cpp"
		"${MPDir}/win32/win_qgl.cpp")
	source_group("win32" FILES ${MPStrawberryProtoRendererWin32Files})
	set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererWin32Files})
else(WIN32)
	set(MPStrawberryProtoRendererSysFiles
		"${MPDir}/sdl/sdl_glimp.cpp")
	source_group("sys" FILES ${MPStrawberryProtoRendererSysFiles})
	set(MPStrawberryProtoRendererFiles ${MPStrawberryProtoRendererFiles} ${MPStrawberryProtoRendererSysFiles})
endif(WIN32)

add_library(${MPStrawberryProtoRenderer} SHARED ${MPStrawberryProtoRendererFiles})

if(NOT WIN32)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES PREFIX "")
endif(NOT WIN32)

if(WIN32)
	install(TARGETS ${MPStrawberryProtoRenderer} RUNTIME DESTINATION ".")
else(WIN32)
	install(TARGETS ${MPStrawberryProtoRenderer} LIBRARY DESTINATION ".")
endif(WIN32)
set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${SharedDefines};${DebugDefines}")
set_target_properties(${MPStrawberryProtoRenderer} PROPERTIES INCLUDE_DIRECTORIES "${MPStrawberryProtoRendererIncludeDirectories}")
target_link_libraries(${MPStrawberryProtoRenderer} ${MPStrawberryProtoRendererLibraries})
