/*
	SPDX-FileCopyrightText: 2016 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_SOLVER_LOGIC_H
#define SIMSU_SOLVER_LOGIC_H

#include <array>

/**
 * Puzzle solver that uses logic methods to imitate human solving.
 */
class SolverLogic
{
	/**
	 * Cell on the game board.
	 */
	class CellValue
	{
	public:
		/**
		 * Constructs a cell.
		 */
		CellValue()
			: m_value(0)
			, m_possible_count(9)
			, m_possible{true,true,true,true,true,true,true,true,true}
		{
		}

		/**
		 * @return how many possible values can go in cell
		 */
		int countPossibles() const
		{
			return m_possible_count;
		}

		/**
		 * @return @c true if @p possible can be placed in cell
		 */
		bool hasPossible(const int possible) const
		{
			return m_possible[possible - 1];
		}

		/**
		 * @return the only value that can be placed in cell
		 */
		int possibleSingle() const;

		/**
		 * @return the values of the cell if it can only have two values
		 */
		int possiblePair() const;

		/**
		 * Removes the possibility to set a specific value in the cell.
		 * @param possible the value to remove
		 * @return if the value was allowed before
		 */
		bool removePossible(const int possible)
		{
			const bool has = m_possible[possible - 1];
			m_possible_count -= has;
			m_possible[possible - 1] = false;
			return has;
		}

		/**
		 * Removes the possibility to set specific values in the cell.
		 * @param possible the values to remove
		 * @return if the values were allowed before
		 */
		bool removePossibles(const int possible);

		/**
		 * Sets the cell to nothing values and allows all possibles.
		 */
		void reset()
		{
			m_value = 0;
			m_possible_count = 9;
			m_possible = {true,true,true,true,true,true,true,true,true};
		}

		/**
		 * Sets the cell to a specific @p value and removes the possibles.
		 */
		void setValue(const int value)
		{
			m_value = value;
			m_possible_count = 0;
			m_possible = {false,false,false,false,false,false,false,false,false};
		}

		/**
		 * @return the value set in the cell
		 */
		int value() const
		{
			return m_value;
		}

	private:
		int m_value; /**< value contained by cell */
		int m_possible_count; /**< total amount of values that can go in the cell */
		std::array<bool, 9> m_possible; /**< values that can go in the cell */
	};

	/**
	 * Pair of cells with the same possible values.
	 */
	struct HiddenPair
	{
		/**
		 * Constructs a hidden pair.
		 */
		HiddenPair()
			: count(0)
			, cell1(0)
			, cell2(0)
		{
		}

		int count; /**< how many cells have been found */
		int cell1; /**< first cell with the possible values */
		int cell2; /**< second cell with the possible values */
	};

public:
	typedef std::array<int, 9> Box;

	/**
	 * Constructs a solver.
	 */
	SolverLogic();

	/**
	 * Find if puzzle has a solution.
	 *
	 * @param givens the values already set on the board
	 * @param max_difficulty the maximum difficulty of checks when solving
	 * @return difficulty of solution found, or Puzzle::Unsolved if none found
	 */
	int solvePuzzle(const std::array<int, 81>& givens, int max_difficulty);

private:
	/**
	 * Fetch the box containing a cell.
	 *
	 * @param c column of the cell
	 * @param r row of the cell
	 * @return const reference to the box
	 */
	const Box& cellBox(int c, int r) const
	{
		return m_boxes[(c / 3) + ((r / 3) * 3)];
	}

	/**
	 * Sets all cells containing a single possible to that value.
	 *
	 * @return @c true if successful
	 */
	bool removeSingles();

	/**
	 * Sets the value of the only cell in a row with a possible value.
	 *
	 * @return @c true if successful
	 */
	bool removeRowHiddenSingle();

	/**
	 * Sets the value of the only cell in a column with a possible value.
	 *
	 * @return @c true if successful
	 */
	bool removeColumnHiddenSingle();

	/**
	 * Sets the value of the only cell in a box with a possible value.
	 *
	 * @return @c true if successful
	 */
	bool removeBoxHiddenSingle();

	/**
	 * Removes possibles from other cells in a row if there are only two
	 * cells in a box that can contain a possible value and they are both
	 * in the same row.
	 *
	 * @return @c true if successful
	 */
	bool removeRowPointingPair();

	/**
	 * Removes possibles from other cells in a column if there are only two
	 * cells in a box that can contain a possible value and they are both
	 * in the same column.
	 *
	 * @return @c true if successful
	 */
	bool removeColumnPointingPair();

	/**
	 * Removes possibles from other cells in a box if they must appear in a
	 * row because they are the only spots in that row which can contain that
	 * possible value.
	 *
	 * @return @c true if successful
	 */
	bool removeRowBoxIntersection();

	/**
	 * Removes possibles from other cells in a box if they must appear in a
	 * column because they are the only spots in that column which can contain
	 * that possible value.
	 *
	 * @return @c true if successful
	 */
	bool removeColumnBoxIntersection();

	/**
	 * Removes possibles from other cells in a column, row, or box if there
	 * are two cells that have the same two values as the only possibility.
	 *
	 * @return @c true if successful
	 */
	bool removeNakedPair();

	/**
	 * Removes possibles from other cells in a row if there are only two
	 * cells that can contain the possible values.
	 *
	 * @return @c true if successful
	 */
	bool removeRowHiddenPair();

	/**
	 * Removes possibles from other cells in a column if there are only two
	 * cells that can contain the possible values.
	 *
	 * @return @c true if successful
	 */
	bool removeColumnHiddenPair();

	/**
	 * Removes possibles from other cells in a box if there are only two
	 * cells that can contain the possible values.
	 *
	 * @return @c true if successful
	 */
	bool removeBoxHiddenPair();

	/**
	 * Sets the value of a cell.
	 *
	 * @param c column of the cell
	 * @param r row of the cell
	 * @param value the value to put in a cell
	 */
	void setValue(const int c, const int r, const int value);

private:
	int m_remaining; /**< how many cells are unset */
	std::array<CellValue, 81> m_cells; /**< the cells of the board */
	static const std::array<Box, 9> m_boxes; /**< the boxes of the board */
};

#endif // SIMSU_SOLVER_LOGIC_H
