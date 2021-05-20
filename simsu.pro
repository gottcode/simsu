!versionAtLeast(QT_VERSION, 5.12) {
	error("Simsu requires Qt 5.12 or greater")
}

TEMPLATE = app
QT += widgets concurrent
CONFIG += c++17

CONFIG(debug, debug|release) {
	CONFIG += warn_on
	DEFINES += QT_DEPRECATED_WARNINGS
	DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
	DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
}

# Allow in-tree builds
MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

# Set program version
VERSION = 1.3.9
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# Set program name
unix: !macx {
	TARGET = simsu
} else {
	TARGET = Simsu
}

# Specify program sources
HEADERS = src/board.h \
	src/cell.h \
	src/frame.h \
	src/locale_dialog.h \
	src/move.h \
	src/new_game_page.h \
	src/pattern.h \
	src/puzzle.h \
	src/solver_dlx.h \
	src/solver_logic.h \
	src/square.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/frame.cpp \
	src/locale_dialog.cpp \
	src/move.cpp \
	src/main.cpp \
	src/new_game_page.cpp \
	src/puzzle.cpp \
	src/solver_dlx.cpp \
	src/solver_logic.cpp \
	src/square.cpp \
	src/window.cpp

# Generate translations
TRANSLATIONS = $$files(translations/simsu_*.ts)
qtPrepareTool(LRELEASE, lrelease)
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Install program data
RESOURCES = icons/images.qrc symmetry/symmetry.qrc
macx {
	ICON = icons/simsu.icns
} else:win32 {
	RC_ICONS = icons/simsu.ico
	QMAKE_TARGET_DESCRIPTION = "Sudoku game"
	QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2021 Graeme Gott"
} else:unix {
	RESOURCES += icons/icon.qrc

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	appdata.files = icons/simsu.appdata.xml
	appdata.path = $$PREFIX/share/metainfo/

	qm.files = $$replace(TRANSLATIONS, .ts, .qm)
	qm.path = $$PREFIX/share/simsu/translations
	qm.CONFIG += no_check_exist

	man.files = doc/simsu.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target icon desktop appdata qm man
}
