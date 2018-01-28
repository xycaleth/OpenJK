if(MSVC)
	string(APPEND CMAKE_C_FLAGS " /arch:SSE2")
	string(APPEND CMAKE_C_FLAGS " /MP")

	string(APPEND CMAKE_CXX_FLAGS " /arch:SSE2")
	string(APPEND CMAKE_CXX_FLAGS " /MP")

	# We don't try to control symbol visibility under MSVC.
	set(OPENJK_VISIBILITY_FLAGS "")
elseif (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang"))
	# I hope this doesn't come back to bite me in the butt later on.
	# Realistically though, can the C and CXX compilers be different?

	# Visibility can't be set project-wide -- it needs to be specified on a
	# per-target basis.  This is primarily due to the bundled copy of ZLib.
	# ZLib explicitly declares symbols hidden, rather than defaulting to hidden.
	#
	# Note that -fvisibility=hidden is stronger than -fvisibility-inlines-hidden.
	set(OPENJK_VISIBILITY_FLAGS "-fvisibility=hidden")

	# removes the -rdynamic flag at linking (which causes crashes for some reason)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

	# additional flags for debug configuration
	string(APPEND CMAKE_C_FLAGS_DEBUG " -ggdb")
	string(APPEND CMAKE_CXX_FLAGS_DEBUG " -ggdb")

	if (X86)
		string(APPEND CMAKE_C_FLAGS " -msse2")
		string(APPEND CMAKE_CXX_FLAGS " -msse2")
	endif()

	string(APPEND CMAKE_C_FLAGS_RELEASE " -O3")
	string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")

	# enable somewhat modern C++
	string(APPEND CMAKE_CXX_FLAGS " -std=c++11")
	
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
		string(APPEND CMAKE_C_FLAGS " -Wall")
		string(APPEND CMAKE_C_FLAGS " -Wno-comment")
		string(APPEND CMAKE_C_FLAGS " -fsigned-char")
		if (X86)
			# "x86 vm will crash without -mstackrealign since MMX
			# instructions will be used no matter what and they
			# corrupt the frame pointer in VM calls"
			# -ioquake3 Makefile
			string(APPEND CMAKE_C_FLAGS " -mstackrealign")
			string(APPEND CMAKE_C_FLAGS " -mfpmath=sse")
		endif()

		if(WIN32)
			# Link libgcc and libstdc++ statically
			string(APPEND CMAKE_C_FLAGS " -static-libgcc")
			string(APPEND CMAKE_CXX_FLAGS " -static-libgcc")
			string(APPEND CMAKE_CXX_FLAGS " -static-libstdc++")
		endif()
	elseif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
		string(APPEND CMAKE_C_FLAGS " -Wall")
		string(APPEND CMAKE_C_FLAGS " -Wno-comment")
	endif()

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		string(APPEND CMAKE_CXX_FLAGS " -Wall")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-invalid-offsetof")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-write-strings")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-comment")
		string(APPEND CMAKE_CXX_FLAGS " -fsigned-char")
		if (X86)
			string(APPEND CMAKE_CXX_FLAGS " -mstackrealign")
			string(APPEND CMAKE_CXX_FLAGS " -mfpmath=sse")
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		string(APPEND CMAKE_CXX_FLAGS " -Wall")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-write-strings")
		#string(APPEND CMAKE_CXX_FLAGS " -Wno-deprecated-writable-strings")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-comment")
		string(APPEND CMAKE_CXX_FLAGS " -Wno-invalid-offsetof")
	endif()
else()
	message(ERROR "Unsupported compiler")
endif()
