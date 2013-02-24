lessThan(QT_VERSION, 4.5) {
	error("Simsu requires Qt 4.5 or greater")
}

TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
CONFIG += warn_on

VERSION = 1.2.3
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

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

TRANSLATIONS = $$files(translations/simsu_*.ts)

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

	pixmap.files = icons/simsu.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/simsu.desktop

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/simsu/translations

	INSTALLS += target icon pixmap desktop qm
}
