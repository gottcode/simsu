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

#include "solver_dlx.h"

#include "puzzle.h"

//-----------------------------------------------------------------------------

SolverDLX::SolverDLX()
	: m_max_columns(324)
	, m_max_rows(729)
	, m_max_nodes(2916)
	, m_columns(m_max_columns)
	, m_output(m_max_columns)
	, m_solutions(0)
	, m_tries(0)
{
	init();
}

//-----------------------------------------------------------------------------

SolverDLX::~SolverDLX()
{
	delete m_header;
}

//-----------------------------------------------------------------------------

bool SolverDLX::solvePuzzle(const std::array<int, 81>& givens)
{
	// Reset matrix
	if (m_tries) {
		m_columns.clear();
		m_output.clear();
		m_rows.clear();
		m_nodes.clear();
		m_solution.fill(nullptr);
		delete m_header;

		m_columns.resize(m_max_columns);
		m_output.resize(m_max_columns);
		init();
	}

	// Build matrix
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			const int g = givens[c + (r * 9)];
			if (!g) {
				for (int i = 0; i < 9; ++i) {
					addRow((c << 8) | (r << 4) | (i + 1));
					addNode(r * 9 + c);
					addNode(r * 9 + i + 81);
					addNode(c * 9 + i + 162);
					addNode((3 * (r / 3) + (c / 3)) * 9 + i + 243);
				}
			} else {
				addRow((c << 8) | (r << 4) | g);
				addNode(r * 9 + c);
				addNode(r * 9 + (g - 1) + 81);
				addNode(c * 9 + (g - 1) + 162);
				addNode((3 * (r / 3) + (c / 3)) * 9 + (g - 1) + 243);
			}
		}
	}

	// Solve matrix
	m_solutions = 0;

	m_tries = 0;

	solve(0);
	return m_solutions == 1;
}

//-----------------------------------------------------------------------------

std::array<int, 81> SolverDLX::solution() const
{
	std::array<int, 81> result;

	// Return null solution array if invalid
	if (m_solution.front() == nullptr) {
		result.fill(0);
		return result;
	}

	// Copy values to solution array
	for (int i = 0; i < 81; ++i) {
		const int id = m_solution[i]->row->id;
		const int c = (id >> 8) & 0xF;
		const int r = (id >> 4) & 0xF;
		const int v = id & 0xF;
		result[c + (r * 9)] = v;
	}
	return result;
}

//-----------------------------------------------------------------------------

void SolverDLX::addRow(unsigned int id)
{
	m_rows.append(HeaderNode());
	HeaderNode* row = &m_rows.back();
	row->id = id;
	row->left = row->right = row->up = row->down = row->column = row;
}

//-----------------------------------------------------------------------------

void SolverDLX::addNode(unsigned int c)
{
	HeaderNode* column = &m_columns[c];
	HeaderNode* row = &m_rows.back();

	m_nodes.append(Node());
	Node* node = &m_nodes.back();

	node->left = row->left;
	node->right = row;
	row->left->right = node;
	row->left = node;

	node->up = column->up;
	node->down = column;
	column->up->down = node;
	column->up = node;

	node->column = column;
	node->row = row;

	column->size++;
}

//-----------------------------------------------------------------------------

void SolverDLX::init()
{
	m_solutions = 0;
	m_tries = 0;

	m_header = new HeaderNode;
	m_header->column = m_header;

	Node* node = m_header;
	HeaderNode* column = 0;
	for (unsigned int i = 0; i < m_max_columns; ++i) {
		column = &m_columns[i];
		column->id = i;
		column->up = column->down = column->column = column;
		column->left = node;
		node->right = column;
		node = column;
	}
	node->right = m_header;

	m_rows.reserve(m_max_rows);
	m_nodes.reserve(m_max_nodes);
}

//-----------------------------------------------------------------------------

void SolverDLX::solve(unsigned int k)
{
	// If matrix is empty a solution has been found.
	if (m_header->right == m_header) {
		++m_solutions;
		std::copy(m_output.cbegin(), m_output.cbegin() + 81, m_solution.begin());
		return;
	}

	if ((m_solutions >= 2) || (++m_tries >= m_max_columns)) {
		return;
	}

	// Choose column with lowest amount of 1s.
	HeaderNode* column = 0;
	unsigned int s = 0xFFFFFFFF;
	for(HeaderNode* i = m_header->right->column; i != m_header; i = i->right->column) {
		if (i->size < s) {
			column = i;
			s = i->size;
		}
	}
	cover(column);

	unsigned int next_k = k + 1;

	for(Node* row = column->down; row != column; row = row->down) {
		m_output[k] = row;

		for(Node* j = row->right; j != row; j = j->right) {
			cover(j->column);
		}

		solve(next_k);

		row = m_output[k];
		column = row->column;

		for(Node* j = row->left; j != row; j = j->left) {
			uncover(j->column);
		}
	}

	uncover(column);
}

//-----------------------------------------------------------------------------

void SolverDLX::cover(HeaderNode* node)
{
	node->right->left = node->left;
	node->left->right = node->right;

	for (Node* i = node->down; i != node; i = i->down) {
		for (Node* j = i->right; j != i; j = j->right) {
			j->down->up = j->up;
			j->up->down = j->down;
			j->column->size--;
		}
	}
}

//-----------------------------------------------------------------------------

void SolverDLX::uncover(HeaderNode* node)
{
	for (Node* i = node->up; i != node; i = i->up) {
		for (Node* j = i->left; j != i; j = j->left) {
			j->column->size++;
			j->down->up = j;
			j->up->down = j;
		}
	}

	node->right->left = node;
	node->left->right = node;
}

//-----------------------------------------------------------------------------
