#ifndef UNDOCOMMANDFACTORY_H
#define UNDOCOMMANDFACTORY_H

#include "../../Alenka-File/include/AlenkaFile/abstractdatamodel.h"

#include <QString>

class QUndoStack;
class QUndoCommand;

class UndoCommandFactory {
  AlenkaFile::DataModel *dataModel;
  QUndoStack *undoStack;

public:
  UndoCommandFactory(AlenkaFile::DataModel *dataModel, QUndoStack *undoStack)
      : dataModel(dataModel), undoStack(undoStack) {}

  void push(QUndoCommand *cmd);
  void beginMacro(const QString &text);
  void endMacro();

  void overwriteDataModel(std::unique_ptr<AlenkaFile::DataModel> newDataModel,
                          const QString &text = "");

  void changeEventType(int i, const AlenkaFile::EventType &value,
                       const QString &text = "") const;
  void changeMontage(int i, const AlenkaFile::Montage &value,
                     const QString &text = "") const;
  void changeEvent(int i, int j, const AlenkaFile::Event &value,
                   const QString &text = "") const;
  void changeTrack(int i, int j, const AlenkaFile::Track &value,
                   const QString &text = "") const;

  void insertEventType(int i, int c = 1, const QString &text = "") const;
  void insertMontage(int i, int c = 1, const QString &text = "") const;
  void insertEvent(int i, int j, int c = 1, const QString &text = "") const;
  void insertTrack(int i, int j, int c = 1, const QString &text = "") const;

  void removeEventType(int i, int c = 1, const QString &text = "") const;
  void removeMontage(int i, int c = 1, const QString &text = "") const;
  void removeEvent(int i, int j, int c = 1, const QString &text = "") const;
  void removeTrack(int i, int j, int c = 1, const QString &text = "") const;

  static std::unique_ptr<AlenkaFile::DataModel> emptyDataModel();
};

#endif // UNDOCOMMANDFACTORY_H
