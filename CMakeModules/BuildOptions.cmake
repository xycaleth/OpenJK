include(CMakeDependentOption)
option(BUILD_PORTABLE "Build portable version (does not read or write files from your user/home directory" OFF)

option(BUILD_JAMP "Create Jedi Academy multi player projects"	ON)
option(BUILD_JASP "Create Jedi Academy single player projects"	OFF)
option(BUILD_JOSP "Create Jedi Outcast single player projects"	OFF)

cmake_dependent_option(BuildMPEngine		"Create Jedi Academy MP client project"				ON "BUILD_JAMP" OFF)
cmake_dependent_option(BuildMPRdVanilla		"Create Jedi Academy MP default renderer project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BuildMPDed			"Create Jedi Academy MP dedicated server project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BuildMPGame			"Create Jedi Academy MP server-side game project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BuildMPCGame			"Create Jedi Academy MP client-side game project"	ON "BUILD_JAMP" OFF)
cmake_dependent_option(BuildMPUI			"Create Jedi Academy MP UI code project"			ON "BUILD_JAMP" OFF)

cmake_dependent_option(BuildSPEngine		"Create Jedi Academy SP engine project"				ON "BUILD_JASP" OFF)
cmake_dependent_option(BuildSPGame			"Create Jedi Academy SP gamecode project"			ON "BUILD_JASP" OFF)
cmake_dependent_option(BuildSPRdVanilla		"Create Jedi Academy SP default renderer project"	ON "BUILD_JASP" OFF)

cmake_dependent_option(BuildJK2SPEngine		"Create Jedi Outcast SP engine project"				OFF "BUILD_JOSP" OFF)
cmake_dependent_option(BuildJK2SPGame		"Create Jedi Outcast SP game project"				OFF "BUILD_JOSP" OFF)
cmake_dependent_option(BuildJK2SPRdVanilla	"Create Jedi Outcast SP default renderer project"	OFF "BUILD_JOSP" OFF)

option(BUILD_TESTS "Build unit tests (requires Boost)" OFF)

cmake_dependent_option(BUILD_SYMBOL_SERVER "Build WIP Windows Symbol Server (experimental and unused)" OFF "NOT WIN32 OR NOT MSVC" OFF)

cmake_dependent_option(USE_INTERNAL_OPENAL "Use bundled OpenAL"  ON "WIN32" OFF)
cmake_dependent_option(USE_INTERNAL_ZLIB   "Use bundled zlib"    ON "WIN32" OFF)
cmake_dependent_option(USE_INTERNAL_PNG    "Use bundled libpng"  ON "WIN32" OFF)
cmake_dependent_option(USE_INTERNAL_JPEG   "Use bundled libjpeg" ON "WIN32 OR APPLE" OFF)
cmake_dependent_option(USE_INTERNAL_SDL2   "Use bundled SDL2"    ON "WIN32" OFF)

if(APPLE)
	option(MakeApplicationBundles "Build .app bundles for executables" ON)
endif()
