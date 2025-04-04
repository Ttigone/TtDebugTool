#ifndef WINDOW_MAIN_WINDOW_H
#define WINDOW_MAIN_WINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QListWidget>

#include <ui/control/buttonbox/TtButtonBox.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>

#include "Def.h"

namespace QWK {
class WidgetWindowAgent;
}  // namespace QWK

namespace Ui {
class TextWidget;
class CustomTabWidget;
class PopWidget;
class TtSvgButton;
class TtWidgetGroup;
class SessionManager;
class TabManager;
class SettingWidget;
}  // namespace Ui

namespace Core {
class SerialPortWorker;
}  // namespace Core

class QtMaterialSnackbar;

namespace Widget {
// class SettingWidget;
}  // namespace Widget

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

  void initLanguageMenu();

 signals:
  void themeChanged();

 private slots:
  void compileTsFilesFinished();

 protected:
  bool event(QEvent* event) override;

 private:
  void installWindowAgent();
  void loadStyleSheet(Theme theme);
  void setLeftBar();
  void connectSignals();
  void registerTabWidget();
  void addDifferentConfiguration(TtFunctionalCategory::Category type,
                                 const QString& title, const QString& uuid);

  QString extractLanguageName(const QString& qmFile);
  void changeLanguage(const QString& qmFile);

  void saveLanguageSetting(const QString& language);
  void restartApplication();

  // void createDockWindows();

  Theme currentTheme{};

  QWidget* central_widget_;
  QPointer<QWK::WidgetWindowAgent> window_agent_;
  Ui::TtHorizontalLayout* layout_;

  // 侧边弹出的菜单栏, 设置串口参数, 其他事项, 按钮组
  QWidget* left_bar_;
  Ui::TtWidgetGroup *left_bar_logic_;
  //bool ishi{true};

  Ui::SessionManager* history_link_list_;
  Ui::SessionManager* history_instruction_list_;
  Ui::SessionManager* history_mock_list_;

  QVector<QVector<QWidget*>> stacked_;

  Ui::TtSvgButton* communication_connection_;
  Ui::TtSvgButton* communication_instruction_;
  Ui::TtSvgButton* realistic_simulation_;
  Ui::TtSvgButton* setting_;

  Ui::SettingWidget* setting_widget_ = nullptr;

  // tab 页面
  Ui::TabManager* tabWidget_;

  Window::FunctionSelectionWindow* function_select_;

  // 弹出的 widget
  // 要赋值父对象, 这个父对象也就是 canves
  // 弹出的 widget 的左侧应该在 left_bar_ 的右侧对齐
  Ui::PopWidget* communication_connection_widget;

  Ui::WidgetGroup* buttonGroup;

  QTranslator* translator_ = nullptr;

  QString savedLanguage_;
};

}  // namespace Window

#endif  // WINDOW_MAIN_WINDOW_H
