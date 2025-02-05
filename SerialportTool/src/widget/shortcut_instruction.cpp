#include "widget/shortcut_instruction.h"
#include <qscrollbar.h>
#include <ui/widgets/buttons.h>

namespace Widget {

ShortcutInstruction::ShortcutInstruction(QWidget* parent) {
  init();
}

ShortcutInstruction::~ShortcutInstruction() {}

void ShortcutInstruction::addCustomWidget(QWidget* widget) {
  widget->adjustSize();
  QSize widgetSize = widget->size();
  QSize itemSize =
      QSize(this->width() - this->verticalScrollBar()->width() - 10,
            widgetSize.height() + 10);
  addWidgetItem(widget, itemSize);
}

void ShortcutInstruction::init() {
  // 不选中
  setSelectionMode(QAbstractItemView::NoSelection);
  // 禁用焦点, 防止显示全局焦点
  setFocusPolicy(Qt::NoFocus);
  setUniformItemSizes(true);
  setFlow(QListView::TopToBottom);
  // setSpacing(5);
  setSelectionMode(QAbstractItemView::NoSelection);
  setResizeMode(QListView::Adjust);

  // 设置自定义样式表以移除选中背景和焦点虚线框
  setStyleSheet(
      "QListWidget::item:selected {"
      "   background-color: transparent;"  // 移除选中背景色
      "   outline: none;"                  // 移除焦点虚线框
      "}"
      "QListWidget::item:focus {"
      "   outline: none;"  // 移除焦点虚线框
      "}"
      // /*
      "QListWidget::item:hover {"
      "   background-color: lightgray;"  // 可选：悬停时背景色
      "}"
      // */
  );

  this->setContentsMargins(QMargins());  // 0 边框
  this->setSpacing(0);
}

void ShortcutInstruction::addWidgetItem(QWidget* widget, QSize itemSize) {
  QListWidgetItem* item = new QListWidgetItem(this);
  item->setSizeHint(itemSize);  // 设置控件内容的大小

  // 控件的尺寸策略
  widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  widget->setFocusPolicy(Qt::NoFocus);  // 根据需求调整

  addItem(item);                // 添加项
  setItemWidget(item, widget);  // 对应项设置 widget
}

HeaderWidget::HeaderWidget(QWidget* parent) {
  init();
}

HeaderWidget::~HeaderWidget() {}

void HeaderWidget::init() {
  this->setFixedHeight(32);
  QHBoxLayout* baseLayout = new QHBoxLayout;
  baseLayout->setContentsMargins(QMargins());
  this->setLayout(baseLayout);
  QWidget* body = new QWidget(this);
  body->setFixedHeight(32);
  QHBoxLayout* layout = new QHBoxLayout;
  body->setLayout(layout);

  QLabel* isEnable = new QLabel(tr("启用"));
  QLabel* name = new QLabel(tr("名称"));
  QLabel* format = new QLabel(tr("格式"));
  QLabel* content = new QLabel(tr("内容"));
  QLabel* delay = new QLabel(tr("延时"));
  QPushButton* addSendBtn = new QPushButton();

  isEnable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  format->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  delay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  addSendBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(isEnable);
  // layout->addSpacing(10);
  layout->addWidget(name);
  layout->addSpacing(30);
  layout->addWidget(format);
  layout->addSpacing(12);
  layout->addWidget(content);
  layout->addSpacing(30);
  layout->addWidget(delay);
  layout->addSpacing(12);
  layout->addWidget(addSendBtn);

  baseLayout->addWidget(body);
}

InstructionWidget::InstructionWidget(QWidget* parent) {
  init();
}

InstructionWidget::~InstructionWidget() {}

void InstructionWidget::init() {
  this->setFixedHeight(32);
  QHBoxLayout* baseLayout = new QHBoxLayout;
  baseLayout->setContentsMargins(QMargins());
  this->setLayout(baseLayout);
  QWidget* body = new QWidget(this);
  body->setFixedHeight(32);
  QHBoxLayout* layout = new QHBoxLayout;
  body->setLayout(layout);

  auto enableBtn = new Ui::TtToggleButton(this);
  // QLabel* isEnable = new QLabel(tr("启用"));
  auto nameLineEdit = new QLineEdit(tr("名称"));
  QComboBox* formatComboBox = new QComboBox();

  QWidget* contentEdit = new QWidget();
  QHBoxLayout* contentEditLayout = new QHBoxLayout;
  QLineEdit* contentLineEdit = new QLineEdit(contentEdit);
  QPushButton* editBtn = new QPushButton(contentEdit);
  contentEditLayout->addWidget(contentLineEdit);
  contentEditLayout->addWidget(editBtn);
  contentEdit->setLayout(contentEditLayout);

  QSpinBox* delaySelect = new QSpinBox();

  QWidget* addSendBtn = new QWidget();
  // QHBoxLayout* BtnLayout = new QHBoxLayout;
  // QPushButton* contentLineEdit = new QLineEdit(contentEdit);
  // QPushButton* editBtn = new QPushButton(contentEdit);
  // contentEditLayout->addWidget(contentLineEdit);
  // contentEditLayout->addWidget(editBtn);
  // contentEdit->setLayout(contentEditLayout);

  enableBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  nameLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  formatComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  contentEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  delaySelect->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  addSendBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(enableBtn);
  // layout->addSpacing(10);
  layout->addWidget(nameLineEdit);
  layout->addSpacing(30);
  layout->addWidget(formatComboBox);
  layout->addSpacing(12);
  layout->addWidget(contentEdit);
  layout->addSpacing(30);
  layout->addWidget(delaySelect);
  layout->addSpacing(12);
  layout->addWidget(addSendBtn);

  baseLayout->addWidget(body);
}

}  // namespace Widget
