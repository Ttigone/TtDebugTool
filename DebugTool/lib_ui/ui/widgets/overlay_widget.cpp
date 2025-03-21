#include "ui/widgets/overlay_widget.h"
#include <QEvent>

namespace Ui {

TtOverlayWidget::TtOverlayWidget(QWidget* parent) : QWidget(parent) {
  if (parent) {
    parent->installEventFilter(this);
  }
}
TtOverlayWidget::~TtOverlayWidget() {}

bool TtOverlayWidget::event(QEvent* event) {
  if (!parent()) {
    // 没有设置父对象
    return QWidget::event(event);
  }
  switch (event->type()) {
    // 为什么需要先删除后重新安装 ??? 
    case QEvent::ParentChange: {
      // 事件监听
      parent()->installEventFilter(this);
      // 父对象改变后, 更新悬浮界面的几何特征
      setGeometry(overlayGeometry());
      break;
    }
    case QEvent::ParentAboutToChange: {
      // 父对象做出改变之前, 先删除这个事件监听器
      parent()->removeEventFilter(this);
      break;
    }
    default:
      break;
  }
  return QWidget::event(event);
}
bool TtOverlayWidget::eventFilter(QObject* obj, QEvent* event) {
  switch (event->type()) {
    case QEvent::Move:
    case QEvent::Resize:
      // 父对象发生时, 都要实时更新覆盖面积的几何形状
      setGeometry(overlayGeometry());
      break;
    default:
      break;
  }
  return QWidget::eventFilter(obj, event);
}
QRect TtOverlayWidget::overlayGeometry() const {
  // 父对象
  QWidget* widget = parentWidget();
  if (!widget) {
    return QRect();
  }
  // 返回父对象的几何, overlay 覆盖全部 parent
  return widget->rect();
}
}  // namespace Ui
