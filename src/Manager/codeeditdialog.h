#ifndef CODEEDITDIALOG_H
#define CODEEDITDIALOG_H

#include <QDialog>

class QTextEdit;
class TrackCodeValidator;

/**
 * @brief Implements a dialog for entering more detailed montage track code.
 */
class CodeEditDialog : public QDialog {
public:
  CodeEditDialog(QWidget *parent);
  ~CodeEditDialog();

  QString getText() const;

  /**
   * @brief Shows a message dialog with the error message.
   */
  static void errorMessageDialog(const QString &message,
                                 QWidget *parent = nullptr);

public slots:
  void setText(const QString &text);

  /**
   * @brief This method gets called when the user closes the dialog.
   */
  virtual void done(int r) override;

private:
  QTextEdit *editor;
  TrackCodeValidator *validator;
};

#endif // CODEEDITDIALOG_H
