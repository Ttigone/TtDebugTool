#ifndef UI_STYLE_TTCOMBOBOX_H
#define UI_STYLE_TTCOMBOBOX_H

#include <QProxyStyle>

#include "ui/Def.h"

namespace style {

class TtComboBoxStyle : public QProxyStyle {
  Q_OBJECT
  Q_PROPERTY_CREATE(qreal, ExpandIconRotate)
  Q_PROPERTY_CREATE(qreal, ExpandMarkWidth)
 public:
  TtComboBoxStyle(QStyle* style = nullptr);
  ~TtComboBoxStyle();
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget = nullptr) const override;
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter,
                   const QWidget* widget = nullptr) const override;
  void drawComplexControl(ComplexControl control,
                          const QStyleOptionComplex* option, QPainter* painter,
                          const QWidget* widget = nullptr) const override;

  QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt,
                       SubControl sc, const QWidget* widget) const override;
  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size,
                         const QWidget* widget) const override;

 private:
  TtThemeType::ThemeMode theme_mode_;
  int shadow_border_width_ = 6;
  int combo_margin_ = 8;
};

}  // namespace style

#endif  // UI_STYLE_TTCOMBOBOX_H
