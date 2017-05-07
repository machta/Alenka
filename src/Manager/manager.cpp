#include "manager.h"

#include "../myapplication.h"
#include "tablemodel.h"
#include "sortproxymodel.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

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

namespace
{

QString replaceTabsAndNewLines(const QString& string)
{
	QString newString = string;

	for (auto& e : newString)
	{
		if (e == '\t' || e == '\n')
			e = ' ';
	}

	return newString;
}

vector<string> splitStringToLines(const string& str)
{
	stringstream textStream(str);
	vector<string> lines;

	while (textStream.good())
	{
		lines.emplace_back("");
		getline(textStream, lines.back());
	}

	// Ignore last empty row.
	if (lines.size() >= 1 && lines.back() == "")
		lines.pop_back();

	return lines;
}

} // namespace

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
	addRowAction->setStatusTip("Add rows at the end");
	connect(addRowAction, SIGNAL(triggered()), this, SLOT(insertRowBack()));
	tableView->addAction(addRowAction);

	QAction* removeRowsAction = new QAction("Remove Rows", this);
	removeRowsAction->setStatusTip("Remove selected rows");
	removeRowsAction->setShortcut(QKeySequence::Delete);
	removeRowsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(removeRowsAction, SIGNAL(triggered()), this, SLOT(removeRows()));
	tableView->addAction(removeRowsAction);

	QAction* setColumnAction = new QAction("Set Column", this);
	setColumnAction->setStatusTip("Set all cells in a column to the selected value");
	connect(setColumnAction, SIGNAL(triggered()), this, SLOT(setColumn()));
	tableView->addAction(setColumnAction);

	addSeparator();

	QAction* copyAction = new QAction("Copy", this);
	copyAction->setStatusTip("Copy as tab separated table");
	copyAction->setShortcut(QKeySequence::Copy);
	copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	tableView->addAction(copyAction);

	QAction* copyHtmlAction = new QAction("Copy HTML", this);
	copyHtmlAction->setStatusTip("Copy as a HTML table");
	connect(copyHtmlAction, SIGNAL(triggered()), this, SLOT(copyHtml()));
	tableView->addAction(copyHtmlAction);

	QAction* pasteAction = new QAction("Paste", this);
	pasteAction->setShortcut(QKeySequence::Paste);
	pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	tableView->addAction(pasteAction);

	addSeparator();

	// Construct buttons.
	QPushButton* addRowButton = new QPushButton("Add Row", this);
	connect(addRowButton, SIGNAL(clicked()), this, SLOT(insertRowBack()));

	QPushButton* removeRowButton = new QPushButton("Remove Rows", this);
	connect(removeRowButton, SIGNAL(clicked()), this, SLOT(removeRows()));

	// Add the widgets to the layout.
	buttonLayout = new QGridLayout;
	addButton(addRowButton);
	addButton(removeRowButton);

	QVBoxLayout* box = new QVBoxLayout;
	box->addLayout(buttonLayout);
	box->addWidget(tableView);
	setLayout(box);
}

void Manager::setModel(TableModel* model)
{
	QSortFilterProxyModel* proxyModel = new SortProxyModel(model);
	proxyModel->setDynamicSortFilter(false);
	proxyModel->setSourceModel(model);
	tableView->setModel(proxyModel);

	tableView->setItemDelegate(model->getDelegate());

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

void Manager::addSeparator()
{
	QAction* separator = new QAction(this);
	separator->setSeparator(true);
	tableView->addAction(separator);
}

void Manager::removeRows()
{
	if (!file)
		return;

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
	if (!file)
		return;

	auto cells = textOfSelection();

	QString text;

	auto iter = cells.begin();
	if (iter != cells.end())
	{
		text += replaceTabsAndNewLines(iter->second);
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

		text += replaceTabsAndNewLines(iter->second);

		++iter;
	}

	MyApplication::clipboard()->setText(text);
}

void Manager::copyHtml()
{
	if (!file)
		return;

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
	if (!file)
		return;

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

	QAbstractItemModel* model = tableView->model();
	vector<string> lines = splitStringToLines(MyApplication::clipboard()->text().toStdString());
	int rowsToInsert = startRow + static_cast<int>(lines.size()) - model->rowCount();

	file->undoFactory->beginMacro("paste");

	bool insertOK = true;
	for (int i = 0; i < rowsToInsert; ++i)
		insertOK &= insertRowBack();

	if (insertOK)
	{
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
					auto val = QVariant(QString::fromStdString(cell));
					model->setData(model->index(startRow + row, startColumn + column), val);
				}

				++column;
			}
		}
	}

	file->undoFactory->endMacro();
}

void Manager::setColumn()
{
	if (!file)
		return;

	auto currentIndex = tableView->selectionModel()->currentIndex();

	if (file && currentIndex.isValid())
	{
		auto model = tableView->model();
		int col = currentIndex.column();
		auto val = model->index(currentIndex.row(), col).data(Qt::EditRole);
		int rc = model->rowCount();

		file->undoFactory->beginMacro("set column");

		for (int i = 0; i < rc; ++i)
			model->setData(model->index(i, col), val);

		file->undoFactory->endMacro();
	}
}
