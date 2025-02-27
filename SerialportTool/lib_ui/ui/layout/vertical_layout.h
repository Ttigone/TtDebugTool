#ifndef UI_LAYOUT_VERTICAL_LAYOUT_H
#define UI_LAYOUT_VERTICAL_LAYOUT_H

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtVerticalLayout : public QVBoxLayout {
 public:
  TtVerticalLayout();
  TtVerticalLayout(QWidget* widget);
  ~TtVerticalLayout();

 private:
  void init();

  Q_DISABLE_COPY(TtVerticalLayout)
};

}  // namespace Ui

#endif  // UI_LAYOUT_VERTICAL_LAYOUT_H
