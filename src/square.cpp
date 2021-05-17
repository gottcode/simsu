/*
	SPDX-FileCopyrightText: 2009-2014 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "square.h"

#include <QResizeEvent>
#include <QStyle>

#include <algorithm>

//-----------------------------------------------------------------------------

Square::Square(QWidget* parent)
	: QWidget(parent)
	, m_child(nullptr)
{
	setMinimumSize(345, 345);
}

//-----------------------------------------------------------------------------

void Square::setChild(QWidget* child)
{
	m_child = child;
	m_child->setParent(this);
	resize(size());
}

//-----------------------------------------------------------------------------

void Square::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	if (m_child) {
		QRect region = contentsRect();
		int size = std::min(region.width(), region.height());
		QRect rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(size, size), region);
		m_child->setGeometry(rect);
	}
}

//-----------------------------------------------------------------------------
