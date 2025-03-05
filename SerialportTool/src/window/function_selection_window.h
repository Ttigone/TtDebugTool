#ifndef WINDOW_FUNCTION_SELECTION_WINDOW_H
#define WINDOW_FUNCTION_SELECTION_WINDOW_H

#include <QWidget>

#include "ui/widgets/window_switcher.h"

namespace Ui {
class CommonButton;
class TtWordsButton;
class TtNormalLabel;
class TtElidedLabel;
} // namespace Ui

namespace Window {

class FunctionSelectionWindow : public QWidget {
  // class FunctionSelectionWindow : public Ui::CustomTabPage {
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
  Ui::TtElidedLabel* label_;

  // 功能窗口布局
  QGridLayout *function_layout_;
};

class SimulateFunctionSelectionWindow : public QWidget {
  Q_OBJECT
 public:
  explicit SimulateFunctionSelectionWindow(QWidget* parent = nullptr);
  ~SimulateFunctionSelectionWindow();

 signals:
  void toTcp();
  void toUdp();
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
  Ui::TtElidedLabel* label_;

  // 功能窗口布局
  QGridLayout* function_layout_;
};

} // namespace Window

#endif  // FUNCTION_SELECTION_WINDOW_H
