#ifndef UI_CONTROL_TTTEXTBUTTON_P_H
#define UI_CONTROL_TTTEXTBUTTON_P_H

#include "ui/TtTheme.h"

namespace Ui {

class TtTextButton;

class TtTextButtonPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtTextButton)
  Q_PROPERTY_CREATE_D(int, BorderRadius)
  Q_PROPERTY_CREATE_D(QColor, LightDefaultColor)
  Q_PROPERTY_CREATE_D(QColor, DarkDefaultColor)
  Q_PROPERTY_CREATE_D(QColor, LightHoverColor)
  Q_PROPERTY_CREATE_D(QColor, DarkHoverColor)
  Q_PROPERTY_CREATE_D(QColor, LightPressColor)
  Q_PROPERTY_CREATE_D(QColor, DarkPressColor)

 public:
  explicit TtTextButtonPrivate(QObject* parent = nullptr);
  ~TtTextButtonPrivate();
  Q_SLOT void onThemeChanged(TtThemeType::ThemeMode themeMode);

 private:
  QColor _lightTextColor;
  QColor _darkTextColor;
  bool _isPressed{false};
  int _shadowBorderWidth{3};
  TtThemeType::ThemeMode _themeMode;
};

}  // namespace Ui
#endif  // UI_CONTROL_TTTEXTBUTTON_P_H
