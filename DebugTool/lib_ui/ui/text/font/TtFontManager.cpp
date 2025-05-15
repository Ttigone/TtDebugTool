#include "ui/text/font/TtFontManager.h"

#include <QFontDatabase>

namespace Ui {
namespace {

// 单次在家

bool loadFont() {
  // int fontId =
  //     QFontDatabase::addApplicationFont(":/font/fontawesome-webfont.ttf");
  // 如何获得对应的名字
  int fontId = QFontDatabase::addApplicationFont(":/font/iconfont.ttf");
  qDebug() << fontId;
  return (fontId != -1);
}
// static bool fontLoaded = loadFont();  // 库加载时自动执行
} // namespace

QString FontManager::getFontAwesomeFamily() {
  static bool fontLoaded = loadFont();
  QStringList families = QFontDatabase::families();
  // qDebug() << families;
  for (const QString &name : families) {
    if (name == "iconfont") {
      return name;
    }
  }
  return "";
}

} // namespace Ui
