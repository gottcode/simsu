/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_WINDOW_H
#define SIMSU_WINDOW_H

class Board;
class NewGamePage;

#include <QMainWindow>
class QActionGroup;
class QBoxLayout;
class QButtonGroup;
class QLabel;
class QStackedWidget;
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
	explicit Window();

protected:
	/** Override parent function to save window geometry. */
	void closeEvent(QCloseEvent* event) override;

	/** Override parent function to handle scrolling through current keys. */
	void wheelEvent(QWheelEvent* event) override;

private Q_SLOTS:
	/**
	 * Start a new game.
	 *
	 * Shows a dialog of options that the player can adjust.
	 */
	void newGame();

	/** Handle player not starting a new game. */
	void newGameCanceled();

	/** Restart the current game. */
	void restartGame();

	/** Prints the current board. */
	void print();

	/** Show the current game details. */
	void showDetails();

	/** Show a dialog that explains how to interact with the game. */
	void showControls();

	/** Show the program details. */
	void about();

	/** Disable interface when game is over. */
	void gameFinished();

	/** Enable interface when game is ready to play. */
	void gameStarted();

	/**
	 * Set which key button is depressed.
	 *
	 * @param key the current key
	 */
	void activeKeyChanged(int key);

	/** Flatten buttons for values that are fully on the board. */
	void flattenUsedKeys();

	/**
	 * Set if notes or answer button should be depressed.
	 *
	 * @param mode if @c true notes button is active; otherwise answer button
	 */
	void notesModeChanged(bool mode);

	/**
	 * Sets how the board handles auto filling notes.
	 *
	 * @param action what notes fill mode to use
	 */
	void autoNotesChanged(QAction* action);

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
	QStackedWidget* m_contents; /**< the layers of display widgets */
	NewGamePage* m_new_game; /**< options to start a new game */
	QLabel* m_load_message; /**< shows player a wait screen */

	Board* m_board; /**< game board */

	QButtonGroup* m_key_buttons; /**< button group to choose which number is active */
	QButtonGroup* m_mode_buttons; /**< button group to choose if in notes or answer mode */
	QActionGroup* m_auto_notes_actions; /**< action group to choose mode for auto filling notes */
	QAction* m_new_action; /**< action for starting a game */
	QAction* m_restart_action; /** action for restarting the current game */
	QAction* m_print_action; /**< action for printing current board */
	QAction* m_details_action; /**< action for showing game details */
	QAction* m_undo_action; /**< action for undoing */
	QAction* m_redo_action; /**< action for redoing */
	QAction* m_check_action; /**< action for checking if cells are valid */
	QAction* m_hint_action; /**< action for getting a hint */

	QBoxLayout* m_keys_layout; /**< QLayout for key buttons */
	QBoxLayout* m_mode_layout; /**< QLayout for mode buttons */
	QBoxLayout* m_layout; /**< QLayout for widgets */
	QList<QToolButton*> m_sidebar_buttons; /**< interface buttons */
};

#endif // SIMSU_WINDOW_H
