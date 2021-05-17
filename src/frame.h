/***********************************************************************
 *
 * Copyright (C) 2009, 2011, 2013, 2016 Graeme Gott <graeme@gottcode.org>
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

#ifndef SIMSU_FRAME_H
#define SIMSU_FRAME_H

#include <QWidget>

/**
 * Graphical frame widget.
 *
 * This class is a custom frame to give the cells and game board edges.
 */
class Frame : public QWidget
{
public:
	/** Constructs a frame. */
	Frame(QWidget* parent = nullptr);

protected:
	/** Override parent function to draw border and custom background color. */
	void paintEvent(QPaintEvent* event);

	/** Sets whether or not background should be drawn in full highlight. */
	void setHighlight(bool highlight)
	{
		m_highlight = highlight;
	}

	/** Sets whether or not border should be drawn in highlight. */
	void setHighlightBorder(bool highlight)
	{
		m_highlight_border = highlight;
	}

	/** Sets whether or not background should be drawn in partial highlight. */
	void setHighlightPartial(bool highlight)
	{
		m_highlight_partial = highlight;
	}

	/** Sets whether or not background should be drawn in partial highlight. */
	void setHighlightMid(bool mid)
	{
		m_highlight_mid = mid;
	}

private:
	bool m_highlight; /**< tracks if background should be drawn highlighted */
	bool m_highlight_border; /**< tracks if border should be drawn highlighted */
	bool m_highlight_partial; /**< tracks if background should be drawn only partially highlighted */
	bool m_highlight_mid; /**< tracks if background should be drawn mid highlighted */
};

#endif // SIMSU_FRAME_H
