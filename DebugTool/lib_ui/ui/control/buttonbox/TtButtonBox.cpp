#include "ui/control/buttonbox/TtButtonBox.h"

#include <QButtonGroup>

#include "ui/widgets/buttons.h"

namespace Ui {

WidgetGroup::WidgetGroup(QObject *parent)
    : QObject(parent), m_currentIndex(-1) {}

void WidgetGroup::addButton(const QString &uuid, int specialType,
                            TtSpecialDeleteButton *button) {
  // if (!buttons_.contains(uuid)) {
  //   buttons_.insert(uuid, button);
  //   connect(button, &TtSpecialDeleteButton::clicked, this,
  //           &WidgetGroup::handleButtonClicked);
  // }
  bool hadFlag = false;
  for (const auto &key : buttons_.keys()) {
    if (key.first == uuid) {
      hadFlag = true;
      // 存在相同的
      break;
    }
  }
  if (!hadFlag) {
    buttons_.insert(qMakePair(uuid, specialType), button);
    connect(button, &TtSpecialDeleteButton::clicked, this,
            &WidgetGroup::handleButtonClicked);
  }
}

void WidgetGroup::setCurrentIndex(QString index) {
  // if (index != current_uuid_) {
  //   buttons_[current_uuid_]->setChecked(false);
  //   buttons_[index]->setChecked(true);
  //   current_uuid_ = index;
  //   emit currentIndexChanged(index);
  // }
  if (index != current_uuid_) {
    if (auto *btn = findButton(current_uuid_)) {
      btn->setChecked((false));
    }
    if (auto *btn = findButton(index)) {
      btn->setChecked(true);
    }
    current_uuid_ = index;
    emit currentIndexChanged(index, -1);
  }
}

int WidgetGroup::currentIndex() const { return m_currentIndex; }

void WidgetGroup::updateUuid(const QString &index) {
  qDebug() << "remove" << index;
  // buttons_.remove(index);
  QMap<QPair<QString, int>, TtSpecialDeleteButton *>::iterator it =
      buttons_.begin();
  while (it != buttons_.end()) {
    // 检查当前键 (QPair) 的第一个元素 (QString) 是否匹配
    if (it.key().first == index) {
      TtSpecialDeleteButton *buttonToDelete = it.value();
      it = buttons_.erase(it);

      // // 3. 删除 TtSpecialDeleteButton 对象 (如果 QMap 拥有所有权)
      // // 如果其他地方管理这个对象的生命周期，则不要 delete
      // delete buttonToDelete;
      // buttonToDelete = nullptr;  // 好习惯
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
  for (auto it = buttons_.cbegin(); it != buttons_.cend(); ++it) {
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
  for (const auto &key : buttons_.keys()) {
    if (key.first == uuid) {
      return buttons_.value(key);
    }
  }
  return nullptr;
}

} // namespace Ui
