/*
	SPDX-FileCopyrightText: 2009-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "locale_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>

/**
 * Program entry point.
 *
 * @param argc amount of command line arguments
 * @param argv command line arguments
 * @return @c 0 on successful exit
 */
int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	app.setApplicationName("Simsu");
	app.setApplicationVersion(VERSIONSTR);
	app.setApplicationDisplayName(Window::tr("Simsu"));
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
	app.setWindowIcon(QIcon::fromTheme("simsu", QIcon(":/simsu.png")));
	app.setDesktopFileName("simsu");
#endif

	const QString appdir = app.applicationDirPath();
	const QStringList datadirs{
#if defined(Q_OS_MAC)
		appdir + "/../Resources"
#elif defined(Q_OS_UNIX)
		DATADIR,
		appdir + "/../share/simsu"
#else
		appdir
#endif
	};

	LocaleDialog::loadTranslator("simsu_", datadirs);

	QCommandLineParser parser;
	parser.setApplicationDescription(Window::tr("A basic Sudoku game"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);

	Window window;
	window.show();

	return app.exec();
}
