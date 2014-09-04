cmake_minimum_required( VERSION 2.8.9 )
project( ModView )

set( CMAKE_PREFIX_PATH
	${CMAKE_PREFIX_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/../qt5/qtbase/lib/cmake/Qt5Core"
)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOMOC ON )

find_package( Qt5Core REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5Gui REQUIRED )
find_package( OpenGL REQUIRED )

set( MODVIEW_ZLIB_FILES
	zlib/deflate.h
	zlib/infblock.h
	zlib/infcodes.h
	zlib/inffast.h
	zlib/inffixed.h
	zlib/inftrees.h
	zlib/infutil.h
	zlib/trees.h
	zlib/zconf.h
	zlib/zlib.h
	zlib/zutil.h

	zlib/adler32.c
	zlib/deflate.c
	zlib/infblock.c
	zlib/infcodes.c
	zlib/inffast.c
	zlib/inflate.c
	zlib/inftrees.c
	zlib/infutil.c
	zlib/trees.c
	zlib/zutil.c
	zlib/crc32.cpp
)
source_group( "ZLib Files" FILES ${MODVIEW_ZLIB_FILES} )

set( MODVIEW_LIBJPEG_FILES
	jpeg6/jconfig.h
	jpeg6/jdct.h
	jpeg6/jdhuff.h
	jpeg6/jerror.h
	jpeg6/jinclude.h
	jpeg6/jmemsys.h
	jpeg6/jmorecfg.h
	jpeg6/jpegint.h
	jpeg6/jpeglib.h
	jpeg6/jversion.h

	jpeg6/jcomapi.c
	jpeg6/jdapimin.c
	jpeg6/jdapistd.c
	jpeg6/jdatasrc.c
	jpeg6/jdcoefct.c
	jpeg6/jdcolor.c
	jpeg6/jddctmgr.c
	jpeg6/jdhuff.c
	jpeg6/jdinput.c
	jpeg6/jdmainct.c
	jpeg6/jdmarker.c
	jpeg6/jdmaster.c
	jpeg6/jdpostct.c
	jpeg6/jdsample.c
	jpeg6/jdtrans.c
	jpeg6/jerror.c
	jpeg6/jidctflt.c
	jpeg6/jmemmgr.c
	jpeg6/jmemnobs.c
	jpeg6/jutils.c
)
source_group( "LibJPEG Files" FILES ${MODVIEW_LIBJPEG_FILES} )

set( MODVIEW_GUI_FILES
	GUIHelpers.h
	MainForm.h
	ChangeFloorDialog.h
	RenderWidget.h
	SceneTreeItem.h
	SceneTreeModel.h
	SceneTreeView.h
	
	GUIHelpers.cpp
	MainForm.cpp
	#ChangeFloorDialog.cpp
	RenderWidget.cpp
	SceneTreeItem.cpp
	SceneTreeModel.cpp
	SceneTreeView.cpp
)
source_group( "GUI Files" FILES ${MODVIEW_GUI_FILES} )

set( MODVIEW_HEADER_FILES
	anims.h
	common_headers.h
	files.h
	generic_stuff.h
	GenericParser2.h
    gl_bits.h
	glm_code.h
	includes.h
	jpeg_interface.h
	matcomp.h
	mc_compress2.h
	md3_format.h
	mdr_format.h
	mdx_format.h
	model.h
	oldskins.h
	parser.h
	png/png.h
	r_common.h
	r_glm.h
	r_image.h
	r_md3.h
	r_mdr.h
	r_model.h
	r_surface.h
	script.h
	sequence.h
	shader.h
	skins.h
	stdafx.h
	stl.h
	StringUtils.h
	text.h
	textures.h
	todo.h
)

set( MODVIEW_SOURCE_FILES
	anims.cpp
	files.cpp
	generic_stuff.cpp
	GenericParser2.cpp
    gl_bits.cpp
	glm_code.cpp
	jpeg_interface.cpp
	Main.cpp
	matcomp.cpp
	mc_compress2.cpp
	model.cpp
	oldskins.cpp
	parser.cpp
	png/png.cpp
	r_glm.cpp
	r_image.cpp
	r_md3.cpp
	r_mdr.cpp
	r_model.cpp
	r_surface.cpp
	script.cpp
	sequence.cpp
	shader.cpp
	skins.cpp
	stdafx.cpp
	StringUtils.cpp
	text.cpp
	textures.cpp
)

qt5_wrap_ui( UI_HEADERS
	MainForm.ui
	ChangeFloor.ui
)
source_group( "Qt UI Files" FILES ${UI_HEADERS} )

qt5_add_resources( MODVIEW_RESOURCES
	ModView.qrc
)
source_group( "Qt Resource Files" FILES ${MODVIEW_RESOURCES} )

include_directories( "." )

add_definitions( -D_M_IX86 )
if( MSVC )
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

if( MSVC )
	set( GUI_TYPE WIN32 )
	set( MODVIEW_RESOURCES ${MODVIEW_RESOURCES} modview.rc )
elseif ( APPLE )
	set( GUI_TYPE MACOSX_BUNDLE )
	set( PROGNAME ModView )
endif()

set( MODVIEW_LIBRARIES
	${OPENGL_LIBRARIES}
	${Qt5Core_QTMAIN_LIBRARIES}
)

add_executable( ModView
	${GUI_TYPE}
	${MODVIEW_RESOURCES}
	${MODVIEW_SOURCE_FILES}
	${MODVIEW_HEADER_FILES}
	${MODVIEW_ZLIB_FILES}
	${MODVIEW_LIBJPEG_FILES}
	${MODVIEW_GUI_FILES}
	${UI_HEADERS}
)

target_link_libraries( ModView
	${MODVIEW_LIBRARIES}
)

qt5_use_modules( ModView
	Widgets
	Gui
	OpenGL
)
