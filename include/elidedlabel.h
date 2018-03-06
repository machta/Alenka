#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>

#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

/**
 * @brief An automatically resizable label that allows hiding of excessively
 * long text.
 *
 * By default the tool tip displays the full text.
 *
 * @attention Use setMinimumWidth() to set the lower bound on width of the
 * label. Without this it will behave like a regular QLabel.
 *
 * Adapted from [wiki.qt.io/Elided_Label](https://wiki.qt.io/Elided_Label).
 */
class ElidedLabel : public QLabel {
  Q_OBJECT

  Qt::TextElideMode elideMode_ = Qt::ElideRight;
  bool autoToolTip_ = true;
  QString cachedElidedText;

public:
  ElidedLabel(QWidget *parent = nullptr, Qt::WindowFlags f = 0)
      : QLabel(parent, f) {}
  ElidedLabel(const QString &txt, QWidget *parent = nullptr,
              Qt::WindowFlags f = 0)
      : QLabel(txt, parent, f) {}
  ElidedLabel(const QString &txt, Qt::TextElideMode elideMode = Qt::ElideRight,
              QWidget *parent = nullptr, Qt::WindowFlags f = 0)
      : QLabel(txt, parent, f), elideMode_(elideMode) {}

  //! Set the elide mode used for displaying text.
  void setElideMode(Qt::TextElideMode elideMode) {
    elideMode_ = elideMode;
    updateGeometry();
  }
  //! Get the elide mode currently used to display text.
  Qt::TextElideMode elideMode() const { return elideMode_; }

  //! Whether to set the tool tip automatically when setting text.
  void setAutoToolTip(bool autoToolTip) { autoToolTip_ = autoToolTip; }

  //! Shadow QLabel::setText().
  void setText(const QString &txt) {
    QLabel::setText(txt);
    if (autoToolTip_)
      setToolTip(txt);
    cacheElidedText(geometry().width());
  }

protected:
  void resizeEvent(QResizeEvent *e) override {
    QLabel::resizeEvent(e);
    cacheElidedText(e->size().width());
  }
  void paintEvent(QPaintEvent *e) override {
    if (elideMode_ == Qt::ElideNone) {
      QLabel::paintEvent(e);
    } else {
      QPainter p(this);
      p.drawText(0, 0, geometry().width(), geometry().height(), alignment(),
                 cachedElidedText);
    }
  }

private:
  //! Cache the elided text so as to not recompute it every paint event.
  void cacheElidedText(int w) {
    cachedElidedText =
        fontMetrics().elidedText(text(), elideMode_, w, Qt::TextShowMnemonic);
  }
};

#endif // ELIDEDLABEL_H
