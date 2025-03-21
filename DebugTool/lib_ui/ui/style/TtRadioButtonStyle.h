#ifndef UI_STYLE_TTRADIOBUTTONSTYLE_H
#define UI_STYLE_TTRADIOBUTTONSTYLE_H

#include <QProxyStyle>

#include "ui/TtTheme.h"

namespace style {

class TtRadioButtonStyle : public QProxyStyle {
  Q_OBJECT
 public:
  TtRadioButtonStyle(QStyle* style = nullptr);
  ~TtRadioButtonStyle();
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget = nullptr) const override;
  int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr) const override;

 private:
  TtThemeType::ThemeMode theme_mode_;
};

}  // namespace style

#endif  // UI_STYLE_TTRADIOBUTTONSTYLE_H
