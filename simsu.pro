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
	src/move.h \
	src/pattern.h \
	src/puzzle.h \
	src/square.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/dancing_links.cpp \
	src/frame.cpp \
	src/move.cpp \
	src/main.cpp \
	src/puzzle.cpp \
	src/square.cpp \
	src/window.cpp

TRANSLATIONS = translations/cs.ts \
	translations/en_US.ts

RESOURCES = icons/icon.qrc symmetry/symmetry.qrc translations/translations.qrc
macx {
	ICON = icons/simsu.icns
} else:win32 {
	RC_FILE = icons/icon.rc
}

unix: !macx {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	target.path = $$PREFIX/bin/

	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
	icon.files = icons/simsu.png

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	INSTALLS += target icon desktop
}
