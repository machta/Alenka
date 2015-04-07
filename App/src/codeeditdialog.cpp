#include "codeeditdialog.h"

#include "openclcontext.h"
#include "options.h"
#include "SignalProcessor/montage.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>

#include <string>

using namespace std;

CodeEditDialog::CodeEditDialog(QWidget* parent) : QDialog(parent)
{
	setWindowFlags(Qt::Window);
	setMinimumWidth(500);
	setMinimumHeight(500);

	QVBoxLayout* box = new QVBoxLayout;

	box->addWidget(new QLabel("Input and output definition: \n\tfloat4 out = 0;\n\tfloat4 in(int);\nReserved names are of the form _*_ plus all OpenCL reserved names are forbidden.\nFunctions from montageHeader.cl file:", this)); // TODO: Move these to help sub dialog.

	QTextEdit* header = new QTextEdit(this);
	header->setPlainText(QString::fromStdString(Montage::readHeader()));
	header->setReadOnly(true);
	box->addWidget(header);

	box->addWidget(new QLabel("Enter code here:", this));

	editor = new QTextEdit(this);
	box->addWidget(editor);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	box->addWidget(buttonBox);

	setLayout(box);
}

CodeEditDialog::~CodeEditDialog()
{
}

QString CodeEditDialog::getText() const
{
	return editor->toPlainText();
}

void CodeEditDialog::setText(const QString& text)
{
	editor->setPlainText(text);
}

void CodeEditDialog::done(int r)
{
	if (QDialog::Accepted == r)
	{
		QString message;

		if (validator.validate(getText(), &message))
		{
			QDialog::done(r);
		}
		else
		{
			validator.errorMessageDialog(message, this);
		}
	}
	else
	{
		QDialog::done(r);
	}
}

CodeEditDialog::Validator::Validator()
{
	context = new OpenCLContext(OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS);
}

CodeEditDialog::Validator::~Validator()
{
	delete context;
}

bool CodeEditDialog::Validator::validate(const QString& input, QString* errorMessage)
{
	string message;

	bool result = Montage::test(input.toStdString(), context, &message);

	*errorMessage = QString::fromStdString(message);

	return result;
}

void CodeEditDialog::Validator::errorMessageDialog(const QString& message, QWidget* parent)
{
	// TODO: Make a better error dialog.
	QMessageBox messageBox(parent);
	messageBox.setWindowTitle("Compilation Error");
	messageBox.setText("The code entered is not valid." + QString(200, ' '));
	messageBox.setDetailedText(message);
	messageBox.setIcon(QMessageBox::Critical);
	messageBox.exec();
}
