/*
	SPDX-FileCopyrightText: 2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "new_game_page.h"

#include "pattern.h"
#include "puzzle.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

NewGamePage::NewGamePage(QWidget* parent)
	: QWidget(parent)
{
	// Create symmetry list
	m_symmetry = new QListWidget(this);
	m_symmetry->setIconSize(QSize(60, 60));
	m_symmetry->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_symmetry->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	for (int i = Pattern::Rotational180; i <= Pattern::None; ++i) {
		new QListWidgetItem(QIcon(Pattern::icon(i)).pixmap(60, 60), Pattern::name(i), m_symmetry);
	}

	// Create difficulty buttons for starting a new game
	for (int i = Puzzle::VeryEasy; i <= Puzzle::Hard; ++i) {
		QPushButton* button = new QPushButton(Puzzle::difficultyString(i), this);
		button->setAutoDefault(true);
		m_difficulty.append(button);
		connect(button, &QPushButton::clicked, this, [this, i]() {
			emit generatePuzzle(m_symmetry->currentRow(), i);
		});
	}

	// Create button to cancel starting a new game
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &NewGamePage::cancel);

	// Lay out page
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(2);
	layout->addWidget(m_symmetry);
	layout->addSpacing(12);
	for (QPushButton* button : qAsConst(m_difficulty)) {
		layout->addWidget(button);
	}
	layout->addSpacing(12);
	layout->addWidget(buttons);

	// Load settings
	QSettings settings;

	const int symmetry = qBound(int(Pattern::Rotational180), settings.value("Symmetry", Pattern::Rotational180).toInt(), int(Pattern::None));
	m_symmetry->setCurrentRow(symmetry);

	const int difficulty = qBound(int(Puzzle::VeryEasy), settings.value("Difficulty", Puzzle::VeryEasy).toInt(), int(Puzzle::Hard)) - Puzzle::VeryEasy;
	m_difficulty[difficulty]->setFocus();
}

//-----------------------------------------------------------------------------
