/*
	SPDX-FileCopyrightText: 2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_NEW_GAME_PAGE_H
#define SIMSU_NEW_GAME_PAGE_H

#include <QWidget>
class QLineEdit;
class QListWidget;
class QPushButton;
class QStackedWidget;

/**
 * @brief The NewGamePage class allows the player to choose the settings for a new game.
 */
class NewGamePage : public QWidget
{
	Q_OBJECT

public:
	/**
	 * Constructs the page.
	 */
	explicit NewGamePage(QWidget* parent = nullptr);

	/**
	 * Resets values and shows settings for computer generated games.
	 */
	void reset();

Q_SIGNALS:
	/**
	 * This signal is emitted when the player cancels starting a new game.
	 */
	void cancel();

	/**
	 * This signal is emitted when the player starts a new game.
	 *
	 * @param symmetry specify mirroring of givens
	 * @param difficulty specify how hard to make puzzle
	 */
	void generatePuzzle(int symmetry, int difficulty);

	/**
	 * This signal is emitted when the player starts a custom game.
	 *
	 * @param givens the values chosen by the player to build the board
	 */
	void loadPuzzle(const std::array<int, 81>& givens);

protected:
	/**
	 * Override parent function to start new game when enter is pressed.
	 */
	void keyPressEvent(QKeyEvent* event) override;

private Q_SLOTS:
	/**
	 * Handles starting a player-entered game.
	 */
	void playGame();

	/**
	 * Shows if player has entered givens that conflict with each other.
	 */
	void showConflicts();

	/**
	 * Switches what settings are visible to the player.
	 *
	 * @param show if @c true, show the widgets for building a custom game
	 */
	void showCustom(bool show);

private:
	QStackedWidget* m_contents; /**< contains widgets for computer games and player entered games */
	QPushButton* m_create_button; /**< button to show settings for a custom game */
	QPushButton* m_play_button; /**< button to start a custom game */

	QListWidget* m_symmetry; /**< list of board symmetries */
	QList<QPushButton*> m_difficulty; /**< buttons to choose difficulty and start game */
	int m_current_difficulty; /**< the difficulty most recently chosen by the player */

	QList<QLineEdit*> m_custom; /**< buttons to set the givens of a new game */
};

#endif // SIMSU_NEW_GAME_PAGE_H
