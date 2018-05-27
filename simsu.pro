lessThan(QT_MAJOR_VERSION, 5) {
	error("Simsu requires Qt 5.2 or greater")
}
equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 2) {
	error("Simsu requires Qt 5.2 or greater")
}

TEMPLATE = app
QT += widgets
CONFIG += c++11

CONFIG(debug, debug|release) {
	CONFIG += warn_on
	DEFINES += QT_DEPRECATED_WARNINGS
	DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051100
	DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
}

# Allow in-tree builds
!win32 {
	MOC_DIR = build
	OBJECTS_DIR = build
	RCC_DIR = build
}

# Set program version
VERSION = 1.3.5
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
	src/dancing_links.h \
	src/frame.h \
	src/locale_dialog.h \
	src/move.h \
	src/pattern.h \
	src/puzzle.h \
	src/square.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/dancing_links.cpp \
	src/frame.cpp \
	src/locale_dialog.cpp \
	src/move.cpp \
	src/main.cpp \
	src/puzzle.cpp \
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
	RC_FILE = icons/icon.rc
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

	pixmap.files = icons/simsu.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	appdata.files = icons/simsu.appdata.xml
	appdata.path = $$PREFIX/share/metainfo/

	qm.files = $$replace(TRANSLATIONS, .ts, .qm)
	qm.path = $$PREFIX/share/simsu/translations
	qm.CONFIG += no_check_exist

	man.files = doc/simsu.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target icon pixmap desktop appdata qm man
}
