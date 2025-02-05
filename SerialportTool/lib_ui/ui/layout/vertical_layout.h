#ifndef UI_LAYOUT_VERTICAL_LAYOUT_H
#define UI_LAYOUT_VERTICAL_LAYOUT_H

namespace Ui {

class VerticalLayout : public QVBoxLayout {
 public:
  VerticalLayout();
  VerticalLayout(QWidget* widget);
  ~VerticalLayout();

 private:
  void init();

};

} // namespace Ui

#endif // UI_LAYOUT_VERTICAL_LAYOUT_H
