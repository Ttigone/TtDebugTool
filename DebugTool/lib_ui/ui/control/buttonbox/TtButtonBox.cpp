#include "ui/control/buttonbox/TtButtonBox.h"

#include <QButtonGroup>

#include "ui/widgets/buttons.h"

namespace Ui {

WidgetGroup::WidgetGroup(QObject* parent)
    : QObject(parent), m_currentIndex(-1) {}

void WidgetGroup::addButton(const QString& uuid,
                            TtSpecialDeleteButton* button) {
  if (!buttons_.contains(uuid)) {
    buttons_.insert(uuid, button);
    connect(button, &TtSpecialDeleteButton::clicked, this,
            &WidgetGroup::handleButtonClicked);
  }
}

void WidgetGroup::setCurrentIndex(QString index) {
  if (index != current_uuid_) {
    buttons_[current_uuid_]->setChecked(false);
    buttons_[index]->setChecked(true);
    current_uuid_ = index;
    emit currentIndexChanged(index);
  }
}

int WidgetGroup::currentIndex() const {
  return m_currentIndex;
}

void WidgetGroup::updateUuid(const QString& index) {
  qDebug() << "remove" << index;
  buttons_.remove(index);
}

void WidgetGroup::handleButtonClicked() {
  // 获取选中的按钮
  TtSpecialDeleteButton* clickedButton =
      qobject_cast<TtSpecialDeleteButton*>(sender());

  QString getUUid("");
  for (auto it = buttons_.cbegin(); it != buttons_.cend(); ++it) {
    if (it.value() == clickedButton) {
      // 点击
      getUUid = it.key();
    }
  }
  // if (getUUid.isEmpty() || getUUid == current_uuid_) {
  if (getUUid.isEmpty()) {
    return;
  }
  // 取消之前选中的按钮
  if (buttons_.contains(current_uuid_)) {
    buttons_.value(current_uuid_)->setChecked(false);
  }
  // 设置新选中按钮
  clickedButton->setChecked(true);
  current_uuid_ = getUUid;
  // 当前需要切换到对应的 uuid 界面
  emit currentIndexChanged(getUUid);

}

}  // namespace Ui
