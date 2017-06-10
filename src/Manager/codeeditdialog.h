#ifndef CODEEDITDIALOG_H
#define CODEEDITDIALOG_H

#include <QDialog>

#include <memory>

class QTextEdit;
class TrackCodeValidator;

/**
 * @brief Implements a dialog for entering more detailed montage track code.
 */
class CodeEditDialog : public QDialog {
public:
  CodeEditDialog(QWidget *parent);

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
  void done(int r) override;

private:
  QTextEdit *editor;
  std::unique_ptr<TrackCodeValidator> validator;
};

#endif // CODEEDITDIALOG_H
