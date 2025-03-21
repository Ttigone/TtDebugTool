#include "ui/style/TtScrollBarStyle.h"

#include "ui/TtTheme.h"
#include "ui/control/TtScrollBar.h"

#include <QPainterPath>

namespace style {

TtScrollBarStyle::TtScrollBarStyle(QStyle* style) {
  pIsExpand_ = false;
  pOpacity_ = 0;
  pScrollBar_ = nullptr;
  pSliderExtent_ = 2.4;
  theme_mode_ = Ui::tTheme->getThemeMode();
  connect(Ui::tTheme, &Ui::TtTheme::themeModeChanged, this,
          [=](TtThemeType::ThemeMode themeMode) { theme_mode_ = themeMode; });
}

TtScrollBarStyle::~TtScrollBarStyle() {}

void TtScrollBarStyle::drawComplexControl(ComplexControl control,
                                          const QStyleOptionComplex* option,
                                          QPainter* painter,
                                          const QWidget* widget) const {
  //QStyle::SC_ScrollBarGroove QStyle::SC_ScrollBarAddLine   QStyle::SC_ScrollBarSubLine上指示器
  switch (control) {
    case QStyle::CC_ScrollBar: {
      if (const QStyleOptionSlider* sopt =
              qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        QRect scrollBarRect = sopt->rect;
        // 展开
        if (pIsExpand_) {
          // 背景绘制
          painter->setOpacity(pOpacity_);
          painter->setPen(Qt::NoPen);
          painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBase));
          painter->drawRoundedRect(scrollBarRect, 6, 6);
          //指示器绘制 center()在此处不适用 存在外围边距宽度 需手动计算
          int sideLength = 8;
          painter->setBrush(Ui::TtThemeColor(theme_mode_, ScrollBarHandle));
          if (sopt->orientation == Qt::Horizontal) {
            QRect leftIndicatorRect = subControlRect(
                control, sopt, QStyle::SC_ScrollBarSubLine, widget);
            QRect rightIndicatorRect = subControlRect(
                control, sopt, QStyle::SC_ScrollBarAddLine, widget);
            // 左三角
            qreal centerLeftX =
                leftIndicatorRect.x() + leftIndicatorRect.width() / 2;
            qreal centerRightX =
                rightIndicatorRect.x() + rightIndicatorRect.width() / 2;
            qreal centerY = leftIndicatorRect.height() / 2;
            QPainterPath leftPath;
            leftPath.moveTo(
                centerLeftX - qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY);
            leftPath.lineTo(
                centerLeftX + qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY + sideLength / 2);
            leftPath.lineTo(
                centerLeftX + qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY - sideLength / 2);
            leftPath.closeSubpath();
            painter->drawPath(leftPath);

            // 右三角
            QPainterPath rightPath;
            rightPath.moveTo(
                centerRightX + qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY);
            rightPath.lineTo(
                centerRightX - qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY + sideLength / 2);
            rightPath.lineTo(
                centerRightX - qCos(30 * M_PI / 180.0) * sideLength / 2,
                centerY - sideLength / 2);
            rightPath.closeSubpath();
            painter->drawPath(rightPath);
          } else {
            QRect upIndicatorRect = subControlRect(
                control, sopt, QStyle::SC_ScrollBarSubLine, widget);
            QRect downIndicatorRect = subControlRect(
                control, sopt, QStyle::SC_ScrollBarAddLine, widget);
            qreal centerToTop = (sideLength / 2) / qCos(30 * M_PI / 180.0);
            qreal centerToBottom = (sideLength / 2) * qTan(30 * M_PI / 180.0);
            // 上三角
            qreal centerX = upIndicatorRect.width() / 2.0;
            qreal centerUpY = upIndicatorRect.center().y() + 2;
            qreal centerDownY = downIndicatorRect.center().y() + 2;
            QPainterPath upPath;
            upPath.moveTo(centerX, centerUpY - centerToTop);
            upPath.lineTo(centerX + sideLength / 2, centerUpY + centerToBottom);
            upPath.lineTo(centerX - sideLength / 2, centerUpY + centerToBottom);
            upPath.closeSubpath();
            painter->drawPath(upPath);

            // 下三角
            QPainterPath downPath;
            downPath.moveTo(centerX, centerDownY + centerToBottom);
            downPath.lineTo(centerX + sideLength / 2,
                            centerDownY - centerToTop);
            downPath.lineTo(centerX - sideLength / 2,
                            centerDownY - centerToTop);
            downPath.closeSubpath();
            painter->drawPath(downPath);
          }
        }
        painter->setOpacity(1);
        //滑块绘制
        QRectF sliderRect =
            subControlRect(control, sopt, QStyle::SC_ScrollBarSlider, widget);
        painter->setBrush(Ui::TtThemeColor(theme_mode_, ScrollBarHandle));
        if (sopt->orientation == Qt::Horizontal) {
          sliderRect.setRect(
              sliderRect.x(),
              sliderRect.bottom() - slider_margin_ - pSliderExtent_,
              sliderRect.width(), pSliderExtent_);
        } else {
          sliderRect.setRect(
              sliderRect.right() - slider_margin_ - pSliderExtent_,
              sliderRect.y(), pSliderExtent_, sliderRect.height());
        }
        painter->drawRoundedRect(sliderRect, pSliderExtent_ / 2.0,
                                 pSliderExtent_ / 2.0);
        painter->restore();
      }
      return;
    }
    default: {
      break;
    }
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

int TtScrollBarStyle::pixelMetric(PixelMetric metric,
                                  const QStyleOption* option,
                                  const QWidget* widget) const {
  // qDebug() << metric << QProxyStyle::pixelMetric(metric, option, widget);
  switch (metric) {
    case QStyle::PM_ScrollBarExtent: {
      return scrollBar_extent_;
    }
    default: {
      break;
    }
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

int TtScrollBarStyle::styleHint(StyleHint hint, const QStyleOption* option,
                                const QWidget* widget,
                                QStyleHintReturn* returnData) const {
  if (hint == QStyle::SH_ScrollBar_LeftClickAbsolutePosition) {
    return true;
  }
  return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void TtScrollBarStyle::startExpandAnimation(bool isExpand) {
  if (isExpand) {
    pIsExpand_ = true;
    QPropertyAnimation* opacityAnimation =
        new QPropertyAnimation(this, "pOpacity");
    connect(opacityAnimation, &QPropertyAnimation::valueChanged, this,
            [=]() { pScrollBar_->update(); });
    opacityAnimation->setDuration(250);
    opacityAnimation->setEasingCurve(QEasingCurve::InOutSine);
    opacityAnimation->setStartValue(pOpacity_);
    opacityAnimation->setEndValue(1);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* extentAnimation =
        new QPropertyAnimation(this, "pSliderExtent");
    extentAnimation->setDuration(250);
    extentAnimation->setEasingCurve(QEasingCurve::InOutSine);
    extentAnimation->setStartValue(pSliderExtent_);
    extentAnimation->setEndValue(scrollBar_extent_ - 2 * slider_margin_);
    extentAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  } else {
    QPropertyAnimation* opacityAnimation =
        new QPropertyAnimation(this, "pOpacity");
    connect(opacityAnimation, &QPropertyAnimation::finished, this,
            [=]() { pIsExpand_ = false; });
    connect(opacityAnimation, &QPropertyAnimation::valueChanged, this,
            [=]() { pScrollBar_->update(); });
    opacityAnimation->setDuration(250);
    opacityAnimation->setEasingCurve(QEasingCurve::InOutSine);
    opacityAnimation->setStartValue(pOpacity_);
    opacityAnimation->setEndValue(0);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* extentAnimation =
        new QPropertyAnimation(this, "pSliderExtent");
    extentAnimation->setDuration(250);
    extentAnimation->setEasingCurve(QEasingCurve::InOutSine);
    extentAnimation->setStartValue(pSliderExtent_);
    extentAnimation->setEndValue(2.4);
    extentAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

}  // namespace style
