#include "ui/control/buttonbox/TtButtonBox.h"

#include <QButtonGroup>

#include "ui/widgets/buttons.h"

namespace Ui {

WidgetGroup::WidgetGroup(QObject *parent)
    : QObject(parent), m_currentIndex(-1) {}

void WidgetGroup::addButton(const QString &uuid, int specialType,
                            TtSpecialDeleteButton *button) {
  // bool hadFlag = false;
  // for (const auto &key : buttons_.keys()) {
  //   if (key.first == uuid) {
  //     hadFlag = true;
  //     // 存在相同的
  //     break;
  //   }
  // }
  // if (!hadFlag) {
  //   buttons_.insert(qMakePair(uuid, specialType), button);
  //   connect(button, &TtSpecialDeleteButton::clicked, this,
  //           &WidgetGroup::handleButtonClicked);
  // }
  // 直接使用 contains 检查键是否存在，避免创建临时容器
  QPair<QString, int> key(uuid, specialType);
  if (!buttons_.contains(key)) {
    // 尝试查找具有相同UUID但不同类型的按钮
    bool hadFlag = false;
    for (auto it = buttons_.begin(); it != buttons_.end(); ++it) {
      if (it.key().first == uuid) {
        hadFlag = true;
        break;
      }
    }

    if (!hadFlag) {
      buttons_.insert(key, button);
      connect(button, &TtSpecialDeleteButton::clicked, this,
              &WidgetGroup::handleButtonClicked);
    }
  }
}

void WidgetGroup::setCurrentIndex(const QString &index) {
  if (index != current_uuid_) {
    if (auto *btn = findButton(current_uuid_)) {
      btn->setChecked(false);
    }
    if (auto *btn = findButton(index)) {
      btn->setChecked(true);
    }
    current_uuid_ = index;
    emit currentIndexChanged(index, -1);
  }
}

int WidgetGroup::currentIndex() const { return m_currentIndex; }

void WidgetGroup::setSpecificOptionStatus(const QString &uuid, bool state) {
  for (auto it = buttons_.begin(); it != buttons_.end(); ++it) {
    if (it.key().first == uuid) {
      // qDebug() << "find the button";
      // 找到了对应的值
      it.value()->setWorkState(state);
      break;
    }
  }
}

void WidgetGroup::updateUuid(const QString &index) {
  // qDebug() << "remove" << index;
  // QMap<QPair<QString, int>, TtSpecialDeleteButton*>::iterator it =
  //     buttons_.begin();
  // while (it != buttons_.end()) {
  //   if (it.key().first == index) {
  //     TtSpecialDeleteButton* buttonToDelete = it.value();
  //     // 本地删除对应的 btn
  //     it = buttons_.erase(it);

  //     // // 3. 删除 TtSpecialDeleteButton 对象 (如果 QMap 拥有所有权)
  //     // // 如果其他地方管理这个对象的生命周期，则不要 delete
  //     // delete buttonToDelete;
  //     // buttonToDelete = nullptr;  // 好习惯
  //   } else {
  //     ++it;
  //   }
  // }
  // qDebug() << "remove" << index;
  auto it = buttons_.begin();
  while (it != buttons_.end()) {
    if (it.key().first == index) {
      // 不再需要存储 buttonToDelete，因为我们不使用它
      it = buttons_.erase(it);
      // 注意：如果需要删除按钮对象，取消注释下面的代码
      // delete it.value(); // 如果 WidgetGroup 拥有按钮的所有权
    } else {
      ++it;
    }
  }
}

void WidgetGroup::handleButtonClicked() {
  // 获取选中的按钮
  TtSpecialDeleteButton *clickedButton =
      qobject_cast<TtSpecialDeleteButton *>(sender());

  QString getUUid("");
  int type = -1;
  for (auto it = buttons_.begin(); it != buttons_.end(); ++it) {
    if (it.value() == clickedButton) {
      getUUid = it.key().first;
      type = it.key().second;
    }
  }
  if (getUUid.isEmpty() || type == -1) {
    return;
  }
  if (auto *btn = findButton(current_uuid_)) {
    btn->setChecked(false);
  }

  // 重复点击, 则会造成多次发送相同信号
  // 设置新选中按钮
  clickedButton->setChecked(true);
  current_uuid_ = getUUid;
  // 当前需要切换到对应的 uuid 界面
  emit currentIndexChanged(getUUid, type);
}

TtSpecialDeleteButton *WidgetGroup::findButton(const QString &uuid) {
  // for (const auto& key : buttons_.keys()) {
  //   if (key.first == uuid) {
  //     return buttons_.value(key);
  //   }
  // }
  // return nullptr;
  // 直接遍历 map，不创建临时容器
  for (auto it = buttons_.begin(); it != buttons_.end(); ++it) {
    if (it.key().first == uuid) {
      return it.value();
    }
  }
  return nullptr;
}

} // namespace Ui
