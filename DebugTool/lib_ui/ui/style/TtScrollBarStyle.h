#ifndef TTSCROLLBARSTYLE_H
#define TTSCROLLBARSTYLE_H

#include <QProxyStyle>

#include "ui/Def.h"

namespace Ui {
class TtScrollBar;
}  // namespace Ui

namespace style {

class TtScrollBarStyle : public QProxyStyle {
  Q_OBJECT
  Q_PRIVATE_CREATE(bool, IsExpand)
  Q_PROPERTY_CREATE(qreal, Opacity)
  Q_PROPERTY_CREATE(qreal, SliderExtent)
  Q_PRIVATE_CREATE(Ui::TtScrollBar*, ScrollBar)
 public:
  explicit TtScrollBarStyle(QStyle* style = nullptr);
  ~TtScrollBarStyle();
  void drawComplexControl(ComplexControl control,
                          const QStyleOptionComplex* option, QPainter* painter,
                          const QWidget* widget = nullptr) const override;
  int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr) const override;
  int styleHint(StyleHint hint, const QStyleOption* option = nullptr,
                const QWidget* widget = nullptr,
                QStyleHintReturn* returnData = nullptr) const override;
  void startExpandAnimation(bool isExpand);

 private:
  TtThemeType::ThemeMode theme_mode_;
  qreal slider_margin_{2.5};
  int scrollBar_extent_{10};
};

}  // namespace style

#endif  // TTSCROLLBARSTYLE_H
