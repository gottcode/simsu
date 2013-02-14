/***********************************************************************
 *
 * Copyright (C) 2009, 2013 Graeme Gott <graeme@gottcode.org>
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
	void redo();

	/** Switch cell to the answer or notes from before this move. */
	void undo();

private:
	Cell* m_cell; /**< cell to modify */
	int m_id; /**< state of cell to start from */
};

#endif // SIMSU_MOVE_H
