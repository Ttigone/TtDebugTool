#include "TtRadioButtonStyle.h"

namespace style {

TtRadioButtonStyle::TtRadioButtonStyle(QStyle* style) {
  theme_mode_ = Ui::tTheme->getThemeMode();
  connect(Ui::tTheme, &Ui::TtTheme::themeModeChanged, this,
          [=](TtThemeType::ThemeMode themeMode) { theme_mode_ = themeMode; });
}

TtRadioButtonStyle::~TtRadioButtonStyle() {}

void TtRadioButtonStyle::drawPrimitive(PrimitiveElement element,
                                       const QStyleOption* option,
                                       QPainter* painter,
                                       const QWidget* widget) const {
  switch (element) {
    case PE_IndicatorRadioButton: {
      const QStyleOptionButton* bopt =
          qstyleoption_cast<const QStyleOptionButton*>(option);
      if (!bopt) {
        break;
      }
      QRect buttonRect = bopt->rect;
      buttonRect.adjust(1, 1, -1, -1);
      painter->save();
      painter->setRenderHints(QPainter::Antialiasing |
                              QPainter::SmoothPixmapTransform);

      if (bopt->state & QStyle::State_Off) {
        painter->setPen(QPen(Ui::TtThemeColor(theme_mode_, BasicBorder), 1.5));
        if (bopt->state & QStyle::State_MouseOver) {
          painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHover));
        } else {
          painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBase));
        }
        painter->drawEllipse(
            QPointF(buttonRect.center().x() + 1, buttonRect.center().y() + 1),
            8.5, 8.5);
      } else {
        painter->setPen(Qt::NoPen);
        // 外圆形
        painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryNormal));
        painter->drawEllipse(
            QPointF(buttonRect.center().x() + 1, buttonRect.center().y() + 1),
            buttonRect.width() / 2, buttonRect.width() / 2);
        // 内圆形
        painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicTextInvert));
        if (bopt->state & QStyle::State_Sunken) {
          if (bopt->state & QStyle::State_MouseOver) {
            painter->drawEllipse(QPointF(buttonRect.center().x() + 1,
                                         buttonRect.center().y() + 1),
                                 buttonRect.width() / 4.5,
                                 buttonRect.width() / 4.5);
          }
        } else {
          if (bopt->state & QStyle::State_MouseOver) {
            painter->drawEllipse(QPointF(buttonRect.center().x() + 1,
                                         buttonRect.center().y() + 1),
                                 buttonRect.width() / 3.5,
                                 buttonRect.width() / 3.5);
          } else {
            painter->drawEllipse(QPointF(buttonRect.center().x() + 1,
                                         buttonRect.center().y() + 1),
                                 buttonRect.width() / 4,
                                 buttonRect.width() / 4);
          }
        }
      }
      painter->restore();
      return;
    }
    default: {
      break;
    }
  }

  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int TtRadioButtonStyle::pixelMetric(PixelMetric metric,
                                    const QStyleOption* option,
                                    const QWidget* widget) const {
  switch (metric) {
    case QStyle::PM_ExclusiveIndicatorWidth: {
      return 20;
    }
    case QStyle::PM_ExclusiveIndicatorHeight: {
      return 20;
    }
    default: {
      break;
    }
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

}  // namespace style
