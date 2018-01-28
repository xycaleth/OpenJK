# Operating settings
if(WIN64)
	list(APPEND SharedDefines "WIN64")
endif()

if (APPLE)
	list(APPEND SharedDefines "MACOS_X")
endif()

if (NOT WIN32 AND NOT APPLE)
	list(APPEND SharedDefines "ARCH_STRING=\"${Architecture}\"")
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
if(MSVC)
	list(APPEND SharedDefines "NOMINMAX")
	list(APPEND SharedDefines "_CRT_SECURE_NO_WARNINGS")
	list(APPEND SharedDefines "_SCL_SECURE_NO_WARNINGS")
	list(APPEND SharedDefines "_CRT_NONSTDC_NO_DEPRECATE")
endif()

if(CMAKE_BUILD_TYPE MATCHES "DEBUG" OR CMAKE_BUILD_TYPE MATCHES "Debug")
	# CMake already defines _DEBUG for MSVC.
	if (NOT MSVC)
		list(APPEND SharedDefines "_DEBUG")
	endif()
else()
	list(APPEND SharedDefines "FINAL_BUILD")
endif()


# Settings
if(BUILD_PORTABLE)
	list(APPEND SharedDefines "_PORTABLE_VERSION")
endif()

# https://reproducible-builds.org/specs/source-date-epoch/
if (NOT ("$ENV{SOURCE_DATE_EPOCH}" STREQUAL ""))
	execute_process(COMMAND "date"
		"--date=@$ENV{SOURCE_DATE_EPOCH}" "+%b %_d %Y"
		OUTPUT_VARIABLE source_date
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	list(APPEND SharedDefines "SOURCE_DATE=\"${source_date}\"")
endif()

# Current Git SHA1 hash
include(GetGitRevisionDescription) 
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
message("Git revision is ${GIT_SHA1}")
list(APPEND SharedDefines "BUILD_COMMIT=\"${GIT_SHA1}\"")

