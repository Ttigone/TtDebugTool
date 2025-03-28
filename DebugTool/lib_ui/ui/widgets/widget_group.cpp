#include "ui/widgets/widget_group.h"

namespace Ui {
TtWidgetGroup::TtWidgetGroup(QObject* parent)
    : QObject(parent), exclusive_(false), checked_widget_(nullptr) {
}
TtWidgetGroup::~TtWidgetGroup() {}

void TtWidgetGroup::addWidget(QWidget* widget) {
  if (!widget || widgets_.contains(widget))
    return;

  widgets_.append(widget);
  connect(widget, &QObject::destroyed, this,
          &TtWidgetGroup::handleWidgetDestroyed);

  widget->installEventFilter(this);
}

void TtWidgetGroup::removeWidget(QWidget* widget) {
  if (!widget || !widgets_.contains(widget))
    return;

  widgets_.removeOne(widget);
  disconnect(widget, 0, this, 0);
  widget->removeEventFilter(this);

  if (checked_widget_ == widget)
    checked_widget_ = nullptr;
}
}  // namespace Ui
