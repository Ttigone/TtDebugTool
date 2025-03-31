#include "TtComboBoxStyle.h"

#include <QFontDatabase>
#include <QPainterPath>

#include "ui/TtTheme.h"

namespace style {

style::TtComboBoxStyle::TtComboBoxStyle(QStyle* style) {
  pExpandIconRotate_ = 0;
  pExpandMarkWidth_ = 0;
  theme_mode_ = Ui::tTheme->getThemeMode();
  connect(Ui::tTheme, &Ui::TtTheme::themeModeChanged, this,
          [=](TtThemeType::ThemeMode themeMode) { theme_mode_ = themeMode; });
}

TtComboBoxStyle::~TtComboBoxStyle() {}

void TtComboBoxStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption* option,
                                    QPainter* painter,
                                    const QWidget* widget) const {
  switch (element) {
    case QStyle::PE_Widget: {
      return;
    }
#ifndef Q_OS_WIN
    case PE_PanelMenu: {
      return;
    }
    case PE_IndicatorArrowDown: {
      return;
    }
#endif
    default: {
      break;
    }
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void TtComboBoxStyle::drawControl(ControlElement element,
                                  const QStyleOption* option, QPainter* painter,
                                  const QWidget* widget) const {
  switch (element) {
    case QStyle::CE_ComboBoxLabel: {
      return;
    }
    case QStyle::CE_ShapedFrame: {
      //container区域
      if (widget->objectName() == "TtComboBoxContainer") {
        QRect viewRect = option->rect;
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing |
                                QPainter::TextAntialiasing);
        Ui::tTheme->drawEffectShadow(painter, viewRect, shadow_border_width_,
                                     6);
        QRect foregroundRect(viewRect.x() + shadow_border_width_, viewRect.y(),
                             viewRect.width() - 2 * shadow_border_width_,
                             viewRect.height() - shadow_border_width_);
        painter->setPen(Ui::TtThemeColor(theme_mode_, PopupBorder));
        painter->setBrush(Ui::TtThemeColor(theme_mode_, PopupBase));
        painter->drawRoundedRect(foregroundRect, 3, 3);
        painter->restore();
      }
      return;
    }
    case QStyle::CE_ItemViewItem: {
      //覆盖高亮
      if (const QStyleOptionViewItem* vopt =
              qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
        int margin = 2;
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing |
                                QPainter::SmoothPixmapTransform);
        painter->setPen(Qt::NoPen);
        QPainterPath path;
        QRect optionRect = option->rect;
        optionRect.adjust(margin, margin, -margin, -margin);
#ifndef Q_OS_WIN
        optionRect.adjust(6, 0, -6, 0);
#endif
        path.addRoundedRect(optionRect, 5, 5);
        if (option->state & QStyle::State_Selected) {
          if (option->state & QStyle::State_MouseOver) {
            // 选中时覆盖
            painter->setBrush(
                Ui::TtThemeColor(theme_mode_, BasicSelectedHoverAlpha));
            painter->drawPath(path);
          } else {
            // 选中
            painter->setBrush(
                Ui::TtThemeColor(theme_mode_, BasicSelectedAlpha));
            painter->drawPath(path);
          }
          //选中Mark
          painter->setPen(Qt::NoPen);
          painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryNormal));
          painter->drawRoundedRect(
              QRectF(optionRect.x() + 3,
                     optionRect.y() + optionRect.height() * 0.2, 3,
                     optionRect.height() - +optionRect.height() * 0.4),
              2, 2);
        } else {
          if (option->state & QStyle::State_MouseOver) {
            // 覆盖时颜色
            painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHoverAlpha));
            painter->drawPath(path);
          }
        }
        // 文字绘制
        // 文本框 左右内边距 10
        QRect textRect(option->rect.x() + 10, option->rect.y(),
                       option->rect.width() - 10, option->rect.height());

        QString elideText(painter->fontMetrics().elidedText(
            vopt->text, Qt::ElideRight, textRect.width()));

        painter->setPen(Ui::TtThemeColor(theme_mode_, BasicText));
        // painter->drawText(
        //     QRect(option->rect.x() + 10, option->rect.y(),
        //           option->rect.width() - 10, option->rect.height()),
        //     Qt::AlignVCenter, vopt->text);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                          elideText);
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

void TtComboBoxStyle::drawComplexControl(ComplexControl control,
                                         const QStyleOptionComplex* option,
                                         QPainter* painter,
                                         const QWidget* widget) const {
  switch (control) {
    case QStyle::CC_ComboBox: {
      //主体显示绘制
      if (const QStyleOptionComboBox* copt =
              qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        painter->save();
        painter->setRenderHints(QPainter::SmoothPixmapTransform |
                                QPainter::Antialiasing |
                                QPainter::TextAntialiasing);
        //背景绘制
        bool isEnabled = copt->state.testFlag(QStyle::State_Enabled);
        painter->setPen(Ui::TtThemeColor(theme_mode_, BasicBorder));
        painter->setBrush(isEnabled
                              ? copt->state.testFlag(QStyle::State_MouseOver)
                                    ? Ui::TtThemeColor(theme_mode_, BasicHover)
                                    : Ui::TtThemeColor(theme_mode_, BasicBase)
                              : Ui::TtThemeColor(theme_mode_, BasicDisable));
        // 框架
        // 高度缩小 10 才正常
        // qDebug() << "1: " << copt->rect;
        // qDebug() << subControlRect(CC_ComboBox, copt, SC_ComboBoxFrame, widget);
        QRect comboBoxRect = copt->rect;
        // QRect comboBoxRect =
        //     subControlRect(CC_ComboBox, copt, SC_ComboBoxFrame, widget);
        // 边距左 1
        comboBoxRect.adjust(shadow_border_width_, 1, -shadow_border_width_, -1);
        painter->drawRoundedRect(comboBoxRect, 3, 3);
        // 底边线绘制
        painter->setPen(Ui::TtThemeColor(theme_mode_, BasicBaseLine));
        painter->drawLine(comboBoxRect.x() + 3,
                          comboBoxRect.y() + comboBoxRect.height(),
                          comboBoxRect.x() + comboBoxRect.width() - 3,
                          comboBoxRect.y() + comboBoxRect.height());

        //文字绘制
        // QRect textRect = subControlRect(QStyle::CC_ComboBox, copt,
        //                                 QStyle::SC_ScrollBarSubLine, widget);
        // QRect textRect = subControlRect(QStyle::CC_ComboBox, copt,
        //                                 QStyle::SC_ComboBoxEditField, widget);
        QRect textRect = subControlRect(QStyle::CC_ComboBox, copt,
                                        QStyle::SC_ComboBoxEditField, widget);

        QString elideText = painter->fontMetrics().elidedText(
            copt->currentText, Qt::ElideRight, textRect.width());

        painter->setPen(isEnabled
                            ? Ui::TtThemeColor(theme_mode_, BasicText)
                            : Ui::TtThemeColor(theme_mode_, BasicTextDisable));
        // painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
        //                   copt->currentText);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                          elideText);
        //展开指示器绘制
        painter->setPen(Qt::NoPen);
        painter->setBrush(Ui::TtThemeColor(theme_mode_, PrimaryNormal));
        painter->drawRoundedRect(
            QRectF(comboBoxRect.center().x() - pExpandMarkWidth_,
                   comboBoxRect.y() + comboBoxRect.height() - 3,
                   pExpandMarkWidth_ * 2, 3),
            2, 2);
        // 展开图标绘制
        QRect expandIconRect = subControlRect(
            QStyle::CC_ComboBox, copt, QStyle::SC_ScrollBarAddPage, widget);
        if (expandIconRect.isValid()) {
          int fontId = QFontDatabase::addApplicationFont(
              ":/font/fontawesome-webfont.ttf");
          QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
          QFont iconFont(family);
          iconFont.setPixelSize(15);
          painter->setFont(iconFont);
          painter->setPen(
              isEnabled ? Ui::TtThemeColor(theme_mode_, BasicText)
                        : Ui::TtThemeColor(theme_mode_, BasicTextDisable));
          painter->translate(
              expandIconRect.x() + (qreal)expandIconRect.width() / 2,
              expandIconRect.y() + (qreal)expandIconRect.height() / 2);
          painter->rotate(pExpandIconRotate_);
          painter->translate(
              -expandIconRect.x() - (qreal)expandIconRect.width() / 2,
              -expandIconRect.y() - (qreal)expandIconRect.height() / 2);
          // 向下的箭头
          // painter->drawText(expandIconRect, Qt::AlignCenter,
          //                   QChar((unsigned short)TtIconType::AngleDown));
          // painter->drawText(expandIconRect, Qt::AlignCenter, QChar('X'));
          painter->drawText(expandIconRect, Qt::AlignCenter, QChar(0xf106));
          painter->restore();
        }
      }
      return;
    }
    default: {
      break;
    }
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QRect TtComboBoxStyle::subControlRect(ComplexControl cc,
                                      const QStyleOptionComplex* opt,
                                      SubControl sc,
                                      const QWidget* widget) const {
  // switch (cc) {
  //   case QStyle::CC_ComboBox: {
  //     switch (sc) {
  //       case QStyle::SC_ScrollBarSubLine: {
  //         //文字区域
  //         QRect textRect = QProxyStyle::subControlRect(cc, opt, sc, widget);
  //         textRect.setLeft(16);
  //         textRect.setRight(textRect.right() - 15);
  //         return textRect;
  //       }
  //       case QStyle::SC_ScrollBarAddPage: {
  //         //展开图标区域
  //         QRect expandIconRect =
  //             QProxyStyle::subControlRect(cc, opt, sc, widget);
  //         expandIconRect.setLeft(expandIconRect.left() - 25);
  //         return expandIconRect;
  //       }
  //       default: {
  //         break;
  //       }
  //     }
  //     break;
  //   }
  //   default: {
  //     break;
  //   }
  // }
  if (cc == QStyle::CC_ComboBox) {
    const QStyleOptionComboBox* cb =
        qstyleoption_cast<const QStyleOptionComboBox*>(opt);
    QRect rect = QProxyStyle::subControlRect(cc, opt, sc, widget);

    switch (sc) {
      case QStyle::SC_ComboBoxFrame:  // 主框架区域
        return rect.adjusted(shadow_border_width_, shadow_border_width_,
                             -shadow_border_width_, -shadow_border_width_);

      case QStyle::SC_ComboBoxEditField: {
        // 文本区域
        // QRect textRect = QProxyStyle::subControlRect(cc, opt, sc, widget);
        // textRect.setLeft(16);
        // textRect.setRight(textRect.right() - 15);
        // return textRect;
        // rect.setLeft(16);                                // 文本起始区域 16
        // rect = rect.adjusted(16, 0, -20, 0);  // 右侧留出30px给图标
        // 左边距 10
        return rect.adjusted(10, 0, -10, 0);
      }
      case QStyle::SC_ComboBoxArrow:  // 箭头区域
        return QRect(rect.right() - 28, rect.top() + (rect.height() - 16) / 2,
                     16, 16);

      default:
        return rect;
    }
  }

  return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

QSize TtComboBoxStyle::sizeFromContents(ContentsType type,
                                        const QStyleOption* option,
                                        const QSize& size,
                                        const QWidget* widget) const {
  switch (type) {
    case QStyle::CT_ItemViewItem: {
      // 子项
      QSize itemSize =
          QProxyStyle::sizeFromContents(type, option, size, widget);
      itemSize.setHeight(35);
      return itemSize;
    }
    case QStyle::CT_ComboBox: {
      const QStyleOptionComboBox* cb =
          qstyleoption_cast<const QStyleOptionComboBox*>(option);
      QSize comboBoxSize =
          QProxyStyle::sizeFromContents(type, option, size, widget);
      // comboBoxSize.setWidth(comboBoxSize.width() + 26);
      // comboBoxSize += QSize(2 * shadow_border_width_, 2 * shadow_border_width_);
      // 根据字体大小动态调整宽度
      if (option) {
        QFontMetrics fm(option->fontMetrics);
        int textWidth = fm.horizontalAdvance(cb->currentText);

        comboBoxSize.setWidth(qMax(textWidth + 30, 50));
        // comboBoxSize.setWidth(
        //     fm.horizontalAdvance(
        //         static_cast<const QStyleOptionComboBox*>(option)->currentText) +
        //     2 * combo_margin_ + 20);  // 20为图标区域固定宽度
      }
      return comboBoxSize;
    }
    default: {
      break;
    }
  }
  return QProxyStyle::sizeFromContents(type, option, size, widget);
}

}  // namespace style
