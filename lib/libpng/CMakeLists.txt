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

# We only execute this script if the user wants to use our bundled libpng.
#
# It is built as a static relocatable library, and linked into any target that
# requires it.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(bundled_libpng
	STATIC
	include/png.h
	include/pngconf.h
	include/pnglibconf.h
	png.c
	pngdebug.h
	pngerror.c
	pngget.c
	pnginfo.h
	pngmem.c
	pngpread.c
	pngpriv.h
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngstruct.h
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c)
target_include_directories(bundled_libpng PUBLIC include)
target_link_libraries(bundled_libpng PRIVATE ZLIB::ZLIB)
if(MSVC)
	target_compile_definitions(bundled_libpng PRIVATE D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

set_property(TARGET bundled_libpng PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(PNG::PNG ALIAS bundled_libpng)

get_property(PNG_INCLUDE_DIRS_ TARGET bundled_libpng PROPERTY INCLUDE_DIRECTORIES)
set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIRS_} CACHE INTERNAL "libpng include path")
set(PNG_LIBRARIES bundled_libpng CACHE INTERNAL "libpng library path")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PNG DEFAULT_MSG PNG_LIBRARIES PNG_INCLUDE_DIRS)
mark_as_advanced(PNG_LIBRARIES PNG_INCLUDE_DIRS)
