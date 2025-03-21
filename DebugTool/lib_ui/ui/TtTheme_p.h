#ifndef UI_TTTHEME_P_H
#define UI_TTTHEME_P_H

#include <QColor>
#include <QMap>
#include <QObject>

#include "ui/Def.h"
#include "ui/ui_pch.h"

namespace Ui {

class TtTheme;

class TtThemePrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtTheme)
 public:
  explicit TtThemePrivate(QObject* parent = nullptr);
  ~TtThemePrivate();

 private:
  void initThemeColor();

  TtThemeType::ThemeMode _themeMode{TtThemeType::Light};
  QColor _lightThemeColorList[40];
  QColor _darkThemeColorList[40];
};

}  // namespace Ui

#endif  // UI_TTTHEME_P_H
