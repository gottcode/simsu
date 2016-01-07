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

#include "frame.h"

#include <QPainter>

//-----------------------------------------------------------------------------

Frame::Frame(QWidget* parent) :
	QWidget(parent),
	m_highlight(false),
	m_highlight_border(false),
	m_highlight_partial(false),
	m_highlight_mid(false)
{
}

//-----------------------------------------------------------------------------

void Frame::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QColor background = palette().highlight().color();

	painter.setPen(QPen((m_highlight || m_highlight_mid) ? background : palette().dark().color(), 0));
	painter.setBrush(palette().color(backgroundRole()));
	painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);

	if (m_highlight) {
		background.setAlphaF(0.5);
		painter.setBrush(background);
		painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);
	} else if (m_highlight_mid) {
		background.setAlphaF(0.3);
		painter.setBrush(background);
		painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);
	} else if (m_highlight_partial) {
		background.setAlphaF(0.1);
		painter.setBrush(background);
		painter.drawRoundedRect(QRectF(0.5, 0.5, width() - 1, height() - 1), 3, 3);
	}

	if (m_highlight_border) {
		painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
		painter.setBrush(Qt::NoBrush);
		painter.drawRoundedRect(QRectF(1.5, 1.5, width() - 3, height() - 3), 3, 3);
	}
}

//-----------------------------------------------------------------------------
