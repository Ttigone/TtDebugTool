#ifndef UI_CONTROL_TTICONBUTTON_H
#define UI_CONTROL_TTICONBUTTON_H

#include <QPushButton>

#include "ui/Def.h"
#include "ui/ui_pch.h"

namespace Ui {

class TtIconButtonPrivate;

class Tt_EXPORT TtIconButton : public QPushButton {
  Q_OBJECT
  Q_Q_CREATE(TtIconButton);

  // 创建属性成员, 但不提供实现
  Q_PROPERTY_CREATE_Q_H(int, BorderRadius)
  Q_PROPERTY_CREATE_Q_H(qreal, Opacity);
  Q_PROPERTY_CREATE_Q_H(QColor, LightHoverColor);
  Q_PROPERTY_CREATE_Q_H(QColor, DarkHoverColor);
  Q_PROPERTY_CREATE_Q_H(QColor, LightIconColor);
  Q_PROPERTY_CREATE_Q_H(QColor, DarkIconColor);
  Q_PROPERTY_CREATE_Q_H(QColor, LightHoverIconColor);
  Q_PROPERTY_CREATE_Q_H(QColor, DarkHoverIconColor);
  Q_PROPERTY_CREATE_Q_H(bool, IsSelected);

 public:
  TtIconButton(QPixmap pix, QWidget* parent = nullptr);
  TtIconButton(TtIconType::IconName awesome, QWidget* parent = nullptr);
  TtIconButton(TtIconType::IconName awesome, int pixelSize,
               QWidget* parent = nullptr);
  TtIconButton(TtIconType::IconName awesome, int pixelSize, int fixedWidth,
               int fixedHeight, QWidget* parent = nullptr);
  ~TtIconButton();
  void setAwesome(TtIconType::IconName awesome);
  TtIconType::IconName getAwesome() const;

  void setPixmap(QPixmap pix);

 protected:
  bool event(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTICONBUTTON_H
