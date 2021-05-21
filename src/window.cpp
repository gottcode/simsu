/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "window.h"

#include "board.h"
#include "locale_dialog.h"
#include "new_game_page.h"
#include "pattern.h"
#include "puzzle.h"
#include "square.h"

#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
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
		SidebarButton(const QString& icon, const QString& text, QWidget* parent = nullptr);
	};

	SidebarButton::SidebarButton(const QString& icon, const QString& text, QWidget* parent)
		: QToolButton(parent)
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
	m_contents = new QStackedWidget(this);
	setCentralWidget(m_contents);

	// Create new game page
	m_new_game = new NewGamePage(this);
	connect(m_new_game, &NewGamePage::cancel, this, &Window::newGameCanceled);
	connect(m_new_game, &NewGamePage::generatePuzzle, this, [this](int symmetry, int difficulty) {
		m_new_action->setEnabled(true);
		m_board->newPuzzle(symmetry, difficulty);
		m_contents->setCurrentIndex(1);
		m_key_buttons->button(1)->click();
	});
	connect(m_new_game, &NewGamePage::loadPuzzle, this, [this](const std::array<int, 81>& givens) {
		if (m_board->newPuzzle(givens)) {
			m_new_action->setEnabled(true);
			m_contents->setCurrentIndex(2);
			m_key_buttons->button(1)->click();
		} else {
			QMessageBox::information(this, tr("Sorry"), tr("You have not created a valid puzzle. Please try again."));
		}
	});
	m_contents->addWidget(m_new_game);

	// Create load screen
	m_load_message = new QLabel(QString("<big><b>%1</b></big><br>%2").arg(tr("Please wait"), tr("Generating puzzle...")), this);
	m_load_message->setAlignment(Qt::AlignCenter);
	m_load_message->setCursor(Qt::BusyCursor);
	m_contents->addWidget(m_load_message);

	// Create game widgets
	QWidget* contents = new QWidget(this);
	m_contents->addWidget(contents);

	// Create board
	Square* square = new Square(contents);
	m_board = new Board(square);
	square->setChild(m_board);
	connect(m_board, &Board::activeKeyChanged, this, &Window::activeKeyChanged);
	connect(m_board, &Board::notesModeChanged, this, &Window::notesModeChanged);
	connect(m_board, &Board::gameStarted, this, &Window::gameStarted);
	connect(m_board, &Board::gameFinished, this, &Window::gameFinished);

	QSettings settings;

	// Create mode buttons
	m_mode_buttons = new QButtonGroup(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
	connect(m_mode_buttons, &QButtonGroup::idClicked, m_board, &Board::setMode);
#else
	connect(m_mode_buttons, qOverload<int>(&QButtonGroup::buttonClicked), m_board, &Board::setMode);
#endif

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
	QShortcut* shortcut = new QShortcut(tr("S"), this);
	connect(shortcut, &QShortcut::activated, this, &Window::toggleMode);

	m_mode_layout = new QHBoxLayout;
	m_mode_layout->setContentsMargins(0, 0, 0, 0);
	m_mode_layout->addWidget(pen_button);
	m_mode_layout->addWidget(pencil_button);
	m_mode_layout->addStretch();
	m_mode_layout->addWidget(highlight_button);

	// Create key buttons
	m_keys_layout = new QHBoxLayout;
	m_keys_layout->setContentsMargins(0, 0, 0, 0);

	m_key_buttons = new QButtonGroup(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
	connect(m_key_buttons, &QButtonGroup::idClicked, m_board, &Board::setActiveKey);
#else
	connect(m_key_buttons, qOverload<int>(&QButtonGroup::buttonClicked), m_board, &Board::setActiveKey);
#endif

	for (int i = 1; i < 10; ++i) {
		QToolButton* key = new QToolButton(this);
		key->setText(QString::number(i));
		key->setCheckable(true);
		key->setFocusPolicy(Qt::NoFocus);
		key->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_key_buttons->addButton(key, i);
		m_keys_layout->addWidget(key);
	}

	// Create notes fill group
	m_auto_notes_actions = new QActionGroup(this);
	m_auto_notes_actions->setExclusive(true);
	connect(m_auto_notes_actions, &QActionGroup::triggered, this, &Window::autoNotesChanged);

	// Layout window
	m_layout = new QVBoxLayout(contents);
	m_layout->addSpacing(6);
	m_layout->addWidget(square, 1);
	m_layout->addSpacing(6);
	m_layout->addLayout(m_keys_layout);
	m_layout->addLayout(m_mode_layout);

	// Create menus
	QMenu* menu = menuBar()->addMenu(tr("&Game"));
	m_new_action = menu->addAction(tr("&New"), this, &Window::newGame, QKeySequence::New);
	m_restart_action = menu->addAction(tr("&Restart"), this, &Window::restartGame, QKeySequence::Refresh);
	m_restart_action->setEnabled(false);
	menu->addSeparator();
	m_details_action = menu->addAction(tr("&Details"), this, &Window::showDetails);
	m_details_action->setEnabled(false);
	menu->addSeparator();
	QAction* action = menu->addAction(tr("&Quit"), qApp, &QApplication::quit, QKeySequence::Quit);
	action->setMenuRole(QAction::QuitRole);

	menu = menuBar()->addMenu(tr("&Move"));
	m_undo_action = menu->addAction(tr("&Undo"), m_board->moves(), &QUndoStack::undo, QKeySequence::Undo);
	m_undo_action->setEnabled(false);
	connect(m_board->moves(), &QUndoStack::canUndoChanged, m_undo_action, &QAction::setEnabled);
	m_redo_action = menu->addAction(tr("&Redo"), m_board->moves(), &QUndoStack::redo, QKeySequence::Redo);
	m_redo_action->setEnabled(false);
	connect(m_board->moves(), &QUndoStack::canRedoChanged, m_redo_action, &QAction::setEnabled);
	menu->addSeparator();
	m_check_action = menu->addAction(tr("&Check"), m_board, [this]() { m_board->showWrong(true); }, tr("C"));
	m_check_action->setEnabled(false);
	m_hint_action = menu->addAction(tr("&Hint"), m_board, &Board::hint, tr("F2"));
	m_hint_action->setEnabled(false);

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
	action = menu->addAction(tr("&Manual Notes"));
	action->setCheckable(true);
	action->setData(Board::ManualNotes);
	m_auto_notes_actions->addAction(action);
	action = menu->addAction(tr("Auto &Clear Notes"));
	action->setCheckable(true);
	action->setData(Board::AutoClearNotes);
	m_auto_notes_actions->addAction(action);
	action = menu->addAction(tr("Auto &Fill Notes"));
	action->setCheckable(true);
	action->setData(Board::AutoFillNotes);
	m_auto_notes_actions->addAction(action);
	menu->addSeparator();
	menu->addAction(tr("Application &Language..."), this, &Window::setLocaleClicked);

	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&Controls"), this, &Window::showControls, QKeySequence::HelpContents);
	menu->addSeparator();

	action = menu->addAction(tr("&About"), this, &Window::about);
	action->setMenuRole(QAction::AboutRole);
	action = menu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
	action->setMenuRole(QAction::AboutQtRole);

	// Restore auto notes mode
	const QString auto_notes = settings.value("AutoNotes").toString();
	if (auto_notes == "Clear") {
		action = m_auto_notes_actions->actions().at(Board::AutoClearNotes);
	} else if (auto_notes == "Fill") {
		action = m_auto_notes_actions->actions().at(Board::AutoFillNotes);
	} else {
		action = m_auto_notes_actions->actions().at(Board::ManualNotes);
	}
	action->trigger();

	// Restore size and position
	restoreGeometry(settings.value("Geometry").toByteArray());

	// Show new game dialog if not able to start previous game
	if (!m_board->loadPuzzle()) {
		newGame();
	}
	m_key_buttons->button(qBound(1, settings.value("Key", 1).toInt(), 10))->click();
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
	if (event->angleDelta().y() < 0) {
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
	m_new_game->reset();

	m_new_action->setEnabled(false);
	m_restart_action->setEnabled(false);
	m_details_action->setEnabled(false);
	m_undo_action->setEnabled(false);
	m_redo_action->setEnabled(false);
	m_check_action->setEnabled(false);
	m_hint_action->setEnabled(false);

	m_contents->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------

void Window::newGameCanceled()
{
	if (!m_board->isLoaded()) {
		qApp->quit();
		return;
	}

	m_new_action->setEnabled(true);

	bool enabled = m_board->isLoaded();
	m_restart_action->setEnabled(enabled);
	m_details_action->setEnabled(enabled);

	enabled = !m_board->isFinished();
	m_undo_action->setEnabled(enabled && m_board->moves()->canUndo());
	m_redo_action->setEnabled(enabled && m_board->moves()->canRedo());
	m_check_action->setEnabled(enabled);
	m_hint_action->setEnabled(enabled);

	m_contents->setCurrentIndex(2);
}

//-----------------------------------------------------------------------------

void Window::restartGame()
{
	QMessageBox message(QMessageBox::Question, tr("Restart Game"), tr("Reset the board to its original state?"), QMessageBox::Cancel, this);
	message.setDefaultButton(QMessageBox::Cancel);
	message.addButton(tr("Restart Game"), QMessageBox::AcceptRole);
	if (message.exec() == QMessageBox::Cancel) {
		return;
	}

	m_board->restartPuzzle();
}

//-----------------------------------------------------------------------------

void Window::showDetails()
{
	QSettings settings;
	QString symmetry = Pattern::name(settings.value("Current/Symmetry").toInt());
	QString icon = Pattern::icon(settings.value("Current/Symmetry").toInt());
	QString difficulty = Puzzle::difficultyString(settings.value("Current/Difficulty").toInt());

	QMessageBox details(QMessageBox::NoIcon,
		tr("Details"),
		QString("<p><b>%1</b> %2<br><b>%3</b> %4</p>")
			.arg(tr("Symmetry:"), symmetry, tr("Difficulty:"), difficulty),
		QMessageBox::Ok,
		this);
	details.setIconPixmap(QIcon(icon).pixmap(60, 60));
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
			tr("Copyright &copy; 2009-%1 Graeme Gott").arg("2021"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""))
	);
}

//-----------------------------------------------------------------------------

void Window::gameFinished()
{
	m_check_action->setEnabled(false);
	m_hint_action->setEnabled(false);
}

//-----------------------------------------------------------------------------

void Window::gameStarted()
{
	m_restart_action->setEnabled(true);
	m_check_action->setEnabled(true);
	m_hint_action->setEnabled(true);
	m_details_action->setEnabled(true);

	m_contents->setCurrentIndex(2);
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

void Window::autoNotesChanged(QAction* action)
{
	m_board->setAutoNotes(action->data().toInt());
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
		m_mode_layout->setParent(nullptr);
		m_layout->insertLayout(0, m_mode_layout);
	} else {
		m_mode_layout->setDirection(QBoxLayout::LeftToRight);
		m_keys_layout->setDirection(QBoxLayout::LeftToRight);
		m_layout->setDirection(QBoxLayout::TopToBottom);
		m_layout->removeItem(m_mode_layout);
		m_mode_layout->setParent(nullptr);
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
