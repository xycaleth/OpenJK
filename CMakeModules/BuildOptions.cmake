include(CMakeDependentOption)

option(BUILD_PORTABLE "Build portable version (does not read or write files from your user/home directory" OFF)

option(BUILD_JAMP "Create Jedi Academy multi player projects"	ON)
option(BUILD_JASP "Create Jedi Academy single player projects"	OFF)
option(BUILD_JOSP "Create Jedi Outcast single player projects"	OFF)

cmake_dependent_option(BUILD_JAMP_CLIENT	"Create Jedi Academy MP client project"				ON "BUILD_JAMP" OFF)
cmake_dependent_option(BUILD_JAMP_RENDERER	"Create Jedi Academy MP default renderer project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BUILD_JAMP_SERVER	"Create Jedi Academy MP dedicated server project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BUILD_JAMP_GAME		"Create Jedi Academy MP server-side game project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BUILD_JAMP_CGAME		"Create Jedi Academy MP client-side game project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BUILD_JAMP_UI		"Create Jedi Academy MP UI code project"			ON "BUILD_JAMP" OFF)

cmake_dependent_option(BUILD_JASP_CLIENT	"Create Jedi Academy SP engine project"				ON "BUILD_JASP" OFF)
cmake_dependent_option(BUILD_JASP_GAME		"Create Jedi Academy SP gamecode project"			ON "BUILD_JASP" OFF)
cmake_dependent_option(BUILD_JASP_RENDERER	"Create Jedi Academy SP default renderer project"	ON "BUILD_JASP" OFF)

cmake_dependent_option(BUILD_JOSP_CLIENT	"Create Jedi Outcast SP engine project"				OFF "BUILD_JOSP" OFF)
cmake_dependent_option(BUILD_JOSP_GAME		"Create Jedi Outcast SP game project"				OFF "BUILD_JOSP" OFF)
cmake_dependent_option(BUILD_JOSP_RENDERER	"Create Jedi Outcast SP default renderer project"	OFF "BUILD_JOSP" OFF)

if(WIN32)
	set(USE_BUNDLED_ZLIB_DEFAULT ON)
	set(USE_BUNDLED_LIBPNG_DEFAULT ON)
	set(USE_BUNDLED_LIBJPEG_DEFAULT ON)
	set(USE_BUNDLED_SDL2_DEFAULT ON)
else()
	set(USE_BUNDLED_ZLIB OFF)
	set(USE_BUNDLED_LIBPNG OFF)
	if(APPLE)
		set(USE_BUNDLED_LIBJPEG_DEFAULT ON)
	else()
		set(USE_BUNDLED_LIBJPEG_DEFAULT OFF)
	endif()
	set(USE_BUNDLED_SDL2_DEFAULT OFF)
endif()
option(USE_BUNDLED_ZLIB		"Use bundled zlib"    ${USE_BUNDLED_ZLIB_DEFAULT})
option(USE_BUNDLED_PNG		"Use bundled libpng"  ${USE_BUNDLED_LIBPNG_DEFAULT})
option(USE_BUNDLED_JPEG		"Use bundled libjpeg" ${USE_BUNDLED_LIBJPEG_DEFAULT})
option(USE_BUNDLED_SDL2		"Use bundled SDL2"	  ${USE_BUNDLED_SDL2_DEFAULT})

cmake_dependent_option(USE_BUNDLED_OPENAL	"Use bundled OpenAL (Windows only)"					ON "WIN32" OFF)

cmake_dependent_option(BUILD_APP_BUNDLES	"Build .app bundles for executables (macOS only)"	ON "APPLE" OFF)

option(BUILD_TESTS "Build unit tests (requires Boost)" OFF)

cmake_dependent_option(BUILD_SYMBOL_SERVER	"Build WIP Windows Symbol Server (experimental and unused)" OFF "NOT MSVC" OFF)
mark_as_advanced(BUILD_TESTS BUILD_SYMBOL_SERVER)
