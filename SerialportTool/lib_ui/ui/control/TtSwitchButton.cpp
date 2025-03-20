#include "ui/control/TtSwitchButton.h"

#include <QMouseEvent>

namespace Ui {

TtSwitchButton::TtSwitchButton(QWidget* parent)
    : QWidget(parent),
      checked(false),
      m_knobPosition(2.0),
      animation(new QPropertyAnimation(this, "knobPosition", this)),
      text_(""),
      textPosition_(TextLeft) {
  updateLayout();
  animation->setDuration(200);
  animation->setEasingCurve(QEasingCurve::OutBounce);
}

// TtSwitchButton::TtSwitchButton(const QString& text, QWidget* parent)
//     : Ui::TtSwitchButton(parent) {
//   setText(text);
// }

TtSwitchButton::TtSwitchButton(const QString& text, QWidget* parent)
    : QWidget(parent),
      checked(false),
      m_knobPosition(2.0),
      animation(new QPropertyAnimation(this, "knobPosition", this)),
      text_(text),
      textPosition_(TextLeft) {
  updateLayout();
  animation->setDuration(200);
  animation->setEasingCurve(QEasingCurve::OutBounce);
}

TtSwitchButton::~TtSwitchButton() {}

bool TtSwitchButton::isChecked() const {
  return checked;
}

void TtSwitchButton::setText(const QString& text) {
  if (text_ != text) {
    text_ = text;
    // QFontMetrics fm(font());
    // int textWidth = fm.horizontalAdvance(text);
    // int totalWidth = textWidth + 10 /* margin */ + 36 /* switch width */;
    // setFixedSize(totalWidth, 20);
    // update();
    updateLayout();
  }
}

QString TtSwitchButton::text() const {
  return text_;
}

void TtSwitchButton::setTextPosition(TextPosition position) {
  if (textPosition_ != position) {
    textPosition_ = position;
    updateLayout();
  }
}

TtSwitchButton::TextPosition TtSwitchButton::textPosition() const {
  return textPosition_;
}

void TtSwitchButton::setChecked(bool checked_) {
  //   if (checked != checked_) {
  //     checked = checked_;
  //     animation->stop();
  //     if (checked) {
  //       animation->setStartValue(m_knobPosition);
  //       animation->setEndValue(width() - height() + 2.0);  // Right position
  //     } else {
  //       animation->setStartValue(m_knobPosition);
  //       animation->setEndValue(2.0);  // Left position
  //     }
  //     animation->start();
  //     emit toggled(checked);
  //   }
  if (checked != checked_) {
    checked = checked_;
    animation->stop();

    // 计算滑块直径（高度固定为 20，滑块上下边距各 2）
    const qreal knobDiameter = switchRect.height() - 4;

    // 计算正确终点位置（右侧位置 = 开关宽度 - 滑块直径 - 右侧边距）
    const qreal endValue =
        checked ? (switchRect.width() - knobDiameter - 2.0) : 2.0;

    animation->setStartValue(m_knobPosition);
    animation->setEndValue(endValue);
    animation->start();
    emit toggled(checked);
  }
}

qreal TtSwitchButton::knobPosition() const {
  return m_knobPosition;
}

void TtSwitchButton::setKnobPosition(qreal position) {
  m_knobPosition = position;
  update();
}

void TtSwitchButton::updateLayout() {
  QFontMetrics fm(font());
  int textWidth = text_.isEmpty() ? 0 : fm.horizontalAdvance(text_);
  int switchWidth = 36;  // 开关固定宽度
  int totalWidth = textWidth + switchWidth + spacing;
  int fixedHeight = 20;  // 固定高度

  // 根据文本位置调整布局
  if (textPosition_ == TextRight) {
    textRect = QRect(switchWidth + spacing, 0, textWidth, fixedHeight);
    switchRect = QRect(0, 0, switchWidth, fixedHeight);
  } else {
    textRect = QRect(0, 0, textWidth, fixedHeight);
    switchRect = QRect(textWidth + spacing, 0, switchWidth, fixedHeight);
  }

  setFixedSize(totalWidth, fixedHeight);
  update();
}

void TtSwitchButton::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 绘制文本
  if (!text_.isEmpty()) {
    painter.setPen(Qt::black);
    painter.setFont(font());
    painter.drawText(
        textRect,
        Qt::AlignVCenter |
            (textPosition_ == TextLeft ? Qt::AlignLeft : Qt::AlignRight),
        text_);
  }

  // 绘制开关背景
  QColor bgColor = checked ? QColor("#4cd964") : QColor("#e5e5e5");
  painter.setBrush(bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(switchRect, height() / 2, height() / 2);

  // 绘制滑块
  QColor knobColor = QColor("#ffffff");
  painter.setBrush(knobColor);
  qreal knobDiameter = switchRect.height() - 4;
  QRectF knobRect(switchRect.left() + m_knobPosition, switchRect.top() + 2,
                  knobDiameter, knobDiameter);
  painter.drawEllipse(knobRect);
}

void TtSwitchButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    // 判断点击区域是否在开关区域内
    if (switchRect.contains(event->pos()) || textRect.contains(event->pos())) {
      setChecked(!checked);
    }
  }
  QWidget::mousePressEvent(event);
}

void TtSwitchButton::resizeEvent(QResizeEvent* event) {
  Q_UNUSED(event)
  // 保持高度固定，防止布局异常
  if (height() != 20)
    setFixedHeight(20);
  updateLayout();
}

}  // namespace Ui
