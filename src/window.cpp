/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2012, 2013, 2014 Graeme Gott <graeme@gottcode.org>
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

#include "window.h"

#include "board.h"
#include "locale_dialog.h"
#include "pattern.h"
#include "square.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSpinBox>
#include <QStackedWidget>
#include <QUndoStack>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>

//-----------------------------------------------------------------------------

namespace
{
	/**
	 * Interface button.
	 *
	 * This class defines the defaults for the action toolbuttons used in the
	 * main window.
	 */
	class SidebarButton : public QToolButton
	{
	public:
		/**
		 * Constructs an interface button.
		 *
		 * @param icon the icon for the button
		 * @param text the text to display on the button
		 * @param parent the parent widget of the button
		 */
		SidebarButton(const QString& icon, const QString& text, QWidget* parent = 0);
	};

	SidebarButton::SidebarButton(const QString& icon, const QString& text, QWidget* parent) :
		QToolButton(parent)
	{
		setText(text);
		setIconSize(QSize(32,32));
		setIcon(QIcon(icon));
		setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		setCheckable(true);
		setFocusPolicy(Qt::NoFocus);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}
}

//-----------------------------------------------------------------------------

Window::Window()
{
	setWindowTitle(tr("Simsu"));

	QSettings settings;

	QWidget* contents = new QWidget(this);
	setCentralWidget(contents);

	// Create board
	Square* square = new Square(contents);
	m_board = new Board(square);
	square->setChild(m_board);
	connect(m_board, &Board::activeKeyChanged, this, &Window::activeKeyChanged);
	connect(m_board, &Board::notesModeChanged, this, &Window::notesModeChanged);

	// Create mode buttons
	m_mode_buttons = new QButtonGroup(this);
	connect(m_mode_buttons, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_board, &Board::setMode);

	QToolButton* pen_button = new SidebarButton(":/pen.png", tr("Pen"), contents);
	m_mode_buttons->addButton(pen_button, 0);
	m_sidebar_buttons.append(pen_button);

	QToolButton* pencil_button = new SidebarButton(":/pencil.png", tr("Pencil"), contents);
	m_mode_buttons->addButton(pencil_button, 1);
	m_sidebar_buttons.append(pencil_button);

	QToolButton* highlight_button = new SidebarButton(":/highlight.png", tr("Highlight"), contents);
	m_sidebar_buttons.append(highlight_button);
	highlight_button->setShortcut(tr("H"));
	connect(highlight_button, &QToolButton::clicked, m_board, &Board::setHighlightActive);
	if (settings.value("Highlight").toBool()) {
		highlight_button->click();
	}

	m_mode_buttons->button(QSettings().value("Mode") == "Pencil")->click();
	new QShortcut(tr("S"), this, SLOT(toggleMode()));

	m_mode_layout = new QHBoxLayout;
	m_mode_layout->setMargin(0);
	m_mode_layout->addWidget(pen_button);
	m_mode_layout->addWidget(pencil_button);
	m_mode_layout->addStretch();
	m_mode_layout->addWidget(highlight_button);

	// Create key buttons
	m_keys_layout = new QHBoxLayout;
	m_keys_layout->setMargin(0);

	m_key_buttons = new QButtonGroup(this);
	connect(m_key_buttons, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_board, &Board::setActiveKey);

	for (int i = 1; i < 10; ++i) {
		QToolButton* key = new QToolButton(this);
		key->setText(QString::number(i));
		key->setCheckable(true);
		key->setFocusPolicy(Qt::NoFocus);
		key->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_key_buttons->addButton(key, i);
		m_keys_layout->addWidget(key);
	}

	m_key_buttons->button(qBound(1, QSettings().value("Key", 1).toInt(), 10))->click();

	// Layout window
	m_layout = new QVBoxLayout(contents);
	m_layout->addSpacing(6);
	m_layout->addWidget(square, 1);
	m_layout->addSpacing(6);
	m_layout->addLayout(m_keys_layout);
	m_layout->addLayout(m_mode_layout);

	// Create menus
	QMenu* menu = menuBar()->addMenu(tr("&Game"));
	menu->addAction(tr("&New"), this, SLOT(newGame()), QKeySequence::New);
	menu->addAction(tr("&Details"), this, SLOT(showDetails()));
	menu->addSeparator();
	QAction* action = menu->addAction(tr("&Quit"), qApp, SLOT(quit()), QKeySequence::Quit);
	action->setMenuRole(QAction::QuitRole);

	menu = menuBar()->addMenu(tr("&Move"));
	action = menu->addAction(tr("&Undo"), m_board->moves(), SLOT(undo()), QKeySequence::Undo);
	connect(m_board->moves(), &QUndoStack::canUndoChanged, action, &QAction::setEnabled);
	action = menu->addAction(tr("&Redo"), m_board->moves(), SLOT(redo()), QKeySequence::Redo);
	connect(m_board->moves(), &QUndoStack::canRedoChanged, action, &QAction::setEnabled);
	menu->addSeparator();
	menu->addAction(tr("&Check"), m_board, SLOT(showWrong()), tr("C"));

#ifndef Q_WS_HILDON
	menu = menuBar()->addMenu(tr("&Settings"));
	action = menu->addAction(tr("&Auto Switch Modes"));
	action->setCheckable(true);
	connect(action, &QAction::toggled, m_board, &Board::setAutoSwitch);
	action->setChecked(settings.value("AutoSwitch", true).toBool());
	action = menu->addAction(tr("&Widescreen Layout"));
	action->setCheckable(true);
	connect(action, &QAction::toggled, this, &Window::toggleWidescreen);
	action->setChecked(settings.value("Widescreen").toBool());
	menu->addSeparator();
	menu->addAction(tr("Application &Language..."), this, SLOT(setLocaleClicked()));

	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&Controls"), this, SLOT(showControls()), QKeySequence::HelpContents);
	menu->addSeparator();
#else
	m_board->setAutoSwitch(false);
	toggleWidescreen(true);

	menu = menuBar()->addMenu(tr("&Help"));
#endif
	action = menu->addAction(tr("&About"), this, SLOT(about()));
	action->setMenuRole(QAction::AboutRole);
	action = menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
	action->setMenuRole(QAction::AboutQtRole);

	// Restore size and position
	restoreGeometry(settings.value("Geometry").toByteArray());
}

//-----------------------------------------------------------------------------

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Geometry", saveGeometry());
	QMainWindow::closeEvent(event);
}

//-----------------------------------------------------------------------------

void Window::wheelEvent(QWheelEvent* event)
{
	int id = m_key_buttons->checkedId();
	if (event->delta() < 0) {
		id++;
		if (id > 9) {
			id = 1;
		}
	} else {
		id--;
		if (id < 1) {
			id = 9;
		}
	}
	m_key_buttons->button(id)->click();
	QMainWindow::wheelEvent(event);
}

//-----------------------------------------------------------------------------

void Window::newGame()
{
	QSettings settings;

	QDialog* dialog = new QDialog(this);
	dialog->setWindowTitle(tr("New Game"));

	QStackedWidget* preview = new QStackedWidget(dialog);

	QComboBox* symmetry_box = new QComboBox(dialog);
	for (int i = Pattern::Rotational180; i <= Pattern::None; ++i) {
		symmetry_box->addItem(Pattern::name(i), i);

		QLabel* image = new QLabel(preview);
		image->setPixmap(Pattern::icon(i));
		preview->addWidget(image);
	}
	preview->setCurrentIndex(0);
	connect(symmetry_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), preview, &QStackedWidget::setCurrentIndex);
	symmetry_box->setCurrentIndex(symmetry_box->findData(settings.value("Symmetry", Pattern::Rotational180).toInt()));

	QComboBox* algorithm_box = new QComboBox(dialog);
	algorithm_box->addItem(tr("Dancing Links"), 0);
	algorithm_box->addItem(tr("Slice and Dice"), 1);
	algorithm_box->setCurrentIndex(algorithm_box->findData(settings.value("Algorithm").toInt()));

	QSpinBox* seed_box = new QSpinBox(dialog);
	seed_box->setRange(0, 2147483647);
	seed_box->setSpecialValueText(tr("Random"));

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
	connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

	QFormLayout* contents_layout = new QFormLayout;
	contents_layout->addRow(QString(), preview);
	contents_layout->addRow(tr("Symmetry:"), symmetry_box);
	contents_layout->addRow(tr("Algorithm:"), algorithm_box);
	contents_layout->addRow(tr("Seed:"), seed_box);

	QVBoxLayout* layout = new QVBoxLayout(dialog);
	layout->addLayout(contents_layout);
	layout->addSpacing(18);
	layout->addWidget(buttons);

	if (dialog->exec() == QDialog::Accepted) {
		int symmetry = symmetry_box->itemData(symmetry_box->currentIndex()).toInt();
		settings.setValue("Symmetry", symmetry);
		int algorithm = algorithm_box->itemData(algorithm_box->currentIndex()).toInt();
		settings.setValue("Algorithm", algorithm);
		m_board->newPuzzle(seed_box->value(), symmetry, algorithm);
	}

	delete dialog;
}

//-----------------------------------------------------------------------------

void Window::showDetails()
{
	QSettings settings;
	QString symmetry = Pattern::name(settings.value("Current/Symmetry").toInt());
	QString icon = Pattern::icon(settings.value("Current/Symmetry").toInt());
	QString algorithm = settings.value("Current/Algorithm", 0).toInt() ? tr("Slice and Dice") : tr("Dancing Links");
	int seed = settings.value("Current/Seed").toInt();

	QMessageBox details(QMessageBox::NoIcon, tr("Details"), tr("<p><b>Symmetry:</b> %1<br><b>Algorithm:</b> %L2<br><b>Seed:</b> %L3</p>").arg(symmetry).arg(algorithm).arg(seed), QMessageBox::Ok, this);
	details.setIconPixmap(icon);
	details.exec();
}

//-----------------------------------------------------------------------------

void Window::showControls()
{
	QMessageBox::information(this, tr("Controls"), tr("<p><big><b>Mouse Controls:</b></big><br>"
		"<b>Left click:</b> Toggle number in pen mode<br>"
		"<b>Right click:</b> Toggle number in pencil mode<br>"
		"<b>Scrollwheel:</b> Change current number</p>"
		"<p><big><b>Keyboard Controls:</b></big><br>"
		"<b>Arrow keys:</b> Move selection<br>"
		"<b>Number keys:</b> Toggle value or note<br>"
		"<b>S:</b> Switch between pen and pencil modes<br>"
		"<b>H:</b> Highlight all instances of current number</p>"));
}

//-----------------------------------------------------------------------------

void Window::about()
{
	QMessageBox::about(this, tr("About Simsu"), QString("<p align='center'><big><b>%1 %2</b></big><br/>%3<br/><small>%4<br/>%5</small></p>")
		.arg(tr("Simsu"), QCoreApplication::applicationVersion(),
			tr("A basic Sudoku game"),
			tr("Copyright &copy; 2009-%1 Graeme Gott").arg("2014"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""))
	);
}

//-----------------------------------------------------------------------------

void Window::activeKeyChanged(int key)
{
	m_key_buttons->button(key)->setChecked(true);
}

//-----------------------------------------------------------------------------

void Window::notesModeChanged(bool mode)
{
	m_mode_buttons->button(mode)->setChecked(true);
}

//-----------------------------------------------------------------------------

void Window::toggleMode()
{
	m_mode_buttons->button(!m_mode_buttons->checkedId())->click();
}

//-----------------------------------------------------------------------------

void Window::toggleWidescreen(bool checked)
{
	QSettings().setValue("Widescreen", checked);

	Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon;
	int width = 0;
	if (checked) {
		m_mode_layout->setDirection(QBoxLayout::TopToBottom);
		m_keys_layout->setDirection(QBoxLayout::TopToBottom);
		m_layout->setDirection(QBoxLayout::LeftToRight);
		width = m_mode_layout->sizeHint().width();
		style = Qt::ToolButtonTextUnderIcon;
		m_layout->removeItem(m_mode_layout);
		m_mode_layout->setParent(0);
		m_layout->insertLayout(0, m_mode_layout);
	} else {
		m_mode_layout->setDirection(QBoxLayout::LeftToRight);
		m_keys_layout->setDirection(QBoxLayout::LeftToRight);
		m_layout->setDirection(QBoxLayout::TopToBottom);
		m_layout->removeItem(m_mode_layout);
		m_mode_layout->setParent(0);
		m_layout->addLayout(m_mode_layout);
	}

	for (int i = 0; i < m_sidebar_buttons.count(); ++i) {
		m_sidebar_buttons[i]->setToolButtonStyle(style);
		m_sidebar_buttons[i]->setMinimumWidth(width);
	}
	for (int i = 1; i < 10; ++i) {
		m_key_buttons->button(i)->setMinimumWidth(width);
	}
}

//-----------------------------------------------------------------------------

void Window::setLocaleClicked()
{
	LocaleDialog dialog(this);
	dialog.exec();
}

//-----------------------------------------------------------------------------
