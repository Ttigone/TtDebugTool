/*****************************************************************//**
 * \file   popup_menu.h
 * \brief  弹出菜单
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_POPUP_MENU_H
#define UI_WIDGETS_POPUP_MENU_H

#include "ui/mb_widget.h"


namespace Ui {
class PopupMenu : public MbWidget {
public:
  enum class VertivalOrigin {
    Top,
    Bottom,
  };
  enum class AnimatePhase {
    Hidden,
    StartShow,
    Shown,
    StartHide,
  };
  PopupMenu(QWidget *parent);
  PopupMenu(QWidget *parent, QMenu *menu);
  ~PopupMenu();
};
} // namespace Ui

#endif // UI_WIDGETS_POPUP_MENU_H

