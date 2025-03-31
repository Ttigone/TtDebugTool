#include <QApplication>
#include <QTextCodec>
#include "qt-easy-logger-main/logger.h"

#include "storage/setting_manager.h"
#include "window/main_window.h"

// 编译器版本为 6.5.3 时, 调整宽度和高度时, 控件会改变布局, qwindowkit bug
// 编译器 < 6.5.3 or 编译器 > 6.6.2

#if (WIN32)
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

void CreateDumpFile(EXCEPTION_POINTERS* exceptionInfo) {
  qDebug() << "create";
  HANDLE hFile = CreateFileW(L"dumpfile.dmp", GENERIC_WRITE, 0, NULL,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = exceptionInfo;
    mdei.ClientPointers = FALSE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpWithFullMemory, &mdei, NULL, NULL);
    CloseHandle(hFile);
  }
}

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
  CreateDumpFile(exceptionInfo);
  return EXCEPTION_EXECUTE_HANDLER;  // 继续执行默认的崩溃处理
}

#endif

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  QCoreApplication::setApplicationName(
      QStringLiteral("SerialportTool-C3H3_Ttigone"));
  QCoreApplication::setOrganizationName(QStringLiteral("WWB-Qt"));

  QFontDatabase::addApplicationFont(":/font/iconfont.ttf");
  auto fid =
      QFontDatabase::addApplicationFont(":/font/fontawesome-webfont.ttf");
  qDebug() << "TEST: " << QFontDatabase::applicationFontFamilies(fid);

  // 语言切换
  // 读取命令行参数中的语言设置
  QString language;
  QStringList args = QApplication::arguments();
  int langIndex = args.indexOf("--lang");
  if (langIndex != -1 && langIndex + 1 < args.size()) {
    language = args.at(langIndex + 1);
  } else {
    // 从配置文件读取默认语言
    QSettings settings("MyCompany", "MyApp");
    language = settings.value("Language", "en_US").toString();
  }

  // 加载对应的翻译文件
  QTranslator translator;
  if (translator.load(":/translations/" + language + ".qm")) {
    app.installTranslator(&translator);
  }

  // 设置全局字体
  QFont font(":/font/roboto/Roboto-Black.ttf", 10);  // 微软雅黑，10号字体
  app.setFont(font);

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
  // setDarkBorderToWindow(); // 仅在 Windows 下调用
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
  // const auto osName = QSysInfo::prettyProductName();
  // if (osName.startsWith("Windows 10") || osName.startsWith("Windows 11")) {
  //   QApplication::setStyle("fusion");
  // }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  // QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
  //     Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

  QString filePath = "config.json";
  Storage::SettingsManager& settingsManager =
      Storage::SettingsManager::instance();
  settingsManager.setTargetStoreFile(filePath);
  settingsManager.saveSettings();

#if (WIN32)
  //注冊异常捕获函数
  // SetUnhandledExceptionFilter(
  // (LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionHandler);
#endif

  qInstallMessageHandler(h::Logger::messageHandler);  // 启用功能

  Window::MainWindow w;
  w.show();

  return app.exec();
}
