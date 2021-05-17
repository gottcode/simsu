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

#ifndef SIMSU_SQUARE_H
#define SIMSU_SQUARE_H

#include <QWidget>

/** Widget that squares a child widget. */
class Square : public QWidget
{
public:
	/**
	 * Constructs a square widget.
	 *
	 * @param parent the parent widget
	 */
	Square(QWidget* parent = nullptr);

	/**
	 * Set the widget to be squared.
	 *
	 * @param child widget to be squared
	 */
	void setChild(QWidget* child);

protected:
	/** Override parent function to handle setting child position and size. */
	void resizeEvent(QResizeEvent* event) override;

private:
	QWidget* m_child; /**< widget to be squared */
};

#endif // SIMSU_SQUARE_H
