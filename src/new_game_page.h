/*
	SPDX-FileCopyrightText: 2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_NEW_GAME_PAGE_H
#define SIMSU_NEW_GAME_PAGE_H

#include <QWidget>
class QListWidget;
class QPushButton;

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

signals:
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

private:
	QListWidget* m_symmetry; /**< list of board symmetries */
	QList<QPushButton*> m_difficulty; /**< buttons to choose difficulty and start game */
};

#endif // SIMSU_NEW_GAME_PAGE_H
