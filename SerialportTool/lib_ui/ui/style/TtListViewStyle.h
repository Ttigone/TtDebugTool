#ifndef UI_STYLE_TTLISTVIEWSTYLE_H
#define UI_STYLE_TTLISTVIEWSTYLE_H

#include <QProxyStyle>

#include "ui/Def.h"

namespace style {

class TtListViewStyle : public QProxyStyle {
  Q_OBJECT
  Q_PROPERTY_CREATE(int, ItemHeight)
  Q_PROPERTY_CREATE(bool, IsTransparent)
 public:
  explicit TtListViewStyle(QStyle* style = nullptr);
  ~TtListViewStyle();
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget = nullptr) const override;
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter,
                   const QWidget* widget = nullptr) const override;
  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size,
                         const QWidget* widget) const override;

 private:
  TtThemeType::ThemeMode theme_mode_;
  int left_padding_{11};
};

}  // namespace style

#endif  // UI_STYLE_TTLISTVIEWSTYLE_H
