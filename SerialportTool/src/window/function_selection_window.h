#ifndef WINDOW_FUNCTION_SELECTION_WINDOW_H
#define WINDOW_FUNCTION_SELECTION_WINDOW_H

#include <QWidget>

namespace Ui {
class CommonButton;
class TtWordsButton;
class TtNormalLabel;
} // namespace Ui

namespace Window {

class FunctionSelectionWindow : public QWidget {
  Q_OBJECT
 public:
  explicit FunctionSelectionWindow(QWidget* parent = nullptr);
  ~FunctionSelectionWindow();

 signals:
  void toSerial();
  void toTcp();
  void toUdp();
  void toBlueTeeth();
  void toModubus();
  void toMQTT();

 signals:
  ///
  /// @brief switchRequested
  /// @param targetWidgetId
  /// 切换到对应的 界面
  void switchRequested(int targetWidgetId);

 private:
  ///
  /// @brief init
  /// 初始化函数
  void init();

  // 标志
  Ui::TtNormalLabel* label_;

  Ui::TtWordsButton* serial_;  // 串口

  // 功能窗口布局
  QGridLayout *function_layout_;
};

} // namespace Window

#endif  // FUNCTION_SELECTION_WINDOW_H
