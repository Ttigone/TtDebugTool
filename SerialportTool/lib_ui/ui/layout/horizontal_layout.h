#ifndef UI_LAYOUT_HORIZONTAL_LAYOUT_H
#define UI_LAYOUT_HORIZONTAL_LAYOUT_H

#include <QHBoxLayout>

namespace Ui {

class HorizontalLayout : public QHBoxLayout {
 public:
  HorizontalLayout();
 private:
  void init();
};

} // namespace Ui

#endif  // UI_LAYOUT_HORIZONTAL_LAYOUT_H
