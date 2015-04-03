#include "signalviewer.h"

using namespace std;

SignalViewer::SignalViewer(QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* box = new QVBoxLayout;
	setLayout(box);
	box->setContentsMargins(0, 0, 0, 0);
	box->setSpacing(0);

	canvas = new Canvas(this);
	//canvas->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

	scrollBar = new QScrollBar(Qt::Horizontal, this);

	box->addWidget(canvas);
	box->addWidget(scrollBar);

	//setLayout(box);
}

SignalViewer::~SignalViewer()
{
}


