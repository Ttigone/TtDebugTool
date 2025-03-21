#include "ui/control/TtPopUpDrawer.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QSignalTransition>

#include "ui/control/TtPopUpDrawer_p.h"

namespace Ui {

TtPopUpDrawerPrivate::TtPopUpDrawerPrivate(TtPopUpDrawer* q) : q_ptr(q) {}

TtPopUpDrawerPrivate::~TtPopUpDrawerPrivate() {}

void TtPopUpDrawerPrivate::init() {
  Q_Q(TtPopUpDrawer);
  widget_ = new TtPopUpDrawerWidget;
  state_machine_ = new TtPopUpDrawerStateMachine(widget_, q);
  window_ = new QWidget;  // 展示设置的窗口
  // width_ = 100;
  //width_ = 10;  // 弹出的宽度
  click_to_close_ = false;
  auto_raise_ = true;
  closed_ = true;
  overlay_ = false;
  direction_ = TtPopUpDirection::Right;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(QMargins());
  layout->addWidget(window_);

  // widget 是最底层的窗口, 上面覆盖了一个 window_, 用于显示外部设备的 widget
  widget_->setLayout(layout);
  // 这里的 16 是左右边距 8 ???
  //widget_->setFixedWidth(width_ + 16);

  // 设置父对象, user->drawer->drawer_widget
  widget_->setParent(q);

  // 启动状态机
  state_machine_->start();
  QCoreApplication::processEvents();
}

void TtPopUpDrawerPrivate::setClosed(bool value) {
  closed_ = value;
}

TtPopUpDrawer::TtPopUpDrawer(QWidget* parent)
    : TtOverlayWidget(parent), d_ptr(new TtPopUpDrawerPrivate(this)) {
  // 隐私类初始化函数
  d_func()->init();
  if (parent) {
    parent->installEventFilter(this);  // 监听父窗口事件
  }
}

TtPopUpDrawer::~TtPopUpDrawer() {}

void TtPopUpDrawer::setDrawerSize(int size) {
  Q_D(TtPopUpDrawer);
  if (size <= 0) {
    d->size_ = 0;
    return;
  }
  d->size_ = size;
  // 更新尺寸后强制重设布局
  // d->widget_->layout()->activate();
  d->state_machine_->updatePropertyAssignments();
  d->widget_->updateGeometry();
  // d->widget_->setFixedWidth(size);
  update();
}

int TtPopUpDrawer::drawerSize() const {
  Q_D(const TtPopUpDrawer);
  return d->size_;
}

void TtPopUpDrawer::setDrawerWidth(int width) {}

// void TtPopUpDrawer::setDrawerWidth(int width) {
//   Q_D(TtPopUpDrawer);

//   // 避免宽度为 0 时显示 Drawer
//   if (width <= 0) {
//     d->width_ = 0;
//     return;
//   }

//   d->width_ = width;
//   // 更新状态机
//   d->state_machine_->updatePropertyAssignments();
//   d->widget_->setFixedWidth(width + 16);
// }

// int TtPopUpDrawer::drawerWidth() const {
//   Q_D(const TtPopUpDrawer);
//   return d->width_;
// }

void TtPopUpDrawer::setDrawerLayout(QLayout* layout) {
  Q_D(TtPopUpDrawer);
  // 设置布局
  d->window_->setLayout(layout);
}

QLayout* TtPopUpDrawer::drawerLayout() const {
  Q_D(const TtPopUpDrawer);
  return d->window_->layout();
}

void TtPopUpDrawer::setClickOutsideToClose(bool state) {
  Q_D(TtPopUpDrawer);
  d->click_to_close_ = state;
}

bool TtPopUpDrawer::clickOutsideToClose() const {
  Q_D(const TtPopUpDrawer);
  return d->click_to_close_;
}

void TtPopUpDrawer::setAutoRaise(bool state) {
  Q_D(TtPopUpDrawer);
  d->auto_raise_ = state;
}

bool TtPopUpDrawer::autoRaise() const {
  Q_D(const TtPopUpDrawer);
  return d->auto_raise_;
}

void TtPopUpDrawer::setOverlayMode(bool value) {
  Q_D(TtPopUpDrawer);
  d->overlay_ = value;
  update();
}

bool TtPopUpDrawer::overlayMode() const {
  Q_D(const TtPopUpDrawer);
  return d->overlay_;
}

void TtPopUpDrawer::openDrawer() {
  Q_D(TtPopUpDrawer);
  // 发出信号, 展开 drawer
  emit d->state_machine_->signalOpen();
  qDebug() << "open";

  // 置顶???
  if (d->auto_raise_) {
    raise();
  }
  setAttribute(Qt::WA_TransparentForMouseEvents, false);
  setAttribute(Qt::WA_NoSystemBackground, false);
}

void TtPopUpDrawer::closeDrawer() {
  Q_D(TtPopUpDrawer);

  emit d->state_machine_->signalClose();
  emit willClose();

  if (d->overlay_) {
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
  }
}

bool TtPopUpDrawer::event(QEvent* event) {
  Q_D(TtPopUpDrawer);

  switch (event->type()) {
    case QEvent::Move:
    case QEvent::Resize:
      // 不是覆盖的模式
      if (!d->overlay_) {
        qDebug() << "region: " << d->widget_->rect();
        qDebug() << "geometry: " << d->widget_->geometry();
        auto widgetGeometry = d->widget_->geometry();
        // setMask(QRegion(d->widget_->rect()));
        setMask(QRegion(widgetGeometry));
      }
      break;
    case QEvent::ParentChange: {
      auto resizeEvent = static_cast<QResizeEvent*>(event);
      if (resizeEvent) {
        qDebug() << resizeEvent->size().width();
      }
      //if (resizeEvent) {
      //  qDebug() << resizeEvent->size().width();
      //}
    } break;
    default:
      break;
  }
  return TtOverlayWidget::event(event);
}

bool TtPopUpDrawer::eventFilter(QObject* obj, QEvent* event) {
  Q_D(TtPopUpDrawer);

  if (obj == parent()) {  // 监听父窗口事件
    if (event->type() == QEvent::Resize) {
      // 父窗口大小变化时强制更新抽屉
      QMetaObject::invokeMethod(
          this,
          [this, d]() {
            if (!d->closed_) {
              // 立即更新抽屉位置
              d->widget_->setOffset(QPoint(0, 0));
              // 更新状态机参数
              d->state_machine_->updatePropertyAssignments();
            }
          },
          Qt::QueuedConnection);
    }
  }
  return TtOverlayWidget::eventFilter(obj, event);

  switch (event->type()) {
    // 监听鼠标摁下
    case QEvent::MouseButtonPress: {
      //qDebug() << "detect click";
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent) {
        const bool canClose = d->click_to_close_ || d->overlay_;
        if (!d->widget_->geometry().contains(mouseEvent->pos()) && canClose) {
          // 鼠标不在 widget 上
          closeDrawer();
        }
      }
      break;
    }
    case QEvent::Move:
    case QEvent::Resize: {
      QLayout* lw = d->widget_->layout();
      if (lw && 16 != lw->contentsMargins().right()) {
        lw->setContentsMargins(0, 0, 16, 0);
      }
      break;
    }
    default:
      break;
  }
  return TtOverlayWidget::eventFilter(obj, event);
}
void TtPopUpDrawer::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)
  Q_D(TtPopUpDrawer);

  TtPopUpDrawer* parentDrawer = qobject_cast<TtPopUpDrawer*>(parent());
  if (!parentDrawer)
    return;

  // 获取实际可用区域
  QRect validRect = geometry().intersected(parentDrawer->rect());
  if (validRect != geometry()) {
    qWarning() << "Drawer position out of bounds, auto correcting...";
    setGeometry(validRect);  // 自动修正到合法区域
  }

  // 处于关闭状态
  if (!d->overlay_ || d->state_machine_->isInClosedState()) {
    return;
  }
  QPainter painter(this);
  // 透明度是动态 0 - 0.4
  // qDebug() << "opacity() " << d->state_machine_->opacity();
  painter.setOpacity(d->state_machine_->opacity());
  //qDebug() << "rect: " << rect();
  painter.fillRect(rect(), Qt::SolidPattern);
}

void TtPopUpDrawer::resizeEvent(QResizeEvent* event) {
  // Q_D(TtPopUpDrawer);
  // d->state_machine_->updatePropertyAssignments();
  // TtOverlayWidget::resizeEvent(event);
  Q_D(TtPopUpDrawer);

  // 确保状态机参数更新使用最新尺寸
  d->state_machine_->updatePropertyAssignments();

  // 如果抽屉处于打开状态，立即更新位置
  if (!d->closed_) {
    d->widget_->setOffset(QPoint(0, 0));  // 强制重设到正确位置
  }

  TtOverlayWidget::resizeEvent(event);
}

TtPopUpDrawerWidget::TtPopUpDrawerWidget(QWidget* parent)
    : TtOverlayWidget(parent), offset_(0, 0) {}

TtPopUpDrawerWidget::~TtPopUpDrawerWidget() {}

// void TtPopUpDrawerWidget::setOffset(int offset) {
void TtPopUpDrawerWidget::setOffset(QPoint offset) {
  // 平移
  // qDebug() << "offset: " << offset;
  offset_ = offset;

  TtPopUpDrawer* parentDrawer = qobject_cast<TtPopUpDrawer*>(parent());
  if (!parentDrawer) {
    return;
  }

  const int size = parentDrawer->drawerSize();
  const auto dir = parentDrawer->getDirection();
  qDebug() << "parent-width" << parentDrawer->width();
  qDebug() << "size: " << size;

  QRect newGeometry;
  switch (dir) {
    case TtPopUpDirection::Left:
      // newGeometry = QRect(-size + offset.x(), 0, size, parentDrawer->height());
      newGeometry = QRect(offset.x(), 0, size, parentDrawer->height());
      break;
    case TtPopUpDirection::Right:
      // newGeometry = QRect(parentDrawer->width() + offset.x(), 0, size,
      //                     parentDrawer->height());
      newGeometry = QRect(parentDrawer->width() - size + offset.x(), 0, size,
                          parentDrawer->height());
      break;
    case TtPopUpDirection::Top:
      // newGeometry = QRect(0, -size + offset.y(), parentDrawer->width(), size);
      newGeometry = QRect(0, offset.y(), parentDrawer->width(), size);
      break;
    case TtPopUpDirection::Bottom:
      // newGeometry = QRect(0, parentDrawer->height() + offset.y(),
      //                     parentDrawer->width(), size);
      newGeometry = QRect(0, parentDrawer->height() - size + offset.y(),
                          parentDrawer->width(), size);
      break;
  }
  qDebug() << "new: " << newGeometry;
  // setStyleSheet("background-color: blue");
  setGeometry(newGeometry);
  update();

  // QWidget* widget = parentWidget();
  // // 弹出界面是 this
  // this->setStyleSheet("background-color: blue");
  // if (widget) {
  //   // 向右平移 ???

  //   qDebug() << "set geometry: " << widget->rect().translated(offset, 0);
  //   setGeometry(widget->rect().translated(offset, 0));
  // }
  // update();
}

void TtPopUpDrawerWidget::paintEvent(QPaintEvent* event) {
  TtPopUpDrawer* parentDrawer = qobject_cast<TtPopUpDrawer*>(parent());
  if (!parentDrawer)
    return;

  TtPopUpDirection::PopUpDirection dir = parentDrawer->getDirection();
  // 根据方向绘制阴影和内容...

  // Q_UNUSED(event)

  QPainter painter(this);

  // QBrush brush;
  // brush.setStyle(Qt::SolidPattern);
  // brush.setColor(Qt::white);
  // painter.setBrush(brush);
  // painter.setPen(Qt::NoPen);

  // painter.drawRect(rect().adjusted(0, 0, -16, 0));

  // QLinearGradient gradient(QPointF(width() - 16, 0), QPointF(width(), 0));
  // gradient.setColorAt(0, QColor(0, 0, 0, 80));
  // gradient.setColorAt(0.5, QColor(0, 0, 0, 20));
  // gradient.setColorAt(1, QColor(0, 0, 0, 0));
  // painter.setBrush(QBrush(gradient));

  // painter.drawRect(width() - 16, 0, 16, height());
  // 主体绘制
  QRect contentRect;
  switch (dir) {
    case TtPopUpDirection::Left:
      contentRect = rect().adjusted(0, 0, -16, 0);
      break;
    case TtPopUpDirection::Right:
      contentRect = rect().adjusted(16, 0, 0, 0);
      break;
    case TtPopUpDirection::Top:
      contentRect = rect().adjusted(0, 0, 0, -16);
      break;
    case TtPopUpDirection::Bottom:
      contentRect = rect().adjusted(0, 16, 0, 0);
      break;
  }
  painter.fillRect(contentRect, Qt::white);

  // 阴影绘制
  QLinearGradient gradient;
  switch (dir) {
    case TtPopUpDirection::Left:
      gradient.setStart(contentRect.right(), 0);
      gradient.setFinalStop(contentRect.right() + 16, 0);
      break;
    case TtPopUpDirection::Right:
      gradient.setStart(contentRect.left() - 16, 0);
      gradient.setFinalStop(contentRect.left(), 0);
      break;
    case TtPopUpDirection::Top:
      gradient.setStart(0, contentRect.bottom());
      gradient.setFinalStop(0, contentRect.bottom() + 16);
      break;
    case TtPopUpDirection::Bottom:
      gradient.setStart(0, contentRect.top() - 16);
      gradient.setFinalStop(0, contentRect.top());
      break;
  }
  gradient.setColorAt(0, QColor(0, 0, 0, 80));
  gradient.setColorAt(0.5, QColor(0, 0, 0, 20));
  gradient.setColorAt(1, QColor(0, 0, 0, 0));
  painter.fillRect(rect(), gradient);
}

QRect TtPopUpDrawerWidget::overlayGeometry() const {
  // return TtOverlayWidget::overlayGeometry().translated(offset_.x(), 0);
  return TtOverlayWidget::overlayGeometry();
}

TtPopUpDrawerStateMachine::TtPopUpDrawerStateMachine(
    TtPopUpDrawerWidget* drawer, TtPopUpDrawer* parent)
    // 两个形参都是 widget
    : QStateMachine(parent),
      drawer_(drawer),
      main_(parent),
      opening_state_(new QState),
      opened_state_(new QState),
      closing_state_(new QState),
      closed_state_(new QState),
      opacity_(0) {
  // 向状态机添加状态
  addState(opening_state_);
  addState(opened_state_);
  addState(closing_state_);
  addState(closed_state_);

  // 设置状态机初始状态
  setInitialState(closed_state_);

  QSignalTransition* transition;
  QPropertyAnimation* animation;

  // 当 this 发送 signalOpen 时
  transition = new QSignalTransition(this, SIGNAL(signalOpen()));
  // 转换目标状态
  transition->setTargetState(opening_state_);
  // 发出 signalOpen 信号时, closed -> opening
  closed_state_->addTransition(transition);

  // 添加展开动画
  // drawer 才有 offser 属性
  animation = new QPropertyAnimation(drawer, "offset", this);
  animation->setDuration(220);
  animation->setEasingCurve(QEasingCurve::OutCirc);
  transition->addAnimation(animation);

  // 添加透明动画
  // this 有 opacity 属性
  animation = new QPropertyAnimation(this, "opacity", this);
  animation->setDuration(220);
  transition->addAnimation(animation);

  // 监听 animation 的状态
  transition = new QSignalTransition(animation, SIGNAL(finished()));
  // 设置目标状态
  transition->setTargetState(opened_state_);
  // opening -> opened  动画完成时, 由过程到结果
  opening_state_->addTransition(transition);

  // drawer 关闭执行
  transition = new QSignalTransition(this, SIGNAL(signalClose()));
  // opening -> closing ??? 为什么不是 opened -> closing
  transition->setTargetState(closing_state_);
  opening_state_->addTransition(transition);
  //opened_state_->addTransition(transition);

  // 透明度动画
  animation = new QPropertyAnimation(this, "opacity", this);
  animation->setDuration(220);
  transition->addAnimation(animation);

  // 宽度变化
  animation = new QPropertyAnimation(drawer, "offset", this);
  animation->setDuration(220);
  animation->setEasingCurve(QEasingCurve::InCirc);
  transition->addAnimation(animation);

  // 动画关闭完成
  transition = new QSignalTransition(animation, SIGNAL(finished()));
  transition->setTargetState(closed_state_);
  closing_state_->addTransition(transition);

  // 发出关闭信号, 进入关闭过程
  transition = new QSignalTransition(this, SIGNAL(signalClose()));
  transition->setTargetState(closing_state_);
  opened_state_->addTransition(transition);

  animation = new QPropertyAnimation(drawer, "offset", this);
  animation->setDuration(220);
  animation->setEasingCurve(QEasingCurve::InCirc);
  transition->addAnimation(animation);

  animation = new QPropertyAnimation(this, "opacity", this);
  animation->setDuration(220);
  transition->addAnimation(animation);

  // 关闭动画执行完毕
  transition = new QSignalTransition(animation, SIGNAL(finished()));
  transition->setTargetState(closed_state_);
  closing_state_->addTransition(transition);

  // 在 TtPopUpDrawerStateMachine 构造函数中：
  QObject::connect(opening_state_, &QState::entered, [this]() {
    if (closing_state_->active()) {
      // machine()->cancelDelayedEvent();
    }
  });

  updatePropertyAssignments();
}
TtPopUpDrawerStateMachine::~TtPopUpDrawerStateMachine() {}

void TtPopUpDrawerStateMachine::setOpacity(qreal opacity) {
  opacity_ = opacity;
  main_->update();
}
qreal TtPopUpDrawerStateMachine::opacity() const {
  return opacity_;
}
bool TtPopUpDrawerStateMachine::isInClosedState() const {
  // 是否处于完全关闭状态
  return closed_state_->active();
}
void TtPopUpDrawerStateMachine::updatePropertyAssignments() {
  //   // 关闭时候的位移
  //   //const qreal closedOffset = -(drawer_->width() + 32);
  //   const qreal closedOffset = -(drawer_->width());
  //   // 672
  //   // qDebug() << "drawer->width" << drawer_->width();

  //   //qDebug() << "close offset: " << closedOffset;

  //   // closing 和 closed
  //   closing_state_->assignProperty(drawer_, "offset", closedOffset);
  //   closed_state_->assignProperty(drawer_, "offset", closedOffset);

  //   // 透明度为 0, 完全看不见
  //   closing_state_->assignProperty(this, "opacity", 0);
  //   closed_state_->assignProperty(this, "opacity", 0);
  //   //closing_state_->assignProperty(this, "opacity", 1);
  //   //closed_state_->assignProperty(this, "opacity", 1);

  //   opening_state_->assignProperty(drawer_, "offset", 0);
  //   //opening_state_->assignProperty(drawer_, "offset", 1);
  //   // 真正能看见的是 this???
  //   //opening_state_->assignProperty(this, "opacity", 0.4);
  //   opening_state_->assignProperty(this, "opacity", 0.4);
  //   //qDebug() << "update";

  QPoint closedOffset;
  // const QWidget* parent = main_->parentWidget();
  const int size = main_->drawerSize();
  const auto dir = main_->getDirection();

  // 确保父窗口尺寸有效
  const QWidget* parent = main_->parentWidget();
  const int parentWidth = parent ? parent->width() : main_->width();
  const int parentHeight = parent ? parent->height() : main_->height();

  switch (dir) {
    case TtPopUpDirection::Left:
      closedOffset = QPoint(-size - 16, 0);
      break;
    case TtPopUpDirection::Right:
      // closedOffset = QPoint(main_->width() + 16, 0);
      closedOffset = QPoint(parentWidth + 16, 0);
      break;
    case TtPopUpDirection::Top:
      closedOffset = QPoint(0, -size - 16);
      break;
    case TtPopUpDirection::Bottom:
      // closedOffset = QPoint(0, main_->height() + 16);
      closedOffset = QPoint(0, parentHeight + 16);
      break;
    default:
      closedOffset = QPoint(parentWidth + 16, 0);
  }

  // closedOffset = QPoint(-300, 0);
  // closedOffset = QPoint(300, 0);
  // qDebug() << "closedOffset" << closedOffset;
  // qDebug() << "closedOffset" << closedOffset;

  closed_state_->assignProperty(drawer_, "offset", closedOffset);
  closing_state_->assignProperty(drawer_, "offset", closedOffset);

  closing_state_->assignProperty(this, "opacity", 0);
  closed_state_->assignProperty(this, "opacity", 0);
  opening_state_->assignProperty(this, "opacity", 0.4);
  drawer_->setOffset(closedOffset);
}

}  // namespace Ui
