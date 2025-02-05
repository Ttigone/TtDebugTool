#ifndef WINDOWBAR_P_H
#define WINDOWBAR_P_H

#include "ui/window/title/windowbar.h"

QT_BEGIN_NAMESPACE
class QSpacerItem;
class QHBoxLayout;
QT_END_NAMESPACE

namespace Ui {

class WindowBarPrivate {
  Q_DECLARE_PUBLIC(WindowBar)

 public:
  WindowBarPrivate();
  virtual ~WindowBarPrivate();

  WindowBar *q_ptr;

  QWidget *w_ = nullptr;
  bool auto_title_ = true;
  bool auto_icon_ = false;
  QHBoxLayout *layout_;

  enum WindowBarItem : int {
    IconButton,
    MenuWidget,
    TitleLabel,
    MinimumButton,
    MaxinumButton,
    CloseButton,
  };

  void init();

  inline QWidget *widgetAt(int index) const;
  void setWidgetAt(int index, QWidget *widget);

  QWidget *takeWidgetAt(int index);

  inline void insertDefaultSpace(int index);

 private:
  Q_DISABLE_COPY(WindowBarPrivate)

};

} // namespace Ui

#endif  // WINDOWBAR_P_H
