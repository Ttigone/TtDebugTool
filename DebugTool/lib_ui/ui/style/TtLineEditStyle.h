#ifndef TTLINEEDITSTYLE_H
#define TTLINEEDITSTYLE_H

#include <QProxyStyle>

#include "ui/Def.h"

namespace style {

class TtLineEditStyle : public QProxyStyle {
  Q_OBJECT
 public:
  explicit TtLineEditStyle(QStyle* style = nullptr);
  ~TtLineEditStyle();
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget = nullptr) const override;

 private:
  TtThemeType::ThemeMode theme_mode_;
};

}  // namespace style

#endif  // TTLINEEDITSTYLE_H
