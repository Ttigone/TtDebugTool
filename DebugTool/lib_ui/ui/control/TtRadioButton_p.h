#ifndef UI_CONTROL_TTRADIOBUTTON_P_H
#define UI_CONTROL_TTRADIOBUTTON_P_H

#include "ui/Def.h"

namespace Ui {

class TtRadioButton;

class TtRadioButtonPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtRadioButton)
 public:
  explicit TtRadioButtonPrivate(QObject* parent = nullptr);
  ~TtRadioButtonPrivate();

 public slots:
  void onThemeChanged(TtThemeType::ThemeMode themeMode);
};

}  // namespace Ui

#endif  // UI_CONTROL_TTRADIOBUTTON_P_H
