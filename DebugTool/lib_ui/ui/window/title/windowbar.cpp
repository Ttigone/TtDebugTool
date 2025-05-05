#include "ui/window/title/windowbar.h"
#include "ui/window/title/windowbar_p.h"

#include <QAbstractButton>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>

namespace Ui {

WindowBarPrivate::WindowBarPrivate(){};

WindowBarPrivate::~WindowBarPrivate() = default;

void WindowBarPrivate::init() {
  Q_Q(WindowBar);
  layout_ = new QHBoxLayout();
  if (QLocale::system().textDirection() == Qt::RightToLeft) {
    layout_->setDirection(QBoxLayout::RightToLeft);
  }

  layout_->setContentsMargins(QMargins());
  layout_->setSpacing(0);
  for (int i = WindowBarItem::IconButton; i <= WindowBarItem::CloseButton;
       ++i) {
    insertDefaultSpace(i);
  }
  q->setLayout(layout_);
}

inline QWidget *WindowBarPrivate::widgetAt(int index) const {
  return layout_->itemAt(index)->widget();
}

void WindowBarPrivate::setWidgetAt(int index, QWidget *widget) {
  auto priWidget = layout_->takeAt(index)->widget();
  if (priWidget) {
    priWidget->deleteLater();
  }
  if (!widget) {
    insertDefaultSpace(index);
  } else {
    layout_->insertWidget(index, widget);
  }
}

QWidget *WindowBarPrivate::takeWidgetAt(int index) {
  auto item = layout_->itemAt(index);
  auto priWidget = item->widget();
  if (priWidget) {
    item = layout_->takeAt(index);
    delete item;
    insertDefaultSpace(index);
  }
  return priWidget;
}

inline void WindowBarPrivate::insertDefaultSpace(int index) {
  layout_->insertSpacerItem(index, new QSpacerItem(0, 0));
}

WindowBar::WindowBar(QWidget *parent)
    : WindowBar(*new WindowBarPrivate(), parent) {}

WindowBar::~WindowBar() = default;

QMenuBar *WindowBar::menuBar() const {
  Q_D(const WindowBar);
  return qobject_cast<QMenuBar *>(d->widgetAt(WindowBarPrivate::MenuWidget));
}

void WindowBar::setMenuBar(QMenuBar *menuBar) {
  Q_D(WindowBar);
  auto pri = takeMenuBar();
  if (pri) {
    pri->deleteLater();
  }
  if (!menuBar) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::MenuWidget, menuBar);
  menuBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
}

QLabel *WindowBar::titleLabel() const {
  Q_D(const WindowBar);
  return qobject_cast<QLabel *>(d->widgetAt(WindowBarPrivate::TitleLabel));
}

void WindowBar::setTitleLabel(QLabel *label) {
  Q_D(WindowBar);
  auto pri = takeTitleLabel();
  if (pri) {
    pri->deleteLater();
  }
  if (!label) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::TitleLabel, label);
  if (d->auto_title_ && d->w_) {
    label->setText(d->w_->windowTitle());
  }
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

QAbstractButton *WindowBar::iconButton() const {
  Q_D(const WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->widgetAt(WindowBarPrivate::IconButton));
}

void WindowBar::setIconButton(QAbstractButton *btn) {
  Q_D(WindowBar);
  auto pri = takeIconButton();
  if (pri) {
    pri->deleteLater();
  }
  if (!btn) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::IconButton, btn);
  if (d->auto_icon_ && d->w_) {
    btn->setIcon(d->w_->windowIcon());
  }
  btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

QAbstractButton *WindowBar::topButton() const {
  Q_D(const WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->widgetAt(WindowBarPrivate::TopButton));
}

void WindowBar::setTopButton(QAbstractButton *btn) {
  Q_D(WindowBar);
  auto pri = takeTopButton();
  if (pri) {
    pri->deleteLater();
  }
  if (!btn) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::TopButton, btn);
  connect(btn, &QAbstractButton::clicked, this, &WindowBar::topRequest);
}

QAbstractButton *WindowBar::minButton() const {
  Q_D(const WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->widgetAt(WindowBarPrivate::MinimumButton));
}

void WindowBar::setMinButton(QAbstractButton *btn) {
  Q_D(WindowBar);
  auto pri = takeMinButton();
  if (pri) {
    pri->deleteLater();
  }
  if (!btn) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::MinimumButton, btn);
  connect(btn, &QAbstractButton::clicked, this, &WindowBar::minimizeRequested);
}

QAbstractButton *WindowBar::maxButton() const {
  Q_D(const WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->widgetAt(WindowBarPrivate::MaxinumButton));
}

void WindowBar::setMaxButton(QAbstractButton *btn) {
  Q_D(WindowBar);
  auto pri = takeMaxButton();
  if (pri) {
    pri->deleteLater();
  }
  if (!btn) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::MaxinumButton, btn);
  connect(btn, &QAbstractButton::clicked, this, &WindowBar::maximizeRequested);
}

QAbstractButton *WindowBar::closeButton() const {
  Q_D(const WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->widgetAt(WindowBarPrivate::CloseButton));
}

void WindowBar::setCloseButton(QAbstractButton *btn) {
  Q_D(WindowBar);
  auto pri = takeCloseButton();
  if (pri) {
    pri->deleteLater();
  }
  if (!btn) {
    return;
  }
  d->setWidgetAt(WindowBarPrivate::CloseButton, btn);
  connect(btn, &QAbstractButton::clicked, this, &WindowBar::closeRequested);
}

QMenuBar *WindowBar::takeMenuBar() {
  Q_D(WindowBar);
  return qobject_cast<QMenuBar *>(
      d->takeWidgetAt(WindowBarPrivate::MenuWidget));
}

QLabel *WindowBar::takeTitleLabel() {
  Q_D(WindowBar);
  return qobject_cast<QLabel *>(d->takeWidgetAt(WindowBarPrivate::TitleLabel));
}

QAbstractButton *WindowBar::takeIconButton() {
  Q_D(WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->takeWidgetAt(WindowBarPrivate::IconButton));
}

QAbstractButton *Ui::WindowBar::takeTopButton() {
  Q_D(WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->takeWidgetAt(WindowBarPrivate::TopButton));
}

QAbstractButton *WindowBar::takeMinButton() {
  Q_D(WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->takeWidgetAt(WindowBarPrivate::MinimumButton));
}

QAbstractButton *WindowBar::takeMaxButton() {
  Q_D(WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->takeWidgetAt(WindowBarPrivate::MaxinumButton));
}

QAbstractButton *WindowBar::takeCloseButton() {
  Q_D(WindowBar);
  return qobject_cast<QAbstractButton *>(
      d->takeWidgetAt(WindowBarPrivate::CloseButton));
}

QWidget *WindowBar::hostWidget() const {
  Q_D(const WindowBar);
  return d->w_;
}

void WindowBar::setHostWidget(QWidget *w) {
  Q_D(WindowBar);
  QWidget *pri = d->w_;
  if (pri) {
    pri->removeEventFilter(this);
  }
  d_ptr->w_ = w;
  if (w) {
    w->installEventFilter(this);
  }
}

bool WindowBar::titleFollowWindow() const {
  Q_D(const WindowBar);
  return d->auto_title_;
}

void WindowBar::setTitleFollowWindow(bool value) {
  Q_D(WindowBar);
  d->auto_title_ = value;
}

bool WindowBar::iconFollowWindow() const {
  Q_D(const WindowBar);
  return d->auto_icon_;
}

void WindowBar::setIconFollowWindow(bool value) {
  Q_D(WindowBar);
  d->auto_icon_ = value;
}

bool WindowBar::eventFilter(QObject *obj, QEvent *event) {
  Q_D(WindowBar);
  auto w = d->w_;
  if (obj == w) {
    QAbstractButton *iconBtn = iconButton();
    QLabel *label = titleLabel();
    QAbstractButton *maxBtn = maxButton();
    switch (event->type()) {
    case QEvent::WindowIconChange: {
      if (d_ptr->auto_icon_ && iconBtn) {
        iconBtn->setIcon(w->windowIcon());
        iconChanged(w->windowIcon());
      }
      break;
    }
    case QEvent::WindowTitleChange: {
      if (d_ptr->auto_title_ && label) {
        label->setText(w->windowTitle());
        titleChanged(w->windowTitle());
      }
      break;
    }
    case QEvent::WindowStateChange: {
      if (maxBtn) {
        maxBtn->setChecked(w->isMaximized());
      }
      break;
    }
    default:
      break;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void WindowBar::titleChanged(const QString &text) { Q_UNUSED(text) }

void WindowBar::iconChanged(const QIcon &icon) { Q_UNUSED(icon) }

WindowBar::WindowBar(WindowBarPrivate &d, QWidget *parent)
    : QFrame(parent), d_ptr(&d) {
  // Public Class Point of Private Class is assigned to the Public Class Point
  d.q_ptr = this;
  // execute private actions
  d.init();
}

} // namespace Ui
