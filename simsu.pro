TEMPLATE = app
CONFIG += warn_on release
macx {
	# Uncomment the following line to compile on PowerPC Macs
	# QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG += x86 ppc
}

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

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

	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
	icon.files = icons/simsu.png

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/simsu/translations

	INSTALLS += target icon desktop qm
}
