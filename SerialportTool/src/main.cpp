#include <QApplication>
#include <QTextCodec>

#include "storage/setting_manager.h"
#include "window/main_window.h"

// 编译器版本为 6.5.3 时, 调整宽度和高度时, 控件会改变布局, qwindowkit bug
// 编译器 < 6.5.3 or 编译器 > 6.6.2

// 异常捕获函数
LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException) {

  EXCEPTION_RECORD* record = pException->ExceptionRecord;

  //QString errCode(QString::number(record->ExceptionCode, 16)),
  //    errAdr(QString::number((uint)record->ExceptionAddress, 16)), errMod;
  //QString crashMsg =
  //    QString("抱歉，软件发生了崩溃，请重启。错误代码：%1，错误地址：%2")
  //        .arg(errCode)
  //        .arg(errAdr);

  return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  QCoreApplication::setApplicationName(
      QStringLiteral("SerialportTool-C3H3_Ttigone"));
  QCoreApplication::setOrganizationName(QStringLiteral("WWB-Qt"));

  // 设置全局字体
  QFont font("Microsoft YaHei", 10);  // 微软雅黑，10号字体
  QApplication::setFont(font);

  //// 设置全局调色板
  QPalette palette;
  palette.setColor(QPalette::Text , Qt::black);   // 按钮文字颜色
  //palette.setColor(QPalette::WindowText, Qt::blue);  // 窗口文字颜色
  //palette.setColor(QPalette::Button, Qt::yellow);    // 按钮颜色
  //palette.setColor(QPalette::ButtonText, Qt::red);   // 按钮文字颜色
  QApplication::setPalette(palette);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  // 适用精确缩放
  // bug, 导致 QComboBox 不能正常工作, 同时字体发虚
  // 以下的作用是取消分辨率
  //QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
  //    Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

#if defined(Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  //const auto osName = QSysInfo::prettyProductName();
  //if (osName.startsWith("Windows 10") || osName.startsWith("Windows 11")) {
  //  // 风格
  //  //QApplication::setStyle("fusion");
  //}
#endif
  // Use Fusion style on Windows 10 & 11. This enables proper dark mode support.
  // See https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5.
  // TODO: Make style configurable, detect -style argument.
#if defined(Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
  const auto osName = QSysInfo::prettyProductName();
  if (osName.startsWith("Windows 10") || osName.startsWith("Windows 11")) {
    QApplication::setStyle("fusion");
  }
#endif

  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

  QString filePath = "serial_settings.json";
  Storage::SettingsManager& settingsManager =
      Storage::SettingsManager::instance();
  settingsManager.setTargetStoreFile(filePath);
  settingsManager.saveSettings();

  //注冊异常捕获函数
  //SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

  Window::MainWindow w;
  w.show();

  return a.exec();
}
