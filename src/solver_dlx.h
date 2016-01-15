/***********************************************************************
 *
 * Copyright (C) 2008, 2009, 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef SIMSU_SOLVER_DLX_H
#define SIMSU_SOLVER_DLX_H

class Puzzle;

#include <QVector>

/**
 * Puzzle solver that uses Dancing Links implementation of Algorithm X.
 *
 * Algorithm X is a recursive backtracking algorithm that finds all solutions
 * to the exact cover problem. It works on a matrix consisting of 0s an 1s.
 * The purpose is to find a combination of rows such that the digit 1 appears
 * in each column only once.
 *
 * To convert an exact cover problem into a sparse matrix solvable by
 * Algorithm X you represent each constraint by a column. Each possible value
 * is then placed into a row with 1s in the columns for the constraints it
 * matches.
 */
class SolverDLX
{
	struct HeaderNode;

	/** %Node in matrix. */
	struct Node
	{
		/** Constructs a node with the value of 1. */
		Node() :
			left(nullptr),
			right(nullptr),
			up(nullptr),
			down(nullptr),
			column(nullptr),
			row(nullptr)
		{
		}

		Node* left; /**< node to the left with value of 1 */
		Node* right; /**< node to the right with value of 1 */
		Node* up; /**< node above with value of 1 */
		Node* down; /**< node below with value of 1 */
		HeaderNode* column; /**< column containing this node */
		HeaderNode* row; /**< row containing this node */
	};

	/** Head node of column or row in matrix. */
	struct HeaderNode : public Node
	{
		/** Constructs an empty column. */
		HeaderNode() :
			size(0),
			id(0)
		{
		}

		unsigned int size; /**< how many nodes with value of 1 are in column */
		unsigned int id; /**< unique identifier */
	};

public:
	/** Construct a solver. */
	SolverDLX();

	/** Clean up solver. */
	~SolverDLX();

	/**
	 * Find if puzzle has a solution.
	 *
	 * @param puzzle the values already set on the board
	 * @return was a solution found
	 */
	bool solvePuzzle(const Puzzle* puzzle);

	/**
	 * Retrieve solution.
	 *
	 * @return last solution found as 2D array
	 */
	std::array<int, 81> solution() const;

private:
	/** Set to initial values. */
	void init();

	/** Add row to matrix. */
	void addRow(unsigned int id);

	/**
	 * Add node to matrix.
	 *
	 * @param column which column in current row to mark as filled
	 */
	void addNode(unsigned int column);

	/**
	 * Run Algorithm X at depth @p k.
	 *
	 * This is a recursive function that hides rows and columns and checks to
	 * see if a solution has been found.
	 */
	void solve(unsigned int k);

	/**
	 * Remove column or row from matrix.
	 *
	 * @param node head node of column or row to remove
	 */
	void cover(HeaderNode* node);

	/**
	 * Add column or row back to matrix.
	 *
	 * @param node head node of column or row to add
	 */
	void uncover(HeaderNode* node);

private:
	const unsigned int m_max_columns; /**< amount of constraints */
	const unsigned int m_max_rows; /**< amount of choices */
	const unsigned int m_max_nodes; /**< amount of nodes */

	HeaderNode* m_header; /**< root element */
	QVector<HeaderNode> m_columns; /**< constraints */
	QVector<HeaderNode> m_rows; /**< rows */
	QVector<Node> m_nodes; /**< row values */
	QVector<Node*> m_output; /**< rows where columns do not conflict */
	std::array<Node*, 81> m_solution; /**< nodes of most recent solution */

	unsigned int m_solutions; /**< how many solutions have been found so far */
	unsigned int m_tries; /**< how many attempts have been made so far */
};

#endif // SIMSU_SOLVER_DLX_H
