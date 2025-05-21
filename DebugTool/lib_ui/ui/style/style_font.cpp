#include "style_font.h"

/// @brief 初始化字体资源
void StyleInitFontsResource() {
#ifdef Q_OS_MAC

#else

#ifndef LIB_UI_USE_PACKAGED_RESOURCES
  // Q_INIT_RESOURCE(style_font);
#endif

#ifdef Q_OS_WIN

#endif

#endif
}

namespace style {

QString SystemFontTag() { return QString(); }

void SetCustomFont(const QString &font) {}

namespace internal {

void StartFonts() {
  if (Started) {
    return;
  }
  // 只会执行一次
  Started = true;

  // 初始化字体资源
  StyleInitFontsResource();

#ifndef LIB_UI_USE_PACKAGED_RESOURCES
  const auto name = QString("Open Sans");

  QFont::insertSubstitution(name, QString("Vazirmatn UI NL"));
#endif
}

void DestoryFonts() {}

int RegisterFontFamily(const QString &family) { return 0; }

} // namespace internal

} // namespace style
