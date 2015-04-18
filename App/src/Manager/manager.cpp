#include "manager.h"

#include "../myapplication.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QAction>
#include <QClipboard>
#include <QTextDocumentFragment>

#include <sstream>
#include <algorithm>
#include <cassert>

using namespace std;

Manager::Manager(QWidget* parent) : QWidget(parent, Qt::Window)
{
	// Construct the table widget.
	tableView = new QTableView(this);

	tableView->setSortingEnabled(true);
	tableView->sortByColumn(0, Qt::AscendingOrder);

	tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);

	tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// Add some actions to the tableView.
	QAction* addRowAction = new QAction("Add Row", this);
	//addRowAction->setShortcut(QKeySequence());
	//addRowAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(addRowAction, SIGNAL(triggered()), this, SLOT(addRow()));
	tableView->addAction(addRowAction);

	QAction* removeRowsAction = new QAction("Remove Rows", this);
	removeRowsAction->setShortcut(QKeySequence::Delete);
	removeRowsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(removeRowsAction, SIGNAL(triggered()), this, SLOT(removeRows()));
	tableView->addAction(removeRowsAction);

	QAction* copyAction = new QAction("Copy", this);
	copyAction->setShortcut(QKeySequence::Copy);
	copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	tableView->addAction(copyAction);

	QAction* copyHtmlAction = new QAction("Copy HTML", this);
	//copyHtmlAction->setShortcut(QKeySequence());
	//copyHtmlAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyHtmlAction, SIGNAL(triggered()), this, SLOT(copyHtml()));
	tableView->addAction(copyHtmlAction);

	QAction* pasteAction = new QAction("Paste", this);
	pasteAction->setShortcut(QKeySequence::Paste);
	pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	tableView->addAction(pasteAction);

	// Construct other.
	QPushButton* addRowButton = new QPushButton("Add Row", this);
	connect(addRowButton, SIGNAL(clicked()), this, SLOT(addRow()));

	QPushButton* removeRowButton = new QPushButton("Remove Rows", this);
	connect(removeRowButton, SIGNAL(clicked()), this, SLOT(removeRows()));

	// Add the widgets to a layout.
	buttonLayout = new QGridLayout;
	addButton(addRowButton);
	addButton(removeRowButton);

	QVBoxLayout* box1 = new QVBoxLayout;
	box1->addLayout(buttonLayout);
	box1->addWidget(tableView);
	setLayout(box1);
}

Manager::~Manager()
{
}

void Manager::setModel(QAbstractTableModel* model)
{
	tableView->setModel(model);

	tableView->sortByColumn(tableView->horizontalHeader()->sortIndicatorSection(), tableView->horizontalHeader()->sortIndicatorOrder());
}

void Manager::addButton(QPushButton* button)
{
	int row = buttonsAdded/buttonsPerRow;
	int col = buttonsAdded%buttonsPerRow;

	buttonLayout->addWidget(button, row, col);

	++buttonsAdded;
}

map<pair<int, int>, QString> Manager::textOfSelection()
{
	QModelIndexList indexes = tableView->selectionModel()->selectedIndexes();

	map<pair<int, int>, QString> cells;

	for (const QModelIndex& e : indexes)
	{
		QVariant value = e.data(Qt::EditRole);

		cells[make_pair(e.row(), e.column())] = value.convert(QMetaType::QString) ? value.toString() : "";
	}

	return cells;
}

void Manager::addRow()
{
	tableView->model()->insertRow(tableView->model()->rowCount());
}

void Manager::removeRows()
{
	auto indexes = tableView->selectionModel()->selection().indexes();

	if (indexes.empty() == false)
	{
		int m = tableView->model()->rowCount();
		int M = 0;

		for (const auto& e : indexes)
		{
			if (e.isValid())
			{
				m = min(m, e.row());
				M = max(M, e.row());
			}
		}

		tableView->model()->removeRows(m, M - m + 1);
	}
}

void Manager::copy()
{
	auto cells = textOfSelection();

	QString text;

	auto iter = cells.begin();
	if (iter != cells.end())
	{
		text += replaceTabsAndBreaks(iter->second);
	}

	int lastRow = iter->first.first;
	++iter;
	while (iter != cells.end())
	{
		if (iter->first.first != lastRow)
		{
			lastRow = iter->first.first;

			text += "\n";
		}
		else
		{
			text += "\t";
		}

		text += replaceTabsAndBreaks(iter->second);

		++iter;
	}

	MyApplication::clipboard()->setText(text);
}

void Manager::copyHtml()
{
	auto cells = textOfSelection();

	QString text;

	text += "<table>";
	text += "<tr>";

	auto iter = cells.begin();
	if (iter != cells.end())
	{
		text += "<td>";
		text += iter->second.toHtmlEscaped();
		text += "</td>";
	}

	int lastRow = iter->first.first;
	++iter;
	while (iter != cells.end())
	{
		if (iter->first.first != lastRow)
		{
			lastRow = iter->first.first;

			text += "</tr><tr>";
		}

		text += "<td>";
		text += iter->second.toHtmlEscaped();
		text += "</td>";

		++iter;
	}

	text += "</table>";

	MyApplication::clipboard()->setText(text);
}

void Manager::paste()
{
	QAbstractItemModel* model = tableView->model();

	int startRow, startColumn;

	auto index = tableView->selectionModel()->currentIndex();
	if (index.isValid())
	{
		startRow = index.row();
		startColumn = index.column();
	}
	else
	{
		startRow = startColumn = 0;
	}

	// Split the input into rows.
	stringstream textStream(MyApplication::clipboard()->text().toStdString());
	vector<string> lines;

	while (textStream.good())
	{
		lines.emplace_back("");
		getline(textStream, lines.back());
	}

	// Ignore last empty row.
	if (lines.size() >= 1 && lines.back() == "")
	{
		lines.pop_back();
	}

	// Insert rows back.
	int rowsToInsert = startRow + lines.size() - model->rowCount();
	if (rowsToInsert > 0)
	{
		bool res = model->insertRows(model->rowCount(), rowsToInsert);

		assert(res == true); (void)res;
	}

	// Process cells.
	for (unsigned int row = 0; row < lines.size(); ++row)
	{
		stringstream lineStream(lines[row]);

		int column = 0;

		while (lineStream.good())
		{
			string cell;
			getline(lineStream, cell, '\t');

			if (startColumn + column < model->columnCount())
			{
				model->setData(model->index(startRow + row, startColumn + column), QVariant(QString::fromStdString(cell)));
			}

			++column;
		}
	}
}
