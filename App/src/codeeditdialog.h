#ifndef CODEEDITDIALOG_H
#define CODEEDITDIALOG_H

#include <QDialog>
#include <QValidator>

class OpenCLContext;
class Validator;
class QTextEdit;

class CodeEditDialog : public QDialog
{
public:
	CodeEditDialog(QWidget* parent);
	~CodeEditDialog();

	QString getText() const;

	class Validator
	{
	public:
		Validator();
		~Validator();

		bool validate(const QString& input, QString* errorMessage);

		static void errorMessageDialog(const QString& message, QWidget* parent = nullptr);

	private:
		OpenCLContext* context;
	};

public slots:
	void setText(const QString& text);
	virtual void done(int r) override;

private:
	QTextEdit* editor;
	Validator validator;
};

#endif // CODEEDITDIALOG_H
