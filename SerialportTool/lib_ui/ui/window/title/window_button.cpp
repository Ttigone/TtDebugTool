#include "ui/window/title/window_button.h"
#include "ui/window/title/window_button_p.h"

#include <QMouseEvent>
#include <QPainter>

namespace Ui {

WindowbarButtonPrivate::WindowbarButtonPrivate() = default;
WindowbarButtonPrivate::~WindowbarButtonPrivate() = default;

void WindowbarButtonPrivate::init() {}
void WindowbarButtonPrivate::reloadIcon() {
  Q_Q(WindowbarButton);

  if (!q->isEnabled() && !iconDisabled.isNull()) {
    q->setIcon(iconDisabled);
    return;
  }
  if (q->isChecked() && !iconChecked.isNull()) {
    q->setIcon(iconChecked);
    return;
  }
  if (!iconNormal.isNull()) {
    q->setIcon(iconNormal);
  }
}

WindowbarButton::WindowbarButton(QWidget* parent) : WindowbarButton(*new WindowbarButtonPrivate(), parent) {}

WindowbarButton::~WindowbarButton() = default;

QIcon WindowbarButton::iconNormal() const {
  Q_D(const WindowbarButton);
  return d->iconNormal;
}

void WindowbarButton::setIconNormal(const QIcon& icon) {
  Q_D(WindowbarButton);
  d->iconNormal = icon;
  d->reloadIcon();
}

QIcon WindowbarButton::iconChecked() const {
  Q_D(const WindowbarButton);
  return d->iconChecked;
}

void WindowbarButton::setIconChecked(const QIcon& icon) {
  Q_D(WindowbarButton);
  d->iconChecked = icon;
  d->reloadIcon();
}

QIcon WindowbarButton::iconDisabled() const {
  Q_D(const WindowbarButton);
  return d->iconDisabled;
}

void WindowbarButton::setIconDisabled(const QIcon& icon) {
  Q_D(WindowbarButton);
  d->iconDisabled = icon;
  d->reloadIcon();
}

void WindowbarButton::checkStateSet() {
  Q_D(WindowbarButton);
  d->reloadIcon();
  QAbstractButton::checkStateSet();
}

void WindowbarButton::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    Q_EMIT doubleClicked();
  }
  QAbstractButton::mouseDoubleClickEvent(event);
}

// void Button::paintEvent(QPaintEvent* event) { QPainter painter(this); }

// void Button::mouseMoveEvent(QMouseEvent* event)
// {
// 	QAbstractButton::mouseMoveEvent(event);
// }

// void Button::mousePressEvent(QMouseEvent* event)
// {
// 	QAbstractButton::mousePressEvent(event);
// }

// void Button::mouseReleaseEvent(QMouseEvent* event)
// {
// 	QAbstractButton::mouseReleaseEvent(event);
// }

WindowbarButton::WindowbarButton(WindowbarButtonPrivate& d, QWidget* parent)
    : QPushButton(parent), d_ptr(&d) {
  d.q_ptr = this;
  d.init();
}
}  // namespace Ui
