/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_MOVE_H
#define SIMSU_MOVE_H

class Cell;

#include <QUndoCommand>

/**
 * Player move.
 *
 * This class represents an action by the player. It only stores the ID of the
 * current state of a cell so that it can tell the cell to switch states.
 */
class Move : public QUndoCommand
{
public:
	/**
	 * Constructs a move.
	 *
	 * @param cell the cell to modify
	 * @param id the current state of the cell
	 * @param column the column of the cell
	 * @param row the row of the cell
	 * @param note the note, if any, that was set in this move
	 * @param value the value, if any, that was set in this move
	 */
	Move(Cell* cell, int id, int column, int row, bool note, int value);

	/** Switch cell to the answer or notes of this move. */
	void redo() override;

	/** Switch cell to the answer or notes from before this move. */
	void undo() override;

private:
	Cell* m_cell; /**< cell to modify */
	int m_id; /**< state of cell to start from */
};

#endif // SIMSU_MOVE_H
