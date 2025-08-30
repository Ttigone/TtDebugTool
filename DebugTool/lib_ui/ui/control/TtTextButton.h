#ifndef UI_CONTROL_TTTEXTBUTTON_H
#define UI_CONTROL_TTTEXTBUTTON_H

#include <QPushButton>

#include "ui/Def.h"

namespace Ui {

class TtTextButtonPrivate;

class Tt_EXPORT TtTextButton : public QPushButton {
  Q_OBJECT
  Q_Q_CREATE(TtTextButton)
  Q_PROPERTY_CREATE_Q_H(int, BorderRadius)
  Q_PROPERTY_CREATE_Q_H(QColor, LightDefaultColor)
  Q_PROPERTY_CREATE_Q_H(QColor, DarkDefaultColor)
  Q_PROPERTY_CREATE_Q_H(QColor, LightHoverColor)
  Q_PROPERTY_CREATE_Q_H(QColor, DarkHoverColor)
  Q_PROPERTY_CREATE_Q_H(QColor, LightPressColor)
  Q_PROPERTY_CREATE_Q_H(QColor, DarkPressColor)

  Q_PROPERTY(QColor checkedColor READ checkedColor WRITE setCheckedColor)
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
  Q_PROPERTY(
      QColor lightTextColor READ getLightTextColor WRITE setLightTextColor)
  Q_PROPERTY(QColor darkTextColor READ getDarkTextColor WRITE setDarkTextColor)

 public:
  explicit TtTextButton(QWidget* parent = nullptr);
  explicit TtTextButton(const QString& text, QWidget* parent = nullptr);
  explicit TtTextButton(const QColor& color, const QString& text,
                        QWidget* parent = nullptr);
  ~TtTextButton();

  void setChecked(bool checked);
  bool isChecked() const { return is_checked_; }

  void setCheckedColor(const QColor& color);
  QColor checkedColor() const { return checked_color_; }

  void setLightTextColor(const QColor& color);
  QColor getLightTextColor() const;

  void setDarkTextColor(const QColor& color);
  QColor getDarkTextColor() const;

 signals:
  void toggled(bool checked);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

 private:
  bool is_checked_ = false;
  QColor checked_color_ = Qt::blue;  // 默认选中颜色
  QColor default_color_;             // 初始文字颜色
};

}  // namespace Ui

#endif  // UI_CONTROL_TTTEXTBUTTON_H
