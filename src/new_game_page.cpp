/*
	SPDX-FileCopyrightText: 2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "new_game_page.h"

#include "pattern.h"
#include "puzzle.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <array>

//-----------------------------------------------------------------------------

NewGamePage::NewGamePage(QWidget* parent)
	: QWidget(parent)
	, m_current_difficulty(0)
{
	m_contents = new QStackedWidget(this);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &NewGamePage::playGame);
	connect(buttons, &QDialogButtonBox::rejected, this, &NewGamePage::cancel);

	m_play_button = buttons->button(QDialogButtonBox::Ok);
	m_play_button->setText(tr("Play Game"));
	m_play_button->hide();

	m_create_button = new QPushButton(tr("Create Your Own"), this);
	m_create_button->setCheckable(true);
	connect(m_create_button, &QPushButton::toggled, this, &NewGamePage::showCustom);

	QGridLayout* layout = new QGridLayout(this);
	layout->setRowStretch(0, 1);
	layout->setRowMinimumHeight(1, 12);
	layout->addWidget(m_contents, 0, 0, 1, 2);
	layout->addWidget(m_create_button, 2, 0);
	layout->addWidget(buttons, 2, 1);


	// Create widgets for generated puzzles
	QWidget* generated = new QWidget(this);
	m_contents->addWidget(generated);

	// Create symmetry list
	m_symmetry = new QListWidget(generated);
	m_symmetry->setIconSize(QSize(60, 60));
	m_symmetry->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_symmetry->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	for (int i = Pattern::Rotational180; i <= Pattern::None; ++i) {
		new QListWidgetItem(QIcon(Pattern::icon(i)).pixmap(60, 60), Pattern::name(i), m_symmetry);
	}

	// Create difficulty buttons for starting a new game
	for (int i = Puzzle::VeryEasy; i <= Puzzle::Hard; ++i) {
		QPushButton* button = new QPushButton(Puzzle::difficultyString(i), generated);
		button->setAutoDefault(true);
		m_difficulty.append(button);
		connect(button, &QPushButton::clicked, this, [this, i]() {
			emit generatePuzzle(m_symmetry->currentRow(), i);
		});
	}

	// Lay out generated widgets
	QVBoxLayout* generated_layout = new QVBoxLayout(generated);
	generated_layout->setContentsMargins(0,0,0,0);
	generated_layout->setSpacing(2);
	generated_layout->addWidget(m_symmetry);
	generated_layout->addSpacing(12);
	for (QPushButton* button : qAsConst(m_difficulty)) {
		generated_layout->addWidget(button);
	}


	// Create widgets for custom puzzles
	QWidget* custom = new QWidget(this);
	m_contents->addWidget(custom);

	// Create line edits
	QIntValidator* validator = new QIntValidator(1, 9, this);
	for (int i = 0; i < 81; ++i) {
		QLineEdit* edit = new QLineEdit(custom);
		edit->setMaxLength(1);
		edit->setValidator(validator);
		edit->setAlignment(Qt::AlignCenter);
		edit->setMinimumWidth(1);
		connect(edit, &QLineEdit::textChanged, this, &NewGamePage::showConflicts);
		m_custom.append(edit);
	}

	// Lay out custom widgets
	QGridLayout* custom_layout = new QGridLayout(custom);
	custom_layout->setContentsMargins(0,0,0,0);
	custom_layout->setColumnStretch(0, 100);
	custom_layout->setColumnStretch(12, 100);
	custom_layout->setColumnMinimumWidth(4, 6);
	custom_layout->setColumnMinimumWidth(8, 6);
	custom_layout->setRowStretch(0, 1);
	custom_layout->setRowStretch(12, 1);
	custom_layout->setRowMinimumHeight(4, 6);
	custom_layout->setRowMinimumHeight(8, 6);
	custom_layout->setSpacing(1);
	for (int r = 0; r < 9; ++r) {
		const int y = r * 9;
		const int y_offset = (y / 27) + 1;
		for (int c = 0; c < 9; ++c) {
			const int x_offset = (c / 3) + 1;
			QLineEdit* edit = m_custom[c + y];
			custom_layout->addWidget(edit, r + y_offset, c + x_offset);
		}
	}
}

//-----------------------------------------------------------------------------

void NewGamePage::reset()
{
	m_create_button->setChecked(false);

	// Reset widgets for custom puzzle
	m_contents->setCurrentIndex(1);

	for (QLineEdit* edit : qAsConst(m_custom)) {
		edit->clear();
	}

	m_custom[0]->setFocus();

	// Reset widgets for generated puzzle
	m_contents->setCurrentIndex(0);

	QSettings settings;

	const int symmetry = qBound(int(Pattern::Rotational180), settings.value("Symmetry", Pattern::Rotational180).toInt(), int(Pattern::None));
	QListWidgetItem* item = m_symmetry->item(symmetry);
	m_symmetry->setCurrentItem(item);
	m_symmetry->scrollToItem(item, QAbstractItemView::PositionAtCenter);

	m_current_difficulty = qBound(int(Puzzle::VeryEasy), settings.value("Difficulty", Puzzle::VeryEasy).toInt(), int(Puzzle::Hard)) - Puzzle::VeryEasy;
	m_difficulty[m_current_difficulty]->setFocus();
}

//-----------------------------------------------------------------------------

void NewGamePage::keyPressEvent(QKeyEvent* event)
{
	if ((m_contents->currentIndex() == 1) && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
		m_play_button->click();
	} else {
		QWidget::keyPressEvent(event);
	}
}

//-----------------------------------------------------------------------------

void NewGamePage::playGame()
{
	// Fetch givens entered by player
	std::array<int, 81> givens;
	for (int i = 0; i < 81; ++i) {
		givens[i] = m_custom[i]->text().toInt();
	}

	// Start game
	emit loadPuzzle(givens);
}

//-----------------------------------------------------------------------------

void NewGamePage::showConflicts()
{
	// Reset cells
	QPalette p = palette();
	for (QLineEdit* edit : qAsConst(m_custom)) {
		edit->setPalette(p);
	}

	p.setColor(QPalette::Text, Qt::white);
	p.setColor(QPalette::Base, Qt::red);
	std::array<QLineEdit*, 9> found;

	// Find conflicts in rows
	for (int r = 0; r < 81; r += 9) {
		found.fill(nullptr);
		for (int c = 0; c < 9; ++c) {
			QLineEdit* edit = m_custom[c + r];
			int value = edit->text().toInt();
			if (!value) {
				continue;
			}
			--value;

			if (found[value]) {
				found[value]->setPalette(p);
				edit->setPalette(p);
			} else {
				found[value] = edit;
			}
		}
	}

	// Find conflicts in columns
	for (int c = 0; c < 9; ++c) {
		found.fill(nullptr);
		for (int r = 0; r < 81; r += 9) {
			QLineEdit* edit = m_custom[c + r];
			int value = edit->text().toInt();
			if (!value) {
				continue;
			}
			--value;

			if (found[value]) {
				found[value]->setPalette(p);
				edit->setPalette(p);
			} else {
				found[value] = edit;
			}
		}
	}

	// Find conflicts in boxes
	for (int box_r = 0; box_r < 9; box_r += 3) {
		for (int box_c = 0; box_c < 9; box_c += 3) {
			found.fill(nullptr);
			for (int r = 0; r < 3; ++r) {
				const int y = (r + box_r) * 9;
				for (int c = 0; c < 3; ++c) {
					QLineEdit* edit = m_custom[c + box_c + y];
					int value = edit->text().toInt();
					if (!value) {
						continue;
					}
					--value;

					if (found[value]) {
						found[value]->setPalette(p);
						edit->setPalette(p);
					} else {
						found[value] = edit;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void NewGamePage::showCustom(bool show)
{
	if (show) {
		m_contents->setCurrentIndex(1);
		m_play_button->show();
		m_custom[0]->setFocus();
	} else {
		m_contents->setCurrentIndex(0);
		m_play_button->hide();
		m_difficulty[m_current_difficulty]->setFocus();
	}
}

//-----------------------------------------------------------------------------
