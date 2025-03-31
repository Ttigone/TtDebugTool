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
  explicit TtWidgetGroup(QObject* parent);
  ~TtWidgetGroup();

  void addWidget(QWidget* widget);

  void removeWidget(QWidget* widget);


  void setExclusive(bool exclusive) { exclusive_ = exclusive; }
  bool isExclusive() const { return exclusive_; }

  QWidget* checkedWidget() const { return checked_widget_; }

  void setCheckedIndex(int index);

  void setHoldingChecked(bool enable);
  bool holdingChecked() const;

 signals:
  // 某个 widget 点击
  void widgetClicked(int index);
  void widgetToggled(QWidget* widget, bool checked);

 protected:
  /**
   * @brief eventFilter 被添加的控件, 其点击事件将被拦截 !!!
   * @param obj
   * @param event
   * @return 
   */
  bool eventFilter(QObject* obj, QEvent* event) override;

 private slots:
  void handleWidgetDestroyed(QObject* obj);

  void handleWidgetToggled(QWidget* widget, bool checked);

 private:
  QList<QWidget*> widgets() const { return widgets_; }

  void updateWidgetState(QWidget* widget, bool checked);

 private:
  bool is_pressed_;
  QList<QWidget*> widgets_;
  QWidget* checked_widget_;
  bool exclusive_;
  bool holding_state_;
};

}  // namespace Ui

#endif  // UI_WIDGETS_WIDGET_GROUP_H

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
