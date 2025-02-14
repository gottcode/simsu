# SPDX-FileCopyrightText: 2021-2025 Graeme Gott <graeme@gottcode.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

# Configure project
project(simsu VERSION 1.4.5 LANGUAGES CXX)

set(project_copyright "2009-2025 Graeme Gott")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Gui LinguistTools PrintSupport Widgets)
include(GNUInstallDirs)

add_compile_definitions(
	QT_NO_KEYWORDS
	$<$<CONFIG:DEBUG>:QT_STRICT_ITERATORS>
	$<$<CONFIG:DEBUG>:QT_NO_NARROWING_CONVERSIONS_IN_CONNECT>
	$<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060800>
)

# Version number
include(cmake/AddVersionCompileDefinition.cmake)
add_version_compile_definition(src/main.cpp VERSIONSTR)

# Create program
qt_add_executable(simsu
	# Headers
	src/board.h
	src/cell.h
	src/frame.h
	src/locale_dialog.h
	src/move.h
	src/new_game_page.h
	src/pattern.h
	src/puzzle.h
	src/solver_dlx.h
	src/solver_logic.h
	src/square.h
	src/window.h
	# Sources
	src/board.cpp
	src/cell.cpp
	src/frame.cpp
	src/locale_dialog.cpp
	src/move.cpp
	src/main.cpp
	src/new_game_page.cpp
	src/puzzle.cpp
	src/solver_dlx.cpp
	src/solver_logic.cpp
	src/square.cpp
	src/window.cpp
	# Resources
	icons/images.qrc
	symmetry/symmetry.qrc
	${translations_QM}
)

target_link_libraries(simsu PRIVATE
	Qt6::Concurrent
	Qt6::Core
	Qt6::Gui
	Qt6::PrintSupport
	Qt6::Widgets
)

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt_add_translations(simsu
	TS_FILES ${translations_SRCS}
	QM_FILES_OUTPUT_VARIABLE translations_QM
	LUPDATE_OPTIONS -no-obsolete -locations none
)

# Optimize build
option(ENABLE_LINK_TIME_OPTIMIZATION "Enable link time optimization" OFF)
if(ENABLE_LINK_TIME_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(simsu PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

option(ENABLE_STRIP "Enable automatic stripping of builds" OFF)
if(ENABLE_STRIP)
	add_custom_command(TARGET simsu
		POST_BUILD
		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:simsu>
	)
endif()

# Install
if(APPLE)
	set(datadir "../Resources")

	set_target_properties(simsu PROPERTIES
		OUTPUT_NAME Simsu
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/mac/Info.plist.in
	)

	include(cmake/BundleResources.cmake)
	bundle_data(simsu ${CMAKE_SOURCE_DIR}/icons/simsu.icns Resources)
	bundle_translations(simsu "${translations_QM}")
elseif(WIN32)
	set(datadir ".")

	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(simsu PROPERTIES
		OUTPUT_NAME Simsu
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Sudoku game"
		QT_TARGET_COPYRIGHT "\\xA9 ${project_copyright}"
		QT_TARGET_PRODUCT_NAME "Simsu"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/simsu.ico
	)
	_qt_internal_generate_win32_rc_file(simsu)
else()
	file(RELATIVE_PATH datadir ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/simsu)

	target_sources(simsu PRIVATE icons/icon.qrc)

	install(TARGETS simsu RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/simsu/translations)
	install(FILES doc/simsu.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

	include(cmake/TranslateMetainfo.cmake)
	process_and_install_metainfo(PO_DIR ${CMAKE_SOURCE_DIR}/icons/po)
endif()

set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS SIMSU_DATADIR="${datadir}")
