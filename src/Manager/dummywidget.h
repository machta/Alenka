#ifndef DUMMMYWIDGET_H
#define DUMMMYWIDGET_H

#include <QWidget>

#include <functional>

/**
 * @brief An auxiliary object that does and displays nothing; it calls a
 * function when it is shown for the first time.
 */
class DummyWidget : public QWidget {
  Q_OBJECT

  std::function<void(void)> fun;
  bool notYetTriggered = true;

public:
  DummyWidget(std::function<void(void)> fun, QWidget *parent = nullptr)
      : QWidget(parent), fun(std::move(fun)) {}

protected:
  void showEvent(QShowEvent * /*event*/) override {
    if (notYetTriggered)
      fun();
    notYetTriggered = false;
  }
};

#endif // DUMMMYWIDGET_H
