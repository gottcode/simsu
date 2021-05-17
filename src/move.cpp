/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "move.h"

#include "cell.h"

//-----------------------------------------------------------------------------

Move::Move(Cell* cell, int id, int column, int row, bool note, int value)
	: m_cell(cell)
	, m_id(id)
{
	setText(QString("%1%2%3%4").arg(column).arg(row).arg(note ? "n" : "v").arg(value));
}

//-----------------------------------------------------------------------------

void Move::redo()
{
	m_cell->setState(m_id + 1);
}

//-----------------------------------------------------------------------------

void Move::undo()
{
	m_cell->setState(m_id);
}

//-----------------------------------------------------------------------------
