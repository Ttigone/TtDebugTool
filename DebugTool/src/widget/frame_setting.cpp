#include "widget/frame_setting.h"

#include <QComboBox>
#include <QLineEdit>

namespace Widget {

FrameSetting::FrameSetting(QWidget *parent) : QWidget(parent) {}

FrameSetting::~FrameSetting() { qDebug() << "Delete" << __FUNCTION__; }

void FrameSetting::link() {
  qDebug() << "链接信号槽";
  // 链接改变的信号
  for (const auto *comboBox : qAsConst(comboBoxes_)) {
    if (comboBox) {
      connect(comboBox, &QComboBox::currentIndexChanged, this,
              &FrameSetting::settingChanged);
    }
  }
  for (const auto *lineEdit : qAsConst(lineEdits_)) {
    if (lineEdit) { // 确保指针有效
      connect(lineEdit, &QLineEdit::textChanged, this,
              &FrameSetting::settingChanged);
    }
  }
  for (const auto *checkBox : qAsConst(checkBox_)) {
    if (checkBox) { // 确保指针有效
      connect(checkBox, &QCheckBox::toggled, this,
              &FrameSetting::settingChanged);
    }
  }
}

void FrameSetting::addComboBox(QComboBox *item) { comboBoxes_ << item; }

void FrameSetting::addLineEdit(QLineEdit *item) { lineEdits_ << item; }

void FrameSetting::addCheckBox(QCheckBox *item) { checkBox_ << item; }

} // namespace Widget
