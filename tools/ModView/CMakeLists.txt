cmake_minimum_required(VERSION 3.1)
project(ModView)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenGL REQUIRED)

set(MODVIEW_GUI_FILES
	GUIHelpers.h
	MainForm.h
	ChangeFloorDialog.h
	RenderWidget.h
	SceneTreeItem.h
	SceneTreeModel.h
	SceneTreeView.h
	
	GUIHelpers.cpp
	MainForm.cpp
	ChangeFloorDialog.cpp
	RenderWidget.cpp
	SceneTreeItem.cpp
	SceneTreeModel.cpp
	SceneTreeView.cpp)
source_group("GUI Files" FILES ${MODVIEW_GUI_FILES})

set(MODVIEW_HEADER_FILES
	anims.h
	common_headers.h
	files.h
	generic_stuff.h
	GenericParser2.h
    gl_bits.h
	glm_code.h
	includes.h
	img_jpeg.h
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
	todo.h)

set(MODVIEW_SOURCE_FILES
	anims.cpp
	files.cpp
	generic_stuff.cpp
	GenericParser2.cpp
    gl_bits.cpp
	glm_code.cpp
	img_jpeg.cpp
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
	textures.cpp)

set(UI_HEADERS MainForm.ui ChangeFloor.ui)
source_group("Qt UI Files" FILES ${UI_HEADERS})

set(MODVIEW_RESOURCES ModView.qrc)
source_group("Qt Resource Files" FILES ${MODVIEW_RESOURCES})

if(MSVC)
	set(GUI_TYPE WIN32)
	set(MODVIEW_RESOURCES ${MODVIEW_RESOURCES} modview.rc)
elseif(APPLE)
	set(GUI_TYPE MACOSX_BUNDLE)
endif()

add_executable(ModView
	${GUI_TYPE}
	${MODVIEW_RESOURCES}
	${MODVIEW_SOURCE_FILES}
	${MODVIEW_HEADER_FILES}
	${MODVIEW_GUI_FILES}
	${UI_HEADERS})
target_compile_features(ModView PRIVATE cxx_std_14)

if(MSVC)
	target_compile_definitions(ModView
		PRIVATE
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
		WIN32_LEAN_AND_MEAN)
endif()

target_link_libraries(ModView
	PRIVATE
	OpenGL::GL
	Qt::OpenGL
	Qt::Widgets
	Qt::Gui
	ZLIB::ZLIB
	JPEG::JPEG)