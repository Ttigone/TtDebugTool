#include "TtLineEditStyle.h"

#include <QPainterPath>

#include "ui/TtTheme.h"

namespace style {

TtLineEditStyle::TtLineEditStyle(QStyle* style) {
  theme_mode_ = Ui::tTheme->getThemeMode();
  connect(Ui::tTheme, &Ui::TtTheme::themeModeChanged, this,
          [=](TtThemeType::ThemeMode themeMode) { theme_mode_ = themeMode; });
}

TtLineEditStyle::~TtLineEditStyle() {}

void TtLineEditStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption* option,
                                    QPainter* painter,
                                    const QWidget* widget) const {
  switch (element) {
    case PE_PanelLineEdit: {
      if (const QStyleOptionFrame* fopt =
              qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        QRect lineEditRect = fopt->rect;
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        // 边框绘制
        painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBorder));
        painter->drawRoundedRect(lineEditRect, 6, 6);
        //  背景绘制
        if (fopt->state & QStyle::State_HasFocus) {
          painter->setBrush(Ui::TtThemeColor(theme_mode_, DialogBase));
        } else if (fopt->state & QStyle::State_MouseOver) {
          painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHover));
        } else {
          painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBase));
        }
        painter->drawRoundedRect(
            QRectF(lineEditRect.x() + 1.5, lineEditRect.y() + 1.5,
                   lineEditRect.width() - 3, lineEditRect.height() - 3),
            // 6, 6);
            0, 0);

        // 底边线绘制
        qreal bottomY = lineEditRect.height() - 1.5;  // 高度

        painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHemline));
        QPainterPath path;
        path.moveTo(6, lineEditRect.height());
        path.lineTo(lineEditRect.width() - 6, lineEditRect.height());

        path.arcTo(QRectF(lineEditRect.width() - 12, lineEditRect.height() - 12,
                          12, 12),
                   -90, 45);

        path.lineTo(6 - 3 * std::sqrt(2),
                    lineEditRect.height() - (6 - 3 * std::sqrt(2)));

        path.arcTo(QRectF(0, lineEditRect.height() - 12, 12, 12), 270, 45);
        path.closeSubpath();
        painter->drawPath(path);
        painter->restore();
      }
      return;
    }
    default: {
      break;
    }
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

// void TtLineEditStyle::drawPrimitive(PrimitiveElement element,
//                                     const QStyleOption* option,
//                                     QPainter* painter,
//                                     const QWidget* widget) const {
//   switch (element) {
//     case PE_PanelLineEdit: {
//       if (const QStyleOptionFrame* fopt =
//               qstyleoption_cast<const QStyleOptionFrame*>(option)) {
//         QRect lineEditRect = fopt->rect;
//         painter->save();
//         painter->setRenderHints(QPainter::Antialiasing);
//         painter->setPen(Qt::NoPen);

//         // 绘制外边框
//         painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBorder));
//         painter->drawRoundedRect(lineEditRect, 6, 6);

//         // 绘制背景
//         QColor bgColor;
//         if (fopt->state & QStyle::State_HasFocus) {
//           bgColor = Ui::TtThemeColor(theme_mode_, DialogBase);
//         } else if (fopt->state & QStyle::State_MouseOver) {
//           bgColor = Ui::TtThemeColor(theme_mode_, BasicHover);
//         } else {
//           bgColor = Ui::TtThemeColor(theme_mode_, BasicBase);
//         }
//         painter->setBrush(bgColor);
//         painter->drawRoundedRect(
//             lineEditRect.adjusted(1, 1, -1, -1),  // 向内缩进1像素避免覆盖边框
//             0, 0                                  // 背景直角
//         );

//         // 仅在焦点状态绘制动态底边线
//         if (fopt->state & QStyle::State_HasFocus) {
//           painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHemline));

//           const qreal lineHeight = 2.0;  // 底边线高度
//           const qreal radius = 6.0;      // 圆角半径

//           QPainterPath path;
//           // 关键点：所有坐标基于实时宽度动态计算
//           const qreal startX = radius;
//           const qreal endX = lineEditRect.width() - radius;
//           const qreal bottomY = lineEditRect.height() - lineHeight / 2;

//           // 中间直线部分（自动跟随宽度变化）
//           path.moveTo(startX, bottomY);
//           path.lineTo(endX, bottomY);

//           // 右侧半圆
//           QRectF rightArcRect(
//               lineEditRect.width() - 2 * radius,  // x: 右侧边缘向左退 2*radius
//               bottomY - radius,                   // y: 向上偏移 radius
//               2 * radius,                         // 宽度
//               2 * radius                          // 高度
//           );
//           path.arcTo(rightArcRect, -90, 180);  // 从-90度开始画180度的圆弧

//           // 左侧半圆
//           QRectF leftArcRect(0,                 // x: 左侧起点
//                              bottomY - radius,  // y: 同上
//                              2 * radius, 2 * radius);
//           path.arcTo(leftArcRect, 90, 180);  // 从90度开始画180度的圆弧

//           painter->drawPath(path);
//         }

//         painter->restore();
//       }
//       return;
//     }
//     default:
//       QProxyStyle::drawPrimitive(element, option, painter, widget);
//   }
// }

// void TtLineEditStyle::drawPrimitive(PrimitiveElement element,
//                                     const QStyleOption* option,
//                                     QPainter* painter,
//                                     const QWidget* widget) const {
//   switch (element) {
//     case PE_PanelLineEdit: {
//       if (const QStyleOptionFrame* fopt =
//               qstyleoption_cast<const QStyleOptionFrame*>(option)) {
//         QRect lineEditRect = fopt->rect;
//         painter->save();
//         painter->setRenderHints(QPainter::Antialiasing);
//         painter->setPen(Qt::NoPen);

//         // 1. 绘制外边框（圆角）
//         painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicBorder));
//         painter->drawRoundedRect(lineEditRect, 6, 6);

//         // 2. 绘制背景（圆角，但比外边框小1像素）
//         QColor bgColor;
//         if (fopt->state & QStyle::State_HasFocus) {
//           bgColor = Ui::TtThemeColor(theme_mode_, DialogBase);
//         } else if (fopt->state & QStyle::State_MouseOver) {
//           bgColor = Ui::TtThemeColor(theme_mode_, BasicHover);
//         } else {
//           bgColor = Ui::TtThemeColor(theme_mode_, BasicBase);
//         }
//         painter->setBrush(bgColor);
//         // 关键修改：背景保持圆角，但向内缩进1像素
//         painter->drawRoundedRect(lineEditRect.adjusted(1, 1, -1, -1), 5, 5);

//         // 3. 仅在焦点状态绘制动态底边线
//         if (fopt->state & QStyle::State_HasFocus) {
//           painter->setBrush(Ui::TtThemeColor(theme_mode_, BasicHemline));

//           const qreal lineHeight = 2.0;  // 底边线高度
//           const qreal radius = 6.0;      // 圆角半径

//           // 关键点：底边线位于背景底部，且不超出背景区域
//           const qreal bottomY = lineEditRect.height() - lineHeight -
//                                 1;  // 向下偏移1像素（边框内）
//           const qreal startX = radius;
//           const qreal endX = lineEditRect.width() - radius;

//           QPainterPath path;
//           path.moveTo(startX, bottomY);
//           path.lineTo(endX, bottomY);

//           // 右侧圆弧（动态跟随宽度）
//           QRectF rightArcRect(
//               lineEditRect.width() - 2 * radius,  // X坐标：右侧边缘 - 2*radius
//               bottomY - radius,  // Y坐标：与底边线对齐
//               2 * radius, 2 * radius);
//           path.arcTo(rightArcRect, -90, 180);

//           // 左侧圆弧
//           QRectF leftArcRect(0,                 // X坐标：左侧起点
//                              bottomY - radius,  // Y坐标同上
//                              2 * radius, 2 * radius);
//           path.arcTo(leftArcRect, 90, 180);

//           painter->drawPath(path);
//         }

//         painter->restore();
//       }
//       return;
//     }
//     default:
//       QProxyStyle::drawPrimitive(element, option, painter, widget);
//   }
// }

}  // namespace style
