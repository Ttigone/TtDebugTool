#include "TtCheckBoxStyle.h"

#include <QPainter>
#include <QStyleOption>

#include "ui/TtTheme.h"

namespace style {

TtCheckBoxStyle::TtCheckBoxStyle(QStyle* style) {
  pCheckIndicatorWidth_ = 21;
  theme_mode_ = Ui::tTheme->getThemeMode();
  // 主题改变
  connect(Ui::tTheme, &Ui::TtTheme::themeModeChanged, this,
          [=](TtThemeType::ThemeMode themeMode) { theme_mode_ = themeMode; });
}

TtCheckBoxStyle::~TtCheckBoxStyle() {}

void TtCheckBoxStyle::drawControl(ControlElement element,
                                  const QStyleOption* option, QPainter* painter,
                                  const QWidget* widget) const {
  switch (element) {
    // 只对 checkbox 绘制
    case QStyle::CE_CheckBox: {
      if (const QStyleOptionButton* bopt =
              qstyleoption_cast<const QStyleOptionButton*>(option)) {
        bool isEnabled = bopt->state.testFlag(QStyle::State_Enabled);
        painter->save();
        // 抗锯齿
        painter->setRenderHints(QPainter::Antialiasing |
                                QPainter::TextAntialiasing);

        // 使用 subElementRect 获取正确的位置
        QRect checkRect =
            subElementRect(QStyle::SE_CheckBoxIndicator, bopt, widget);
        checkRect.adjust(1, 1, -1, -1);  // 内缩 1px
        // 勾选框 正方形
        QRect checkBoxRect = bopt->rect;
        // QRect checkRect(checkBoxRect.x(), checkBoxRect.y(),
        //                 pCheckIndicatorWidth_, pCheckIndicatorWidth_);
        // 内缩 1px
        checkRect.adjust(1, 1, -1, -1);
        //复选框绘制
        painter->setPen(Qt::NoPen);
        if (bopt->state.testFlag(QStyle::State_On) ||
            bopt->state.testFlag(QStyle::State_NoChange)) {
          painter->setPen(Qt::NoPen);
          if (bopt->state.testFlag(QStyle::State_Sunken)) {
            painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryPress));
          } else {
            if (bopt->state.testFlag(QStyle::State_MouseOver)) {
              painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryHover));
            } else {
              painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryNormal));
            }
          }
        } else {
          if (bopt->state.testFlag(QStyle::State_Sunken)) {
            painter->setPen(Ui::TtThemeColor(theme_mode_, BasicBorderDeep));
          } else {
            painter->setPen(Ui::TtThemeColor(theme_mode_, BasicBorderDeep));
            if (bopt->state.testFlag(QStyle::State_MouseOver)) {
              painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHover));
            } else {
              painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBase));
            }
          }
        }
        painter->drawRoundedRect(checkRect, 2, 2);
        //图标绘制
        painter->setPen(Ui::TtThemeColor(TtThemeType::Dark, BasicText));
        if (bopt->state.testFlag(QStyle::State_On)) {
          painter->save();
          // 字体中有不同的 16进制
          QFont iconFont = QFont(":/font/iconfont.ttf");
          iconFont.setPixelSize(pCheckIndicatorWidth_ * 0.75);
          painter->setFont(iconFont);
          // 勾选符号
          painter->drawText(checkRect, Qt::AlignCenter,
                            QChar((unsigned short)0xe61c));
          painter->restore();
        } else if (bopt->state.testFlag(QStyle::State_NoChange)) {
          QLine checkLine(checkRect.x() + 3, checkRect.center().y(),
                          checkRect.right() - 3, checkRect.center().y());
          painter->drawLine(checkLine);
        }
        //文字绘制
        painter->setPen(isEnabled
                            ? Ui::TtThemeColor(theme_mode_, BasicText)
                            : Ui::TtThemeColor(theme_mode_, BasicTextDisable));
        QRect textRect(checkRect.right() + 10, checkBoxRect.y(),
                       checkBoxRect.width(), checkBoxRect.height());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          bopt->text);
        painter->restore();
      }
      return;
    }
    default: {
      break;
    }
  }

  QProxyStyle::drawControl(element, option, painter, widget);
}

int TtCheckBoxStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                 const QWidget* widget) const {
  switch (metric) {
    case QStyle::PM_IndicatorWidth: {
      return pCheckIndicatorWidth_;
    }
    case QStyle::PM_IndicatorHeight: {
      return pCheckIndicatorWidth_;
    }
    case QStyle::PM_CheckBoxLabelSpacing: {
      return 10;
    }
    default: {
      break;
    }
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

QRect TtCheckBoxStyle::subElementRect(SubElement element,
                                      const QStyleOption* option,
                                      const QWidget* widget) const {
  QRect rect = QProxyStyle::subElementRect(element, option, widget);
  if (element == QStyle::SE_CheckBoxIndicator) {
    if (const QStyleOptionButton* btnOpt =
            qstyleoption_cast<const QStyleOptionButton*>(option)) {
      if (btnOpt->text.isEmpty()) {
        // 获取复选框尺寸
        int w = pixelMetric(QStyle::PM_IndicatorWidth, btnOpt, widget);
        int h = pixelMetric(QStyle::PM_IndicatorHeight, btnOpt, widget);
        // 计算居中位置
        QRect totalRect = btnOpt->rect;
        int x = (totalRect.width() - w) / 2;
        int y = (totalRect.height() - h) / 2;
        rect = QRect(totalRect.x() + x, totalRect.y() + y, w, h);
      }
    }
  }
  return rect;
}

}  // namespace style
