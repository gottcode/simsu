/***********************************************************************
 *
 * Copyright (C) 2009, 2011, 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef SIMSU_BOARD_H
#define SIMSU_BOARD_H

class Cell;
class Puzzle;

#include "frame.h"
class QLabel;
class QSettings;
class QUndoStack;

/**
 * The game board.
 *
 * This class controls all of the game objects as well as the move history.
 */
class Board : public Frame
{
	Q_OBJECT

public:
	/**
	 * Initializes board and starts a new game. If there is a saved game, it
	 * loads that instead of starting a new game.
	 */
	Board(QWidget* parent = 0);

	/**
	 * Cleans up board and removes current game if it is finished so that a new
	 * game will start on next program launch.
	 */
	~Board();

	/**
	 * Clears out current game and starts a new game.
	 *
	 * @param symmetry specify mirroring of givens; if @c -1 use previous symmetry, defaults to Pattern::Rotational180
	 * @param algorithm specify method to generate givens; if @c -1 use previous algorithm, defaults to PuzzleDancingLinks
	 */
	void newPuzzle(int symmetry = -1, int algorithm = -1);

	/**
	 * Clears out current game and loads a game.
	 *
	 * @return @c true if the puzzle could be loaded
	 */
	bool loadPuzzle();

	/**
	 * Saves current game.
	 */
	void savePuzzle();

	/** Returns currently selected number. */
	int activeKey() const
	{
		return m_active_key;
	}

	/** Returns @c true if auto switching modes is enabled; @c false otherwise */
	bool autoSwitch() const
	{
		return m_auto_switch;
	}

	/** Returns @c true if highlighting is enabled; @c false otherwise */
	bool highlightActive() const
	{
		return m_highlight_active;
	}

	/**
	 * Return the amount of times @p key appears on the board.
	 *
	 * @param key the number to check
	 * @return the amount of instances of a @p key
	 */
	int keyCount(int key) const
	{
		return m_key_count[key - 1];
	}

	/** Returns @c true if in notes mode, @c false if in answers mode. */
	bool notesMode() const
	{
		return m_notes_mode;
	}

	/** Returns @c true if game is over, @c false otherwise */
	bool isFinished() const
	{
		return m_finished;
	}

	/**
	 * Checks if the board has been completed successfully. If it has, it informs
	 * the player by showing the success message.
	 */
	void checkFinished();

	/**
	 * Move the focus highlight.
	 *
	 * @param column starting cell column
	 * @param row starting cell row
	 * @param deltax distance to move horizontally
	 * @param deltay distance to move vertically
	 */
	void moveFocus(int column, int row, int deltax, int deltay);

	/**
	 * Reduce amount of times @c key appears on the board.
	 *
	 * @param key the number which instances will be reduced
	 */
	void decreaseKeyCount(int key);

	/**
	 * Increase amount of times @c key appears on the board.
	 *
	 * @param key the number which instances will be increased
	 */
	void increaseKeyCount(int key);

	/**
	 * Fetch a cell instance.
	 *
	 * @param column cell column
	 * @param row cell row
	 * @return pointer to Cell instance; owned by Board
	 */
	Cell* cell(int column, int row) const
	{
		column = qBound(0, column, 8);
		row = qBound(0, row, 8);
		return m_cells[column][row];
	}

	/** Returns move history. */
	QUndoStack* moves()
	{
		return m_moves;
	}

public slots:
	/**
	 * If @p show is @c true, it highlights any cells that the player has filled
	 * incorrectly. Otherwise, it clears the highlight of incorrect cells.
	 */
	void showWrong(bool show = true);

	/**
	 * Set which key is currently being used.
	 *
	 * @param key the current key
	 */
	void setActiveKey(int key);

	/**
	 * Set which cell is currently active.
	 *
	 * @param cell the current cell
	 */
	void setActiveCell(Cell* cell);

	/**
	 * Sets if it should automatically switch between answers and notes mode.
	 *
	 * @param auto_switch if @c true enables auto-switching
	 */
	void setAutoSwitch(bool auto_switch);

	/**
	 * Sets if all instances of current key should be highlighted.
	 *
	 * @param highlight if @c true it will highlight all instances.
	 */
	void setHighlightActive(bool highlight);

	/**
	 * Sets if notes mode is enabled.
	 *
	 * @param mode if @c true notes mode is enabled; otherwise answers mode is enabled
	 */
	void setMode(int mode);

signals:
	/**
	 * This signal is emitted when the current key is changed.
	 *
	 * @param key current key
	 */
	void activeKeyChanged(int key);

	/**
	 * This signal is emitted when notes mode is enabled or disabled.
	 *
	 * @param mode @c true if notes mode is enabled
	 */
	void notesModeChanged(bool mode);

private:
	Cell* m_cells[9][9]; /**< game data */
	int m_key_count[9]; /**< how many instances of each key are on the board */
	Puzzle* m_puzzle; /**< the algorithm used to generate the board */
	int m_active_key; /**< the current key */
	Cell* m_active_cell; /**< the current cell */
	bool m_auto_switch; /**< auto-switching is enabled */
	bool m_highlight_active; /**< tracks if all instances of the current key should be highlighted */
	bool m_notes_mode; /**< tracks if in notes mode */
	bool m_finished; /**< tracks if game is finished */
	QLabel* m_message; /**< used to show messages to the player */
	QUndoStack* m_moves; /**< history of player actions */
};

#endif // SIMSU_BOARD_H
