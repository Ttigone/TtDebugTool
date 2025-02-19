#ifndef UI_LAYOUT_HORIZONTAL_LAYOUT_H
#define UI_LAYOUT_HORIZONTAL_LAYOUT_H

#include <QHBoxLayout>

namespace Ui {

class TtHorizontalLayout : public QHBoxLayout {
 public:
  TtHorizontalLayout();
  TtHorizontalLayout(QWidget* widget);
  ~TtHorizontalLayout();

 private:
  void init();

  Q_DISABLE_COPY(TtHorizontalLayout)
};

}  // namespace Ui

#endif  // UI_LAYOUT_HORIZONTAL_LAYOUT_H
