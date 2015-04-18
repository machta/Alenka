#ifndef CODEEDITDIALOG_H
#define CODEEDITDIALOG_H

#include <QDialog>

#include "../DataFile/trackcodevalidator.h"

class OpenCLContext;
class QTextEdit;

class CodeEditDialog : public QDialog
{
public:
	CodeEditDialog(QWidget* parent);
	~CodeEditDialog();

	QString getText() const;

	static void errorMessageDialog(const QString& message, QWidget* parent = nullptr);

public slots:
	void setText(const QString& text);
	virtual void done(int r) override;

private:
	QTextEdit* editor;
	TrackCodeValidator validator;
};

#endif // CODEEDITDIALOG_H
