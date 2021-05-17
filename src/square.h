/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
	explicit Square(QWidget* parent = nullptr);

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
