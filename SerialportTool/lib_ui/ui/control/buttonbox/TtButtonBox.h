#ifndef UI_CONTROL_TTBUTTONBOX_H
#define UI_CONTROL_TTBUTTONBOX_H


namespace Ui {

class TtButtonGroup : public QWidget {
  Q_OBJECT

 public:
  explicit TtButtonGroup(QWidget* parent = nullptr);

 signals:
  void firstButtonClicked();
  void secondButtonClicked();

 private slots:
  void buttonClicked(QAbstractButton* button);
  void animateButton(QPushButton* button, qreal scale);

 private:
  void setupUI();
  void initConnections();

  QPushButton* button1;
  QPushButton* button2;
  QButtonGroup* buttonGroup;

  // Animation instances
  QSequentialAnimationGroup* animation1;
  QSequentialAnimationGroup* animation2;

  // Initial sizes
  QSize button1InitialSize;
  QSize button2InitialSize;

  // Stylesheet
  static const QString styleSheet;
};


// 按钮组 管理一群按钮的点击事件和触发事件, 配对
class TtButtonBox {
 public:
  TtButtonBox();
};

} // namespace Ui

#endif  // UI_CONTROL_BUTTONBOX_H
