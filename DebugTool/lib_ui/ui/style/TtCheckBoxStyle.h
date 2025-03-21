#ifndef UI_STYLE_TTCHECKBOXSTYLE_H
#define UI_STYLE_TTCHECKBOXSTYLE_H

#include <QProxyStyle>

#include "ui/Def.h"

namespace style {

class TtCheckBoxStyle : public QProxyStyle {
  Q_OBJECT
  Q_PRIVATE_CREATE(int, CheckIndicatorWidth)
 public:
  explicit TtCheckBoxStyle(QStyle* style = nullptr);
  ~TtCheckBoxStyle();
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter,
                   const QWidget* widget = nullptr) const override;
  int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr) const override;
  QRect subElementRect(SubElement element, const QStyleOption* option,
                       const QWidget* widget = nullptr) const override;

 private:
  TtThemeType::ThemeMode theme_mode_;
};

}  // namespace style

#endif  // UI_STYLE_TTCHECKBOXSTYLE_H
