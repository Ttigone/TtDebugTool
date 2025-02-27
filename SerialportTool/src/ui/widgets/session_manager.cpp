#include "ui/widgets/session_manager.h"

#include <ui/widgets/buttons.h>

namespace Ui {

SessionManager::SessionManager(QWidget* parent) : QListWidget(parent) {
  // setSpacing(5);
  // 禁用水平滚动条
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // 开启自动调整
  setResizeMode(QListView::Adjust);
  // 设置统一大小
  // setUniformItemSizes(true);
  setFrameStyle(QFrame::NoFrame);

  setSelectionMode(QAbstractItemView::NoSelection);
}

bool SessionManager::addAdaptiveWidget(const QString& title,
                                       const QString& uuid, QWidget* widget) {
  if (uuids_.contains(uuid)) {
    // 保存记录的功能
    // 更新标题
    Ui::TtSpecialDeleteButton* button =
        qobject_cast<Ui::TtSpecialDeleteButton*>(
            itemWidget(uuids_.value(uuid)));
    button->setTitle(title);
    // qDebug() << "已经存在了";
    return false;
  }
  QListWidgetItem* item = new QListWidgetItem(this);
  int contentWidth = contentsRect().width();
  item->setSizeHint(QSize(contentWidth, widget->sizeHint().height())
                        .shrunkBy(QMargins(3, 3, 3, 3)));
  addItem(item);
  setItemWidget(item, widget);

  Ui::TtSpecialDeleteButton* button =
      qobject_cast<Ui::TtSpecialDeleteButton*>(widget);

  // uuids_.append(uuid, item);
  uuids_.insert(uuid, item);

  // 连接按钮的 clicked 信号到删除项的槽
  connect(button, &Ui::TtSpecialDeleteButton::deleteRequest, this,
          [this, uuid]() {
            qDebug() << "delete" << uuid;
            auto item = uuids_.value(uuid);
            // indexFromItem(item);
            setItemWidget(item, nullptr);  // 移除 widget
            delete item;                   // 删除项
            uuids_.remove(uuid);
            // emit
            // 通知 tabwidget, 删除对应的存储
            emit uuidsChanged(uuid);
          });

  updateAllItemSizes();
  return true;
}

void SessionManager::updateItemSize(QListWidgetItem* item) {
  if (!item)
    return;
  QWidget* widget = itemWidget(item);
  if (!widget)
    return;

  // qDebug() << "widget width" << widget->width();

  int width = contentsRect().width();
  item->setSizeHint(QSize(width, widget->sizeHint().height()));
}

void SessionManager::updateAllItemSizes() {
  for (int i = 0; i < count(); ++i) {
    updateItemSize(item(i));
  }
}

}  // namespace Ui
