/*
	SPDX-FileCopyrightText: 2009-2016 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "frame.h"

#include <QPainter>

//-----------------------------------------------------------------------------

Frame::Frame(QWidget* parent)
	: QWidget(parent)
	, m_highlight(false)
	, m_highlight_border(false)
	, m_highlight_partial(false)
	, m_highlight_mid(false)
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
