#include "TtTextButton.h"

#include <QApplication>
#include <QPainterPath>

#include "TtTextButton_p.h"

namespace Ui {

Q_PROPERTY_CREATE_Q_CPP(TtTextButton, int, BorderRadius)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, LightDefaultColor)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, DarkDefaultColor)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, LightHoverColor)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, DarkHoverColor)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, LightPressColor)
Q_PROPERTY_CREATE_Q_CPP(TtTextButton, QColor, DarkPressColor)

TtTextButton::TtTextButton(QWidget* parent)
    : QPushButton(parent), d_ptr(new TtTextButtonPrivate) {
  Q_D(TtTextButton);
  default_color_ = palette().color(QPalette::ButtonText);

  d->q_ptr = this;
  d->pBorderRadius_ = 3;
  // d->_themeMode = ElaApplication::getInstance()->getThemeMode();
  const QColor base = qApp->palette().color(QPalette::Window);
  d->_themeMode =
      (base.lightness() < 128) ? TtThemeType::Dark : TtThemeType::Light;
  // d->_themeMode = TtThemeType::Light;
  d->pLightDefaultColor_ = QColor(0x2A, 0x2A, 0x2A);
  d->pDarkDefaultColor_ = QColor(0x3E, 0x3E, 0x3E);
  d->pLightHoverColor_ = QColor(0xF6, 0xF6, 0xF6);
  d->pDarkHoverColor_ = QColor(0x4F, 0x4F, 0x4F);
  d->pLightPressColor_ = QColor(0xF2, 0xF2, 0xF2);
  d->pDarkPressColor_ = QColor(0x1C, 0x1C, 0x1C);
  d->_lightTextColor = Qt::white;
  d->_darkTextColor = Qt::white;
  setMouseTracking(true);
  setFixedSize(76, 32);
  QFont font = this->font();
  font.setPointSize(11);
  setFont(font);
  setObjectName("TtTextButton");
  setStyleSheet("#TtTextButton{background-color:transparent;}");
  d->onThemeChanged(d->_themeMode);
  // connect(ElaApplication::getInstance(), &ElaApplication::themeModeChanged,
  // d,
  //         &ElaPushButtonPrivate::onThemeChanged);
}

TtTextButton::TtTextButton(const QString& text, QWidget* parent)
    : Ui::TtTextButton(parent) {
  setText(text);
}

TtTextButton::TtTextButton(const QColor& color, const QString& text,
                           QWidget* parent)
    : Ui::TtTextButton(text, parent) {
  setCheckedColor(color);
}

TtTextButton::~TtTextButton() {}

void TtTextButton::setChecked(bool checked) {
  if (is_checked_ != checked) {
    is_checked_ = checked;
    update();
    emit toggled(is_checked_);
  }
}

void TtTextButton::setCheckedColor(const QColor& color) {
  checked_color_ = color;
}

void TtTextButton::setLightTextColor(const QColor& color) {
  Q_D(TtTextButton);
  d->_lightTextColor = color;
  if (d->_themeMode == TtThemeType::ThemeMode::Light) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::ButtonText, color);
    setPalette(palette);
  }
  update();
}

QColor TtTextButton::getLightTextColor() const {
  Q_D(const TtTextButton);
  return d->_lightTextColor;
}

void TtTextButton::setDarkTextColor(const QColor& color) {
  Q_D(TtTextButton);
  d->_darkTextColor = color;
  if (d->_themeMode == TtThemeType::ThemeMode::Dark) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::ButtonText, color);
    setPalette(palette);
  }
  update();
}

QColor TtTextButton::getDarkTextColor() const {
  Q_D(const TtTextButton);
  return d->_darkTextColor;
}

void TtTextButton::paintEvent(QPaintEvent* event) {
  Q_D(TtTextButton);
  QPainter painter(this);
  painter.setRenderHints(QPainter::SmoothPixmapTransform |
                         QPainter::Antialiasing | QPainter::TextAntialiasing);
  painter.save();
  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  // QColor color =
  //     d->_themeMode == TtThemeType::Light
  //         ? ElaApplication::getInstance()->getLightShadowEffectColor()
  //         : ElaApplication::getInstance()->getDarkShadowEffectColor();
  QColor color = d->_themeMode == TtThemeType::Light
                     ? QColor(165, 165, 165, 165)
                     : QColor(185, 185, 185, 185);
  for (int i = 0; i < d->_shadowBorderWidth; i++) {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(d->_shadowBorderWidth - i, d->_shadowBorderWidth - i,
                        this->width() - (d->_shadowBorderWidth - i) * 2,
                        this->height() - (d->_shadowBorderWidth - i) * 2,
                        d->pBorderRadius_ + i, d->pBorderRadius_ + i);
    int alpha = 5 * (d->_shadowBorderWidth - i + 1);
    color.setAlpha(alpha > 255 ? 255 : alpha);
    painter.setPen(color);
    painter.drawPath(path);
  }
  painter.restore();

  painter.save();
  QRect foregroundRect(d->_shadowBorderWidth, d->_shadowBorderWidth,
                       width() - 2 * (d->_shadowBorderWidth),
                       height() - 2 * d->_shadowBorderWidth);
  if (d->_themeMode == TtThemeType::Light) {
    painter.setPen(QPen(QColor(0xDF, 0xDF, 0xDF), 1));
    if (is_checked_) {
      painter.setBrush(checked_color_);
    } else {
      painter.setBrush(d->_isPressed ? d->pLightPressColor_
                                     : (underMouse() ? d->pLightHoverColor_
                                                     : d->pLightDefaultColor_));
    }
  } else {
    painter.setPen(Qt::NoPen);
    painter.setBrush(d->_isPressed ? d->pDarkPressColor_
                                   : (underMouse() ? d->pDarkHoverColor_
                                                   : d->pDarkDefaultColor_));
  }
  painter.drawRoundedRect(foregroundRect, d->pBorderRadius_, d->pBorderRadius_);
  // 底边线绘制
  if (!d->_isPressed) {
    painter.setPen(QPen(QColor(0xBC, 0xBC, 0xBC), 1));
    painter.drawLine(foregroundRect.x() + d->pBorderRadius_,
                     height() - d->_shadowBorderWidth, foregroundRect.width(),
                     height() - d->_shadowBorderWidth);
  }
  painter.restore();
  QPushButton::paintEvent(event);
}

void TtTextButton::mouseReleaseEvent(QMouseEvent* event) {
  Q_D(TtTextButton);
  d->_isPressed = false;
  Q_EMIT clicked();
  QPushButton::mouseReleaseEvent(event);
}

void TtTextButton::mousePressEvent(QMouseEvent* event) {
  Q_D(TtTextButton);
  d->_isPressed = true;
  QPushButton::mousePressEvent(event);
}

TtTextButtonPrivate::TtTextButtonPrivate(QObject* parent) : QObject(parent) {}

TtTextButtonPrivate::~TtTextButtonPrivate() {}

void TtTextButtonPrivate::onThemeChanged(TtThemeType::ThemeMode themeMode) {
  Q_Q(TtTextButton);
  _themeMode = themeMode;
  if (_themeMode == TtThemeType::Light) {
    QPalette palette = q->palette();
    palette.setColor(QPalette::ButtonText, _lightTextColor);
    q->setPalette(palette);
  } else {
    QPalette palette = q->palette();
    palette.setColor(QPalette::ButtonText, _darkTextColor);
    q->setPalette(palette);
  }
}

}  // namespace Ui
