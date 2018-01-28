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

# Minizip  is built as a static relocatable library, and linked into any target
# that requires it.  Other targets in OpenJK that consume need to depend on the
# target MINIZIP::MINIZIP.

# The bundled copy is produced by taking the files ioapi.{c,h} and unzip.{c,h}
# from the contrib/minizip directory of the zlib release tarball.  The only
# changes applied are modifications of the macros ALLOC and TRYFREE in unzip.c,
# and a compatibility definition of the macro OF in include/ioapi.h
#
# Since minizip is expected to be reference by user code as
#
#     #include <minizip/unzip.h>
#
# the public header files are split into the include/minizip subdirectory.
#
# The current bundled copy comes from the zlib 1.2.8 release.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(bundled_minizip STATIC ioapi.c unzip.c)
target_link_libraries(bundled_minizip PRIVATE ZLIB::ZLIB)
target_include_directories(bundled_minizip
	PUBLIC include
	PRIVATE include/minizip)
if(MSVC)
	target_compile_definitions(bundled_minizip PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "BSD")
	target_compile_definitions(bundled_minizip PRIVATE IOAPI_NO_64)
endif()
set_property(TARGET bundled_minizip PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(MINIZIP::MINIZIP ALIAS bundled_minizip)
