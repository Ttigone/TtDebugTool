#include "ui/controls/TtTableView.h"

#include <qcombobox.h>
#include <qlogging.h>
#include <qtablewidget.h>
#include <qwidget.h>
#include <ui/control/TtCheckBox.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtSwitchButton.h>

#include "ui/layout/horizontal_layout.h"
#include "ui/widgets/buttons.h"

#include <qheaderview.h>
#include <qlineedit.h>
#include <QSpinBox>
#include <QTableWidgetItem>

#include <ui/layout/vertical_layout.h>

namespace Ui {

TtTableWidget::TtTableWidget(QWidget* parent) : QTableWidget(1, 7, parent) {

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
  // this->verticalHeader()->hide();
  // this->horizontalHeader()->hide();
  setShowGrid(false);  // 关闭网格线
  setSelectionMode(NoSelection);
  setFrameStyle(QFrame::NoFrame);  // 移除表格框架

  // setupHeaderRow();

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

  horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
  setColumnWidth(0, 64);
  setColumnWidth(5, 64);
  setColumnWidth(6, 42);

  // 第二列到第五列自适应宽度
  horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

  initHeader();
}

TtTableWidget::~TtTableWidget() {}

void TtTableWidget::setupHeaderRow() {
  setCellWidget(0, 0, createHeaderWidget(tr("启用"), true));
  setCellWidget(0, 1, createHeaderWidget(tr("名称"), true));
  setCellWidget(0, 2, createHeaderWidget(tr("格式"), true));
  setCellWidget(0, 3, createHeaderWidget(tr("内容"), true));
  setCellWidget(0, 4, createHeaderWidget(tr("延时"), true));
  setCellWidget(0, 5, createHeaderAddRowWidget());
  setCellWidget(0, 6, createHeaderSendMsgWidget());
}

void TtTableWidget::setupTable(const QJsonObject& record) {
  record_ = record;
}

QJsonObject TtTableWidget::getTableRecord() {
  // int rows = rowCount();
  // int cols = columnCount();
  // for (int i = 1; i < rows; ++i) {
  //   QJsonArray record;
  //   for (int j = 0; j < cols; ++j) {
  //     switch (j) {
  //       case 0: {
  //         TtSwitchButton* item =
  //             cellWidget(i, j)->findChild<TtSwitchButton*>("isEnableBtn");
  //         if (item) {
  //           bool isEnabled = item->isChecked();
  //           //qDebug() << "test: " << isEnabled;
  //           record.append(QJsonValue(isEnabled));
  //         }
  //         break;
  //       }
  //       case 1: {
  //         TtLineEdit* item = cellWidget(i,
  //         j)->findChild<TtLineEdit*>("name"); if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 2: {
  //         QComboBox* item = cellWidget(i, j)->findChild<QComboBox*>("type");
  //         if (item) {
  //           auto text = item->currentText();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 3: {
  //         TtLineEdit* item =
  //             cellWidget(i, j)->findChild<TtLineEdit*>("content");
  //         if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 4: {
  //         QSpinBox* item = cellWidget(i, j)->findChild<QSpinBox*>("delay");
  //         if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //     }
  //   }
  //   // 序号, 数组
  //   record_.insert(QString::number(i), record);
  // }
  // return record_;
  // QJsonObject record;
  // for (int row = 1; row < rowCount(); ++row) {
  //   QJsonArray rowData;
  //   for (int col = 0; col < columnCount(); ++col) {
  //     // 特定行列
  //     QWidget* cell = cellWidget(row, col);
  //     if (!cell)
  //       continue;
  //     // 直接从缓存获取控件
  //     auto cache = cellWidgetCache_.value(cell);
  //     switch (col) {
  //       case 0: {
  //         TtSwitchButton* btn = cache.value<TtSwitchButton*>("isEnableBtn");
  //         rowData.append(btn->isChecked());
  //         break;
  //       }
  //         // 其他列类似处理...
  //     }
  //   }
  //   record[QString::number(row)] = rowData;
  // }
  // return record;
  QJsonObject root;
  qDebug() << "size: " << rowsData_.size();
  for (int i = 0; i < rowsData_.size(); ++i) {
    const auto& row = rowsData_[i];
    QJsonArray data;
    data.append(row.enableBtn->isChecked());
    data.append(row.nameEdit->text());
    data.append(row.typeCombo->currentText());
    data.append(row.contentEdit->text());
    data.append(row.delaySpin->value());
    root[QString::number(i + 1)] = data;
  }
  return root;
}

void TtTableWidget::setCellWidget(int row, int column, QWidget* widget) {
  QTableWidget::setCellWidget(row, column, widget);
  cellWidgetCache_[widget][row] = widget;  // 缓存控件
}

void TtTableWidget::onAddRowButtonClicked() {
  // 在表格末尾插入新行
  int newRowIndex = rowCount();
  // qDebug() << newRowIndex;
  insertRow(newRowIndex);
  setupRow(newRowIndex);

  // // 为新行的每一列创建固定的widget
  // setCellWidget(newRowIndex, 0, createFirstColumnWidget());
  // setCellWidget(newRowIndex, 1, createSecondColumnWidget());
  // setCellWidget(newRowIndex, 2, createThirdColumnWidget());
  // setCellWidget(newRowIndex, 3, createFourthColumnWidget());
  // setCellWidget(newRowIndex, 4, createFifthColumnWidget());
  // setCellWidget(newRowIndex, 5, createSixthColumnWidget());
  // setCellWidget(newRowIndex, 6, createSeventhColumnWidget());
  // // 确保调整大小
  resizeRowsToContents();
  resizeColumnsToContents();
}

void TtTableWidget::initHeader() {
  QStringList headers = {tr("启用"), tr("名称"), tr("格式"), tr("内容"),
                         tr("延时"), "",         ""};

  for (int col = 0; col < 7; ++col) {
    QWidget* header = nullptr;
    if (col == 5) {
      header = createAddButton();
    } else if (col == 6) {
      header = createSendButton();
    } else if (col < 5) {
      header = createHeaderCell(headers[col], col != 4);
    }

    setCellWidget(0, col, header);
  }
}

void TtTableWidget::setupRow(int row) {
  if (!isRowVisible(row)) {
    // 可见时才创建控件
    return;
  }

  TableRow data;
  data.enableBtn = createSwitchButton();
  data.enableBtn->setChecked(true);
  data.nameEdit = new TtLineEdit(this);
  data.typeCombo = createTypeComboBox();
  data.contentEdit = new TtLineEdit(this);
  data.delaySpin = createDelaySpin();

  auto makeCell = [this](QWidget* content) {
    return createCellWrapper(content);
  };

  setCellWidget(row, 0, makeCell(data.enableBtn));
  setCellWidget(row, 1, makeCell(data.nameEdit));
  setCellWidget(row, 2, makeCell(data.typeCombo));
  setCellWidget(row, 3, makeCell(data.contentEdit));
  setCellWidget(row, 4, makeCell(data.delaySpin));
  setCellWidget(row, 5, createDeleteButton());
  setCellWidget(row, 6, createRowSendButton());

  rowsData_.append(data);
}

void TtTableWidget::recycleRow(TableRow& row) {
  // if (row.enableBtn) {
  //   row.enableBtn->hide();
  //   switchPool_.append(row.enableBtn);
  // }
  // if (row.typeCombo) {
  //   row.typeCombo->hide();
  //   comboPool_.append(row.typeCombo);
  // }
  // if (row.delaySpin) {
  //   row.delaySpin->hide();
  //   spinPool_.append(row.delaySpin);
  // }
  // if (row.nameEdit) {
  //   row.nameEdit->hide();
  //   spinPool_.append(row.nameEdit);
  // }
}

TtSwitchButton* TtTableWidget::createSwitchButton() {
  if (!switchPool_.isEmpty()) {
    auto btn = switchPool_.takeLast();
    btn->show();
    return btn;
  }
  return new TtSwitchButton(this);
}

TtComboBox* TtTableWidget::createTypeComboBox() {
  TtComboBox* combo =
      comboPool_.isEmpty() ? new TtComboBox(this) : comboPool_.takeLast();
  combo->clear();
  combo->addItems({tr("TEXT"), tr("HEX")});
  return combo;
}

QSpinBox* TtTableWidget::createDelaySpin() {
  QSpinBox* spin =
      spinPool_.isEmpty() ? new QSpinBox(this) : spinPool_.takeLast();
  spin->setMinimum(0);
  spin->setMaximum(9999);
  return spin;
}

QWidget* TtTableWidget::createCellWrapper(QWidget* content) {
  // QWidget* wrapper =
  //     widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  // Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(wrapper);
  // layout->addWidget(content);
  // return wrapper;
  if (content->metaObject()->className() == QString("TtVerticalLayout")) {
    return content;
  }
  QWidget* wrapper =
      widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(wrapper);
  layout->setContentsMargins(2, 2, 2, 2);
  layout->addWidget(content);
  return wrapper;
}

int TtTableWidget::findRowIndex(QWidget* context, const int& col,
                                bool deep) const {
  if (!context) {
    return -1;
  }
  QWidget* parent = context->parentWidget();
  if (!parent) {
    return -1;
  }

  if (!deep) {
    for (int row = 1; row < rowCount(); ++row) {
      if (cellWidget(row, col) == parent) {
        return row;
      }
    }
  } else {
    QWidget* grandparent = parent->parentWidget();
    if (!grandparent) {
      return -1;
    }
    for (int row = 1; row < rowCount(); ++row) {
      if (cellWidget(row, col) == grandparent) {
        return row;
      }
    }
  }
  return -1;
}

QWidget* TtTableWidget::createHeaderCell(const QString& text, bool border) {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(border);
  return container;
}

QWidget* TtTableWidget::createAddButton() {
  auto* btn = new QPushButton(QIcon(":/sys/plus-circle.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this,
          [this] { onAddRowButtonClicked(); });
  return createCellWrapper(btn);
}

QWidget* TtTableWidget::createSendButton() {
  auto* btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  // 群发
  connect(btn, &QPushButton::clicked, this, [this]() {
    qDebug() << "clicked";
    QVector<QPair<QString, int>> msg;
    for (int i = 1; i < rowCount(); ++i) {
      if (rowsData_.at(i - 1).enableBtn->isChecked()) {
        // 还有延时部分
        msg.append(qMakePair(rowsData_.at(i - 1).contentEdit->text(),
                             rowsData_.at(i - 1).delaySpin->text().toInt()));
      }
    }
    emit sendRowsMsg(msg);
  });
  return createCellWrapper(btn);
}

QWidget* TtTableWidget::createDeleteButton() {
  auto* btn = new QPushButton(QIcon(":/sys/trash.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto* btn = qobject_cast<QPushButton*>(sender())) {
      int row = findRowIndex(btn, 5);
      if (row > 0) {
        // 回收控件
        recycleRow(rowsData_[row - 1]);
        // 移除行
        removeRow(row);
        rowsData_.remove(row - 1);
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget* TtTableWidget::createRowSendButton() {
  auto* btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto* btn = qobject_cast<QPushButton*>(sender())) {
      int row = findRowIndex(btn, 6);
      if (row > 0) {
        emit sendRowMsg(rowsData_[row - 1].contentEdit->text());
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget* TtTableWidget::createHeaderWidget(const QString& text,
                                           bool paintBorder) {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(paintBorder);
  return container;
}

QWidget* TtTableWidget::createHeaderAddRowWidget() {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* addSendBtn = new TtImageButton(":/sys/plus-circle.svg");
  addSendBtn->setFixedSize(22, 22);

  connect(addSendBtn, &TtImageButton::clicked, this,
          &TtTableWidget::onAddRowButtonClicked);

  layout->addWidget(addSendBtn);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtTableWidget::createHeaderSendMsgWidget() {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* sendBtn = new TtImageButton(":/sys/send.svg");
  sendBtn->setFixedSize(22, 22);
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

  TtSwitchButton* isEnableBtn = new TtSwitchButton(container);
  isEnableBtn->setObjectName("isEnableBtn");  // 设置对象名称
  layout->addWidget(isEnableBtn);

  return container;
}

QWidget* TtTableWidget::createSecondColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit* lineEdit = new TtLineEdit(tr("名称"), container);
  lineEdit->setObjectName("name");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtTableWidget::createThirdColumnWidget() {
  // QWidget* container = new QWidget(this);
  // QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);

  // // 创建下拉框和数字输入框
  // Ui::TtComboBox* comboBox = new Ui::TtComboBox(container);
  // comboBox->setObjectName("type");  // 设置对象名称
  // comboBox->addItems({tr("TEXT"), tr("HEX")});

  // layout->addWidget(comboBox);

  // return container;
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(0, 0, 0, 0);

  Ui::TtComboBox* comboBox = nullptr;
  if (!comboPool_.isEmpty()) {
    comboBox = comboPool_.takeLast();  // 从池中取出
  } else {
    comboBox = new Ui::TtComboBox(this);  // 池为空时新建
    comboBox->addItems({tr("TEXT"), tr("HEX")});
  }
  comboBox->setObjectName("type");
  layout->addWidget(comboBox);
  return container;
}

QWidget* TtTableWidget::createFourthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit* lineEdit = new TtLineEdit(tr("内容"), container);
  lineEdit->setObjectName("content");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtTableWidget::createFifthColumnWidget() {
  QWidget* container = new QWidget(this);
  container->setMinimumHeight(32);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  QSpinBox* spinBox = new QSpinBox(container);
  spinBox->setObjectName("delay");  // 设置对象名称
  layout->addWidget(spinBox);

  return container;
}

QWidget* TtTableWidget::createSixthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* deleteBtn = new TtImageButton(":/sys/trash.svg", container);
  deleteBtn->setFixedSize(22, 22);
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
  sendBtn->setFixedSize(22, 22);
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

int TtTableWidget::visibleRowCount() {
  // 计算表格视图中可见的行数
  if (!isVisible() || height() <= horizontalHeader()->height()) {
    return 0;
  }

  // 可见区域高度减去表头高度
  int availableHeight = viewport()->height();

  // 没有行或行高为0则返回0
  if (rowCount() <= 0 || rowHeight(0) <= 0) {
    return 0;
  }

  // 假设所有行高度相同
  return availableHeight / rowHeight(0);
}

TtTableWidget::HeaderWidget::~HeaderWidget() {}

void TtTableWidget::HeaderWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);

  if (paint_) {
    QPainter painter(this);
    // painter.setPen(QPen(QColor("#212121")));  // 设置边框颜色
    painter.setPen(QPen(QColor("#c6c6c6")));  // 设置边框颜色

    // 画一个右边框，只在自定义区域内
    // qDebug() << this->height();
    painter.drawLine(width() - 1, 4, width() - 1, this->height() - 4);
  }
}

TtModbusTableWidget::TtModbusTableWidget(TtModbusRegisterType::Type type,
                                         QWidget* parent)
    : QTableWidget(1, 6, parent), type_(type) {
  // 1 行流列
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
      "   background-color: transparent;"     // 选中项背景透明
      "   color: black;"                      // 选中项文字颜色
      "   border: none;"                      // 确保选中项无边框
      "   border-bottom: 1px solid #E5E5E5;"  // 选中项只保留底部边框
      "}"
      "QTableWidget:focus {"
      "   outline: none;"  // 去除焦点边框
      "}");

  setSelectionMode(QAbstractItemView::NoSelection);
  this->verticalHeader()->setVisible(false);
  this->horizontalHeader()->setVisible(false);
  // this->verticalHeader()->hide();
  // this->horizontalHeader()->hide();
  setShowGrid(false);  // 关闭网格线
  setSelectionMode(NoSelection);
  setFrameStyle(QFrame::NoFrame);  // 移除表格框架

  // setupHeaderRow();

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

  horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
  // horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
  setColumnWidth(0, 64);
  setColumnWidth(5, 64);
  // setColumnWidth(6, 42);

  // 第二列到第五列自适应宽度
  horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

  initHeader();

  connectSignals();
}

TtModbusTableWidget::~TtModbusTableWidget() {}

void TtModbusTableWidget::setRowValue(int row, int col, const QString& data) {}

QVector<int> TtModbusTableWidget::getAddressValue() {
  auto values = getRowValue(1);
  QVector<int> results;
  for (auto it = values.cbegin(); it != values.cend(); ++it) {
    results.append(it->toInt());
  }
  // qDebug() << "get address: " << results;
  return results;
}

void TtModbusTableWidget::setValue(const QString& data) {
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget* widget = cellWidget(i, 3);
    if (widget) {
      TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
      if (lineEdit) {
        lineEdit->setText(data);
      }
    }
  }
}

void TtModbusTableWidget::setValue(const int& addr,
                                   const QVector<quint16>& data) {
  switch (type_) {
    case TtModbusRegisterType::Coils: {  // Coils
      for (int i = 1; i < this->rowCount(); ++i) {
        QWidget* widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget* widget = cellWidget(i, 3);
            if (widget) {
              TtSwitchButton* btn = widget->findChild<TtSwitchButton*>();
              if (btn) {
                btn->setChecked(data[0]);
              }
            }
          }
        }
      }
      break;
    }
    case TtModbusRegisterType::DiscreteInputs: {
      for (int i = 1; i < this->rowCount(); ++i) {
        QWidget* widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget* widget = cellWidget(i, 3);
            if (widget) {
              TtSwitchButton* btn = widget->findChild<TtSwitchButton*>();
              if (btn) {
                btn->setChecked(data[0]);
              }
            }
          }
        }
      }
      break;
    }
    case TtModbusRegisterType::HoldingRegisters: {  // Holding
      for (int i = 1; i < this->rowCount(); ++i) {
        QWidget* widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget* widget = cellWidget(i, 3);
            if (widget) {
              TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
              if (lineEdit) {
                lineEdit->setText(QString::number(data[0]));
              }
            }
          }
        }
      }
      break;
    }
    case TtModbusRegisterType::InputRegisters: {
      break;
    }
  }
}

void TtModbusTableWidget::setValue(const QVector<quint16>& data) {
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget* widget = cellWidget(i, 3);
    if (widget) {
      TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
      if (lineEdit) {
        lineEdit->setText(QString::number((data[i - 1])));
      }
    }
  }
}

void TtModbusTableWidget::setTable(const QJsonObject& record) {
  record_ = record;
}

QJsonObject TtModbusTableWidget::getTableRecord() {
  // int rows = rowCount();
  // int cols = columnCount();
  // for (int i = 1; i < rows; ++i) {
  //   QJsonArray record;
  //   for (int j = 0; j < cols; ++j) {
  //     switch (j) {
  //       case 0: {
  //         TtSwitchButton* item =
  //             cellWidget(i, j)->findChild<TtSwitchButton*>("isEnableBtn");
  //         if (item) {
  //           bool isEnabled = item->isChecked();
  //           //qDebug() << "test: " << isEnabled;
  //           record.append(QJsonValue(isEnabled));
  //         }
  //         break;
  //       }
  //       case 1: {
  //         TtLineEdit* item = cellWidget(i,
  //         j)->findChild<TtLineEdit*>("name"); if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 2: {
  //         QComboBox* item = cellWidget(i, j)->findChild<QComboBox*>("type");
  //         if (item) {
  //           auto text = item->currentText();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 3: {
  //         TtLineEdit* item =
  //             cellWidget(i, j)->findChild<TtLineEdit*>("content");
  //         if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //       case 4: {
  //         QSpinBox* item = cellWidget(i, j)->findChild<QSpinBox*>("delay");
  //         if (item) {
  //           auto text = item->text();
  //           //qDebug() << "test: " << text;
  //           record.append(QJsonValue(text));
  //         }
  //         break;
  //       }
  //     }
  //   }
  //   // 序号, 数组
  //   record_.insert(QString::number(i), record);
  // }
  // return record_;
  // QJsonObject record;
  // for (int row = 1; row < rowCount(); ++row) {
  //   QJsonArray rowData;
  //   for (int col = 0; col < columnCount(); ++col) {
  //     // 特定行列
  //     QWidget* cell = cellWidget(row, col);
  //     if (!cell)
  //       continue;
  //     // 直接从缓存获取控件
  //     auto cache = cellWidgetCache_.value(cell);
  //     switch (col) {
  //       case 0: {
  //         TtSwitchButton* btn = cache.value<TtSwitchButton*>("isEnableBtn");
  //         rowData.append(btn->isChecked());
  //         break;
  //       }
  //         // 其他列类似处理...
  //     }
  //   }
  //   record[QString::number(row)] = rowData;
  // }
  // return record;
  QJsonObject root;
  qDebug() << "size: " << rowsData_.size();
  for (int i = 0; i < rowsData_.size(); ++i) {
    const auto& row = rowsData_[i];
    if (!row.checkBtn || !row.address || !row.addressName) {
      qWarning() << "跳过行 " << i << ": 基本要素缺失";
      continue;  // 跳过无效行
    }
    QJsonArray data;
    data.append(row.checkBtn->isChecked());
    data.append(row.address->text());
    data.append(row.addressName->text());
    switch (type_) {
      case TtModbusRegisterType::Coils:
      case TtModbusRegisterType::DiscreteInputs:
        // 线圈和离散输入
        if (!row.valueButton) {
          qWarning() << "跳过行 " << i << ": valueButton 为空";
          continue;
        }
        // FIXME 数据选项不需要保存
        data.append(row.valueButton->isChecked());
        break;
      case TtModbusRegisterType::HoldingRegisters:
      case TtModbusRegisterType::InputRegisters:
        // 保持寄存器和输入寄存器
        if (!row.value) {
          qWarning() << "跳过行 " << i << ": value 为空";
          continue;
        }
        // FIXME 数据选项不需要保存
        data.append(row.value->text());
        break;
    }
    if (!row.description) {
      qWarning() << "跳过行 " << i << ": description 为空";
      continue;
    }
    data.append(row.description->text());
    root[QString::number(i + 1)] = data;
  }
  qDebug() << "root: " << root;
  return root;
}

void TtModbusTableWidget::setCellWidget(int row, int column, QWidget* widget) {
  // 不是居中显示的
  QTableWidget::setCellWidget(row, column, widget);
  cellWidgetCache_[widget][row] = widget;  // 缓存控件
}

void setEnable(bool enable) {
  qDebug() << "SetEnable" << enable;
}

QVector<QString> TtModbusTableWidget::getRowValue(int col) {
  QVector<QString> result;
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget* widget = cellWidget(i, col);
    if (widget) {
      TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
      if (lineEdit) {
        if (!lineEdit->text().isEmpty()) {
          result.append(lineEdit->text());
        }
      }
    }
  }
  return result;
}

void TtModbusTableWidget::setupVisibleRows() {
  // 滚动条滚动时触发
  // 处理当前可见区域中所有行
  // for (int row = 1; row < rowCount(); ++row) {
  //   // 当前行是否可见
  //   if (isRowVisible(row)) {
  //     // 检查该行是否已有控件
  //     QWidget *cellWidget = this->cellWidget(row, 0);
  //     if (!cellWidget || cellWidget->children().isEmpty()) {
  //       // 该行没有控件，需要创建
  //       setupRow(row);
  //     }
  //   }
  // }
  bool needResize = false;
  for (int row = 1; row < rowCount(); ++row) {
    // 当前行是否可见
    if (isRowVisible(row)) {
      // 检查该行是否已有控件
      // 获取当前行的第一个单元格控件, 判断是否存在
      QWidget* cellWidget = this->cellWidget(row, 0);
      if (!cellWidget || cellWidget->children().isEmpty()) {
        // 创建了控件
        setupRow(row);
        needResize = true;
      }
    } else if (rowHeight(row) < 30) {
      // 高度小于最小高度 30
      needResize = true;
    }
  }
  if (needResize) {
    QTimer::singleShot(0, this, [this]() {
      // 可以调整某一特定行的高度吗
      resizeRowsToContents();
    });
  }
}

void TtModbusTableWidget::addRow() {
  // // 添加行
  // int newRowIndex = rowCount();
  // insertRow(newRowIndex);
  // setupRow(newRowIndex);

  // // BUG 删除行后, 重新添加行, 会出现添加空白的清空, 并且横线存在
  // qDebug() << "y";

  // // 调整大小
  // resizeRowsToContents();
  // resizeColumnsToContents();

  // 添加行
  int newRowIndex = rowCount();  // 当前的行数
  insertRow(newRowIndex);        // 插入新行

  // 默认处于可视状态
  bool wasVisible = true;
  // 尾行的最后 2 行一定是可见区域
  if (!isRowVisible(newRowIndex)) {
    // 不可视状态
    wasVisible = false;
    // 滚动到插入的新行
    scrollToItem(item(newRowIndex, 0));
  }
  // 点击 addRow, 才会重新调整底部添加的信号
  // 强制创建控件
  setupRow(newRowIndex);
  // 调整行高
  resizeRowsToContents();
  // 调整列宽
  resizeColumnsToContents();
  if (!wasVisible) {
    // 不可见
    scrollToItem(item(newRowIndex, 0));
  }
}

void TtModbusTableWidget::showEvent(QShowEvent* event) {
  QTableWidget::showEvent(event);
  // 确保在显示时设置可见行
  QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
}

void TtModbusTableWidget::resizeEvent(QResizeEvent* event) {
  QTableWidget::resizeEvent(event);
  // 调整 size 后, 重新设置可见行
  QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
}

void TtModbusTableWidget::onValueChanged() {
  TtLineEdit* valueEdit = qobject_cast<TtLineEdit*>(sender());
  if (!valueEdit)
    return;

  // 查找对应的行
  for (int i = 0; i < rowsData_.size(); ++i) {
    if (rowsData_[i].value == valueEdit) {
      rowsData_[i].confirmButton->show();
      rowsData_[i].cancelButton->show();
      rowsData_[i].value->setReadOnly(true);
      // emit valueModified(i + 1); // +1 因为首行是标题
      break;
    }
  }
}

void TtModbusTableWidget::onConfirmClicked() {
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if (!btn)
    return;

  // 查找对应的行
  for (int i = 0; i < rowsData_.size(); ++i) {
    if (rowsData_[i].confirmButton == btn) {
      rowsData_[i].originalValue = rowsData_[i].value->text();
      btn->hide();
      rowsData_[i].cancelButton->hide();
      rowsData_[i].editButton->show();
      rowsData_[i].value->setReadOnly(true);
      emit valueConfirmed(rowsData_[i].address->text().toInt(),
                          rowsData_[i].value->text().toInt());
      break;
    }
  }
}

void TtModbusTableWidget::onCancelClicked() {
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if (!btn)
    return;

  // 查找对应的行
  for (int i = 0; i < rowsData_.size(); ++i) {
    if (rowsData_[i].cancelButton == btn) {
      rowsData_[i].value->setText(rowsData_[i].originalValue);
      btn->hide();
      rowsData_[i].confirmButton->hide();
      rowsData_[i].editButton->show();
      rowsData_[i].value->setReadOnly(true);
      // emit valueCancelled(i + 1);
      break;
    }
  }
}

void TtModbusTableWidget::onSwitchButtonToggle(bool toggled) {
  TtSwitchButton* btn = qobject_cast<TtSwitchButton*>(sender());
  if (!btn) {
    return;
  }
  // qDebug() << "switchBtn clicked";

  for (int i = 0; i < rowsData_.size(); ++i) {
    if (rowsData_[i].valueButton == btn) {
      emit valueConfirmed(rowsData_[i].address->text().toInt(),
                          toggled ? 1 : 0);
      break;
    }
  }
  // qDebug() << "yes";
}

void TtModbusTableWidget::adjustRowHeights() {
  for (int row = 1; row < rowCount(); ++row) {
    if (isRowVisible(row)) {
      if (rowHeight(row) < 30) {
        setRowHeight(row, 30);
      }
    }
  }
}

void TtModbusTableWidget::connectSignals() {
  // connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
  //         &TtModbusTableWidget::setupVisibleRows);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
    // 创建控件
    setupVisibleRows();
    // 调整行高
    QTimer::singleShot(10, this, &TtModbusTableWidget::adjustRowHeights);
  });

  connect(
      this, &QTableWidget::windowTitleChanged, this, [this](const QString&) {
        // 使用windowTitleChanged作为一个安全的信号，以便在UI完全加载后触发一次
        QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
        // 调整行高
        QTimer::singleShot(10, this, &TtModbusTableWidget::adjustRowHeights);
      });
}

bool TtModbusTableWidget::isRowVisible(int row) {
  if (row < 0 || row >= rowCount() || !isVisible()) {
    return false;
  }

  // 当前垂直滚动条的位置
  int scrollPos = verticalScrollBar()->value();

  // 当前可视行数
  int calculatedVisibleRows = visibleRowCount();

  // 动态调整预加载行数，基于当前可见行数
  int extraRows = qMax(2, calculatedVisibleRows / 5);
  int visibleCount = calculatedVisibleRows + extraRows;

  // qDebug() << "Row:" << row << "ScrollPos:" << scrollPos;
  // qDebug() << "Visible rows:" << calculatedVisibleRows;  // 可见行数 11
  // qDebug() << "Extra rows:" << extraRows;  // 额外预加载行数 2
  // qDebug() << "Total visible count:" << visibleCount;  // 前者相加

  // 判断行是否在可见范围内
  return (row >= scrollPos && row <= (scrollPos + visibleCount));
}

void TtModbusTableWidget::initHeader() {
  // BUG 的高度是否过高 ???
  QStringList headers = {"", "", tr("名称"), tr("值"), tr("描述"), ""};

  // 全选或者全都不选
  // QWidget *header = createCheckButton();
  // auto *controlHeaderCheck = createCheckButton();
  check_state_ = createCheckButton();
  setCellWidget(0, 0, check_state_);

  // FIXME 似乎需要 addBtton 点击后, 其边距才能正确使用
  // header = createTypeComboBox(QStringList{tr("地址(HEX)"), tr("地址(DEX)")});

  // 决定地址这一栏的数据类型
  // auto *comboBox =
  //     createTypeComboBox(QStringList{tr("地址(HEX)"), tr("地址(DEX)")});
  // 记录当前的 hex 还是 DEC
  data_format_ =
      createTypeComboBox(QStringList{tr("地址(HEX)"), tr("地址(DEC)")});
  // comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  // setCellWidget(0, 1, header);
  setCellWidget(0, 1, createCellWrapper(data_format_));

  for (int col = 2; col < 5; ++col) {
    QWidget* header = nullptr;
    header = createHeaderCell(headers[col], col != 4);
    setCellWidget(0, col, header);
  }
  // 刷新
  // header = createRefreshButton();
  // setCellWidget(0, 5, header);
}

void TtModbusTableWidget::setupRow(int row) {
  if (!isRowVisible(row)) {
    // 可见时才创建控件
    return;
  }

  TableRow data;
  qDebug() << "this "
           << this;  // 往下移动的时候, 才会动态创建, 创建之后, 就固定存在了
  switch (type_) {
    case TtModbusRegisterType::Coils: {
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.valueButton = new TtSwitchButton(this);
      data.description = new TtLineEdit(this);

      auto makeCell = [this](QWidget* content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(data.valueButton));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());
      // 添加到了 rowsData 中
      rowsData_.append(data);

      // 获取刚刚添加的行的引用，而不是使用临时变量
      TableRow& newRow = rowsData_.last();

      connect(data.valueButton, &Ui::TtSwitchButton::toggled, this,
              &TtModbusTableWidget::onSwitchButtonToggle);
      break;
    }
    case TtModbusRegisterType::DiscreteInputs: {
      // TableRow data;
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.valueButton = new TtSwitchButton(this);
      data.description = new TtLineEdit(this);
      auto makeCell = [this](QWidget* content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(data.valueButton));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());
      rowsData_.append(data);
      break;
    }
    case TtModbusRegisterType::HoldingRegisters: {
      // TableRow data;
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.value = new TtLineEdit(this);
      data.editButton = new QPushButton(QIcon(":/sys/edit.svg"), "", this);
      data.confirmButton = new QPushButton(QIcon(":/sys/link.svg"), "", this);
      data.cancelButton = new QPushButton(QIcon(":/sys/trash.svg"), "", this);
      data.confirmButton->setFixedSize(20, 20);
      data.cancelButton->setFixedSize(20, 20);
      data.confirmButton->hide();
      data.cancelButton->hide();
      data.originalValue = data.value->text();  // 保存初始值

      // 创建包含 Value 编辑框和按钮的容器
      QWidget* valueContainer = new QWidget(this);
      QHBoxLayout* valueLayout = new QHBoxLayout(valueContainer);
      valueLayout->setContentsMargins(0, 0, 0, 0);
      valueLayout->setSpacing(2);
      valueLayout->addWidget(data.value, 1);
      valueLayout->addWidget(data.cancelButton);
      valueLayout->addWidget(data.confirmButton);
      valueLayout->addWidget(data.editButton);
      data.cancelButton->setVisible(false);
      data.cancelButton->setVisible(false);
      data.editButton->setVisible(true);
      data.value->setReadOnly(true);
      data.value->setReadOnlyNoClearButton(true);

      data.description = new TtLineEdit(this);

      auto makeCell = [this](QWidget* content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(valueContainer));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());

      connect(data.editButton, &QPushButton::clicked, this, [this, data]() {
        auto btn = qobject_cast<QPushButton*>(sender());
        if (!btn) {
          return;
        }

        data.cancelButton->setVisible(true);
        data.confirmButton->setVisible(true);
        data.editButton->setVisible(false);
        data.value->setReadOnly(false);
      });

      connect(data.confirmButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onConfirmClicked);
      connect(data.cancelButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onCancelClicked);

      rowsData_.append(data);
      break;
    }
    case TtModbusRegisterType::InputRegisters: {
      // TableRow data;
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.value = new TtLineEdit(this);
      data.editButton = new QPushButton(QIcon(":/sys/edit.svg"), "", this);
      data.confirmButton = new QPushButton(QIcon(":/sys/link.svg"), "", this);
      data.cancelButton = new QPushButton(QIcon(":/sys/trash.svg"), "", this);
      data.confirmButton->setFixedSize(20, 20);
      data.cancelButton->setFixedSize(20, 20);
      data.confirmButton->hide();
      data.cancelButton->hide();
      data.originalValue = data.value->text();  // 保存初始值

      // 创建包含 Value 编辑框和按钮的容器
      QWidget* valueContainer = new QWidget(this);
      QHBoxLayout* valueLayout = new QHBoxLayout(valueContainer);
      valueLayout->setContentsMargins(0, 0, 0, 0);
      valueLayout->setSpacing(2);
      valueLayout->addWidget(data.value, 1);
      valueLayout->addWidget(data.cancelButton);
      valueLayout->addWidget(data.confirmButton);
      valueLayout->addWidget(data.editButton);
      data.cancelButton->setVisible(false);
      data.cancelButton->setVisible(false);
      data.editButton->setVisible(true);
      data.value->setReadOnly(true);
      data.value->setReadOnlyNoClearButton(true);

      data.description = new TtLineEdit(this);

      auto makeCell = [this](QWidget* content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(valueContainer));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());

      connect(data.editButton, &QPushButton::clicked, this, [this, data]() {
        auto btn = qobject_cast<QPushButton*>(sender());
        if (!btn)
          return;

        data.cancelButton->setVisible(true);
        data.confirmButton->setVisible(true);
        data.editButton->setVisible(false);
        data.value->setReadOnly(false);
      });

      connect(data.confirmButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onConfirmClicked);
      connect(data.cancelButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onCancelClicked);

      rowsData_.append(data);
      break;
    }
  }
}

void TtModbusTableWidget::recycleRow(TableRow& row) {
  qDebug() << "recycle Row 回收对象";
  // 这里可以将 row 中的控件隐藏并放入池中
  // if (row.enableBtn) {
  //   row.enableBtn->hide();
  //   switchPool_.append(row.enableBtn);
  // }
  // if (row.typeCombo) {
  //   row.typeCombo->hide();
  //   comboPool_.append(row.typeCombo);
  // }
  // if (row.delaySpin) {
  //   row.delaySpin->hide();
  //   spinPool_.append(row.delaySpin);
  // }
  // if (row.nameEdit) {
  //   row.nameEdit->hide();
  //   spinPool_.append(row.nameEdit);
  // }
}

TtCheckBox* TtModbusTableWidget::createCheckButton() {
  if (!switchPool_.isEmpty()) {
    auto btn = switchPool_.takeLast();
    btn->show();
    return btn;
  }
  return new TtCheckBox(this);
}

TtSwitchButton* TtModbusTableWidget::createSwitchButton() {
  return new TtSwitchButton(this);
}

TtComboBox* TtModbusTableWidget::createTypeComboBox(const QStringList& strs) {
  TtComboBox* combo =
      comboPool_.isEmpty() ? new TtComboBox(this) : comboPool_.takeLast();
  combo->clear();
  combo->addItems(strs);
  return combo;
}

TtSvgButton* TtModbusTableWidget::createRefreshButton() {
  auto btn = new TtSvgButton(":/sys/refresh-normal.svg");
  btn->setEnableHoldToCheck(true);
  btn->setSvgSize(18, 18);
  return btn;
}

// QSpinBox* TtModbusTableWidget::createDelaySpin() {
//   QSpinBox* spin =
//       spinPool_.isEmpty() ? new QSpinBox(this) : spinPool_.takeLast();
//   spin->setMinimum(0);
//   spin->setMaximum(9999);
//   return spin;
// }

QWidget* TtModbusTableWidget::createCellWrapper(QWidget* content) {
  // QWidget *wrapper =
  //     widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  QWidget* wrapper = new QWidget(this);
  // 有问题
  Ui::TtHorizontalLayout* layout = new Ui::TtHorizontalLayout(wrapper);
  layout->setContentsMargins(8, 4, 8, 4);
  layout->addWidget(content);
  return wrapper;
}

int TtModbusTableWidget::findRowIndex(QWidget* context, bool deep) const {
  if (!context) {
    return -1;
  }
  QWidget* parent = context->parentWidget();
  if (!parent) {
    return -1;
  }

  if (!deep) {
    for (int row = 1; row < rowCount(); ++row) {
      if (cellWidget(row, 5) == context->parentWidget()) {
        return row;
      }
    }
  } else {
    QWidget* grandparent = parent->parentWidget();
    if (!grandparent) {
      return -1;
    }
    for (int row = 1; row < rowCount(); ++row) {
      if (cellWidget(row, 5) == grandparent) {
        return row;
      }
    }
  }
  return -1;
}

QWidget* TtModbusTableWidget::createHeaderCell(const QString& text,
                                               bool border) {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins(2, 2, 2, 2));

  QLabel* label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(border);
  return container;
}

QWidget* TtModbusTableWidget::createAddButton() {
  // 添加按钮
  auto* btn = new QPushButton(QIcon(":/sys/plus-circle.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] { addRow(); });
  return createCellWrapper(btn);
}

QWidget* TtModbusTableWidget::createSendButton() {
  auto* btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this]() {});
  return createCellWrapper(btn);
}

QWidget* TtModbusTableWidget::createGraphAndDeleteButton() {
  QWidget* buttonGroup = new QWidget;
  QHBoxLayout* layout = new QHBoxLayout(buttonGroup);
  auto graphBtn = new TtSvgButton(":/sys/graph-up.svg", buttonGroup);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  connect(graphBtn, &TtSvgButton::toggled, this, [this](bool toggle) {
    if (auto* btn = qobject_cast<TtSvgButton*>(sender())) {
      int row = findRowIndex(btn, true);
      if (row > 0) {
        TtLineEdit* edit = rowsData_[row - 1].address;
        if (edit) {
          emit requestShowGraph(type_, edit->text().toInt(), toggle);
        }
      }
    }
  });

  auto deleteBtn = new TtSvgButton(":/sys/trash.svg", buttonGroup);
  deleteBtn->setColors(Qt::black, Qt::blue);
  deleteBtn->setEnableHoldToCheck(true);
  deleteBtn->setSvgSize(18, 18);
  connect(deleteBtn, &TtSvgButton::clicked, this, [this] {
    if (auto* btn = qobject_cast<TtSvgButton*>(sender())) {
      int row = findRowIndex(btn, true);
      if (row > 0) {
        recycleRow(rowsData_[row - 1]);
        removeRow(row);
        rowsData_.remove(row - 1);
      }
    }
  });
  layout->addWidget(graphBtn);
  layout->addWidget(deleteBtn);
  return createCellWrapper(buttonGroup);
}

QWidget* TtModbusTableWidget::createDeleteButton() {
  auto* btn = new QPushButton(QIcon(":/sys/trash.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto* btn = qobject_cast<QPushButton*>(sender())) {
      int row = findRowIndex(btn);
      if (row > 0) {
        // 回收控件
        recycleRow(rowsData_[row - 1]);
        // 移除行
        removeRow(row);
        rowsData_.remove(row - 1);
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget* TtModbusTableWidget::createRowSendButton() {
  auto* btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto* btn = qobject_cast<QPushButton*>(sender())) {
      int row = findRowIndex(btn);
      if (row > 0) {
        // 回收控件
        recycleRow(rowsData_[row - 1]);
        // 移除行
        removeRow(row);
        rowsData_.remove(row - 1);
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget* TtModbusTableWidget::createHeaderWidget(const QString& text,
                                                 bool paintBorder) {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel* label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(paintBorder);
  return container;
}

QWidget* TtModbusTableWidget::createHeaderAddRowWidget() {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* addSendBtn = new TtImageButton(":/sys/plus-circle.svg");
  addSendBtn->setFixedSize(22, 22);

  connect(addSendBtn, &TtImageButton::clicked, this,
          &TtModbusTableWidget::addRow);

  layout->addWidget(addSendBtn);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtModbusTableWidget::createHeaderSendMsgWidget() {
  HeaderWidget* container = new HeaderWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* sendBtn = new TtImageButton(":/sys/send.svg");
  sendBtn->setFixedSize(22, 22);
  layout->addWidget(sendBtn);
  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setPaintRightBorder(false);
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget* TtModbusTableWidget::createFirstColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  TtSwitchButton* isEnableBtn = new TtSwitchButton(container);
  isEnableBtn->setObjectName("isEnableBtn");  // 设置对象名称
  layout->addWidget(isEnableBtn);

  return container;
}

QWidget* TtModbusTableWidget::createSecondColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit* lineEdit = new TtLineEdit(tr("名称"), container);
  lineEdit->setObjectName("name");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtModbusTableWidget::createThirdColumnWidget() {
  // QWidget* container = new QWidget(this);
  // QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);

  // // 创建下拉框和数字输入框
  // Ui::TtComboBox* comboBox = new Ui::TtComboBox(container);
  // comboBox->setObjectName("type");  // 设置对象名称
  // comboBox->addItems({tr("TEXT"), tr("HEX")});

  // layout->addWidget(comboBox);

  // return container;
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  Ui::TtComboBox* comboBox = nullptr;
  if (!comboPool_.isEmpty()) {
    comboBox = comboPool_.takeLast();  // 从池中取出
  } else {
    comboBox = new Ui::TtComboBox(this);  // 池为空时新建
    comboBox->addItems({tr("TEXT"), tr("HEX")});
  }
  comboBox->setObjectName("type");
  layout->addWidget(comboBox);
  return container;
}

QWidget* TtModbusTableWidget::createFourthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit* lineEdit = new TtLineEdit(tr("内容"), container);
  lineEdit->setObjectName("content");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget* TtModbusTableWidget::createFifthColumnWidget() {
  QWidget* container = new QWidget(this);
  container->setMinimumHeight(32);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  QSpinBox* spinBox = new QSpinBox(container);
  spinBox->setObjectName("delay");  // 设置对象名称
  layout->addWidget(spinBox);

  return container;
}

QWidget* TtModbusTableWidget::createSixthColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* deleteBtn = new TtImageButton(":/sys/trash.svg", container);
  deleteBtn->setFixedSize(22, 22);
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

QWidget* TtModbusTableWidget::createSeventhColumnWidget() {
  QWidget* container = new QWidget(this);
  QHBoxLayout* layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton* sendBtn = new TtImageButton(":/sys/send.svg", container);
  sendBtn->setFixedSize(22, 22);
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

int TtModbusTableWidget::visibleRowCount() {
  // // 不可视 或者 表格高度小于等于表头高度, 但是我的表头高度不是隐藏了吗?
  // qDebug() << "horizontalHeader()->height():" <<
  // horizontalHeader()->height(); if (!isVisible() || height() <=
  // horizontalHeader()->height()) {
  //   // 一直进入
  //   return 10;
  // }
  // // 可见区域的高度
  // int availableHeight = viewport()->height();

  // // 使用第 0 行的高度作为行高 ?
  // // 但我不是使用第 0 行作为表头吗?
  // int rowHeight = (rowCount() > 0) ? this->rowHeight(0) : 30;
  // if (rowHeight <= 0) {
  //   rowHeight = 30; // 默认行高
  // }
  // // 可视高度 / 行高 = 可见行数
  // return std::max(availableHeight / rowHeight, 10);
  // 移除可能导致始终进入的条件
  if (!isVisible()) {
    return 10;  // 如果不可见，返回默认值
  }

  // qDebug() << "Height:" << height();
  // qDebug() << "Header height:" << horizontalHeader()->height();

  // 可见区域的高度
  int availableHeight = viewport()->height();
  // qDebug() << "Available height:" << availableHeight;  // 正确获取高度

  // 使用第1行的高度作为标准行高（第0行可能是标题行）
  int standardRowHeight = (rowCount() > 1) ? rowHeight(1) : 30;
  if (standardRowHeight <= 0) {
    standardRowHeight = 30;  // 默认行高
  }
  // qDebug() << "Standard row height:" << standardRowHeight;  // 30

  // 计算实际可见行数，不强制最小值为10
  int calculatedRows = availableHeight / standardRowHeight;
  // qDebug() << "Calculated rows:" << calculatedRows;  // 11

  // 如果计算结果合理，则使用它；否则使用默认值
  return calculatedRows > 0 ? calculatedRows : 10;
}

inline void TtModbusTableWidget::HeaderWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);

  if (paint_) {
    QPainter painter(this);
    // painter.setPen(QPen(QColor("#212121")));  // 设置边框颜色
    painter.setPen(QPen(QColor("#c6c6c6")));  // 设置边框颜色

    // 画一个右边框，只在自定义区域内
    // qDebug() << this->height();
    painter.drawLine(width() - 1, 4, width() - 1, this->height() - 4);
  }
}

}  // namespace Ui
