#include "ui/control/TtIconButton.h"
#include "ui/control/TtIconButton_p.h"

#include <QEvent>
#include <QPainterPath>

#include "ui/TtTheme.h"

namespace Ui {

Q_PROPERTY_CREATE_Q_CPP(TtIconButton, int, BorderRadius)
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, qreal, Opacity);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, LightHoverColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, DarkHoverColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, LightIconColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, DarkIconColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, LightHoverIconColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, QColor, DarkHoverIconColor);
Q_PROPERTY_CREATE_Q_CPP(TtIconButton, bool, IsSelected);

TtIconButton::TtIconButton(QPixmap pix, QWidget* parent)
    : QPushButton(parent), d_ptr(new TtIconButtonPrivate()) {
  Q_D(TtIconButton);
  d->q_ptr = this;
  d->icon_pix_ = pix.copy();
  d->pHoverAlpha_ = 0;
  d->pOpacity_ = 1;
  d->pLightHoverColor_ = TtThemeColor(TtThemeType::Light, BasicHoverAlpha);
  d->pDarkHoverColor_ = TtThemeColor(TtThemeType::Dark, BasicHoverAlpha);
  d->pLightIconColor_ = TtThemeColor(TtThemeType::Light, BasicText);
  d->pDarkIconColor_ = TtThemeColor(TtThemeType::Dark, BasicText);
  d->pLightHoverIconColor_ = TtThemeColor(TtThemeType::Light, BasicText);
  d->pDarkHoverIconColor_ = TtThemeColor(TtThemeType::Dark, BasicText);

  d->pIsSelected_ = false;
  d->pBorderRadius_ = 0;
  d->theme_mode_ = tTheme->getThemeMode();
  connect(this, &TtIconButton::pIsSelectedChanged, this, [=]() { update(); });
  connect(
      tTheme, &TtTheme::themeModeChanged, this,
      [=](TtThemeType::ThemeMode themeMode) { d->theme_mode_ = themeMode; });
}

TtIconButton::TtIconButton(TtIconType::IconName awesome, QWidget* parent) {}

TtIconButton::TtIconButton(TtIconType::IconName awesome, int pixelSize,
                           QWidget* parent) {}

TtIconButton::TtIconButton(TtIconType::IconName awesome, int pixelSize,
                           int fixedWidth, int fixedHeight, QWidget* parent) {}

TtIconButton::~TtIconButton() {}

void TtIconButton::setAwesome(TtIconType::IconName awesome) {
  Q_D(TtIconButton);
  d->pAwesome_ = awesome;
  this->setText(QChar((unsigned short)awesome));
}

TtIconType::IconName TtIconButton::getAwesome() const {
  return this->d_ptr->pAwesome_;
}

void TtIconButton::setPixmap(QPixmap pix) {
  Q_D(TtIconButton);
  d->icon_pix_ = pix.copy();
}

bool TtIconButton::event(QEvent* event) {
  Q_D(TtIconButton);
  switch (event->type()) {
    case QEvent::Enter: {
      if (isEnabled() && !d->pIsSelected_) {
        d->is_alpha_animation_finished_ = false;
        QPropertyAnimation* alphaAnimation =
            new QPropertyAnimation(d, "pHoverAlpha");
        connect(alphaAnimation, &QPropertyAnimation::valueChanged, this,
                [=](const QVariant& value) { update(); });
        connect(alphaAnimation, &QPropertyAnimation::finished, this,
                [=]() { d->is_alpha_animation_finished_ = true; });
        alphaAnimation->setDuration(175);
        alphaAnimation->setStartValue(d->pHoverAlpha_);
        alphaAnimation->setEndValue(d->theme_mode_ == TtThemeType::Light
                                        ? d->pLightHoverColor_.alpha()
                                        : d->pDarkHoverColor_.alpha());
        alphaAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      }
      break;
    }
    case QEvent::Leave: {
      if (isEnabled() && !d->pIsSelected_) {
        d->is_alpha_animation_finished_ = false;
        QPropertyAnimation* alphaAnimation =
            new QPropertyAnimation(d, "pHoverAlpha");
        connect(alphaAnimation, &QPropertyAnimation::valueChanged, this,
                [=](const QVariant& value) { update(); });
        connect(alphaAnimation, &QPropertyAnimation::finished, this,
                [=]() { d->is_alpha_animation_finished_ = true; });
        alphaAnimation->setDuration(175);
        alphaAnimation->setStartValue(d->pHoverAlpha_);
        alphaAnimation->setEndValue(0);
        alphaAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      }
      break;
    }
    default: {
      break;
    }
  }
  return QPushButton::event(event);
}

void TtIconButton::paintEvent(QPaintEvent* event) {
  Q_D(TtIconButton);
  QPainter painter(this);
  painter.save();
  painter.setOpacity(d->pOpacity_);
  painter.setRenderHints(QPainter::SmoothPixmapTransform |
                         QPainter::Antialiasing | QPainter::TextAntialiasing);
  painter.setPen(Qt::NoPen);
  if (d->is_alpha_animation_finished_ || d->pIsSelected_) {
    painter.setBrush(d->pIsSelected_ ? d->theme_mode_ == TtThemeType::Light
                                           ? d->pLightHoverColor_
                                           : d->pDarkHoverColor_
                     : isEnabled()   ? underMouse()
                                           ? d->theme_mode_ == TtThemeType::Light
                                                 ? d->pLightHoverColor_
                                                 : d->pDarkHoverColor_
                                           : Qt::transparent
                                     : Qt::transparent);
  } else {
    QColor hoverColor = d->theme_mode_ == TtThemeType::Light
                            ? d->pLightHoverColor_
                            : d->pDarkHoverColor_;
    hoverColor.setAlpha(d->pHoverAlpha_);
    painter.setBrush(hoverColor);
  }
  painter.drawRoundedRect(rect(), d->pBorderRadius_, d->pBorderRadius_);
  // 图标绘制
  if (!d->icon_pix_.isNull()) {
    QPainterPath path;
    path.addEllipse(rect());
    painter.setClipPath(path);
    painter.drawPixmap(rect(), d->icon_pix_);
  } else {
    painter.setPen(isEnabled()
                       ? d->theme_mode_ == TtThemeType::Light
                             ? underMouse() ? d->pLightHoverIconColor_
                                            : d->pLightIconColor_
                         : underMouse() ? d->pDarkHoverIconColor_
                                        : d->pDarkIconColor_
                       : TtThemeColor(d->theme_mode_, BasicTextDisable));
    painter.drawText(rect(), Qt::AlignCenter,
                     QChar((unsigned short)d->pAwesome_));
  }
  painter.restore();
}

TtIconButtonPrivate::TtIconButtonPrivate(QObject* parent) : QObject(parent) {}

TtIconButtonPrivate::~TtIconButtonPrivate() {}

}  // namespace Ui
