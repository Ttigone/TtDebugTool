#include "ui/widgets/widget_group.h"

namespace Ui {
TtWidgetGroup::TtWidgetGroup(QObject *parent)
    : QObject(parent), checked_widget_(nullptr), exclusive_(false),
      holding_state_(false) {}
TtWidgetGroup::~TtWidgetGroup() {}

void TtWidgetGroup::addWidget(QWidget *widget) {
  if (!widget || widgets_.contains(widget))
    return;

  widgets_.append(widget);
  connect(widget, &QObject::destroyed, this,
          &TtWidgetGroup::handleWidgetDestroyed);

  widget->installEventFilter(this);
}

void TtWidgetGroup::removeWidget(QWidget *widget) {
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
    QWidget *widget = widgets_.at(index);
    updateWidgetState(widget, true);
  }
}

void TtWidgetGroup::setHoldingChecked(bool enable) { holding_state_ = enable; }

bool TtWidgetGroup::holdingChecked() const { return holding_state_; }

bool TtWidgetGroup::eventFilter(QObject *obj, QEvent *event) {
  // 鼠标点击事件
  if (event->type() == QEvent::MouseButtonPress) {
    // if (event->type() == QEvent::MouseButtonPress ||
    //     event->type() == QEvent::MouseButtonDblClick) {
    QWidget *widget = qobject_cast<QWidget *>(obj);
    if (widget && widgets_.contains(widget)) {
      // 获取 check 属性
      // 有 bug, 为什么会突然变为 false
      bool currentChecked = widget->property("checked").toBool();
      qDebug() << "当前的 check" << currentChecked;

      // 保持状态
      if (holding_state_) { // true
        // check 状态, 当前 widget
        // 第一次进入 check_widget 为空
        if (currentChecked && widget == checked_widget_) {
          emit widgetClicked(widgets_.indexOf(widget));
          return true;
        }
      }
      bool checked = !currentChecked;
      updateWidgetState(widget, checked);
      emit widgetClicked(widgets_.indexOf(widget));
      return true; // 阻止事件继续传播
    }
  } else if (event->type() == QEvent::MouseButtonDblClick) {
    qDebug() << "this double clicked";
    QWidget *widget = qobject_cast<QWidget *>(obj);
    emit widgetClicked(widgets_.indexOf(widget));
    // bug left 有问题
    return true;
  }
  return QObject::eventFilter(obj, event);
}

void TtWidgetGroup::handleWidgetDestroyed(QObject *obj) {
  QWidget *widget = static_cast<QWidget *>(obj);
  widgets_.removeAll(widget);
  if (checked_widget_ == widget)
    checked_widget_ = nullptr;
}

void TtWidgetGroup::handleWidgetToggled(QWidget *widget, bool checked) {
  updateWidgetState(widget, checked);
}

void TtWidgetGroup::updateWidgetState(QWidget *widget, bool checked) {
  if (exclusive_ && checked) {
    // 互斥模式下先取消其他widget的选中状态
    for (QWidget *w : widgets_) {
      if (w != widget && w->property("checked").toBool()) {
        w->setProperty("checked", false);
        emit widgetToggled(w, false);
      }
    }
  }
  // 设置当前选中
  bool stateChanged = (widget->property("checked").toBool() != checked);
  widget->setProperty("checked", checked);

  if (exclusive_) {
    // 选中模式
    if (checked) {
      // 更新选中的当前 widget
      checked_widget_ = widget;
    } else if (checked_widget_ == widget) {
      // 移出
      checked_widget_ = nullptr;
    }
  } else {
    // 非互斥
    checked_widget_ = checked ? widget : nullptr;
  }

  // 通知当前选中的窗口
  if (stateChanged) {
    // 状态改变
    // 没选中 -> 选中  选中->没选中(不会发生)
    // checked_widget_ = checked ? widget : nullptr;
    emit widgetToggled(widget, checked);
  }
}

} // namespace Ui
