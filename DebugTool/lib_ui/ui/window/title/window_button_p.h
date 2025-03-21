#ifndef WINDOWBAR_BUTTON_P_H
#define WINDOWBAR_BUTTON_P_H

#include "ui/window/title/window_button.h"

namespace Ui {

class WindowbarButtonPrivate {
  Q_DECLARE_PUBLIC(WindowbarButton)

 public:
  WindowbarButtonPrivate();
  virtual ~WindowbarButtonPrivate();

  void init();

  WindowbarButton *q_ptr;

  QIcon iconNormal;
  QIcon iconChecked;
  QIcon iconDisabled;

  void reloadIcon();
};

}  // namespace Ui

#endif  // WINDOWBAR_BUTTON_P_H
