#include "ui/widgets/widget_group.h"

namespace Ui {
TtWidgetGroup::TtWidgetGroup(QObject* parent)
    : QObject(parent),
      checked_widget_(nullptr),
      exclusive_(false),
      holding_state_(false) {}
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

void TtWidgetGroup::setCheckedIndex(int index) {
  if (index >= 0 && index < widgets_.size()) {
    QWidget* widget = widgets_.at(index);
    updateWidgetState(widget, true);
  }
}

void TtWidgetGroup::setHoldingChecked(bool enable) {
  holding_state_ = enable;
}

bool TtWidgetGroup::holdingChecked() const {
  return holding_state_;
}

bool TtWidgetGroup::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() == QEvent::MouseButtonRelease) {
    QWidget* widget = qobject_cast<QWidget*>(obj);
    if (widget && widgets_.contains(widget)) {
      bool currentChecked = widget->property("checked").toBool();

      if (holding_state_) {
        if (currentChecked && widget == checked_widget_) {
          emit widgetClicked(widgets_.indexOf(widget));
          return true;
        }
      }
      bool checked = !currentChecked;
      updateWidgetState(widget, checked);
      emit widgetClicked(widgets_.indexOf(widget));
      return true;  // 阻止事件继续传播
    }
  }
  return QObject::eventFilter(obj, event);
}

void TtWidgetGroup::handleWidgetDestroyed(QObject* obj) {
  QWidget* widget = static_cast<QWidget*>(obj);
  widgets_.removeAll(widget);
  if (checked_widget_ == widget)
    checked_widget_ = nullptr;
}

void TtWidgetGroup::handleWidgetToggled(QWidget* widget, bool checked) {
  updateWidgetState(widget, checked);
}

void TtWidgetGroup::updateWidgetState(QWidget* widget, bool checked) {
  if (exclusive_ && checked) {
    // 互斥模式下先取消其他widget的选中状态
    for (QWidget* w : widgets_) {
      if (w != widget && w->property("checked").toBool()) {
        w->setProperty("checked", false);
      }
    }
  }

  // 设置当前选中
  bool stateChanged = (widget->property("checked").toBool() != checked);
  widget->setProperty("checked", checked);

  // 通知当前选中的窗口
  if (stateChanged) {
    checked_widget_ = checked ? widget : nullptr;
    emit widgetToggled(widget, checked);
  }
}
}  // namespace Ui
