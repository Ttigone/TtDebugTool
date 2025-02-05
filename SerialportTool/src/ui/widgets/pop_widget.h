#ifndef UI_POP_WIDGET_H
#define UI_POP_WIDGET_H

class QtMaterialDrawer;

namespace Ui {

class TtPopUpDrawer;

class PopWidget : public QWidget {
  Q_OBJECT
 public:
  explicit PopWidget(QWidget *parent = nullptr);
  ~PopWidget();

  void setWidget(QWidget *w);
  void openDrawer();
  void closeDrawer();
  void triggerSwitch();

signals:
  void isClosed();

protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void init();

  bool is_open_;
  // 弹出的窗口
  //QtMaterialDrawer *const drawer_;
  TtPopUpDrawer *const drawer_;

  // 弹出窗口的内容
  QVBoxLayout *drawer_layout_;
};

} // namespace Ui

#endif  // UI_POP_WIDGET_H
