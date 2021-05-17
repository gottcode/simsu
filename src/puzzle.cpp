/***********************************************************************
 *
 * Copyright (C) 2009, 2013, 2014, 2016 Graeme Gott <graeme@gottcode.org>
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

#include "puzzle.h"

#include "pattern.h"
#include "solver_dlx.h"
#include "solver_logic.h"

#include <algorithm>

//-----------------------------------------------------------------------------

Puzzle::Puzzle()
	: m_random(QRandomGenerator::securelySeeded())
	, m_pattern(0)
	, m_difficulty(VeryEasy)
	, m_generated(INT_MAX)
{
}

//-----------------------------------------------------------------------------

Puzzle::~Puzzle()
{
	delete m_pattern;
}

//-----------------------------------------------------------------------------

void Puzzle::generate(int symmetry, int difficulty)
{
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

	int givens = 0;
	do {
		m_generated = INT_MAX;
		createSolution();
		createGivens();

		givens = 81;
		for (int i = 0; i < 81; ++i) {
			givens -= !m_givens[i];
		}
	} while ((givens > 30) || (m_generated != m_difficulty));
}

//-----------------------------------------------------------------------------

bool Puzzle::load(const std::array<int, 81>& givens)
{
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
	static const QVector<int> initial{1,2,3,4,5,6,7,8,9};
	std::array<QVector<int>, 81> cells;
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

		QVector<int>& cell = cells[i];
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

void Puzzle::createGivens()
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
		const QVector<int> positions = m_pattern->pattern(cell % 9, cell / 9);

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
	}
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
