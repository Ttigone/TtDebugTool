#include "ui/widgets/collapsible_panel.h"

namespace Ui {


DrawerButton::DrawerButton(const QString& title, QWidget* parent): QPushButton(title, parent), m_arrowRotation(0) {
  setStyleSheet(R"(
    DrawerButton {
        text-align: left;
        padding-left: 20px;
        background-color: transparent;
        border: none;
    }
    DrawerButton:pressed {
        background-color: rgba(0, 0, 0, 0.1); /* 按下时轻微变暗 */
    }
    DrawerButton:hover {
        background-color: rgba(0, 0, 0, 0.05); /* 悬停时轻微变亮 */
    }
)");
  // setAttribute(Qt::WA_OpaquePaintEvent);  // 避免背景重绘
  setAttribute(Qt::WA_NoSystemBackground);  // 禁用系统背景
  initializeArrow();
}

qreal DrawerButton::arrowRotation() const { return m_arrowRotation; }

void DrawerButton::setArrowRotation(qreal rotation) {
  if (qFuzzyCompare(m_arrowRotation, rotation))
    return;
  m_arrowRotation = rotation;
  update();
}

void DrawerButton::paintEvent(QPaintEvent* event) {
  QPushButton::paintEvent(event);

  QPainter painter(this);
  painter.setRenderHints(QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform);
  QStyleOptionButton option;
  initStyleOption(&option);

  painter.save();
  // 计算箭头位置
  QRect arrowRect(10, height() / 2 - 8, 16, 16);  // 左侧箭头的矩形区域

  // 旋转箭头
  painter.translate(arrowRect.center());
  painter.rotate(m_arrowRotation);
  painter.translate(-arrowRect.center());

  painter.setBrush(option.palette.buttonText());
  painter.setPen(Qt::NoPen);
  painter.drawPath(m_arrowPath.translated(arrowRect.topLeft()));

  painter.restore();
}

void DrawerButton::initializeArrow() {
  m_arrowPath.moveTo(0, 5);
  m_arrowPath.lineTo(8, 0);
  m_arrowPath.lineTo(16, 5);
  m_arrowPath.lineTo(8, 10);
  m_arrowPath.closeSubpath();
}

Drawer::Drawer(const QString& title, QWidget* contentWidget, QWidget* parent)
    : QWidget(parent), contentWidget(contentWidget), isOpen(false) {
  initializeLayout(title);
  initializeAnimations();
  connectSignals();
  contentWidget->installEventFilter(this);
}

bool Drawer::eventFilter(QObject* obj, QEvent* event) {
  if (obj == contentWidget && event->type() == QEvent::Resize) {
    updateAnimationEndValue();
  }
  return QWidget::eventFilter(obj, event);
}

void Drawer::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  if (isOpen) {
    updateAnimationEndValue();
  }
}

void Drawer::toggle() {
  isOpen = !isOpen;
  if (isOpen) {
    openDrawer();
  } else {
    closeDrawer();
  }
}

void Drawer::initializeLayout(const QString& title) {
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  toggleButton = new DrawerButton(title, this);
  toggleButton->setMinimumHeight(40);

  wrapperWidget = new QWidget(this);
  wrapperWidget->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Preferred);
  wrapperWidget->setMaximumHeight(0);

  QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapperWidget);
  wrapperLayout->setContentsMargins(0, 0, 0, 0);
  wrapperLayout->addWidget(contentWidget);

  mainLayout->addWidget(toggleButton);
  mainLayout->addWidget(wrapperWidget);
}

void Drawer::initializeAnimations() {
  animation = new QPropertyAnimation(wrapperWidget, "maximumHeight", this);
  animation->setDuration(200);
  animation->setEasingCurve(QEasingCurve::OutQuad);

  arrowAnimation =
      new QPropertyAnimation(toggleButton, "arrowRotation", this);
  arrowAnimation->setDuration(200);
  arrowAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void Drawer::connectSignals() {
  connect(toggleButton, &QPushButton::clicked, this, &Drawer::toggle);
  connect(animation, &QPropertyAnimation::finished, this,
          [this]() { wrapperWidget->setVisible(isOpen); });
}

void Drawer::openDrawer() {
  wrapperWidget->setVisible(true);  // 确保内容可见
  startAnimation(0, contentHeight());
  startArrowAnimation(0, 90);
}

void Drawer::closeDrawer() {
  startAnimation(contentHeight(), 0);
  startArrowAnimation(90, 0);
}

void Drawer::startAnimation(int start, int end) {
  animation->stop();
  animation->setStartValue(start);
  animation->setEndValue(end);
  animation->start();
}

void Drawer::startArrowAnimation(qreal start, qreal end) {
  arrowAnimation->stop();
  arrowAnimation->setStartValue(start);
  arrowAnimation->setEndValue(end);
  arrowAnimation->start();
}

void Drawer::updateAnimationEndValue() {
  if (isOpen && animation->endValue().toInt() != contentHeight()) {
    animation->setEndValue(contentHeight());
    if (animation->state() != QAbstractAnimation::Running) {
      wrapperWidget->setMaximumHeight(contentHeight());
    }
  }
}

int Drawer::contentHeight() const {
  return contentWidget->sizeHint().height() +
         wrapperWidget->layout()->contentsMargins().top() +
         wrapperWidget->layout()->contentsMargins().bottom();
}

} // namespace Ui

