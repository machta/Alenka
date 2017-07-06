#ifndef MONTAGETEMPLATEDIALOG_H
#define MONTAGETEMPLATEDIALOG_H

#include <QDialog>
#include <QDir>
#include <QFileinfo>

#include <array>
#include <string>
#include <vector>

namespace AlenkaFile {
class AbstractMontageTable;
}
class QListWidget;
class QTextEdit;

struct TemplateFile {
  QString fileName;
  std::vector<std::array<QString, 2>> rows;
};

class MontageTemplateDialog : public QDialog {
  Q_OBJECT

  AlenkaFile::AbstractMontageTable *montageTable;
  QDir dir;
  std::vector<TemplateFile> templates;
  QListWidget *list;
  QTextEdit *textEdit;

public:
  explicit MontageTemplateDialog(AlenkaFile::AbstractMontageTable *montageTable,
                                 QWidget *parent = nullptr);

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
