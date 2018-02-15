#include "codeeditdialog.h"

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/trackcodevalidator.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

#include <string>

using namespace std;

// TODO: Improve the help text in the dialog, possibly make a wiki entry and put
// here a link to it.
CodeEditDialog::CodeEditDialog(QWidget *parent)
    : QDialog(parent), validator(new TrackCodeValidator()) {
  setWindowFlags(Qt::Window);
  setMinimumWidth(600);
  setMinimumHeight(600);

  auto splitter = new QSplitter(Qt::Vertical);
  QVBoxLayout *box = new QVBoxLayout;

  const char *help = R"(Input and output:
    float out = 0;
    float in(int channelIndex);

Useful variables:
    int INDEX -- montage row index
    int IN_COUNT -- input row count

Code common for all montage formulas:)";

  box->addWidget(new QLabel(help)); // TODO: Move these to help sub dialog.

  header = new QTextEdit(this);
  header->setPlainText(OpenDataFile::infoTable.getGlobalMontageHeader());
  box->addWidget(header);

  QWidget *widget = new QWidget;
  widget->setLayout(box);
  splitter->addWidget(widget);
  box = new QVBoxLayout;

  box->addWidget(new QLabel("Enter montage code here:", this));

  editor = new QTextEdit(this);
  box->addWidget(editor);

  auto buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  auto button = new QPushButton("Validate", this);
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);
  box->addWidget(buttonBox);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(button, SIGNAL(clicked(bool)), this, SLOT(validate()));

  widget = new QWidget;
  widget->setLayout(box);
  splitter->addWidget(widget);

  setLayout(new QVBoxLayout);
  layout()->setMargin(0);
  layout()->addWidget(splitter);
}

CodeEditDialog::~CodeEditDialog() {}

QString CodeEditDialog::getText() const { return editor->toPlainText(); }

void CodeEditDialog::errorMessageDialog(const QString &message,
                                        QWidget *parent) {
  // TODO: Make a better error dialog.
  QMessageBox messageBox(parent);
  messageBox.setWindowTitle("Compilation Error");
  messageBox.setText("The code entered is not valid." + QString(200, ' '));
  messageBox.setDetailedText(message);
  messageBox.setIcon(QMessageBox::Critical);
  messageBox.exec();
}

void CodeEditDialog::setText(const QString &text) {
  editor->setPlainText(text);
}

void CodeEditDialog::done(int r) {
  if (QDialog::Accepted == r) {
    QString message;
    bool res = validator->validate(getText(), header->toPlainText(), &message);

    if (res) {
      OpenDataFile::infoTable.setGlobalMontageHeader(header->toPlainText());
      QDialog::done(r);
    } else {
      errorMessageDialog(message, this);
    }
  } else {
    QDialog::done(r);
  }
}

void CodeEditDialog::validate() {
  QString message;
  bool res = validator->validate(getText(), header->toPlainText(), &message);

  if (res) {
    QMessageBox::information(this, "Compilation Successful",
                             "Montage code compiled correctly");
  } else {
    errorMessageDialog(message, this);
  }
}
