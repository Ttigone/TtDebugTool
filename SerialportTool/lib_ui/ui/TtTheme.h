#ifndef UI_TTTHEME_H
#define UI_TTTHEME_H

#include <QObject>

#include "ui/Def.h"
#include "ui/singleton.h"
#include "ui/ui_pch.h"

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

namespace Ui {

// 获取单例
#define tTheme TtTheme::getInstance()

#define TtThemeColor(themeMode, themeColor) \
  tTheme->getThemeColor(themeMode, TtThemeType::themeColor)

class TtThemePrivate;

// class Tt_EXPORT TtTheme : public QObject {
class TtTheme : public QObject {
  Q_OBJECT
  Q_Q_CREATE(TtTheme)
  // 单例
  Q_SINGLETON_CREATE(TtTheme)

 public:
  // 设置 theme 主题
  void setThemeMode(TtThemeType::ThemeMode themeMode);
  TtThemeType::ThemeMode getThemeMode() const;

  // 绘制阴影
  void drawEffectShadow(QPainter* painter, QRect widgetRect,
                        int shadowBorderWidth, int borderRadius);
  // 设置主题颜色
  void setThemeColor(TtThemeType::ThemeMode themeMode,
                     TtThemeType::ThemeColor themeColor, QColor newColor);
  // 获取主题颜色
  const QColor& getThemeColor(TtThemeType::ThemeMode themeMode,
                              TtThemeType::ThemeColor themeColor);

 Q_SIGNALS:
  void themeModeChanged(TtThemeType::ThemeMode themeMode);

 private:
  explicit TtTheme(QObject* parent = nullptr);
  ~TtTheme();
};

}  // namespace Ui

#endif  // UI_TTTHEME_H
