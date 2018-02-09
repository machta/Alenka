#ifndef MONTAGETEMPLATEDIALOG_H
#define MONTAGETEMPLATEDIALOG_H

#include <QDialog>
#include <QDir>

#include <array>
#include <string>
#include <vector>

class OpenDataFile;
class QListWidget;
class QTextEdit;
class QFileInfo;

struct TemplateFile {
  QString fileName;
  std::vector<std::array<QString, 2>> rows;
};

/**
 * @brief This class implements the dialog window that manages montage
 * templates.
 */
class MontageTemplateDialog : public QDialog {
  Q_OBJECT

  OpenDataFile *file;
  QDir dir;
  std::vector<TemplateFile> templates;
  QListWidget *list;
  QTextEdit *textEdit;

public:
  explicit MontageTemplateDialog(OpenDataFile *file, QWidget *parent = nullptr);

private:
  void readTemplates();
  void populateList();
  bool getUniqueFileName(QString newName, QFileInfo *fileInfo);

private slots:
  void addMontage();
  void deleteTemplate();
  void renameTemplate();
  void saveCurrentMontage();
  void showTemplateText(int index);
};

#endif // MONTAGETEMPLATEDIALOG_H
