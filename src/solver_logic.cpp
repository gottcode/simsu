/*
	SPDX-FileCopyrightText: 2016-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "solver_logic.h"

#include "puzzle.h"

//-----------------------------------------------------------------------------

/**
 * Create groups of cells on the game board.
 *
 * A box is a 3x3 group of cells on the game board. This function represents
 * them as lists of integer indexes for fast access. To convert an index into
 * coordinates use index % 9 for the column and index / 9 for the row.
 */
static constexpr std::array<SolverLogic::Box, 9> initBoxes()
{
	std::array<SolverLogic::Box, 9> result{};
	for (unsigned int i = 0; i < 9; ++i) {
		const unsigned int box_row = (i / 3) * 3;
		const unsigned int box_col = (i % 3) * 3;

		SolverLogic::Box& box = result[i];
		int pos = 0;
		for (unsigned int r = box_row; r < box_row + 3; ++r) {
			for (unsigned int c = box_col; c < box_col + 3; ++c) {
				const int index = c + (r * 9);
				box[pos] = index;
				++pos;
			}
		}
	}
	return result;
}
const std::array<SolverLogic::Box, 9> SolverLogic::m_boxes = initBoxes();

//-----------------------------------------------------------------------------

SolverLogic::SolverLogic() :
	m_remaining(81)
{
}

//-----------------------------------------------------------------------------

void SolverLogic::loadPuzzle(const std::array<int, 81>& givens)
{
	// Reset board so that setValue works
	for (CellValue& cell : m_cells) {
		cell.reset();
	}
	m_remaining = 81;

	// Load givens onto board
	for (int i = 0; i < 81; ++i) {
		const int value = givens[i];
		if (value) {
			setValue(i % 9, i / 9, value);
		}
	}
}

//-----------------------------------------------------------------------------

int SolverLogic::solvePuzzle(const std::array<int, 81>& givens, int max_difficulty)
{
	loadPuzzle(givens);

	// Reduce board until solved
	int remaining = m_remaining;
	int veryeasy = 0;
	int easy = 0;
	int medium = 0;
	while (m_remaining) {
		if (removeSingles()) {
			++veryeasy;
			continue;
		} else if (max_difficulty == Puzzle::VeryEasy) {
			return Puzzle::Unsolved;
		}

		if (removeRowHiddenSingle()
				|| removeColumnHiddenSingle()
				|| removeBoxHiddenSingle()) {
			++easy;
			continue;
		} else if (max_difficulty == Puzzle::Easy) {
			return Puzzle::Unsolved;
		}

		// Too many easy passes in a row makes the puzzle feel easy
		if ((max_difficulty < Puzzle::Unsolved) && (veryeasy > 3)) {
			return Puzzle::Unsolved;
		}

		if (removeRowPointingPair()
				|| removeColumnPointingPair()
				|| removeRowBoxIntersection()
				|| removeColumnBoxIntersection()
				|| removeNakedPair()
				|| removeRowHiddenPair()
				|| removeColumnHiddenPair()
				|| removeBoxHiddenPair()) {
			++medium;
			veryeasy = 0;
			continue;
		} else if (max_difficulty == Puzzle::Medium) {
			return Puzzle::Unsolved;
		}

		// Too many moves before a hard move makes the puzzle feel easy
		if ((max_difficulty < Puzzle::Unsolved) && ((m_remaining - remaining) > 15)) {
			return Puzzle::Unsolved;
		}

		return Puzzle::Hard;
	}

	if (medium) {
		return Puzzle::Medium;
	} else if (easy || (veryeasy > 7)) {
		return Puzzle::Easy;
	} else if (veryeasy) {
		return Puzzle::VeryEasy;
	}
	return Puzzle::Unsolved;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeSingles()
{
	bool removed = false;
	for (int i = 0; i < 81; ++i) {
		const int value = m_cells[i].possibleSingle();
		if (value) {
			setValue(i % 9, i / 9, value);
			removed = true;
		}
	}
	return removed;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeRowHiddenSingle()
{
	for (int r = 0; r < 9; ++r) {
		const int y = r * 9;
		for (int value = 1; value < 10; ++value) {
			int found = 0;
			int found_c = 0;
			for (int c = 0; c < 9; ++c) {
				if (m_cells[c + y].hasPossible(value)) {
					++found;
					found_c = c;
				}
				if (found > 1) {
					break;
				}
			}
			if (found == 1) {
				setValue(found_c, r, value);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeColumnHiddenSingle()
{
	for (int c = 0; c < 9; ++c) {
		for (int value = 1; value < 10; ++value) {
			int found = 0;
			int found_r = 0;
			for (int r = 0; r < 9; ++r) {
				if (m_cells[c + (r * 9)].hasPossible(value)) {
					++found;
					found_r = r;
				}
				if (found > 1) {
					break;
				}
			}
			if (found == 1) {
				setValue(c, found_r, value);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeBoxHiddenSingle()
{
	for (const Box& box : m_boxes) {
		for (int value = 1; value < 10; ++value) {
			int found = 0;
			int found_c = 0;
			int found_r = 0;
			for (int cell : box) {
				if (m_cells[cell].hasPossible(value)) {
					++found;
					if (found > 1) {
						break;
					}
					found_r = cell / 9;
					found_c = cell % 9;
				}
			}
			if (found == 1) {
				setValue(found_c, found_r, value);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeRowPointingPair()
{
	for (const Box& box : m_boxes) {
		for (int value = 1; value < 10; ++value) {
			int found = 0;
			int found_r = -1;
			int found_c1 = 0;
			int found_c2 = 9;
			for (int cell : box) {
				if (m_cells[cell].hasPossible(value)) {
					++found;
					if (found == 4) {
						found = 0;
						break;
					}

					const int r = cell / 9;
					const int c = cell % 9;

					if (found_r == -1) {
						found_r = r;
						found_c1 = c;
					} else if (found_r != r) {
						found = 0;
						break;
					}
					found_c2 = c;
				}
			}
			if (found < 2) {
				continue;
			}

			// Remove possibles from row
			bool removed = false;
			for (int c = 0; c < found_c1; ++c) {
				removed |= m_cells[c + (found_r * 9)].removePossible(value);
			}
			for (int c = found_c2 + 1; c < 9; ++c) {
				removed |= m_cells[c + (found_r * 9)].removePossible(value);
			}
			if (removed) {
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeColumnPointingPair()
{
	for (const Box& box : m_boxes) {
		for (int value = 1; value < 10; ++value) {
			int found = 0;
			int found_c = -1;
			int found_r1 = 0;
			int found_r2 = 9;
			for (int cell : box) {
				if (m_cells[cell].hasPossible(value)) {
					++found;
					if (found == 4) {
						found = 0;
						break;
					}

					const int r = cell / 9;
					const int c = cell % 9;

					if (found_c == -1) {
						found_c = c;
						found_r1 = r;
					} else if (found_c != c) {
						found = 0;
						break;
					}
					found_r2 = r;
				}
			}
			if (found < 2) {
				continue;
			}

			// Remove possibles from column
			bool removed = false;
			for (int r = 0; r < found_r1; ++r) {
				removed |= m_cells[found_c + (r * 9)].removePossible(value);
			}
			for (int r = found_r2 + 1; r < 9; ++r) {
				removed |= m_cells[found_c + (r * 9)].removePossible(value);
			}
			if (removed) {
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeRowBoxIntersection()
{
	for (int r = 0; r < 9; ++r) {
		const int y = r * 9;
		for (int value = 1; value < 10; ++value) {
			// Find min and max locations of value in column
			int found = 0;
			int found_c1 = -3;
			int found_c2 = 0;
			for (int c = 0; c < 9; ++c) {
				if (m_cells[c + y].hasPossible(value)) {
					++found;
					if (found == 1) {
						found_c1 = c;
					} else if (found == 4) {
						found_c1 = -3;
						break;
					}
					found_c2 = c;
				}
			}

			// Skip to next value if min and max are in different boxes
			if ((found_c1 / 3) != (found_c2 / 3)) {
				continue;
			}

			// Remove possibles from box
			bool removed = false;
			const Box& box = cellBox(found_c1, r);
			for (int cell : box) {
				const int cell_r = cell / 9;
				if (cell_r == r) {
					continue;
				}
				removed |= m_cells[cell].removePossible(value);
			}
			if (removed) {
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeColumnBoxIntersection()
{
	for (int c = 0; c < 9; ++c) {
		for (int value = 1; value < 10; ++value) {
			// Find min and max locations of value in row
			int found = 0;
			int found_r1 = -3;
			int found_r2 = 0;
			for (int r = 0; r < 9; ++r) {
				if (m_cells[c + (r * 9)].hasPossible(value)) {
					++found;
					if (found == 1) {
						found_r1 = r;
					} else if (found == 4) {
						found_r1 = -3;
						break;
					}
					found_r2 = r;
				}
			}

			// Skip to next value if min and max are in different boxes
			if ((found_r1 / 3) != (found_r2 / 3)) {
				continue;
			}

			// Remove possibles from box
			bool removed = false;
			const Box& box = cellBox(c, found_r1);
			for (int cell : box) {
				const int cell_c = cell % 9;
				if (cell_c == c) {
					continue;
				}
				removed |= m_cells[cell].removePossible(value);
			}
			if (removed) {
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeNakedPair()
{
	for (int i = 0; i < 81; ++i) {
		const int pair = m_cells[i].possiblePair();
		if (!pair) {
			continue;
		}

		const int r = i / 9;
		const int c = i % 9;

		// Check row for matching pair
		for (int x = c + 1; x < 9; ++x) {
			if (m_cells[x + (r * 9)].possiblePair() == pair) {
				bool found = false;
				for (int j = 0; j < 9; ++j) {
					if ((j != c) && (j != x)) {
						found |= m_cells[j + (r * 9)].removePossibles(pair);
					}
				}
				if (found) {
					return true;
				}
			}
		}

		// Check column for matching pair
		for (int y = r + 1; y < 9; ++y) {
			if (m_cells[c + (y * 9)].possiblePair() == pair) {
				bool found = false;
				for (int j = 0; j < 9; ++j) {
					if ((j != r) && (j != y)) {
						found |= m_cells[c + (j * 9)].removePossibles(pair);
					}
				}
				if (found) {
					return true;
				}
			}
		}

		// Check box for matching pair
		const Box& box = cellBox(c, r);
		for (int cell : box) {
			if ((cell != i) && (m_cells[cell].possiblePair() == pair)) {
				bool found = false;
				for (int j : box) {
					if ((j != i) && (j != cell)) {
						found |= m_cells[j].removePossibles(pair);
					}
				}
				if (found) {
					return true;
				}
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeRowHiddenPair()
{
	for (int r = 0; r < 9; ++r) {
		const int y = r * 9;
		HiddenPair possibles[9];

		for (int value = 1; value < 10; ++value) {
			HiddenPair& p = possibles[value - 1];

			for (int c = 0; c < 9; ++c) {
				const int cell = c + y;
				if (!m_cells[cell].hasPossible(value)) {
					continue;
				}

				++p.count;
				if (p.count == 1) {
					p.cell1 = cell;
				} else if (p.count == 2) {
					p.cell2 = cell;
					if ((m_cells[p.cell1].countPossibles() + m_cells[p.cell2].countPossibles()) < 5) {
						p.count = 0;
						break;
					}
				} else {
					p.count = 0;
					break;
				}
			}
		}

		for (int i = 1; i < 10; ++i) {
			const HiddenPair& p1 = possibles[i - 1];
			if (p1.count != 2) {
				continue;
			}

			for (int j = i + 1; j < 10; ++j) {
				const HiddenPair& p2 = possibles[j - 1];
				if ((p2.count != 2) || (p1.cell1 != p2.cell1) || (p1.cell2 != p2.cell2)) {
					continue;
				}

				CellValue& cell1 = m_cells[p1.cell1];
				CellValue& cell2 = m_cells[p1.cell2];
				for (int value = 1; value < 10; ++value) {
					if (value == i || value == j) {
						continue;
					}
					cell1.removePossible(value);
					cell2.removePossible(value);
				}
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeColumnHiddenPair()
{
	for (int c = 0; c < 9; ++c) {
		HiddenPair possibles[9];

		for (int value = 1; value < 10; ++value) {
			HiddenPair& p = possibles[value - 1];

			for (int r = 0; r < 9; ++r) {
				const int cell = c + (r * 9);
				if (!m_cells[cell].hasPossible(value)) {
					continue;
				}

				++p.count;
				if (p.count == 1) {
					p.cell1 = cell;
				} else if (p.count == 2) {
					p.cell2 = cell;
					if ((m_cells[p.cell1].countPossibles() + m_cells[p.cell2].countPossibles()) < 5) {
						p.count = 0;
						break;
					}
				} else {
					p.count = 0;
					break;
				}
			}
		}

		for (int i = 1; i < 10; ++i) {
			const HiddenPair& p1 = possibles[i - 1];
			if (p1.count != 2) {
				continue;
			}

			for (int j = i + 1; j < 10; ++j) {
				const HiddenPair& p2 = possibles[j - 1];
				if ((p2.count != 2) || (p1.cell1 != p2.cell1) || (p1.cell2 != p2.cell2)) {
					continue;
				}

				CellValue& cell1 = m_cells[p1.cell1];
				CellValue& cell2 = m_cells[p1.cell2];
				for (int value = 1; value < 10; ++value) {
					if (value == i || value == j) {
						continue;
					}
					cell1.removePossible(value);
					cell2.removePossible(value);
				}
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

bool SolverLogic::removeBoxHiddenPair()
{
	for (const Box& box : m_boxes) {
		HiddenPair possibles[9];

		for (int value = 1; value < 10; ++value) {
			HiddenPair& p = possibles[value - 1];

			for (const int cell : box) {
				if (!m_cells[cell].hasPossible(value)) {
					continue;
				}

				++p.count;
				if (p.count == 1) {
					p.cell1 = cell;
				} else if (p.count == 2) {
					p.cell2 = cell;
					if ((m_cells[p.cell1].countPossibles() + m_cells[p.cell2].countPossibles()) < 5) {
						p.count = 0;
						break;
					}
				} else {
					p.count = 0;
					break;
				}
			}
		}

		for (int i = 1; i < 10; ++i) {
			const HiddenPair& p1 = possibles[i - 1];
			if (p1.count != 2) {
				continue;
			}

			for (int j = i + 1; j < 10; ++j) {
				const HiddenPair& p2 = possibles[j - 1];
				if ((p2.count != 2) || (p1.cell1 != p2.cell1) || (p1.cell2 != p2.cell2)) {
					continue;
				}

				CellValue& cell1 = m_cells[p1.cell1];
				CellValue& cell2 = m_cells[p1.cell2];
				for (int value = 1; value < 10; ++value) {
					if (value == i || value == j) {
						continue;
					}
					cell1.removePossible(value);
					cell2.removePossible(value);
				}
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

void SolverLogic::setValue(const int c, const int r, const int value)
{
	--m_remaining;

	// Set cell value
	const int y = r * 9;
	m_cells[c + y].setValue(value);

	// Remove all instances of value in column
	for (int row = 0; row < 81; row += 9) {
		m_cells[c + row].removePossible(value);
	}

	// Remove all instances of value in row
	for (int col = 0; col < 9; ++col) {
		m_cells[col + y].removePossible(value);
	}

	// Remove all instances of value in box
	const Box& box = cellBox(c, r);
	for (const int cell : box) {
		m_cells[cell].removePossible(value);
	}
}

//-----------------------------------------------------------------------------

int SolverLogic::CellValue::possibleSingle() const
{
	if (m_possible_count == 1) {
		for (int i = 0; i < 9; ++i) {
			if (m_possible[i]) {
				return i + 1;
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

int SolverLogic::CellValue::possiblePair() const
{
	if (m_possible_count != 2) {
		return 0;
	}

	int value = 0;
	for (int i = 0; i < 9; ++i) {
		if (!m_possible[i]) {
			continue;
		}
		value |= (1 << i);
	}
	return value;
}

//-----------------------------------------------------------------------------

bool SolverLogic::CellValue::removePossibles(const int possible)
{
	bool changed = false;
	for (int i = 0; i < 9; ++i) {
		if (possible & (1 << i)) {
			changed |= m_possible[i];
			m_possible_count -= m_possible[i];
			m_possible[i] = false;
		}
	}
	return changed;
}

//-----------------------------------------------------------------------------
