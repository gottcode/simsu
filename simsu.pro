TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
CONFIG += warn_on
macx {
	CONFIG += x86_64
}

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

VERSION = $$system(git rev-parse --short HEAD)
isEmpty(VERSION) {
	VERSION = 0
}
DEFINES += VERSIONSTR=\\\"git.$${VERSION}\\\"

unix: !macx {
	TARGET = simsu
} else {
	TARGET = Simsu
}

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

TRANSLATIONS = translations/simsu_ca.ts \
	translations/simsu_cs.ts \
	translations/simsu_el.ts \
	translations/simsu_es.ts \
	translations/simsu_es_CL.ts \
	translations/simsu_en.ts \
	translations/simsu_fr.ts \
	translations/simsu_ru.ts \
	translations/simsu_uk.ts

RESOURCES = icons/icon.qrc symmetry/symmetry.qrc
macx {
	ICON = icons/simsu.icns
} else:win32 {
	RC_FILE = icons/icon.rc
}

unix: !macx {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = icons/simsu_32.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/simsu/translations

	INSTALLS += target icon pixmap desktop qm
}
