#ifndef UI_CONTROL_TTPOPUPDRAWER_P_H
#define UI_CONTROL_TTPOPUPDRAWER_P_H

#include "ui/Def.h"

namespace Ui {

class TtPopUpDrawer;
class TtPopUpDrawerWidget;
class TtPopUpDrawerStateMachine;

class TtPopUpDrawerPrivate {
  Q_DISABLE_COPY(TtPopUpDrawerPrivate)
  Q_DECLARE_PUBLIC(TtPopUpDrawer)

 public:
  TtPopUpDrawerPrivate(TtPopUpDrawer* q);
  ~TtPopUpDrawerPrivate();

  void init();
  void setClosed(bool value = true);

  TtPopUpDrawer* const q_ptr;  // public 指针
  TtPopUpDrawerWidget* widget_;
  TtPopUpDrawerStateMachine* state_machine_;  // 状态机
  QWidget* window_;
  TtPopUpDirection::PopUpDirection direction_;
  // int width_;
  int size_;
  bool click_to_close_;
  bool auto_raise_;
  bool closed_;
  bool overlay_;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTPOPUPDRAWER_P_H
