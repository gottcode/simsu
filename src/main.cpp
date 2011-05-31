/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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
	app.setApplicationVersion("1.2.1");
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");

	LocaleDialog::loadTranslator("simsu_");

	Window window;
	window.show();

	return app.exec();
}
