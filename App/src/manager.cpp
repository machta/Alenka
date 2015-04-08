#include "manager.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QTextDocumentFragment>

using namespace std;

Manager::Manager(QWidget* parent) : QWidget(parent, Qt::Window)
{
	// Construct the table widget.
	tableView = new QTableView(this);

	tableView->setSortingEnabled(true);
	tableView->sortByColumn(0, Qt::AscendingOrder);

	tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);

	QAction* copyAction = new QAction("Copy", this);
	copyAction->setShortcut(QKeySequence::Copy);
	copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	tableView->addAction(copyAction);

	QAction* copyHtmlAction = new QAction("Copy HTML", this);
	copyHtmlAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
	copyHtmlAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyHtmlAction, SIGNAL(triggered()), this, SLOT(copyHtml()));
	tableView->addAction(copyHtmlAction);

	// Construct other.
	QPushButton* addRowButton = new QPushButton("Add Row", this);
	connect(addRowButton, SIGNAL(clicked()), this, SLOT(addRow()));

	QPushButton* removeRowButton = new QPushButton("Remove Row", this);
	connect(removeRowButton, SIGNAL(clicked()), this, SLOT(removeRow()));

	// Add the widgets to a layout.
	buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(addRowButton);
	buttonLayout->addWidget(removeRowButton);

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

void Manager::removeRow()
{
	QModelIndex currentIndex = tableView->selectionModel()->currentIndex();
	if (currentIndex.isValid())
	{
		tableView->model()->removeRow(currentIndex.row());
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

	QApplication::clipboard()->setText(text);
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

	QApplication::clipboard()->setText(text);
}
