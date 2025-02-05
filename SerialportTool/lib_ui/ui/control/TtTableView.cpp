// QTableView* table = new QTableView(la_w);
// // 取消底色

// table->setSelectionMode(QAbstractItemView::NoSelection);
// table->verticalHeader()->setVisible(false);
// table->horizontalHeader()->setVisible(false);

// CustomModel* model = new CustomModel();
// table->setModel(model);

// 设置委托
// ButtonDelegate* buttonDelegate = new ButtonDelegate(model, table);
// ButtonDelegate* buttonDelegate = new ButtonDelegate(table);
// table->setItemDelegateForColumn(0, buttonDelegate);

// LineEditDelegate* lineEditDelegate = new LineEditDelegate(table);
// table->setItemDelegateForColumn(1, lineEditDelegate);

// ComboBoxDelegate* comboBoxDelegate = new ComboBoxDelegate(table);
// table->setItemDelegateForColumn(2, comboBoxDelegate);
// 创建并设置 MultiDelegate
// MultiDelegate* multiDelegate = new MultiDelegate();
// table->setItemDelegate(multiDelegate);

// // 开启持久化编辑器
// for (int row = 0; row < model->rowCount(); ++row) {
//   for (int col = 0; col < model->columnCount(); ++col) {
//     QModelIndex index = model->index(row, col);
//     if (col == 0 || col == 1 || col == 2) {
//       // if (col < 6) {
//       table->openPersistentEditor(index);
//     }
//   }
// }
// // 连接行插入信号，以便新行也开启持久化编辑器
// QObject::connect(model, &CustomModel::rowsInserted,
//                  [model, table](const QModelIndex&, int start, int end) {
//                    for (int row = start; row <= end; ++row) {
//                      // qDebug() << model->columnCount();
//                      for (int col = 0; col < model->columnCount(); ++col) {
//                        // qDebug() << "inside";
//                        if (col == 0 || col == 1 || col == 2) {
//                          // if (col < 6) {
//                          table->openPersistentEditor(model->index(row, col));
//                        }
//                      }
//                    }
//                  });
// 连接按钮点击信号
// QObject::connect(multiDelegate, &MultiDelegate::buttonClicked,
//                  [&](const QModelIndex& index) {
//                    if (index.row() == 0 &&
//                        index.column() == 2) {  // 首行最后一列的“添加”按钮
//                      model->addRow();
//                    }
//                  });

#include "ui/control/TtTableView.h"

#include <ui/widgets/buttons.h>
#include <ui/widgets/fields/customize_fields.h>

#include <qheaderview.h>
#include <qlineedit.h>
#include <QSpinBox>
#include <QTableWidgetItem>

namespace Ui {

TtTableWidget::TtTableWidget(QWidget* parent) : QTableWidget(parent) {
  init();
}

TtTableWidget::~TtTableWidget() {}

void TtTableWidget::setupHeaderRow() {

  setCellWidget(0, 0, createHeaderEnableWidget());
  setCellWidget(0, 1, createHeaderNameWidget());
  setCellWidget(0, 2, createHeaderFormatWidget());
  setCellWidget(0, 3, createHeaderContentWidget());
  setCellWidget(0, 4, createHeaderDelayWidget());
  setCellWidget(0, 5, createHeaderAddRowWidget());
  setCellWidget(0, 6, createHeaderSendMsgWidget());
}

void TtTableWidget::onAddRowButtonClicked() {
  // 在表格末尾插入新行
  int newRowIndex = rowCount();
  insertRow(newRowIndex);

  // 为新行的每一列创建固定的widget
  setCellWidget(newRowIndex, 0, createFirstColumnWidget());
  setCellWidget(newRowIndex, 1, createSecondColumnWidget());
  setCellWidget(newRowIndex, 2, createThirdColumnWidget());
  setCellWidget(newRowIndex, 3, createFourthColumnWidget());
  setCellWidget(newRowIndex, 4, createFifthColumnWidget());
  setCellWidget(newRowIndex, 5, createSixthColumnWidget());
  setCellWidget(newRowIndex, 6, createSeventhColumnWidget());
}

void TtTableWidget::init() {

  // setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // 样式设置
  setStyleSheet(
      "QTableWidget {"
      "   background-color: transparent;"  // 背景透明
      "   border: none;"                   // 无外边框
      "   outline: 0;"                     // 去除外边框
      "}"
      "QTableWidget::item {"
      "   border: none;"                      // 先清除所有边框
      "   border-top: 1px solid #f0f0f0;"     // 只添加底部边框
      "   border-bottom: 1px solid #f0f0f0;"  // 只添加底部边框
      // "   padding: 5px;"                      // 单元格内边距
      "}"
      "QTableWidget::item:selected {"
      "   background-color: transparent;"  // 选中项背景透明
      "   color: black;"                   // 选中项文字颜色
      "   border: none;"                   // 确保选中项无边框
      // "   border-bottom: 1px solid #E5E5E5;"  // 选中项只保留底部边框
      "}"
      "QTableWidget:focus {"
      "   outline: none;"  // 去除焦点边框
      "}");

  setSelectionMode(QAbstractItemView::NoSelection);
  this->verticalHeader()->setVisible(false);
  this->horizontalHeader()->setVisible(false);

  setShowGrid(false);              // 关闭网格线
  setFrameStyle(QFrame::NoFrame);  // 移除表格框架

  // 设置行列数
  setRowCount(1);
  setColumnCount(7);

  setupHeaderRow();

  // horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  // horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  // for (int col = 0; col < columnCount(); ++col) {
  //   setColumnWidth(col, 20);  // 这里的120是最小宽度示例
  // }
  // 设置最小宽度
  // setColumnWidth(0, 64);
  // setColumnWidth(1, 260);
  // setColumnWidth(2, 124);
  // setColumnWidth(3, 260);
  // setColumnWidth(4, 130);
  // setColumnWidth(5, 80);
  // setColumnWidth(6, 64);

  // 固定第一列宽度
  horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  setColumnWidth(0, 64);  // 固定第一列宽度为 64px

  // 第二列到第五列自适应宽度
  horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

  // 固定第六列宽度
  horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
  setColumnWidth(5, 64);  // 固定第六列宽度为 80px

  // 最后一列自适应宽度
  horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
  setColumnWidth(6, 42);  // 固定第六列宽度为 80px
}

QWidget* TtTableWidget::createHeaderEnableWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);

  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());

  QLabel* isEnable = new QLabel(tr("启用"));
  isEnable->setStyleSheet("border:none");

  layout->addWidget(isEnable, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderNameWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);

  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(6, 0, 0, 0);

  QLabel* name = new QLabel(tr("名称"));
  // QTableWidget 项会影响
  name->setStyleSheet("border:none");

  layout->addWidget(name, 0, Qt::AlignLeft);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderFormatWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* format = new QLabel(tr("格式"));
  format->setStyleSheet("border:none");

  layout->addWidget(format, 0, Qt::AlignLeft);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");

  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderContentWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* content = new QLabel(tr("内容"));
  content->setStyleSheet("border:none");

  layout->addWidget(content, 0, Qt::AlignCenter);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderDelayWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* delay = new QLabel(tr("延时"));
  delay->setStyleSheet("border:none");

  layout->addWidget(delay, 0, Qt::AlignCenter);
  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderAddRowWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* addSendBtn = new TtImageButton(":/sys/plus-circle.svg");

  connect(addSendBtn, &TtImageButton::clicked, this,
          &TtTableWidget::onAddRowButtonClicked);

  layout->addWidget(addSendBtn);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderSendMsgWidget() {
  // QWidget* container = new QWidget(this);
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* sendBtn = new TtImageButton(":/sys/send.svg");

  layout->addWidget(sendBtn);
  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setPaintRightBorder(false);
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createFirstColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  TtToggleButton* isEnableBtn = new TtToggleButton(container);
  layout->addWidget(isEnableBtn);

  return container;
}

QWidget* TtTableWidget::createSecondColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());

  // 创建下拉框和数字输入框
  Widgets::TtCustomizeFields* lineEdit =
      // new Widgets::TtCustomizeFields(container);
      new Widgets::TtCustomizeFields(tr("名称"), container);
  // lineEdit->setMinimumWidth(260);  // 无法生效
  // lineEdit->setFixedWidth(260);  // 无法生效
  lineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtTableWidget::createThirdColumnWidget() {

  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  // 创建下拉框和数字输入框
  QComboBox* comboBox = new QComboBox(container);
  comboBox->addItems({tr("TEXT"), tr("HEX")});

  layout->addWidget(comboBox);

  return container;
}

QWidget* TtTableWidget::createFourthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());

  // // 创建下拉框和数字输入框
  // QLineEdit* lineEdit = new QLineEdit(container);
  // 创建下拉框和数字输入框
  Widgets::TtCustomizeFields* lineEdit =
      // new Widgets::TtCustomizeFields(container);
      new Widgets::TtCustomizeFields(tr("内容"), container);
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtTableWidget::createFifthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  QSpinBox* spinBox = new QSpinBox(container);

  layout->addWidget(spinBox);

  return container;
}

QWidget* TtTableWidget::createSixthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* deleteBtn = new TtImageButton(":/sys/trash.svg", container);

  layout->addWidget(deleteBtn);

  // 连接删除按钮信号
  QObject::connect(deleteBtn, &TtImageButton::clicked, this, [=]() {
    int row = this->rowAt(container->pos().y());
    if (row > 0) {  // 确保不删除首行
      this->removeRow(row);
    }
  });

  return container;
}

QWidget* TtTableWidget::createSeventhColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* sendBtn = new TtImageButton(":/sys/send.svg", container);

  layout->addWidget(sendBtn);

  // 连接删除按钮信号
  // connect(delButton, &QPushButton::clicked, this, [=]() {
  //   int row = this->rowAt(container->pos().y());
  //   if (row > 0) {  // 确保不删除首行
  //     this->removeRow(row);
  //   }
  // });

  return container;
}

}  // namespace Ui
