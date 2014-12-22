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

#ifndef SIMSU_PUZZLE_H
#define SIMSU_PUZZLE_H

class Pattern;

#include <QList>
#include <QPoint>

#include <random>

/**
 * Layout generator.
 *
 * This class is the abstract base of the board layout creation classes.
 *
 */
class Puzzle
{
public:
	/** Constructs a puzzle. */
	Puzzle();

	/** Clean up puzzle. */
	virtual ~Puzzle();

	/**
	 * Creates a new layout.
	 *
	 * @param seed the value to prime the random number generator
	 * @param symmetry the pattern to use when laying out givens
	 */
	void generate(unsigned int seed, int symmetry);

	/**
	 * Returns the given at the requested position.
	 *
	 * @param x the column of the given
	 * @param y the row of the given
	 */
	int given(int x, int y) const
	{
		x = qBound(0, x, 9);
		y = qBound(0, y, 9);
		return m_givens[x][y];
	}

	/**
	 * Returns the value at the requested position.
	 *
	 * @param x the column of the cell
	 * @param y the row of the cell
	 */
	int value(int x, int y) const
	{
		x = qBound(0, x, 9);
		y = qBound(0, y, 9);
		return m_solution[x][y];
	}

protected:
	/** Randomly shuffle contents of a cell */
	void shuffleCell(QList<int>& cell)
	{
		std::shuffle(cell.begin(), cell.end(), m_random);
	}

private:
	/** Fills the board with unique values. */
	void createSolution();

	/** Finds a set of givens to show the player. */
	void createGivens();

	/** Check if the givens on the board have a unique solution. */
	virtual bool isUnique() = 0;

private:
	int m_solution[9][9]; /**< board solution */
	int m_givens[9][9]; /**< board givens */
	Pattern* m_pattern; /**< the pattern used to lay out the givens */
	std::mt19937 m_random; /**< random number generator */
};

/** Layout generator that uses Algorithm X. */
class PuzzleDancingLinks : public Puzzle
{
private:
	/** Check for uniqueness by using Algorithm X. */
	bool isUnique();
};

/** Layout generator that uses a brute force solving method. */
class PuzzleSliceAndDice : public Puzzle
{
private:
	/**
	 * Check for uniqueness by filling each cell with all possibilities and
	 * then removing duplicates. Makes boards that are easier to solve.
	 */
	bool isUnique();
};

#endif // SIMSU_PUZZLE_H
