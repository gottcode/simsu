/*
	SPDX-FileCopyrightText: 2009-2016 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_PUZZLE_H
#define SIMSU_PUZZLE_H

class Pattern;

#include <QFuture>
#include <QObject>
#include <QRandomGenerator>
#include <QStringList>

#include <array>
#include <atomic>

/**
 * Layout generator.
 *
 * This class will build a layout of givens based upon the settings chosen.
 */
class Puzzle : public QObject
{
	Q_OBJECT

public:
	enum Difficulty {
		VeryEasy = 1,
		Easy,
		Medium,
		Hard,
		Unsolved
	};

	/** Constructs a puzzle. */
	explicit Puzzle(QObject* parent = nullptr);

	/** Clean up puzzle. */
	~Puzzle();

	/** Abort currently running generation. */
	void cancel();

	/**
	 * Creates a new layout.
	 *
	 * @param symmetry the pattern to use when laying out givens
	 * @param difficulty specify how hard to make puzzle
	 */
	void generate(int symmetry, int difficulty);

	/**
	  * Loads a layout.
	  *
	  * @param givens list of givens to use
	  * @return @c true if the puzzle could be loaded
	  */
	bool load(const std::array<int, 81>& givens);

	/**
	 * Returns the given at the requested position.
	 *
	 * @param x the column of the given
	 * @param y the row of the given
	 */
	int given(int x, int y) const
	{
		Q_ASSERT(x >= 0);
		Q_ASSERT(x < 9 );
		Q_ASSERT(y >= 0);
		Q_ASSERT(y < 9);
		return m_givens[x + (y * 9)];
	}

	/**
	 * Returns the value at the requested position.
	 *
	 * @param x the column of the cell
	 * @param y the row of the cell
	 */
	int value(int x, int y) const
	{
		Q_ASSERT(x >= 0);
		Q_ASSERT(x < 9 );
		Q_ASSERT(y >= 0);
		Q_ASSERT(y < 9);
		return m_solution[x + (y * 9)];
	}

	/**
	 * Returns the human readable name for a difficulty level.
	 *
	 * @param difficulty fetch the name of the specified difficulty level
	 */
	static QString difficultyString(int difficulty)
	{
		static const QStringList names = QStringList()
				<< tr("Simple")
				<< tr("Easy")
				<< tr("Medium")
				<< tr("Hard");
		return names.at(qBound(1, difficulty, names.size()) - 1);
	}

signals:
	/**
	 * Emitted when puzzle has finished generating.
	 *
	 * @param symmetry specify mirroring of givens
	 * @param difficulty specify how hard to make puzzle
	 */
	void generated(int symmetry, int difficulty);

private:
	/** Fills the board with unique values. */
	void createSolution();

	/** Finds a set of givens to show the player. */
	bool createGivens();

	/** Check if the givens on the board have a unique solution. */
	bool isUnique();

private:
	std::array<int, 81> m_solution; /**< board solution */
	std::array<int, 81> m_givens; /**< board givens */

	QRandomGenerator m_random; /**< random number generator */

	Pattern* m_pattern; /**< the pattern used to lay out the givens */
	Difficulty m_difficulty; /**< requested difficulty setting */
	int m_generated; /**< actual difficulty of board */

	QFuture<void> m_future; /**< results of puzzle generation */
	std::atomic<bool> m_canceled; /**< if the generation has been aborted by player */
};

#endif // SIMSU_PUZZLE_H
