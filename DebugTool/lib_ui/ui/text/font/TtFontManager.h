#ifndef UI_TEXT_FONT_TTFONTMANAGER_H
#define UI_TEXT_FONT_TTFONTMANAGER_H

#include <QString>

namespace Ui {
class FontManager {
 public:
  // 获取 FontAwesome 的字体家族名称
  static QString getFontAwesomeFamily();

 private:
  // 禁止实例化
  FontManager() = delete;
  ~FontManager() = delete;
};
}  // namespace Ui

#endif
