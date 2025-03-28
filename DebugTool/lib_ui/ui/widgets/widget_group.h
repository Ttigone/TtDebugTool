/*****************************************************************/ /**
 * \file   widget_group.h
 * \brief  互斥窗口组
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_WIDGETS_WIDGET_GROUP_H
#define UI_WIDGETS_WIDGET_GROUP_H

#include <QEvent>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtWidgetGroup : public QObject {
  Q_OBJECT
 public:
  TtWidgetGroup(QObject* parent);
  ~TtWidgetGroup();

  void addWidget(QWidget* widget);

  void removeWidget(QWidget* widget);

  QList<QWidget*> widgets() const { return widgets_; }

  void setExclusive(bool exclusive) { exclusive_ = exclusive; }
  bool isExclusive() const { return exclusive_; }

  QWidget* checkedWidget() const { return checked_widget_; }

  void setCheckedIndex(int index) {
    if (index >= 0 && index < widgets_.size()) {
      QWidget* widget = widgets_.at(index);
      updateWidgetState(widget, true);
    }
  }

 signals:
  // 某个 widget 点击
  void widgetClicked(int index);
  void widgetToggled(QWidget* widget, bool checked);

 protected:
  // bool eventFilter(QObject* obj, QEvent* event) override {
  //   if (event->type() == QEvent::MouseButtonPress) {
  //     //if (event->type() == QEvent::MouseButtonRelease) {
  //     //event
  //     QWidget* widget = qobject_cast<QWidget*>(obj);
  //     if (widget && widgets_.contains(widget)) {
  //       is_pressed_ = true;
  //       //  bool checked = !widget->property("checked").toBool();
  //       //  updateWidgetState(widget, checked);
  //       //  //qDebug() << widgets_.indexOf(widget);
  //       //  emit widgetClicked(widgets_.indexOf(widget));
  //       //  //return true;  // 阻止事件继续传播
  //     }
  //   } else if (event->type() == QEvent::MouseButtonRelease) {
  //     QWidget* widget = qobject_cast<QWidget*>(obj);
  //     if (widget && widgets_.contains(widget)) {
  //       if (is_pressed_) {
  //         is_pressed_ = false;
  //         bool checked = !widget->property("checked").toBool();
  //         updateWidgetState(widget, checked);
  // 			//qDebug() << widgets_.indexOf(widget);
  //         emit widgetClicked(widgets_.indexOf(widget));
  //         return true;  // 阻止事件继续传播
  //       }
  //     }
  //   }
  //   return QObject::eventFilter(obj, event);
  // }

  bool eventFilter(QObject* obj, QEvent* event) override {
    if (event->type() == QEvent::MouseButtonRelease) {
      QWidget* widget = qobject_cast<QWidget*>(obj);
      if (widget && widgets_.contains(widget)) {
        bool checked = !widget->property("checked").toBool();
        updateWidgetState(widget, checked);
        emit widgetClicked(widgets_.indexOf(widget));
        return true;  // 阻止事件继续传播
      }
    }
    return QObject::eventFilter(obj, event);
  }

 private slots:
  void handleWidgetDestroyed(QObject* obj) {
    QWidget* widget = static_cast<QWidget*>(obj);
    widgets_.removeAll(widget);
    if (checked_widget_ == widget)
      checked_widget_ = nullptr;
  }

  void handleWidgetToggled(QWidget* widget, bool checked) {
    updateWidgetState(widget, checked);
  }

 private:
  void updateWidgetState(QWidget* widget, bool checked) {
    if (exclusive_ && checked) {
      // 互斥模式下先取消其他widget的选中状态
      for (QWidget* w : widgets_) {
        if (w != widget && w->property("checked").toBool()) {
          w->setProperty("checked", false);
          //emit widgetToggled(w, false);
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

 private:
  bool is_pressed_;
  QList<QWidget*> widgets_;
  QWidget* checked_widget_;
  bool exclusive_;
};

}  // namespace Ui

#endif  // UI_WIDGETS_WIDGET_GROUP_H
