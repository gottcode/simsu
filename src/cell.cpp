/***********************************************************************
 *
 * Copyright (C) 2009-2020 Graeme Gott <graeme@gottcode.org>
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

#include "cell.h"

#include "board.h"
#include "move.h"
#include "puzzle.h"

#include <QKeyEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QUndoStack>

#include <algorithm>

//-----------------------------------------------------------------------------

static int cell_size = 0;
static int pen_size = 1;
static int pencil_size = 1;

//-----------------------------------------------------------------------------

Cell::Cell(int column, int row, Board* board, QWidget* parent) :
	Frame(parent),
	m_current_state(0),
	m_column(column),
	m_row(row),
	m_wrong(false),
	m_given(false),
	m_board(board),
	m_puzzle(0)
{
	State state;
	state.value = 0;
	for (int i = 0; i < 9; ++i) {
		state.notes[i] = false;
	}
	m_states.append(state);

	setFocusPolicy(Qt::StrongFocus);
	setForegroundRole(QPalette::Text);
	setMinimumSize(35, 35);
}

//-----------------------------------------------------------------------------

int Cell::column() const
{
	return m_column;
}

//-----------------------------------------------------------------------------

int Cell::row() const
{
	return m_row;
}

//-----------------------------------------------------------------------------

bool Cell::isCorrect() const
{
	Q_ASSERT(m_puzzle != 0);
	return m_states[m_current_state].value == m_puzzle->value(m_column, m_row);
}

//-----------------------------------------------------------------------------

void Cell::setPuzzle(Puzzle* puzzle)
{
	Q_ASSERT(puzzle != 0);
	m_puzzle = puzzle;

	m_conflicts.clear();
	State state = m_states.first();
	m_states.clear();
	m_states.append(state);
	m_current_state = 0;

	state.value = m_puzzle->given(m_column, m_row);
	m_given = (state.value > 0);
	if (m_given) {
		m_states.append(state);
		m_current_state++;
		m_board->increaseKeyCount(state.value);
	}
	updateFont();

	QFont f = font();
	if (m_given) {
		setBackgroundRole(QPalette::AlternateBase);
		f.setBold(true);
	} else {
		setBackgroundRole(QPalette::Base);
		f.setBold(false);
	}
	setFont(f);
}

//-----------------------------------------------------------------------------

void Cell::setState(int state)
{
	// Check if all cells with matching values are found
	m_board->decreaseKeyCount(m_states[m_current_state].value);
	m_board->increaseKeyCount(m_states[state].value);

	m_current_state = state;

	// Check for conflicts
	for (Cell* cell : m_conflicts) {
		cell->m_conflicts.removeOne(this);
		cell->update();
	}
	m_conflicts.clear();

	for (int c = 0; c < 9; ++c) {
		checkConflict(m_board->cell(c, m_row));
	}

	for (int r = 0; r < 9; ++r) {
		checkConflict(m_board->cell(m_column, r));
	}

	int col = (m_column / 3) * 3;
	int max_col = col + 3;
	int row = (m_row / 3) * 3;
	int max_row = row + 3;
	for (int r = row; r < max_row; ++r) {
		for (int c = col; c < max_col; ++c) {
			checkConflict(m_board->cell(c, r));
		}
	}

	// Redraw cell
	updateFont();
	m_board->showWrong(false);

	// Check if game is over
	m_board->checkFinished();
}

//-----------------------------------------------------------------------------

void Cell::showWrong(bool show)
{
	m_wrong = show ? !isCorrect() : false;
	update();
}

//-----------------------------------------------------------------------------

void Cell::focusInEvent(QFocusEvent* event)
{
	m_board->setActiveCell(this);

	if (!m_given) {
		setBackgroundRole(QPalette::Highlight);
		setForegroundRole(QPalette::HighlightedText);
	} else {
		setHighlightBorder(true);
	}

	// Highlight column and row
	for (int c = 0; c < 9; ++c) {
		Cell* cell = m_board->cell(c, m_row);
		if (cell != this) {
			cell->setHighlightPartial(true);
			cell->update();
		}
	}
	for (int r = 0; r < 9; ++r) {
		Cell* cell = m_board->cell(m_column, r);
		if (cell != this) {
			cell->setHighlightPartial(true);
			cell->update();
		}
	}

	Frame::focusInEvent(event);
}

//-----------------------------------------------------------------------------

void Cell::focusOutEvent(QFocusEvent* event)
{
	if (!m_given) {
		setBackgroundRole(QPalette::Base);
		setForegroundRole(QPalette::Text);
	} else {
		setHighlightBorder(false);
	}

	// Remove highlight of column and row
	for (int c = 0; c < 9; ++c) {
		Cell* cell = m_board->cell(c, m_row);
		if (cell != this) {
			cell->setHighlightPartial(false);
			cell->update();
		}
	}
	for (int r = 0; r < 9; ++r) {
		Cell* cell = m_board->cell(m_column, r);
		if (cell != this) {
			cell->setHighlightPartial(false);
			cell->update();
		}
	}

	Frame::focusOutEvent(event);
}

//-----------------------------------------------------------------------------

void Cell::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_1:
	case Qt::Key_2:
	case Qt::Key_3:
	case Qt::Key_4:
	case Qt::Key_5:
	case Qt::Key_6:
	case Qt::Key_7:
	case Qt::Key_8:
	case Qt::Key_9:
		if (!m_given) {
			m_board->setActiveKey(event->key() - Qt::Key_0);
			updateValue();
		}
		break;
	case Qt::Key_Left:
		m_board->moveFocus(m_column, m_row, -1, 0);
		break;
	case Qt::Key_Right:
		m_board->moveFocus(m_column, m_row, 1, 0);
		break;
	case Qt::Key_Up:
		m_board->moveFocus(m_column, m_row, 0, -1);
		break;
	case Qt::Key_Down:
		m_board->moveFocus(m_column, m_row, 0, 1);
		break;
	default:
		break;
	}
	Frame::keyPressEvent(event);
}

//-----------------------------------------------------------------------------

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
void Cell::enterEvent(QEnterEvent* event)
#else
void Cell::enterEvent(QEvent* event)
#endif
{
	setFocus();
	Frame::enterEvent(event);
}

//-----------------------------------------------------------------------------

void Cell::mousePressEvent(QMouseEvent* event)
{
	if (!m_given && !m_board->isFinished()) {
		if (m_board->autoSwitch()) {
			if (event->button() == Qt::LeftButton) {
				m_board->setMode(false);
			} else if (event->button() == Qt::RightButton) {
				m_board->setMode(true);
			}
		}
		updateValue();
	}
	Frame::mousePressEvent(event);
}

//-----------------------------------------------------------------------------

void Cell::paintEvent(QPaintEvent* event)
{
	setHighlight(m_board->highlightActive() && m_states[m_current_state].value == m_board->activeKey());
	Frame::paintEvent(event);

	QPainter painter(this);

	const State& state = m_states[m_current_state];

	if (state.value) {
		QColor color = palette().color(foregroundRole());
		if (m_wrong) {
			color = Qt::blue;
		} else if (!m_conflicts.isEmpty()) {
			color = Qt::red;
		} else if (m_board->highlightActive() && m_states[m_current_state].value == m_board->activeKey() && m_board->keyCount(state.value) == 9) {
			color = palette().highlightedText().color();
		}
		painter.setPen(color);

		painter.drawText(rect(), Qt::AlignCenter, QString::number(state.value));
	} else {
		painter.setPen(palette().color(foregroundRole()));

		int w = (width() - 8) / 3;
		int h = (height() - 8) / 3;
		for (int i = 0; i < 9; ++i) {
			int c = i % 3;
			int r = i / 3;
			if (state.notes[i]) {
				painter.drawText(QRect(c * w + 4, r * h + 4, w, h), Qt::AlignCenter, QString::number(i + 1));
			}
		}
	}
}

//-----------------------------------------------------------------------------

void Cell::resizeEvent(QResizeEvent* event)
{
	int size = std::min(event->size().width(), event->size().height());
	if (cell_size != size) {
		cell_size = size;
		pen_size = size - 8;
		pencil_size = pen_size / 3;
	}
	updateFont();
}

//-----------------------------------------------------------------------------

void Cell::checkConflict(Cell* cell)
{
	if (cell != this && !m_conflicts.contains(cell)) {
		if (cell->m_states[cell->m_current_state].value == m_states[m_current_state].value) {
			m_conflicts.append(cell);
			cell->m_conflicts.append(this);
			cell->update();
		}
	}
}

//-----------------------------------------------------------------------------

void Cell::updateValue()
{
	// Find key pressed
	int key = m_board->activeKey();

	State state = m_states[m_current_state];
	if (m_board->notesMode()) {
		// Toggle note
		state.notes[key - 1] = !state.notes[key - 1];
		state.value = 0;
	} else {
		// Toggle value
		for (int i = 0; i < 9; ++i) {
			state.notes[i] = false;
		}
		state.value = (key != state.value) ? key : 0;
	}

	// Add state to list of states
	m_states = m_states.mid(0, m_current_state + 1);
	m_states.append(state);
	m_board->moves()->push(new Move(this, m_current_state, m_column, m_row, state.value == 0, key));
}

//-----------------------------------------------------------------------------

void Cell::updateFont()
{
	QFont f = font();
	f.setPixelSize(m_states[m_current_state].value ? pen_size : pencil_size);
	setFont(f);
}

//-----------------------------------------------------------------------------
