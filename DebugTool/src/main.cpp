#include <base/glog_helper.h>

#include <QApplication>
#include <QFontDatabase>

#include "lang/translation_manager.h"
#include "storage/configs_manager.h"
#include "storage/setting_manager.h"
#include "window/main_window.h"

// 编译器版本为 6.5.3 时, 调整宽度和高度时, 控件会改变布局, qwindowkit bug
// 编译器 < 6.5.3 or 编译器 > 6.6.2

using namespace std::chrono_literals;

#if (!WIN32)
#include <Windows.h>
#include <dbghelp.h>

#include <QSettings>
#include <QTranslator>
// 异常捕获函数
// LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException) {

//   EXCEPTION_RECORD* record = pException->ExceptionRecord;
//   QString errCode(QString::number(record->ExceptionCode, 16));
//   // (uint*)record->ExceptionAddress, errMod;
//   QString crashMsg =
//       QString("抱歉，软件发生了崩溃，请重启。错误代码：%1，错误地址：%2")
//           .arg(errCode);
//   // .arg(errAdr);
//   qDebug() << crashMsg;

//   return EXCEPTION_EXECUTE_HANDLER;
// }

// void CreateDumpFile(EXCEPTION_POINTERS* exceptionInfo) {
//   qDebug() << "create";
//   HANDLE hFile = CreateFileW(L"dumpfile.dmp", GENERIC_WRITE, 0, NULL,
//                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//   if (hFile != INVALID_HANDLE_VALUE) {
//     MINIDUMP_EXCEPTION_INFORMATION mdei;
//     mdei.ThreadId = GetCurrentThreadId();
//     mdei.ExceptionPointers = exceptionInfo;
//     mdei.ClientPointers = FALSE;
//
//     MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
//                       MiniDumpWithFullMemory, &mdei, NULL, NULL);
//     CloseHandle(hFile);
//   }
// }

// LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
//   CreateDumpFile(exceptionInfo);
//   return EXCEPTION_EXECUTE_HANDLER;  // 继续执行默认的崩溃处理
// }

#endif

int main(int argc, char *argv[]) {
  // 屏蔽系统样式表
  qputenv("QT_QPA_PLATFORM", "windows:darkmode=1");
  QApplication app(argc, argv);

  // 注册表使用
  QCoreApplication::setApplicationName(
      QStringLiteral("TtDebugTool"));  // 程序名
  QCoreApplication::setOrganizationName(
      QStringLiteral("C3H3_Ttigone"));  // 组织名

  // 主应用字体
  QFontDatabase::addApplicationFont(":/font/iconfont.ttf");

  Storage::TtConfigsManager &configManager =
      Storage::TtConfigsManager::instance();
  configManager.setTargetStoreFile("config.ini");

  // 语言也有问题
  QString curLang =
      configManager.getConfigVaule("Language", "TtDebugTool_zh.qm").toString();

  // 这个有 bug
  QString fr = QApplication::applicationDirPath() + "/translations/";
  if (curLang == "TtDebugTool_zh.qm") {
    qDebug() << "zh";
    Lang::TtTranslationManager::instance().setLanguage(fr, "TtDebugTool_zh.qm");
  } else {
    qDebug() << "en";
    Lang::TtTranslationManager::instance().setLanguage(fr, "TtDebugTool_en.qm");
  }
  // 设置全局字体
  QFont font(":/font/roboto/Roboto-Black.ttf", 10);  // 微软雅黑，10号字体
  app.setFont(font);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  // 适用精确缩放
  // bug, 导致 QComboBox 不能正常工作, 同时字体发虚
  // 以下的作用是取消分辨率
  // QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
  //    Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

#if defined(Q_OS_WIN) && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  const auto osName = QSysInfo::prettyProductName();
  if (osName.startsWith("Windows 10") || osName.startsWith("Windows 11")) {
    QApplication::setStyle("fusion");
  }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
  QApplication::setQuitOnLastWindowClosed(true);

  QString filePath = "config.json";
  Storage::SettingsManager &settingsManager =
      Storage::SettingsManager::instance();
  settingsManager.setTargetStoreFile(filePath);
  settingsManager.saveSettings();
  settingsManager.setSaveDelay(2000);

  // 日志
  QDir("log").mkpath(".");
  glog_helper::InitGLog("TtDebugTool.exe", "log/", "log/TtDebugTool");
  // 自动清理30天前的日志
  google::EnableLogCleaner(24h * 30);

  // #if (WIN32)
  //   //注冊异常捕获函数
  //   // SetUnhandledExceptionFilter(
  //   // (LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
  //   SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionHandler);
  // #endif

  // qputenv("QT_STYLE_OVERRIDE", "");
  // app.setStyle("windows");  // 或 "fusion", "windowsvista"

  Window::MainWindow tt_debug_tool;

  tt_debug_tool.show();

  return app.exec();
}
