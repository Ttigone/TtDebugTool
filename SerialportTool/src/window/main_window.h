#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QObject>

#include "ui/window/window_switcher.h"

#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>

namespace QWK {
class WidgetWindowAgent;
} // namespace QWK

namespace Ui {
class TextWidget;
class CustomTabWidget;
class PopWidget;
class TtSvgButton;
} // namespace Ui

namespace Core {
class SerialPort;
} // namespace Core

class QtMaterialSnackbar;

namespace Widget {
class SerialSetting;
class SerialOperation;
} // namespace Widght

namespace Window {

class FunctionSelectionWindow;

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);
  virtual ~MainWindow();

  enum Theme {
    Dark,
    Light,
  };
  Q_ENUM(Theme)

 public Q_SLOTS:
    void showSnackbar();

Q_SIGNALS:
    void themeChanged();

protected:
    bool event(QEvent *event) override;

private:
    void installWindowAgent();
    void loadStyleSheet(Theme theme);
    void setLeftBar();
    void connectSignals();
    void registerTabWidget();

    // void createDockWindows();

    Theme currentTheme{};

    QWidget* central_widget_;

    QPointer<QWK::WidgetWindowAgent> window_agent_;


    Ui::HorizontalLayout* layout_;

    // 侧边弹出的菜单栏, 设置串口参数, 其他事项, 按钮组
    QWidget* left_bar_;

    Ui::TtSvgButton* communication_connection;
    Ui::TtSvgButton* communication_instruction;
    Ui::TtSvgButton* realistic_simulation;

    // tab 页面
    Ui::TabManager* tabWidget_;

    Window::FunctionSelectionWindow* function_select_;

    // 弹出的 widget
    // 要赋值父对象, 这个父对象也就是 canves
    // 弹出的 widget 的左侧应该在 left_bar_ 的右侧对齐
    Ui::PopWidget* communication_connection_widget;
};

} // namespace Window

#endif // MAIN_WINDOW_H

