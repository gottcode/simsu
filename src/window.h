/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_WINDOW_H
#define SIMSU_WINDOW_H

class Board;

#include <QMainWindow>
class QBoxLayout;
class QButtonGroup;
class QToolButton;

/**
 * Main window of the game.
 *
 * This class is the main window of the game. It handles the menubar and
 * placement of the interface buttons.
 */
class Window : public QMainWindow
{
	Q_OBJECT

public:
	/**
	 * Constructs the main window.
	 */
	Window();

protected:
	/** Override parent function to save window geometry. */
	void closeEvent(QCloseEvent* event) override;

	/** Override parent function to handle scrolling through current keys. */
	void wheelEvent(QWheelEvent* event) override;

private slots:
	/**
	 * Start a new game.
	 *
	 * Shows a dialog of options that the player can adjust.
	 */
	void newGame();

	/** Show the current game details. */
	void showDetails();

	/** Show a dialog that explains how to interact with the game. */
	void showControls();

	/** Show the program details. */
	void about();

	/**
	 * Set which key button is depressed.
	 *
	 * @param key the current key
	 */
	void activeKeyChanged(int key);

	/**
	 * Set if notes or answer button should be depressed.
	 *
	 * @param mode if @c true notes button is active; otherwise answer button
	 */
	void notesModeChanged(bool mode);

	/** Switch between entering answers and notes. */
	void toggleMode();

	/**
	 * Switch window layouts.
	 *
	 * @param checked if @c true buttons are on left and right; otherwise on bottom
	 */
	void toggleWidescreen(bool checked);

	/** Allows the player to change the application language. */
	void setLocaleClicked();

private:
	Board* m_board; /**< game board */
	QButtonGroup* m_key_buttons; /**< button group to choose which number is active */
	QButtonGroup* m_mode_buttons; /**< button group to choose if in notes or answer mode */
	QBoxLayout* m_keys_layout; /**< QLayout for key buttons */
	QBoxLayout* m_mode_layout; /**< QLayout for mode buttons */
	QBoxLayout* m_layout; /**< QLayout for widgets */
	QList<QToolButton*> m_sidebar_buttons; /**< interface buttons */
};

#endif // SIMSU_WINDOW_H
