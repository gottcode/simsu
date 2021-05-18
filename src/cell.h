/*
	SPDX-FileCopyrightText: 2009-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_CELL_H
#define SIMSU_CELL_H

#include "frame.h"
class Board;
class Puzzle;

/**
 * %Cell on the game board.
 *
 * This class represents a single number cell on the board. It tracks what
 * answers or notes the player has set in a list of Cell::State instances.
 * It also tracks if the current state is an incorrect answer, as well as
 * which other cells it may be in conflict with.
 */
class Cell : public Frame
{
public:
	/**
	 * Constrcts a cell.
	 *
	 * @param column cell column
	 * @param row cell row
	 * @param board the game board
	 * @param parent the parent widget
	 */
	Cell(int column, int row, Board* board, QWidget* parent = nullptr);

	/** @return column of cell. */
	int column() const;

	/** @return row of cell. */
	int row() const;

	/** Returns @c true if cell is correct value; @c false otherwise. */
	bool isCorrect() const;

	/** Returns the current value of the cell. */
	int value() const
	{
		return m_states[m_current_state].value;
	}

	/**
	 * Sets the puzzle used to determine if cell is a given, and what value it
	 * should have.
	 *
	 * @param puzzle the puzzle instance
	 */
	void setPuzzle(Puzzle* puzzle);

	/**
	 * Set which state to use for values.
	 *
	 * @param state the state to use for values.
	 */
	void setState(int state);

	/** Sets if it should show player if the value they set is wrong. */
	void showWrong(bool show);

protected:
	/** Override parent function to add partial highlight to cells in column and row. */
	void focusInEvent(QFocusEvent* event) override;

	/** Override parent function to remove partial highlight from cells in column and row. */
	void focusOutEvent(QFocusEvent* event) override;

	/** Override parent function to handle moving focus or inputting a guess. */
	void keyPressEvent(QKeyEvent* event) override;

	/** Override parent function to make sure cell has focus highlight. */
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	void enterEvent(QEnterEvent* event) override;
#else
	void enterEvent(QEvent* event) override;
#endif

	/** Override parent function to handle inputting a guess. */
	void mousePressEvent(QMouseEvent* event) override;

	/** Override parent function to draw cell value and highlight. */
	void paintEvent(QPaintEvent* event) override;

	/** Override parent function to set font size used in paintEvent(). */
	void resizeEvent(QResizeEvent* event) override;

private:
	/**
	 * %Cell state class.
	 *
	 * This class is used to track what answer and guesses a player has inputted.
	 */
	struct State
	{
		int value; /**< answer set by player */
		bool notes[9]; /**< notes set by player */
	};
	QList<State> m_states; /**< list of all changes player has made */
	int m_current_state; /**< which state is current */

private:
	/**
	 * Determine if cell has the same answer as another cell.
	 *
	 * @param cell the cell to check for conflicts
	 */
	void checkConflict(Cell* cell);

	/**
	 * Store answer or notes.
	 *
	 * Inserts a new state containing answer or notes. It first removes all
	 * states after the current state to make undo/redo work properly.
	 */
	void updateValue();

	/**
	 * Determine font to use for in paintEvent().
	 *
	 * Checks if current state has notes or answer and choose a font size
	 * that is appropriate for notes or answers.
	 */
	void updateFont();

private:
	int m_column; /**< column on board */
	int m_row; /**< row on board */
	QList<Cell*> m_conflicts; /**< cells the current state conflicts with */
	bool m_wrong; /**< track if value of current state is wrong */
	bool m_given; /**< @c true if a given on the board and therefore not editable */
	Board* m_board; /**< the game board */
	Puzzle* m_puzzle; /**< the algorithm used to generate the board */
};

#endif // SIMSU_CELL_H
