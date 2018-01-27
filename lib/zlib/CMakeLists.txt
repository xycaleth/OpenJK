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

# We only execute this script if the user wants to use our bundled zlib/minizip.
#
# It is built as a static relocatable library, and linked into any target that
# requires it.
#
# The bundled copy can be produced from a zlib tarball by copying the c and h
# files from the root of the tarball to this folder, and then moving the public
# header files (zlib.h, zconf.h) into the include/ subfolder.
#
# The current bundled copy comes from the zlib 1.2.8 release.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(bundled_zlib
	STATIC
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c)
target_include_directories(bundled_zlib PUBLIC include)
if(MSVC)
	target_compile_options(bundled_zlib PRIVATE "/wd4996")
	target_compile_definitions(bundled_zlib PUBLIC _CRT_SECURE_NO_WARNINGS)
endif(MSVC)
set_property(TARGET bundled_zlib PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(ZLIB::ZLIB ALIAS bundled_zlib)
