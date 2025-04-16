#include "TtColorButton.h"

#include <QLineEdit>

TtColorButton::TtColorButton(QWidget* parent)
    : QWidget(parent),
      is_pressed_(false),
      color_size_(16, 16),  // 默认颜色块大小
      is_checked_(false),
      enable_hold_to_check_(false),
      is_hovered_(false),
      current_color_(Qt::blue),
      normal_color_(Qt::lightGray) {}

TtColorButton::TtColorButton(const QColor& color, const QString& text,
                             QWidget* parent)
    : TtColorButton(parent) {
  current_color_ = color;
  text_ = text;
}

TtColorButton::~TtColorButton() {}

void TtColorButton::setColors(const QColor& color) {
  current_color_ = color;
  update();
}

QString TtColorButton::getText() const {
  return text_;
}

QColor TtColorButton::getColor() const {
  // 背景色
  return normal_color_;
}

void TtColorButton::setText(const QString& text) {
  text_ = text;
  update();
}

QColor TtColorButton::getHoverBackgroundColor() const {
  return normal_color_;
}

void TtColorButton::setHoverBackgroundColor(const QColor& color) {
  normal_color_ = color;
  update();
}

void TtColorButton::setCheckBlockColor(const QColor& color) {
  check_block_color_ = color;
  update();
}

QColor TtColorButton::getCheckBlockColor() const {
  return check_block_color_;
}

bool TtColorButton::isChecked() const {
  return is_checked_;
}

void TtColorButton::setChecked(bool checked) {
  if (is_checked_ != checked) {
    is_checked_ = checked;
    emit toggled(is_checked_);
    update();
  }
}

void TtColorButton::setEnableHoldToCheck(bool enable) {
  enable_hold_to_check_ = enable;
}

void TtColorButton::setEnable(bool enabled) {
  setEnabled(enabled);
  update();
}

void TtColorButton::modifyText() {
  if (rename_editor_) {
    return;
  }

  original_text_ = text_;

  // 创建编辑器
  rename_editor_ = new QLineEdit(this);
  rename_editor_->setText(text_);
  rename_editor_->setGeometry(
      rect().adjusted(color_size_.width() + 10, 0, -10, 0));  // 对齐文本区域
  rename_editor_->installEventFilter(this);
  rename_editor_->setFont(font());  // 保持字体与控件一致
  rename_editor_->setStyleSheet(
      "QLineEdit {"
      "  border: 0px;"
      "  background: white;"
      "  padding: 2px 5px;"
      "  background: transparent;"
      "  padding: 0px;"
      "}");

  rename_editor_->show();
  rename_editor_->setFocus();

  connect(rename_editor_, &QLineEdit::editingFinished, this, [=]() {
    if (rename_editor_->text() != original_text_) {
      setText(rename_editor_->text());
      emit textChanged(rename_editor_->text());
    }
    clearupEditor();
  });
}

void TtColorButton::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 绘制背景（取消默认的蓝底色）
  QRect bgRect = rect();
  if (!isEnabled()) {
    painter.fillRect(bgRect, QColor(220, 220, 220));
  } else {
    // 仅在有交互状态时绘制背景
    // if (is_pressed_ || is_checked_) {
    // painter.fillRect(bgRect, current_color_.lighter(120));
    // } else if
    if (is_hovered_) {
      QColor hoverBg = normal_color_;
      hoverBg.setAlpha(100);  // 半透明效果
      painter.fillRect(bgRect, hoverBg);
    }
  }

  QRect cbRect = checkBlockRect();
  painter.setPen(Qt::NoPen);
  painter.setBrush(check_block_color_);
  painter.drawRoundedRect(cbRect, 3, 3);

  if (!rename_editor_ || !rename_editor_->isVisible()) {
    // 当编辑器没有显示时, 绘制文本, 避免重复显示
    painter.setPen(isEnabled() ? palette().text().color() : Qt::gray);
    QRect textRect(cbRect.right() + 10, 0, width() - cbRect.width() - 16,
                   height());
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text_);
  }

  if (is_checked_) {
    painter.setPen(Qt::white);  // 选中时的标记颜色
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(cbRect, Qt::AlignCenter, "✓");  // 可改成你想要的字符
  }
}

void TtColorButton::enterEvent(QEnterEvent* event) {
  is_hovered_ = true;
  update();
  QWidget::enterEvent(event);
}

void TtColorButton::leaveEvent(QEvent* event) {
  is_hovered_ = false;
  update();
  QWidget::leaveEvent(event);
}

void TtColorButton::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && isEnabled()) {
    checkbox_pressed_ = checkBlockRect().contains(event->pos());  // false
    is_pressed_ = true;
    update();
  }
  QWidget::mousePressEvent(event);
}

void TtColorButton::mouseReleaseEvent(QMouseEvent* event) {
  if (ignore_next_release_) {
    ignore_next_release_ = false;  // 只忽略一次
    is_pressed_ = true;
  }

  if (event->button() == Qt::LeftButton && is_pressed_ && isEnabled()) {
    is_pressed_ = false;
    if (ignore_next_release_ == false) {
      if (checkbox_pressed_ == false) {
        setChecked(is_checked_);
      } else {
        setChecked(!is_checked_);
      }
    }
    emit clicked();
    update();
  }
  QWidget::mouseReleaseEvent(event);
}

void TtColorButton::mouseDoubleClickEvent(QMouseEvent* event) {
  if (checkBlockRect().contains(event->pos())) {  // 包含
    ignore_next_release_ = true;
    event->accept();
    return;
  }
  if (event->button() == Qt::LeftButton && isEnabled()) {
    modifyText();
    event->accept();
  } else {
    QWidget::mouseDoubleClickEvent(event);
  }
}

bool TtColorButton::eventFilter(QObject* watched, QEvent* event) {
  if (watched == rename_editor_ && event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      clearupEditor();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

QSize TtColorButton::sizeHint() const {
  QFontMetrics fm(font());
  int textWidth = fm.horizontalAdvance(text_);
  int totalWidth = color_size_.width() + textWidth + 20;  // 左右留白
  int height = qMax(color_size_.height(), fm.height()) + 8;
  return QSize(totalWidth, height);
}

void TtColorButton::clearupEditor() {
  if (rename_editor_) {
    rename_editor_->deleteLater();
    rename_editor_ = nullptr;
    update();  // 强制重绘显示原始文本
  }
}

QRect TtColorButton::checkBlockRect() const {
  // 计算颜色块绘制的区域，保持与 paintEvent 中一致
  int margin = 6;
  int y = (height() - color_size_.height()) / 2;
  return QRect(margin, y, color_size_.width(), color_size_.height());
}
