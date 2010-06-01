/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
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

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
class QBoxLayout;
class QButtonGroup;
class QToolButton;
class Board;

class Window : public QMainWindow {
	Q_OBJECT
public:
	Window();

protected:
	virtual void closeEvent(QCloseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);

private slots:
	void newGame();
	void showDetails();
	void showControls();
	void about();
	void activeKeyChanged(int key);
	void notesModeChanged(bool mode);
	void toggleMode();
	void toggleWidescreen(bool checked);

private:
	Board* m_board;
	QButtonGroup* m_key_buttons;
	QButtonGroup* m_mode_buttons;
	QBoxLayout* m_keys_layout;
	QBoxLayout* m_mode_layout;
	QBoxLayout* m_layout;
	QList<QToolButton*> m_sidebar_buttons;
};

#endif
