#include "ui/controls/TtTableView.h"

#include <ui/control/TtCheckBox.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtSwitchButton.h>

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
  //         TtLineEdit* item = cellWidget(i, j)->findChild<TtLineEdit*>("name");
  //         if (item) {
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
  // qDebug()
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
  TableRow data;
  data.enableBtn = createSwitchButton();
  data.nameEdit = new TtLineEdit(tr("名称"), this);
  data.typeCombo = createTypeComboBox();
  data.contentEdit = new TtLineEdit(tr("内容"), this);
  data.delaySpin = createDelaySpin();

  auto makeCell = [this](QWidget* content) {
    return createCellWrapper(content);
  };

  setCellWidget(row, 0, makeCell(data.enableBtn));
  setCellWidget(row, 1, makeCell(data.nameEdit));
  // setCellWidget(row, 2, makeCell(data.typeCombo));
  setCellWidget(row, 2, makeCell(new QComboBox(this)));
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
  QWidget* wrapper =
      widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(wrapper);
  layout->addWidget(content);
  return wrapper;
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
  TtLineEdit* lineEdit =
      new TtLineEdit(tr("名称"), container);
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

inline void TtTableWidget::HeaderWidget::paintEvent(QPaintEvent* event) {
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

TtModbusTableWidget::TtModbusTableWidget(QWidget* parent)
    : QTableWidget(1, 6, parent) {

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
  //         TtLineEdit* item = cellWidget(i, j)->findChild<TtLineEdit*>("name");
  //         if (item) {
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
  for (int i = 0; i < rowsData_.size(); ++i) {
    const auto& row = rowsData_[i];
    QJsonArray data;
    data.append(row.checkBtn->isChecked());
    data.append(row.address->text());
    data.append(row.addressName->text());
    data.append(row.value->text());
    data.append(row.description->text());
    root[QString::number(i + 1)] = data;
  }
  return root;
  // qDebug()
}

void TtModbusTableWidget::setCellWidget(int row, int column, QWidget* widget) {
  QTableWidget::setCellWidget(row, column, widget);
  cellWidgetCache_[widget][row] = widget;  // 缓存控件
}

QVector<QString> TtModbusTableWidget::getRowValue(int col) {
  QVector<QString> result;
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget* widget = cellWidget(i, col);
    if (widget) {
      TtLineEdit* lineEdit = widget->findChild<TtLineEdit*>();
      if (lineEdit) {
        result.append(lineEdit->text());
      }
    }
  }
  return result;
}

void TtModbusTableWidget::addRow() {
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

void TtModbusTableWidget::initHeader() {
  QStringList headers = {"", "", tr("名称"), tr("值"), tr("描述"), ""};

  QWidget* header = createCheckButton();
  // header->setStyleSheet("background-color: green");
  setCellWidget(0, 0, header);

  header = createTypeComboBox(QStringList{tr("地址(HEX)"), tr("地址(DEX)")});
  setCellWidget(0, 1, header);
  // setCellWidget(0, 1, new QComboBox(this));

  for (int col = 2; col < 5; ++col) {
    QWidget* header = nullptr;
    header = createHeaderCell(headers[col], col != 4);
    setCellWidget(0, col, header);
  }
  // header = createRefreshButton();
  // setCellWidget(0, 5, header);
}

void TtModbusTableWidget::setupRow(int row) {
  TableRow data;
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
  // setCellWidget(row, 3, makeCell(data.value));
  setCellWidget(row, 3, makeCell(valueContainer));
  setCellWidget(row, 4, makeCell(data.description));
  setCellWidget(row, 5, createGraphAndDeleteButton());

  connect(data.editButton, &QPushButton::clicked, this, [this, data]() {
    auto btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
      return;

    // // 查找对应的行
    // for (int i = 0; i < rowsData_.size(); ++i) {
    //   if (rowsData_[i].value == valueEdit) {
    //     rowsData_[i].confirmButton->show();
    //     rowsData_[i].cancelButton->show();
    //     // emit valueModified(i + 1); // +1 因为首行是标题
    //     break;
    //   }
    // }
    // 编辑
    data.cancelButton->setVisible(true);
    data.confirmButton->setVisible(true);
    data.editButton->setVisible(false);
    data.value->setReadOnly(false);
  });

  // 连接信号
  // connect(data.value, &TtLineEdit::textChanged, this,
  //         &TtModbusTableWidget::onValueChanged);
  connect(data.confirmButton, &QPushButton::clicked, this,
          &TtModbusTableWidget::onConfirmClicked);
  connect(data.cancelButton, &QPushButton::clicked, this,
          &TtModbusTableWidget::onCancelClicked);

  rowsData_.append(data);
}

void TtModbusTableWidget::recycleRow(TableRow& row) {
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
  QWidget* wrapper =
      widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(wrapper);
  layout->addWidget(content);
  return wrapper;
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
