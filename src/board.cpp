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
#include <QPagedPaintDevice>
#include <QPageLayout>
#include <QPainter>
#include <QSettings>
#include <QUndoStack>

//-----------------------------------------------------------------------------

Board::Board(QWidget* parent)
	: Frame(parent)
	, m_puzzle(new Puzzle(this))
	, m_notes(new SolverLogic)
	, m_active_key(1)
	, m_active_cell(nullptr)
	, m_hint_cell(nullptr)
	, m_auto_switch(true)
	, m_highlight_active(false)
	, m_notes_mode(false)
	, m_finished(false)
	, m_loaded(false)
	, m_auto_notes(ManualNotes)
{
	setBackgroundRole(QPalette::Mid);

	connect(m_puzzle, &Puzzle::generated, this, [this](int symmetry, int difficulty) {
		puzzleGenerated(symmetry, difficulty);
		savePuzzle();
	});

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

	delete m_notes;
}

//-----------------------------------------------------------------------------

void Board::newPuzzle(int symmetry, int difficulty)
{
	QSettings settings;

	if (symmetry == -1) {
		symmetry = settings.value("Symmetry", Pattern::Rotational180).toInt();
	}
	settings.setValue("Symmetry", symmetry);
	if (symmetry == Pattern::Random) {
		symmetry = QRandomGenerator::global()->bounded(Pattern::Rotational180, Pattern::Random);
	}

	if (difficulty == -1) {
		difficulty = settings.value("Difficulty", Puzzle::VeryEasy).toInt();
	}
	settings.setValue("Difficulty", difficulty);

	// Create puzzle
	reset();
	m_puzzle->generate(symmetry, difficulty);
}

//-----------------------------------------------------------------------------

bool Board::newPuzzle(const std::array<int, 81>& givens)
{
	// Create puzzle
	if (!m_puzzle->load(givens)) {
		return false;
	}

	// Load puzzle
	reset();
	puzzleGenerated(Pattern::None, SolverLogic().solvePuzzle(givens, Puzzle::Unsolved));

	// Store puzzle layout
	savePuzzle();

	return true;
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

	// Fetch puzzle details
	const int difficulty = settings.value("Difficulty").toInt();
	const int symmetry = settings.value("Symmetry").toInt();
	const QStringList moves = settings.value("Moves").toStringList();
	const QStringList active = settings.value("Active").toString().split('x');

	// Load puzzle
	reset();
	puzzleGenerated(symmetry, difficulty);

	// Load moves
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
	if (active.count() == 2) {
		m_active_cell = cell(active[0].toInt(), active[1].toInt());
		m_active_cell->setFocus();
	}

	// Store puzzle layout and moves
	savePuzzle();

	return true;
}

//-----------------------------------------------------------------------------

void Board::savePuzzle()
{
	if (!m_loaded) {
		return;
	}

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

void Board::restartPuzzle()
{
	if (!m_loaded) {
		return;
	}

	// Fetch puzzle details
	QSettings settings;
	const int difficulty = settings.value("Difficulty").toInt();
	const int symmetry = settings.value("Symmetry").toInt();

	// Load puzzle
	reset();
	puzzleGenerated(symmetry, difficulty);
}

//-----------------------------------------------------------------------------

bool Board::hasPossible(int column, int row, int value) const
{
	return m_notes->hasPossible(column, row, value);
}

//-----------------------------------------------------------------------------

void Board::print(QPagedPaintDevice* printer) const
{
	if (!m_loaded) {
		return;
	}

	// Make sure page margins are at least 2cm
	QMarginsF margins = printer->pageLayout().margins(QPageLayout::Millimeter);
	qreal min = 20.0;
	if (margins.left() > min) {
		min = margins.left();
	}
	if (margins.top() > min) {
		min = margins.top();
	}
	if (margins.right() > min) {
		min = margins.right();
	}
	if (margins.bottom() > min) {
		min = margins.bottom();
	}
	printer->setPageMargins(QMarginsF(min, min, min, min), QPageLayout::Millimeter);

	// Begin painting
	QPainter painter;
	painter.begin(printer);

	// Center board on the page
	QRect rect = painter.viewport();
	const qreal scale = rect.width() / 472.0;
	if (rect.height() > rect.width()) {
		const int offset = (rect.height() - rect.width()) / 2;
		painter.translate(0, offset);
		rect.setHeight(rect.width());
	} else {
		const int offset = (rect.width() - rect.height()) / 2;
		painter.translate(offset, 0);
		rect.setWidth(rect.height());
	}

	// Operate on a scaled viewport so that calculations are easier
	painter.save();
	painter.scale(scale, scale);

	// Draw box divider lines
	painter.setPen(QPen(Qt::black, 4));
	for (int i = 0; i < 4; ++i) {
		int pos = (152 * i) + (i * 4) + 2;
		painter.drawLine(2, pos, 470, pos);
		painter.drawLine(pos, 2, pos, 470);
	}

	// Draw cell divider lines
	painter.setPen(QPen(Qt::black, 1));
	for (int i = 1; i < 3; ++i) {
		int pos = (51 * i) + 3;
		painter.drawLine(1, pos, 471, pos);
		painter.drawLine(pos, 1, pos, 471);
	}
	for (int i = 4; i < 6; ++i) {
		int pos = (51 * i) + 6;
		painter.drawLine(1, pos, 471, pos);
		painter.drawLine(pos, 1, pos, 471);
	}
	for (int i = 7; i < 9; ++i) {
		int pos = (51 * i) + 9;
		painter.drawLine(1, pos, 471, pos);
		painter.drawLine(pos, 1, pos, 471);
	}

	// Draw values
	const QPen pen_value(QColor(0x88, 0x88, 0x88));
	const QPen pen_given(Qt::black);

	QFont font_value = painter.font();
	font_value.setPixelSize(24);

	QFont font_given = font_value;
	font_given.setBold(true);

	for (int r = 0; r < 9; ++r) {
		const int r_offset = 4 + (r % 3) + ((r / 3) * 6);
		for (int c = 0; c < 9; ++c) {
			int given = m_puzzle->given(c, r);
			int value = m_cells[c][r]->value();
			if (given) {
				value = given;
				painter.setPen(pen_given);
				painter.setFont(font_given);
			} else if (value) {
				painter.setPen(pen_value);
				painter.setFont(font_value);
			} else {
				continue;
			}

			const int c_offset = 4 + (c % 3) + ((c / 3) * 6);
			painter.drawText((c * 50) + c_offset, (r * 50) + r_offset, 50, 50, Qt::AlignCenter, QString::number(value));
		}
	}

	// Finish
	painter.restore();
	painter.end();
}

//-----------------------------------------------------------------------------

void Board::checkFinished()
{
	bool was_finished = m_finished;

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
		emit gameFinished();
	} else if (was_finished) {
		m_message->hide();
		update();
		emit gameStarted();
	}
}

//-----------------------------------------------------------------------------

void Board::hint()
{
	if (m_finished) {
		return;
	}

	if (!m_hint_cell || m_hint_cell->isCorrect()) {
		// Find status of cells
		QList<Cell*> incorrect;
		QList<Cell*> empty;
		empty.reserve(81);
		for (int r = 0; r < 9; ++r) {
			for (int c = 0; c < 9; ++c) {
				Cell* cell = m_cells[c][r];
				if (cell->isCorrect()) {
					continue;
				}
				if (cell->value()) {
					incorrect.append(cell);
				} else {
					empty.append(cell);
				}
			}
		}

		// Show an incorrect cell if they exist
		if (!incorrect.isEmpty()) {
			std::shuffle(incorrect.begin(), incorrect.end(), *QRandomGenerator::global());
			m_hint_cell = incorrect.first();
			m_hint_cell->showWrong(true);
			return;
		}

		// Find a cell to fill
		if (!empty.isEmpty()) {
			std::shuffle(empty.begin(), empty.end(), *QRandomGenerator::global());
			m_hint_cell = empty.first();
		}
	}

	// Fill cell with correct value
	if (m_hint_cell) {
		const int key = m_puzzle->value(m_hint_cell->column(), m_hint_cell->row());
		QKeyEvent event(QEvent::KeyPress, Qt::Key_0 + key, Qt::NoModifier);
		QApplication::sendEvent(m_hint_cell, &event);
		m_hint_cell->setFocus();
		m_hint_cell = nullptr;
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

void Board::puzzleGenerated(int symmetry, int difficulty)
{
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			m_cells[c][r]->setPuzzle(m_puzzle);
		}
	}

	updatePossibles();

	// Store puzzle details
	QSettings settings;
	settings.remove("Current");
	settings.beginGroup("Current");
	settings.setValue("Version", 5);
	settings.setValue("Difficulty", difficulty);
	settings.setValue("Symmetry", symmetry);

	m_loaded = true;

	emit gameStarted();
}

//-----------------------------------------------------------------------------

void Board::reset()
{
	showWrong(false);
	m_finished = false;
	m_loaded = false;
	m_message->hide();
	m_moves->clear();
	for (int i = 0; i < 9; ++i) {
		m_key_count[i] = 0;
	}
}

//-----------------------------------------------------------------------------
