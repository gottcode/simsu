/*
	SPDX-FileCopyrightText: 2009-2016 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "puzzle.h"

#include "pattern.h"
#include "solver_dlx.h"
#include "solver_logic.h"

#include <QtConcurrentRun>

#include <algorithm>

//-----------------------------------------------------------------------------

Puzzle::Puzzle(QObject* parent)
	: QObject(parent)
	, m_random(QRandomGenerator::securelySeeded())
	, m_pattern(nullptr)
	, m_difficulty(VeryEasy)
	, m_generated(INT_MAX)
	, m_canceled(false)
{
}

//-----------------------------------------------------------------------------

Puzzle::~Puzzle()
{
	cancel();
	delete m_pattern;
}

//-----------------------------------------------------------------------------

void Puzzle::cancel()
{
	blockSignals(true);
	m_canceled.store(true, std::memory_order_relaxed);
	m_future.waitForFinished();
	m_canceled.store(false, std::memory_order_relaxed);
	blockSignals(false);
}

//-----------------------------------------------------------------------------

void Puzzle::generate(int symmetry, int difficulty)
{
	cancel();

	delete m_pattern;
	switch (symmetry) {
	case Pattern::FullDihedral:
		m_pattern = new PatternFullDihedral;
		break;
	case Pattern::Rotational180:
		m_pattern = new PatternRotational180;
		break;
	case Pattern::RotationalFull:
		m_pattern = new PatternRotationalFull;
		break;
	case Pattern::Horizontal:
		m_pattern = new PatternHorizontal;
		break;
	case Pattern::Vertical:
		m_pattern = new PatternVertical;
		break;
	case Pattern::HorizontalVertical:
		m_pattern = new PatternHorizontalVertical;
		break;
	case Pattern::Diagonal:
		m_pattern = new PatternDiagonal;
		break;
	case Pattern::AntiDiagonal:
		m_pattern = new PatternAntiDiagonal;
		break;
	case Pattern::DiagonalAntiDiagonal:
		m_pattern = new PatternDiagonalAntiDiagonal;
		break;
	case Pattern::None:
	default:
		m_pattern = new PatternNone;
		break;
	}

	m_difficulty = qBound(VeryEasy, Difficulty(difficulty), Hard);

	m_future = QtConcurrent::run([this, symmetry]() {
		int givens = 0;
		do {
			m_generated = INT_MAX;
			createSolution();
			if (!createGivens()) {
				return;
			}

			givens = 81;
			for (int i = 0; i < 81; ++i) {
				givens -= !m_givens[i];
			}
		} while ((givens > 30) || (m_generated != m_difficulty));

		emit generated(symmetry, m_difficulty);
	});
}

//-----------------------------------------------------------------------------

bool Puzzle::load(const std::array<int, 81>& givens)
{
	cancel();

	SolverDLX solver;
	if (solver.solvePuzzle(givens)) {
		m_givens = givens;
		m_solution = solver.solution();
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------

void Puzzle::createSolution()
{
	// Create list of initial values
	static const QList<int> initial{1,2,3,4,5,6,7,8,9};
	std::array<QList<int>, 81> cells;
	cells.fill(initial);

	// Reset solution grid
	m_solution.fill(0);

	// Fill solution grid
	for (int i = 0; i < 81; ++i) {
		const unsigned int r = i / 9;
		const unsigned int c = i % 9;
		const unsigned int box_row = (r / 3) * 3;
		const unsigned int box_col = (c / 3) * 3;

		// Reset cell in case of backtrack
		m_solution[i] = 0;

		QList<int>& cell = cells[i];
		std::shuffle(cell.begin(), cell.end(), m_random);

		forever {
			// Backtrack if there are no possiblities
			if (cell.isEmpty()) {
				cell = initial;
				i -= 2;
				break;
			}

			// Fetch value
			int value = cell.takeLast();

			// Check for conflicts
			bool conflicts = false;
			for (unsigned int j = 0; j < 9; ++j) {
				conflicts |= (m_solution[j + (r * 9)] == value);
				conflicts |= (m_solution[c + (j * 9)] == value);
			}
			for (unsigned int row = box_row; row < box_row + 3; ++row) {
				for (unsigned int col = box_col; col < box_col + 3; ++col) {
					conflicts |= (m_solution[col + (row * 9)] == value);
				}
			}
			if (!conflicts) {
				m_solution[i] = value;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------

bool Puzzle::createGivens()
{
	// Initialize givens
	m_givens = m_solution;

	// Initialize cells
	std::array<int, 81> cells;
	for (int i = 0; i < 81; ++i) {
		cells[i] = i;
	}
	std::shuffle(cells.begin(), cells.end(), m_random);

	// Remove as many givens as possible
	for (const int cell : cells) {
		const QList<int> positions = m_pattern->pattern(cell % 9, cell / 9);

		bool valid = true;
		for (const int pos : positions) {
			valid &= bool(m_givens[pos]);
		}
		if (!valid) {
			continue;
		}

		for (const int pos : positions) {
			m_givens[pos] = 0;
		}
		if (!isUnique()) {
			for (const int pos : positions) {
				m_givens[pos] = m_solution[pos];
			}
		}

		if (m_canceled.load(std::memory_order_relaxed)) {
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

bool Puzzle::isUnique()
{
	if (m_difficulty >= Hard) {
		static SolverDLX solver;
		if (!solver.solvePuzzle(m_givens)) {
			return false;
		}
	}

	static SolverLogic solver;
	const int generated = solver.solvePuzzle(m_givens, m_difficulty);
	if (generated > m_difficulty) {
		return false;
	}

	m_generated = generated;

	return true;
}

//-----------------------------------------------------------------------------
