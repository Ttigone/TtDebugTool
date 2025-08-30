#ifndef WINDOW_MAIN_WINDOW_H
#define WINDOW_MAIN_WINDOW_H

#include <QFileDialog>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>

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
// class TabManager;
class TabWindow;
class SettingWidget;

class TtHorizontalLayout;
class TtVerticalLayout;
class TabWindow;
class WidgetGroup;

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
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  enum Theme {
    Dark,
    Light,
  };
  Q_ENUM(Theme)

  void initLanguageMenu();

 signals:
  void themeChanged();

 private slots:
  void closeWindow();
  void compileTsFilesFinished();
  void saveCsvFile();
  void switchToOtherTabPage(const QString &uuid, const int &type);
  void addSelectToolPage();
  void addSelectAllToolPage();

 protected:
  bool event(QEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

 private:
  void installWindowAgent();
  void loadStyleSheet(Theme theme);
  void setLeftBar();
  void connectSignals();
  void registerTabWidget();
  void addDifferentConfiguration(TtFunctionalCategory::Category type,
                                 TtProtocolRole::Role role,
                                 const QString &title, const QString &uuid);

  QString extractLanguageName(const QString &qmFile);  // 解析语言后缀名
  void changeLanguage(const QString &qmFile);          // 切换语言

  void saveLanguageSetting(const QString &language);  // 语言选项保存至配置文件
  void restartApplication();                          // 重启应用

  void readingProjectConfiguration();  // 读取全部配置
  ///
  /// @brief getSpecificConfiguration
  /// @param index
  /// @param role
  /// @return
  /// 根据 index 获取单个 config
  QJsonObject getSpecificConfiguration(const QString index,
                                       TtProtocolRole::Role role);

  ///
  /// @brief processConfigsByType
  /// @param configs
  /// @param protocolRole
  /// 处理不同配置类型
  void processConfigsByType(const QHash<QString, QJsonObject> &configs,
                            TtProtocolRole::Role protocolRole);

  Theme currentTheme{};

  QWidget *central_widget_;
  QPointer<QWK::WidgetWindowAgent> window_agent_;
  Ui::TtHorizontalLayout *layout_;

  // 侧边弹出的菜单栏, 设置串口参数, 其他事项, 按钮组
  QWidget *left_bar_;
  Ui::TtWidgetGroup *left_bar_logic_;

  Ui::SessionManager *history_link_list_;
  Ui::SessionManager *history_instruction_list_;
  Ui::SessionManager *history_mock_list_;

  QVector<QVector<QWidget *>> stacked_;

  Ui::TtSvgButton *communication_connection_;
  Ui::TtSvgButton *communication_instruction_;
  Ui::TtSvgButton *realistic_simulation_;
  Ui::TtSvgButton *setting_;

  Ui::SettingWidget *setting_widget_ = nullptr;

  // tab 页面
  Ui::TabWindow *tabWidget_;

  Window::FunctionSelectionWindow *function_select_;

  // 弹出的 widget
  // 要赋值父对象, 这个父对象也就是 canves
  // 弹出的 widget 的左侧应该在 left_bar_ 的右侧对齐
  Ui::PopWidget *communication_connection_widget;
  Ui::WidgetGroup *buttonGroup;
  QString saved_language_;
};

}  // namespace Window

#endif  // WINDOW_MAIN_WINDOW_H
