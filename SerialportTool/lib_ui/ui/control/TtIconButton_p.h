#ifndef UI_CONTROL_TTICONBUTTON_P_H
#define UI_CONTROL_TTICONBUTTON_P_H

#include <QColor>
#include <QObject>
#include <QPixmap>

#include "ui/Def.h"
#include "ui/ui_pch.h"

namespace Ui {

class TtIconButton;

class TtIconButtonPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtIconButton)

  // 定义属性成员
  Q_PROPERTY_CREATE_D(int, BorderRadius)
  Q_PROPERTY_CREATE_D(qreal, Opacity);
  Q_PROPERTY_CREATE_D(TtIconType::IconName, Awesome)
  Q_PROPERTY_CREATE_D(QColor, LightHoverColor);
  Q_PROPERTY_CREATE_D(QColor, DarkHoverColor);
  Q_PROPERTY_CREATE_D(QColor, LightIconColor);
  Q_PROPERTY_CREATE_D(QColor, DarkIconColor);
  Q_PROPERTY_CREATE_D(QColor, LightHoverIconColor);
  Q_PROPERTY_CREATE_D(QColor, DarkHoverIconColor);
  Q_PROPERTY_CREATE_D(bool, IsSelected);
  Q_PROPERTY_CREATE(int, HoverAlpha)

 public:
  explicit TtIconButtonPrivate(QObject* parent = nullptr);
  ~TtIconButtonPrivate();

 private:
  QPixmap iconPix_;
  bool isAlphaAnimationFinished_{true};
  TtThemeType::ThemeMode _themeMode;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTICONBUTTON_P_H
