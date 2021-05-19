/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "board.h"

#include "cell.h"
#include "pattern.h"
#include "puzzle.h"
#include "solver_logic.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QSettings>
#include <QUndoStack>

//-----------------------------------------------------------------------------

Board::Board(QWidget* parent)
	: Frame(parent)
	, m_puzzle(new Puzzle)
	, m_notes(new SolverLogic)
	, m_active_key(1)
	, m_active_cell(nullptr)
	, m_auto_switch(true)
	, m_highlight_active(false)
	, m_notes_mode(false)
	, m_finished(false)
	, m_auto_notes(ManualNotes)
{
	setBackgroundRole(QPalette::Mid);

	m_moves = new QUndoStack(this);

	QGridLayout* layout = new QGridLayout(this);
	layout->setContentsMargins(3, 3, 3, 3);
	layout->setSpacing(0);

	// Create cells
	for (int i = 0; i < 9; ++i) {
		int col = (i % 3) * 3;
		int max_col = col + 3;
		int row = (i / 3) * 3;
		int max_row = row + 3;

		QGridLayout* box = new QGridLayout;
		box->setContentsMargins(2, 2, 2, 2);
		box->setSpacing(1);
		layout->addLayout(box, row / 3, col / 3);

		for (int r = row; r < max_row; ++r) {
			for (int c = col; c < max_col; ++c) {
				Cell* cell = new Cell(c, r, this, this);
				box->addWidget(cell, r - row, c - col);
				m_cells[c][r] = cell;
			}
		}
	}

	// Create success message
	m_message = new QLabel(this);
	QFontMetrics metrics(QFont("Sans", 24));
	int width = metrics.boundingRect(tr("Success")).width();
	int height = metrics.height();
	int ratio = devicePixelRatio();
	QPixmap success(QSize(width + height, height * 2) * ratio);
	success.setDevicePixelRatio(ratio);
	success.fill(QColor(0, 0, 0, 0));
	{
		QPainter painter(&success);

		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(0, 0, 0, 200));
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.drawRoundedRect(0, 0, width + height, height * 2, 10, 10);

		painter.setFont(QFont("Sans", 24));
		painter.setPen(Qt::white);
		painter.setRenderHint(QPainter::TextAntialiasing, true);
		painter.drawText(height / 2, height / 2 + metrics.ascent(), tr("Success"));
	}
	m_message->setPixmap(success);
	m_message->hide();
	layout->addWidget(m_message, 0, 0, 3, 3, Qt::AlignCenter);

	// Load current puzzle
	if (!loadPuzzle()) {
		newPuzzle();
	}
}

//-----------------------------------------------------------------------------

Board::~Board()
{
	QSettings settings;
	if (!m_finished) {
		settings.setValue("Key", m_active_key);
		savePuzzle();
	} else {
		settings.remove("Current");
	}
	delete m_puzzle;
	delete m_notes;
}

//-----------------------------------------------------------------------------

void Board::newPuzzle(int symmetry, int difficulty)
{
	QSettings settings;

	if (symmetry == -1) {
		symmetry = settings.value("Symmetry", Pattern::Rotational180).toInt();
	}
	if (symmetry == Pattern::Random) {
		symmetry = QRandomGenerator::global()->bounded(Pattern::Rotational180, Pattern::Random);
	}

	if (difficulty == -1) {
		difficulty = settings.value("Difficulty", Puzzle::VeryEasy).toInt();
	}

	// Reset board
	showWrong(false);
	m_finished = false;
	m_message->hide();
	m_moves->clear();
	for (int i = 0; i < 9; ++i) {
		m_key_count[i] = 0;
	}

	// Create puzzle
	m_puzzle->generate(symmetry, difficulty);

	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			m_cells[c][r]->setPuzzle(m_puzzle);
		}
	}
	updatePossibles();

	// Store puzzle details
	settings.remove("Current");
	settings.beginGroup("Current");
	settings.setValue("Version", 5);
	settings.setValue("Difficulty", difficulty);
	settings.setValue("Symmetry", symmetry);

	// Store puzzle layout
	savePuzzle();
}

//-----------------------------------------------------------------------------

bool Board::loadPuzzle()
{
	QSettings settings;
	settings.beginGroup("Current");

	// Check version number
	if (settings.value("Version", 0).toInt() != 5) {
		return false;
	}

	// Load board layout
	const QString cells = settings.value("Board").toString();
	if (cells.length() != 81) {
		return false;
	}

	// Create puzzle
	std::array<int, 81> givens;
	for (int i = 0; i < 81; ++i) {
		givens[i] = cells[i].digitValue();
	}

	if (!m_puzzle->load(givens)) {
		return false;
	}

	// Reset board
	showWrong(false);
	m_finished = false;
	m_message->hide();
	m_moves->clear();
	for (int i = 0; i < 9; ++i) {
		m_key_count[i] = 0;
	}

	// Load puzzle
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			m_cells[c][r]->setPuzzle(m_puzzle);
		}
	}
	updatePossibles();

	// Load moves
	const QStringList moves = settings.value("Moves").toStringList();
	for (const QString& move : moves) {
		if (move.length() == 4) {
			m_notes_mode = (move[2] == 'n');
			Cell* c = cell(move[0].digitValue(), move[1].digitValue());
			QKeyEvent event(QEvent::KeyPress, Qt::Key_0 + move[3].digitValue(), Qt::NoModifier);
			QApplication::sendEvent(c, &event);
		}
	}
	m_notes_mode = false;

	// Select current cell
	const QStringList active = settings.value("Active").toString().split('x');
	if (active.count() == 2) {
		m_active_cell = cell(active[0].toInt(), active[1].toInt());
		m_active_cell->setFocus();
	}

	// Store puzzle details
	const int difficulty = settings.value("Difficulty").toInt();
	const int symmetry = settings.value("Symmetry").toInt();

	settings.endGroup();
	settings.remove("Current");
	settings.beginGroup("Current");
	settings.setValue("Version", 5);
	settings.setValue("Difficulty", difficulty);
	settings.setValue("Symmetry", symmetry);

	// Store puzzle layout and moves
	savePuzzle();

	return true;
}

//-----------------------------------------------------------------------------

void Board::savePuzzle()
{
	QSettings settings;
	settings.beginGroup("Current");

	// Store board layout
	QString cells;
	cells.reserve(81);
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			cells.append(QChar(m_puzzle->given(c, r) + '0'));
		}
	}
	settings.setValue("Board", cells);

	// Store moves
	QStringList moves;
	int count = m_moves->index();
	for (int i = 0; i < count; ++i) {
		moves += m_moves->text(i);
	}
	if (count) {
		settings.setValue("Moves", moves);
	}

	// Store current cell
	if (m_active_cell) {
		settings.setValue("Active", QString("%1x%2").arg(m_active_cell->column()).arg(m_active_cell->row()));
	}
}

//-----------------------------------------------------------------------------

bool Board::hasPossible(int column, int row, int value) const
{
	return m_notes->hasPossible(column, row, value);
}

//-----------------------------------------------------------------------------

void Board::checkFinished()
{
	m_finished = true;
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			m_finished = m_finished && m_cells[c][r]->isCorrect();
		}
	}

	if (m_finished) {
		for (int r = 0; r < 9; ++r) {
			for (int c = 0; c < 9; ++c) {
				m_cells[c][r]->clearFocus();
				m_cells[c][r]->setFocusPolicy(Qt::NoFocus);
			}
		}
		m_message->show();
		update();
	}
}

//-----------------------------------------------------------------------------

void Board::showWrong(bool show)
{
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			m_cells[c][r]->showWrong(show);
		}
	}
}

//-----------------------------------------------------------------------------

void Board::moveFocus(int column, int row, int xdelta, int ydelta)
{
	xdelta = qBound(-1, xdelta, 2);
	ydelta = qBound(-1, ydelta, 2);
	Q_ASSERT(xdelta != ydelta);

	if (column + xdelta < 0) {
		column = 9;
	} else if (column + xdelta > 8) {
		column = -1;
	}
	column += xdelta;

	if (row + ydelta < 0) {
		row = 9;
	} else if (row + ydelta > 8) {
		row = -1;
	}
	row += ydelta;

	m_cells[column][row]->setFocus();
}

//-----------------------------------------------------------------------------

void Board::decreaseKeyCount(int key)
{
	key--;
	if (key < 0 || key > 8) {
		return;
	}
	m_key_count[key]--;
}

//-----------------------------------------------------------------------------

void Board::increaseKeyCount(int key)
{
	key--;
	if (key < 0 || key > 8) {
		return;
	}
	m_key_count[key]++;
}

//-----------------------------------------------------------------------------

void Board::updatePossibles()
{
	std::array<int, 81> givens;
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			givens[c + (r * 9)] = m_cells[c][r]->value();
		}
	}
	m_notes->loadPuzzle(givens);
}

//-----------------------------------------------------------------------------

QList<Cell*> Board::cellNeighbors(int column, int row) const
{
	QList<Cell*> cells;

	// Fetch neighbors in row
	for (unsigned int c = 0; c < 9; ++c) {
		Cell* cell = m_cells[c][row];
		if (!cells.contains(cell)) {
			cells.append(cell);
		}
	}

	// Fetch neighbors in column
	for (unsigned int r = 0; r < 9; ++r) {
		Cell* cell = m_cells[column][r];
		if (!cells.contains(cell)) {
			cells.append(cell);
		}
	}

	// Fetch neighbors in box
	const unsigned int box_r = (row / 3) * 3;
	const unsigned int box_c = (column / 3) * 3;
	for (unsigned int r = box_r; r < (box_r + 3); ++r) {
		for (unsigned int c = box_c; c < (box_c + 3); ++c) {
			Cell* cell = m_cells[c][r];
			if (!cells.contains(cell)) {
				cells.append(cell);
			}
		}
	}

	// Do not include cell in neighbors list
	cells.removeOne(m_cells[column][row]);

	return cells;
}

//-----------------------------------------------------------------------------

void Board::setActiveKey(int key)
{
	m_active_key = qBound(1, key, 10);
	update();
	emit activeKeyChanged(m_active_key);
}

//-----------------------------------------------------------------------------

void Board::setActiveCell(Cell* cell)
{
	m_active_cell = cell;
}

//-----------------------------------------------------------------------------

void Board::setAutoNotes(int auto_notes)
{
	m_auto_notes = qBound(ManualNotes, AutoNotes(auto_notes), AutoFillNotes);

	QString name;
	switch (m_auto_notes) {
	case AutoClearNotes:
		name = "Clear";
		break;
	case AutoFillNotes:
		name = "Fill";
		break;
	case ManualNotes:
	default:
		name = "None";
		break;
	}
	QSettings().setValue("AutoNotes", name);

	update();
}

//-----------------------------------------------------------------------------

void Board::setAutoSwitch(bool auto_switch)
{
	m_auto_switch = auto_switch;
	QSettings().setValue("AutoSwitch", m_auto_switch);
}

//-----------------------------------------------------------------------------

void Board::setHighlightActive(bool highlight)
{
	m_highlight_active = highlight;
	QSettings().setValue("Highlight", m_highlight_active);
	update();
}

//-----------------------------------------------------------------------------

void Board::setMode(int mode)
{
	m_notes_mode = mode;
	QSettings().setValue("Mode", (m_notes_mode ? "Pencil" : "Pen"));
	emit notesModeChanged(mode);
}

//-----------------------------------------------------------------------------
