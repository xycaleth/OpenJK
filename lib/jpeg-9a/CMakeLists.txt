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

# We only execute this script if the user wants to use our bundled libjpeg.
#
# It is built as a static relocatable library, and linked into any target that
# requires it.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(bundled_libjpeg
	STATIC
	include/jconfig.h
    include/jmorecfg.h
    include/jpegint.h
    include/jpeglib.h
    cderror.h
    jaricom.c
    jcapimin.c
    jcapistd.c
    jcarith.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdarith.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jdct.h
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jerror.h
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jinclude.h
    jmemmgr.c
    jmemnobs.c
    jquant1.c
    jquant2.c
    jutils.c
    jversion.h
    rdbmp.c
    rdcolmap.c
    rdgif.c
    rdppm.c
    rdrle.c
    rdswitch.c
    rdtarga.c
    wrbmp.c
    wrgif.c
    wrppm.c
    wrrle.c
    wrtarga.c)
target_include_directories(bundled_libjpeg PUBLIC include)
if(MSVC)
	target_compile_definitions(bundled_libjpeg PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

set_property(TARGET bundled_libjpeg PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(JPEG::JPEG ALIAS bundled_libjpeg)

get_property(JPEG_INCLUDE_DIR_ TARGET bundled_libjpeg PROPERTY INCLUDE_DIRECTORIES)
set(JPEG_INCLUDE_DIR ${JPEG_INCLUDE_DIR_} CACHE INTERNAL "libjpeg include path")
set(JPEG_LIBRARIES bundled_libjpeg CACHE INTERNAL "libjpeg library path")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JPEG DEFAULT_MSG JPEG_LIBRARIES JPEG_INCLUDE_DIR)
mark_as_advanced(JPEG_LIBRARIES JPEG_INCLUDE_DIR)
