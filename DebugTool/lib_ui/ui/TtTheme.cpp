#include "ui/TtTheme.h"
#include "ui/TtTheme_p.h"

#include <QPainterPath>

namespace Ui {

TtThemePrivate::TtThemePrivate(QObject* parent) : QObject(parent) {}

TtThemePrivate::~TtThemePrivate() {}

void TtThemePrivate::initThemeColor() {
  //ElaScrollBar
  _lightThemeColorList[TtThemeType::ScrollBarHandle] = QColor(0xA0, 0xA0, 0xA0);
  _darkThemeColorList[TtThemeType::ScrollBarHandle] = QColor(0x9F, 0x9F, 0x9F);

  //ElaToggleSwitch
  _lightThemeColorList[TtThemeType::ToggleSwitchNoToggledCenter] =
      QColor(0x5A, 0x5A, 0x5A);
  _darkThemeColorList[TtThemeType::ToggleSwitchNoToggledCenter] =
      QColor(0xD0, 0xD0, 0xD0);

  // 主题颜色
  _lightThemeColorList[TtThemeType::PrimaryNormal] = QColor(0x00, 0x67, 0xC0);
  _darkThemeColorList[TtThemeType::PrimaryNormal] = QColor(0x4C, 0xC2, 0xFF);
  _lightThemeColorList[TtThemeType::PrimaryHover] = QColor(0x19, 0x75, 0xC5);
  _darkThemeColorList[TtThemeType::PrimaryHover] = QColor(0x47, 0xB1, 0xE8);
  _lightThemeColorList[TtThemeType::PrimaryPress] = QColor(0x31, 0x83, 0xCA);
  _darkThemeColorList[TtThemeType::PrimaryPress] = QColor(0x42, 0xA1, 0xD2);

  // 通用颜色
  // 普通窗体
  _lightThemeColorList[TtThemeType::WindowBase] = QColor(0xF3, 0xF3, 0xF3);
  _darkThemeColorList[TtThemeType::WindowBase] = QColor(0x20, 0x20, 0x20);
  _lightThemeColorList[TtThemeType::WindowCentralStackBase] =
      QColor(0xFF, 0xFF, 0xFF, 120);
  _darkThemeColorList[TtThemeType::WindowCentralStackBase] =
      QColor(0x3E, 0x3E, 0x3E, 60);

  // 浮动窗体
  _lightThemeColorList[TtThemeType::PopupBorder] = QColor(0xD6, 0xD6, 0xD6);
  _darkThemeColorList[TtThemeType::PopupBorder] = QColor(0x47, 0x47, 0x47);
  _lightThemeColorList[TtThemeType::PopupBorderHover] =
      QColor(0xCC, 0xCC, 0xCC);
  _darkThemeColorList[TtThemeType::PopupBorderHover] = QColor(0x54, 0x54, 0x54);
  _lightThemeColorList[TtThemeType::PopupBase] = QColor(0xFA, 0xFA, 0xFA);
  _darkThemeColorList[TtThemeType::PopupBase] = QColor(0x2C, 0x2C, 0x2C);
  _lightThemeColorList[TtThemeType::PopupHover] = QColor(0xF0, 0xF0, 0xF0);
  _darkThemeColorList[TtThemeType::PopupHover] = QColor(0x38, 0x38, 0x38);

  // Dialog窗体
  _lightThemeColorList[TtThemeType::DialogBase] = Qt::white;
  _darkThemeColorList[TtThemeType::DialogBase] = QColor(0x1F, 0x1F, 0x1F);
  _lightThemeColorList[TtThemeType::DialogLayoutArea] =
      QColor(0xF3, 0xF3, 0xF3);
  _darkThemeColorList[TtThemeType::DialogLayoutArea] = QColor(0x20, 0x20, 0x20);

  // 基础颜色
  _lightThemeColorList[TtThemeType::BasicText] = Qt::black;
  _darkThemeColorList[TtThemeType::BasicText] = Qt::white;
  _lightThemeColorList[TtThemeType::BasicTextInvert] = Qt::white;
  _darkThemeColorList[TtThemeType::BasicTextInvert] = Qt::black;
  _lightThemeColorList[TtThemeType::BasicDetailsText] =
      QColor(0x87, 0x87, 0x87);
  _darkThemeColorList[TtThemeType::BasicDetailsText] = QColor(0xAD, 0xAD, 0xB0);
  _lightThemeColorList[TtThemeType::BasicTextNoFocus] =
      QColor(0x86, 0x86, 0x8A);
  _darkThemeColorList[TtThemeType::BasicTextNoFocus] = QColor(0x86, 0x86, 0x8A);
  _lightThemeColorList[TtThemeType::BasicTextDisable] =
      QColor(0xB6, 0xB6, 0xB6);
  _darkThemeColorList[TtThemeType::BasicTextDisable] = QColor(0xA7, 0xA7, 0xA7);
  _lightThemeColorList[TtThemeType::BasicTextPress] = QColor(0x5A, 0x5A, 0x5D);
  _darkThemeColorList[TtThemeType::BasicTextPress] = QColor(0xBB, 0xBB, 0xBF);
  _lightThemeColorList[TtThemeType::BasicBorder] = QColor(0xE5, 0xE5, 0xE5);
  _darkThemeColorList[TtThemeType::BasicBorder] = QColor(0x4B, 0x4B, 0x4B);
  _lightThemeColorList[TtThemeType::BasicBorderDeep] = QColor(0xA8, 0xA8, 0xA8);
  _darkThemeColorList[TtThemeType::BasicBorderDeep] = QColor(0x5C, 0x5C, 0x5C);
  _lightThemeColorList[TtThemeType::BasicBorderHover] =
      QColor(0xDA, 0xDA, 0xDA);
  _darkThemeColorList[TtThemeType::BasicBorderHover] = QColor(0x57, 0x57, 0x57);
  _lightThemeColorList[TtThemeType::BasicBase] = QColor(0xFD, 0xFD, 0xFD);
  _darkThemeColorList[TtThemeType::BasicBase] = QColor(0x34, 0x34, 0x34);
  _lightThemeColorList[TtThemeType::BasicBaseDeep] = QColor(0xE6, 0xE6, 0xE6);
  _darkThemeColorList[TtThemeType::BasicBaseDeep] = QColor(0x61, 0x61, 0x61);
  _lightThemeColorList[TtThemeType::BasicDisable] = QColor(0xF5, 0xF5, 0xF5);
  _darkThemeColorList[TtThemeType::BasicDisable] = QColor(0x2A, 0x2A, 0x2A);
  _lightThemeColorList[TtThemeType::BasicHover] = QColor(0xF3, 0xF3, 0xF3);
  _darkThemeColorList[TtThemeType::BasicHover] = QColor(0x40, 0x40, 0x40);
  _lightThemeColorList[TtThemeType::BasicPress] = QColor(0xF7, 0xF7, 0xF7);
  _darkThemeColorList[TtThemeType::BasicPress] = QColor(0x3A, 0x3A, 0x3A);
  _lightThemeColorList[TtThemeType::BasicBaseLine] = QColor(0xD1, 0xD1, 0xD1);
  _darkThemeColorList[TtThemeType::BasicBaseLine] = QColor(0x45, 0x45, 0x45);
  _lightThemeColorList[TtThemeType::BasicHemline] = QColor(0x86, 0x86, 0x86);
  _darkThemeColorList[TtThemeType::BasicHemline] = QColor(0x9A, 0x9A, 0x9A);
  _lightThemeColorList[TtThemeType::BasicIndicator] = QColor(0x75, 0x7C, 0x87);
  _darkThemeColorList[TtThemeType::BasicIndicator] = QColor(0x75, 0x7C, 0x87);
  _lightThemeColorList[TtThemeType::BasicChute] = QColor(0xD6, 0xD6, 0xD6);
  _darkThemeColorList[TtThemeType::BasicChute] = QColor(0x63, 0x63, 0x63);

  // 基础透明
  _lightThemeColorList[TtThemeType::BasicAlternating] =
      QColor(0xEF, 0xEF, 0xEF, 160);
  _darkThemeColorList[TtThemeType::BasicAlternating] =
      QColor(0x45, 0x45, 0x45, 125);
  _lightThemeColorList[TtThemeType::BasicBaseAlpha] =
      QColor(0xFF, 0xFF, 0xFF, 160);
  _darkThemeColorList[TtThemeType::BasicBaseAlpha] =
      QColor(0x45, 0x45, 0x45, 95);
  _lightThemeColorList[TtThemeType::BasicBaseDeepAlpha] =
      QColor(0xCC, 0xCC, 0xCC, 160);
  _darkThemeColorList[TtThemeType::BasicBaseDeepAlpha] =
      QColor(0x72, 0x72, 0x72, 95);
  _lightThemeColorList[TtThemeType::BasicHoverAlpha] =
      QColor(0xCC, 0xCC, 0xCC, 60);
  _darkThemeColorList[TtThemeType::BasicHoverAlpha] =
      QColor(0x4B, 0x4B, 0x4B, 75);
  _lightThemeColorList[TtThemeType::BasicPressAlpha] =
      QColor(0xCC, 0xCC, 0xCC, 40);
  _darkThemeColorList[TtThemeType::BasicPressAlpha] =
      QColor(0x4B, 0x4B, 0x4B, 55);
  _lightThemeColorList[TtThemeType::BasicSelectedAlpha] =
      QColor(0xCC, 0xCC, 0xCC, 60);
  _darkThemeColorList[TtThemeType::BasicSelectedAlpha] =
      QColor(0x4B, 0x4B, 0x4B, 75);
  _lightThemeColorList[TtThemeType::BasicSelectedHoverAlpha] =
      QColor(0xCC, 0xCC, 0xCC, 40);
  _darkThemeColorList[TtThemeType::BasicSelectedHoverAlpha] =
      QColor(0x4B, 0x4B, 0x4B, 55);

  // 状态颜色
  _lightThemeColorList[TtThemeType::StatusDanger] = QColor(0xE8, 0x11, 0x23);
  _darkThemeColorList[TtThemeType::StatusDanger] = QColor(0xE8, 0x11, 0x23);
}

void TtTheme::setThemeMode(TtThemeType::ThemeMode themeMode) {
  Q_D(TtTheme);
  d->_themeMode = themeMode;
  // 主题改变的信号
  Q_EMIT themeModeChanged(d->_themeMode);
}

TtThemeType::ThemeMode TtTheme::getThemeMode() const {
  Q_D(const TtTheme);
  return d->_themeMode;
}

void TtTheme::drawEffectShadow(QPainter* painter, QRect widgetRect,
                               int shadowBorderWidth, int borderRadius) {
  Q_D(TtTheme);
  painter->save();
  painter->setRenderHints(QPainter::Antialiasing);
  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  QColor color = d->_themeMode == TtThemeType::Light ? QColor(0x70, 0x70, 0x70)
                                                     : QColor(0x9C, 0x9B, 0x9E);
  for (int i = 0; i < shadowBorderWidth; i++) {
    path.addRoundedRect(shadowBorderWidth - i, shadowBorderWidth - i,
                        widgetRect.width() - (shadowBorderWidth - i) * 2,
                        widgetRect.height() - (shadowBorderWidth - i) * 2,
                        borderRadius + i, borderRadius + i);
    int alpha = 1 * (shadowBorderWidth - i + 1);
    color.setAlpha(alpha > 255 ? 255 : alpha);
    painter->setPen(color);
    painter->drawPath(path);
  }
  painter->restore();
}

void TtTheme::setThemeColor(TtThemeType::ThemeMode themeMode,
                            TtThemeType::ThemeColor themeColor,
                            QColor newColor) {
  Q_D(TtTheme);
  if (themeMode == TtThemeType::Light) {
    d->_lightThemeColorList[themeColor] = newColor;
  } else {
    d->_darkThemeColorList[themeColor] = newColor;
  }
}

const QColor& TtTheme::getThemeColor(TtThemeType::ThemeMode themeMode,
                                     TtThemeType::ThemeColor themeColor) {
  Q_D(TtTheme);
  if (themeMode == TtThemeType::Light) {
    return d->_lightThemeColorList[themeColor];
  } else {
    return d->_darkThemeColorList[themeColor];
  }
}

TtTheme::TtTheme(QObject* parent)
    : QObject{parent},
      d_ptr(new TtThemePrivate())

{
  Q_D(TtTheme);
  d->q_ptr = this;
  d->initThemeColor();
}

TtTheme::~TtTheme() {}

}  // namespace Ui
