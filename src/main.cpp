/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011, 2012 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "locale_dialog.h"
#include "window.h"

#include <QApplication>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	app.setApplicationName("Simsu");
	app.setApplicationVersion(VERSIONSTR);
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
	{
		QIcon fallback(":/hicolor/256x256/apps/simsu.png");
		fallback.addFile(":/hicolor/128x128/apps/simsu.png");
		fallback.addFile(":/hicolor/64x64/apps/simsu.png");
		fallback.addFile(":/hicolor/48x48/apps/simsu.png");
		fallback.addFile(":/hicolor/32x32/apps/simsu.png");
		fallback.addFile(":/hicolor/24x24/apps/simsu.png");
		fallback.addFile(":/hicolor/22x22/apps/simsu.png");
		fallback.addFile(":/hicolor/16x16/apps/simsu.png");
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
		app.setWindowIcon(QIcon::fromTheme("simsu", fallback));
#else
		app.setWindowIcon(fallback);
#endif
	}

	LocaleDialog::loadTranslator("simsu_");

	Window window;
	window.show();

	return app.exec();
}
