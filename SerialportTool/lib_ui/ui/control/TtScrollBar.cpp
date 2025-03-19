#include "ui/control/TtScrollBar.h"
#include "ui/control/TtScrollBar_p.h"

#include "ui/style/TtScrollBarStyle.h"

#include <QApplication>
#include <QEvent>
#include <QWheelEvent>

namespace Ui {

Q_PROPERTY_CREATE_Q_CPP(TtScrollBar, bool, IsAnimation)
Q_PROPERTY_CREATE_Q_CPP(TtScrollBar, qreal, SpeedLimit)

TtScrollBar::TtScrollBar(QWidget* parent)
    : QScrollBar(parent), d_ptr(new TtScrollBarPrivate()) {
  Q_D(TtScrollBar);
  d->q_ptr = this;
  setSingleStep(1);
  setObjectName("TtScrollBar");
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  d->pSpeedLimit_ = 20;  // 滚动速度
  d->pTargetMaximum_ = 0;
  d->pIsAnimation_ = false;
  connect(this, &TtScrollBar::rangeChanged, d,
          &TtScrollBarPrivate::onRangeChanged);
  style::TtScrollBarStyle* scrollBarStyle =
      new style::TtScrollBarStyle(style());
  scrollBarStyle->setScrollBar(this);
  setStyle(scrollBarStyle);
  d->slide_smooth_animation_ = new QPropertyAnimation(this, "value");
  d->slide_smooth_animation_->setEasingCurve(QEasingCurve::OutSine);
  d->slide_smooth_animation_->setDuration(300);
  connect(d->slide_smooth_animation_, &QPropertyAnimation::finished, this,
          [=]() { d->scroll_value_ = value(); });

  d->expand_timer_ = new QTimer(this);
  connect(d->expand_timer_, &QTimer::timeout, this, [=]() {
    d->expand_timer_->stop();
    d->is_expand_ = underMouse();
    scrollBarStyle->startExpandAnimation(d->is_expand_);
  });
}

TtScrollBar::TtScrollBar(Qt::Orientation orientation, QWidget* parent)
    : TtScrollBar(parent) {
  setOrientation(orientation);
}

TtScrollBar::TtScrollBar(QScrollBar* originScrollBar,
                         QAbstractScrollArea* parent)
    : TtScrollBar(parent) {
  Q_D(TtScrollBar);
  if (!originScrollBar || !parent) {
    qCritical() << "Invalid origin or parent!";
    return;
  }
  d->origin_scrollArea_ = parent;
  Qt::Orientation orientation = originScrollBar->orientation();
  setOrientation(orientation);
  orientation == Qt::Horizontal
      ? parent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff)
      : parent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  parent->installEventFilter(this);

  d->origin_scrollBar_ = originScrollBar;
  d->initAllConfig();

  connect(d->origin_scrollBar_, &QScrollBar::valueChanged, this,
          [=](int value) { d->handleScrollBarValueChanged(this, value); });
  connect(this, &QScrollBar::valueChanged, this, [=](int value) {
    d->handleScrollBarValueChanged(d->origin_scrollBar_, value);
  });
  connect(d->origin_scrollBar_, &QScrollBar::rangeChanged, this,
          [=](int min, int max) { d->handleScrollBarRangeChanged(min, max); });
}

TtScrollBar::~TtScrollBar() {}

bool TtScrollBar::event(QEvent* event) {
  Q_D(TtScrollBar);
  switch (event->type()) {
    case QEvent::Enter: {
      d->expand_timer_->stop();
      if (!d->is_expand_) {
        d->expand_timer_->start(350);
      }
      break;
    }
    case QEvent::Leave: {
      d->expand_timer_->stop();
      if (d->is_expand_) {
        d->expand_timer_->start(350);
      }
      break;
    }
    default: {
      break;
    }
  }
  return QScrollBar::event(event);
}

bool TtScrollBar::eventFilter(QObject* watched, QEvent* event) {
  Q_D(TtScrollBar);
  switch (event->type()) {
    case QEvent::Show:
    case QEvent::Resize:
    case QEvent::LayoutRequest: {
      d->handleScrollBarGeometry();
      break;
    }
    default: {
      break;
    }
  }
  return QScrollBar::eventFilter(watched, event);
}

void TtScrollBar::mousePressEvent(QMouseEvent* event) {
  Q_D(TtScrollBar);
  d->slide_smooth_animation_->stop();
  d->scroll_value_ = value();
  QScrollBar::mousePressEvent(event);
}

void TtScrollBar::mouseReleaseEvent(QMouseEvent* event) {
  Q_D(TtScrollBar);
  d->slide_smooth_animation_->stop();
  QScrollBar::mouseReleaseEvent(event);
  d->scroll_value_ = value();
}

void TtScrollBar::mouseMoveEvent(QMouseEvent* event) {
  Q_D(TtScrollBar);
  d->slide_smooth_animation_->stop();
  QScrollBar::mouseMoveEvent(event);
  d->scroll_value_ = value();
}

void TtScrollBar::wheelEvent(QWheelEvent* event) {
  Q_D(TtScrollBar);
  int verticalDelta = event->angleDelta().y();
  if (d->slide_smooth_animation_->state() == QAbstractAnimation::Stopped) {
    d->scroll_value_ = value();
  }
  if (verticalDelta != 0) {
    if ((value() == minimum() && verticalDelta > 0) ||
        (value() == maximum() && verticalDelta < 0)) {
      QScrollBar::wheelEvent(event);
      return;
    }
    d->scroll(event->modifiers(), verticalDelta);
  } else {
    int horizontalDelta = event->angleDelta().x();
    if ((value() == minimum() && horizontalDelta > 0) ||
        (value() == maximum() && horizontalDelta < 0)) {
      QScrollBar::wheelEvent(event);
      return;
    }
    d->scroll(event->modifiers(), horizontalDelta);
  }
  event->accept();
}

void TtScrollBar::contextMenuEvent(QContextMenuEvent* event) {
  Q_D(TtScrollBar);
  bool horiz = this->orientation() == Qt::Horizontal;
  // QPointer<ElaMenu> menu = new ElaMenu(this);
  // menu->setMenuItemHeight(27);
  // // Scroll here
  // QAction* actScrollHere =
  //     menu->addElaIconAction(ElaIconType::UpDownLeftRight, tr("滚动到此处"));
  // menu->addSeparator();
  // // Left edge Top
  // QAction* actScrollTop = menu->addElaIconAction(
  //     horiz ? ElaIconType::ArrowLeftToLine : ElaIconType::ArrowUpToLine,
  //     horiz ? tr("左边缘") : tr("顶端"));
  // // Right edge Bottom
  // QAction* actScrollBottom = menu->addElaIconAction(
  //     horiz ? ElaIconType::ArrowRightToLine : ElaIconType::ArrowDownToLine,
  //     horiz ? tr("右边缘") : tr("底部"));
  // menu->addSeparator();
  // // Page left Page up
  // QAction* actPageUp = menu->addElaIconAction(
  //     horiz ? ElaIconType::AnglesLeft : ElaIconType::AnglesUp,
  //     horiz ? tr("向左翻页") : tr("向上翻页"));
  // //Page right Page down
  // QAction* actPageDn = menu->addElaIconAction(
  //     horiz ? ElaIconType::AnglesRight : ElaIconType::AnglesDown,
  //     horiz ? tr("向右翻页") : tr("向下翻页"));
  // menu->addSeparator();
  // //Scroll left Scroll up
  // QAction* actScrollUp = menu->addElaIconAction(
  //     horiz ? ElaIconType::AngleLeft : ElaIconType::AngleUp,
  //     horiz ? tr("向左滚动") : tr("向上滚动"));
  // //Scroll right Scroll down
  // QAction* actScrollDn = menu->addElaIconAction(
  //     horiz ? ElaIconType::AngleRight : ElaIconType::AngleDown,
  //     horiz ? tr("向右滚动") : tr("向下滚动"));
  // QAction* actionSelected = menu->exec(event->globalPos());
  // delete menu;
  // if (!actionSelected) {
  //   return;
  // }
  // if (actionSelected == actScrollHere) {
  //   setValue(
  //       d->_pixelPosToRangeValue(horiz ? event->pos().x() : event->pos().y()));
  // } else if (actionSelected == actScrollTop) {
  //   triggerAction(QAbstractSlider::SliderToMinimum);
  // } else if (actionSelected == actScrollBottom) {
  //   triggerAction(QAbstractSlider::SliderToMaximum);
  // } else if (actionSelected == actPageUp) {
  //   triggerAction(QAbstractSlider::SliderPageStepSub);
  // } else if (actionSelected == actPageDn) {
  //   triggerAction(QAbstractSlider::SliderPageStepAdd);
  // } else if (actionSelected == actScrollUp) {
  //   triggerAction(QAbstractSlider::SliderSingleStepSub);
  // } else if (actionSelected == actScrollDn) {
  //   triggerAction(QAbstractSlider::SliderSingleStepAdd);
  // }
}

TtScrollBarPrivate::TtScrollBarPrivate(QObject* parent) : QObject(parent) {}

TtScrollBarPrivate::~TtScrollBarPrivate() {}

void TtScrollBarPrivate::onRangeChanged(int min, int max) {
  Q_Q(TtScrollBar);
  if (q->isVisible() && pIsAnimation_ && max != 0) {
    QPropertyAnimation* rangeSmoothAnimation =
        new QPropertyAnimation(this, "pTargetMaximum");
    connect(rangeSmoothAnimation, &QPropertyAnimation::finished, this,
            [=]() { Q_EMIT q->rangeAnimationFinished(); });
    connect(rangeSmoothAnimation, &QPropertyAnimation::valueChanged, this,
            [=](const QVariant& value) {
              q->blockSignals(true);
              q->setMaximum(value.toUInt());
              q->blockSignals(false);
              q->update();
            });
    rangeSmoothAnimation->setEasingCurve(QEasingCurve::OutSine);
    rangeSmoothAnimation->setDuration(250);
    rangeSmoothAnimation->setStartValue(pTargetMaximum_);
    rangeSmoothAnimation->setEndValue(max);
    rangeSmoothAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  } else {
    if (max == 0) {
      scroll_value_ = -1;
    }
    pTargetMaximum_ = max;
  }
}

void TtScrollBarPrivate::initAllConfig() {
  Q_Q(TtScrollBar);
  handleScrollBarRangeChanged(origin_scrollBar_->minimum(),
                              origin_scrollBar_->maximum());
  // 设置步长
  q->setSingleStep(origin_scrollBar_->singleStep());
  q->setPageStep(origin_scrollBar_->pageStep());
}

void TtScrollBarPrivate::handleScrollBarValueChanged(QScrollBar* scrollBar,
                                                     int value) {
  // 动态改变值
  scrollBar->setValue(value);
}

void TtScrollBarPrivate::handleScrollBarRangeChanged(int min, int max) {
  Q_Q(TtScrollBar);
  q->setRange(min, max);
  if (max <= 0) {
    q->setVisible(false);
  } else {
    q->setVisible(true);
  }
}

void TtScrollBarPrivate::handleScrollBarGeometry() {
  Q_Q(TtScrollBar);
  q->raise();
  q->setSingleStep(origin_scrollBar_->singleStep());
  q->setPageStep(origin_scrollBar_->pageStep());
  if (q->orientation() == Qt::Horizontal) {
    q->setGeometry(0, origin_scrollArea_->height() - 10,
                   origin_scrollArea_->width(), 10);
  } else {
    q->setGeometry(origin_scrollArea_->width() - 10, 0, 10,
                   origin_scrollArea_->height());
  }
}

void TtScrollBarPrivate::scroll(Qt::KeyboardModifiers modifiers, int delta) {
  Q_Q(TtScrollBar);
  int stepsToScroll = 0;
  qreal offset = qreal(delta) / 120;
  int pageStep = 10;
  int singleStep = q->singleStep();
  if ((modifiers & Qt::ControlModifier) || (modifiers & Qt::ShiftModifier)) {
    stepsToScroll = qBound(-pageStep, int(offset * pageStep), pageStep);
  } else {
    stepsToScroll = QApplication::wheelScrollLines() * offset * singleStep;
  }
  if (abs(scroll_value_ - q->value()) > abs(stepsToScroll * pSpeedLimit_)) {
    scroll_value_ = q->value();
  }
  scroll_value_ -= stepsToScroll;
  slide_smooth_animation_->stop();
  slide_smooth_animation_->setStartValue(q->value());
  slide_smooth_animation_->setEndValue(scroll_value_);
  slide_smooth_animation_->start();
}

int TtScrollBarPrivate::pixelPosToRangeValue(int pos) const {
  Q_Q(const TtScrollBar);
  QStyleOptionSlider opt;
  q->initStyleOption(&opt);
  QRect gr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                        QStyle::SC_ScrollBarGroove, q);
  QRect sr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                        QStyle::SC_ScrollBarSlider, q);
  int sliderMin, sliderMax, sliderLength;
  if (q->orientation() == Qt::Horizontal) {
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
    if (q->layoutDirection() == Qt::RightToLeft) {
      opt.upsideDown = !opt.upsideDown;
    }
  } else {
    sliderLength = sr.height();
    sliderMin = gr.y();
    sliderMax = gr.bottom() - sliderLength + 1;
  }
  return QStyle::sliderValueFromPosition(q->minimum(), q->maximum(),
                                         pos - sliderMin, sliderMax - sliderMin,
                                         opt.upsideDown);
}

}  // namespace Ui
