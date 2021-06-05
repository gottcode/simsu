/*
	SPDX-FileCopyrightText: 2009-2017 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SIMSU_PATTERN_H
#define SIMSU_PATTERN_H

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QString>

/**
 * %Pattern of givens.
 *
 * This class controls the placement of the givens to make them symmetrical.
 */
class Pattern
{
	Q_DECLARE_TR_FUNCTIONS(Pattern);

public:
	/** Specify how givens are mirrored when placed. */
	enum Symmetry {
		Rotational180, /**< Givens are rotated 180&deg; */
		RotationalFull, /**< Givens are rotated 90&deg; four times */
		Horizontal, /**< Givens are reflected across horizontal axis */
		Vertical, /**< Givens are reflected across vertical axis */
		HorizontalVertical, /**< Givens are reflected across both horizontal and vertical axes */
		Diagonal, /**< Givens are reflected across top-left to bottom-right axis */
		AntiDiagonal, /**< Givens are reflected across top-right to bottom-left axis */
		DiagonalAntiDiagonal, /**< Givens are reflected across both diagonal axes */
		FullDihedral, /**< Givens are reflected across both horizontal and vertical axes and rotated 90&deg; four times */
		Random, /**< Choose a symmetry at random */
		None /**< No symmetry at all */
	};

	/** Clean up pattern */
	virtual ~Pattern()
	{
	}

	/** Returns amount of cells that are mirrored for each step. */
	virtual int count() const = 0;

	/**
	 * Returns the mirrored positions for a cell.
	 *
	 * @param cell the cell to mirror.
	 */
	virtual QList<int> pattern(const int c, const int r) const = 0;

	/**
	 * Returns the human readable name for a pattern.
	 *
	 * @param symmetry fetch the name of the specified pattern
	 */
	static QString name(int symmetry)
	{
		static QHash<int, QString> names;
		if (names.isEmpty()) {
			names[Rotational180] = tr("180° Rotational");
			names[RotationalFull] = tr("Full Rotational");
			names[Horizontal] = tr("Horizontal");
			names[Vertical] = tr("Vertical");
			names[HorizontalVertical] = tr("Horizontal & Vertical");
			names[Diagonal] = tr("Diagonal");
			names[AntiDiagonal] = tr("Anti-Diagonal");
			names[DiagonalAntiDiagonal] = tr("Diagonal & Anti-Diagonal");
			names[FullDihedral] = tr("Full Dihedral");
			names[Random] = tr("Random");
			names[None] = tr("None");
		}
		return names.value(symmetry);
	}

	/**
	 * Returns the preview image for a pattern.
	 *
	 * @param symmetry fetch the preview image for the specified pattern
	 */
	static QString icon(int symmetry)
	{
		static QHash<int, QString> icons;
		if (icons.isEmpty()) {
			icons[Rotational180] = ":/rotational_180.png";
			icons[RotationalFull] = ":/rotational_full.png";
			icons[Horizontal] = ":/horizontal.png";
			icons[Vertical] = ":/vertical.png";
			icons[HorizontalVertical] = ":/horizontal_vertical.png";
			icons[Diagonal] = ":/diagonal.png";
			icons[AntiDiagonal] = ":/anti_diagonal.png";
			icons[DiagonalAntiDiagonal] = ":/diagonal_anti_diagonal.png";
			icons[FullDihedral] = ":/dihedral.png";
			icons[Random] = ":/random.png";
			icons[None] = ":/none.png";
		}
		return icons.value(symmetry);
	}

protected:
	/**
	 * Returns the array index of a point.
	 *
	 * @param c the column
	 * @param r the row
	 */
	int point(const int c, const int r) const
	{
		return c + (r * 9);
	}
};

/**
 * %Pattern where givens are reflected across both horizontal and vertical
 * axes and rotated 90&deg; four times.
 */
class PatternFullDihedral : public Pattern
{
public:
	int count() const override
	{
		return 8;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(c, 8 - r),
			point(8 - r, c),
			point(r, c),
			point(8 - c, 8 - r),
			point(8 - c, r),
			point(r, 8 - c),
			point(8 - r, 8 - c)
		};
	}
};

/** %Pattern where givens are rotated 180&deg;. */
class PatternRotational180 : public Pattern
{
public:
	int count() const override
	{
		return 2;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(8 - c, 8 - r)
		};
	}
};

/** %Pattern where givens are rotated 90&deg; four times. */
class PatternRotationalFull : public Pattern
{
public:
	int count() const override
	{
		return 4;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(8 - r, c),
			point(8 - c, 8 - r),
			point(r, 8 - c)
		};
	}
};

/** %Pattern where givens are reflected across horizontal axis. */
class PatternHorizontal : public Pattern
{
public:
	int count() const override
	{
		return 2;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(8 - c, r)
		};
	}
};

/** %Pattern where givens are reflected across vertical axis. */
class PatternVertical : public Pattern
{
public:
	int count() const override
	{
		return 2;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(c, 8 - r)
		};
	}
};

/** %Pattern where givens are reflected across both horizontal and vertical axes. */
class PatternHorizontalVertical : public Pattern
{
public:
	int count() const override
	{
		return 4;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(8 - c, r),
			point(c, 8 - r),
			point(8 - c, 8 - r)
		};
	}
};

/** %Pattern where givens are reflected across top-left to bottom-right axis. */
class PatternDiagonal : public Pattern
{
public:
	int count() const override
	{
		return 2;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(r, c)
		};
	}
};

/** %Pattern where givens are reflected across top-right to bottom-left axis. */
class PatternAntiDiagonal : public Pattern
{
public:
	int count() const override
	{
		return 2;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(8 - r, 8 - c)
		};
	}
};

/** %Pattern where givens are reflected across both diagonal axes. */
class PatternDiagonalAntiDiagonal : public Pattern
{
public:
	int count() const override
	{
		return 4;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r),
			point(r, c),
			point(8 - r, 8 - c),
			point(8 - c, 8 - r)
		};
	}
};

/** %Pattern with no symmetry at all. */
class PatternNone : public Pattern
{
public:
	int count() const override
	{
		return 1;
	}

	QList<int> pattern(const int c, const int r) const override
	{
		return {
			point(c, r)
		};
	}
};

#endif // SIMSU_PATTERN_H
