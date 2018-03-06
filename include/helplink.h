#ifndef HELPLINK_H
#define HELPLINK_H

#include <QPushButton>

#include <QDesktopServices>
#include <QStyle>

class HelpLink : public QPushButton {
public:
  HelpLink(const QString &link, const QString &title = "",
           QWidget *parent = nullptr)
      : QPushButton(parent) {
    setToolTip(title.isEmpty() ? link : title);
    setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));

    connect(this, &HelpLink::clicked,
            [link]() { QDesktopServices::openUrl(link); });
  }
};

#endif // HELPLINK_H
