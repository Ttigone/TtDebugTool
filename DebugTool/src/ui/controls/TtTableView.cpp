#include "ui/controls/TtTableView.h"

#include <qcombobox.h>
#include <qheaderview.h>
#include <qjsonarray.h>
#include <qlayoutitem.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qoverload.h>
#include <qtablewidget.h>
#include <qwidget.h>
#include <ui/control/TtCheckBox.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/layout/vertical_layout.h>

#include <QSpinBox>
#include <QTableWidgetItem>

#include "Def.h"
#include "ui/layout/horizontal_layout.h"
#include "ui/widgets/buttons.h"

namespace Ui {

TtTableWidget::TtTableWidget(QWidget *parent) : QTableWidget(1, 7, parent) {
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

  // 虚拟化：滚动/尺寸变化时按需创建可见行控件
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          [this] { setupVisibleRows(); });
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
          [this] { setupVisibleRows(); });
}

TtTableWidget::~TtTableWidget() {
  // 回收已绑定的可见行
  for (auto it = widget_rows_.begin(); it != widget_rows_.end(); ++it) {
    // 防御：逐行解绑
    unbindRow(it.key());
  }
  // 清理池
  qDeleteAll(combo_pool_);
  qDeleteAll(switch_pool_);
  qDeleteAll(spin_pool_);
  qDeleteAll(line_edit_pool_);
  qDeleteAll(widget_pool_);
  combo_pool_.clear();
  switch_pool_.clear();
  spin_pool_.clear();
  line_edit_pool_.clear();
  widget_pool_.clear();
}

void TtTableWidget::SetupHeaderRow() {
  setCellWidget(0, 0, createHeaderWidget(tr("启用"), true));
  setCellWidget(0, 1, createHeaderWidget(tr("名称"), true));
  setCellWidget(0, 2, createHeaderWidget(tr("格式"), true));
  setCellWidget(0, 3, createHeaderWidget(tr("内容"), true));
  setCellWidget(0, 4, createHeaderWidget(tr("延时"), true));
  setCellWidget(0, 5, createHeaderAddRowWidget());
  setCellWidget(0, 6, createHeaderSendMsgWidget());
}

// void TtTableWidget::SetupTable(const QJsonObject &record) {
//   record_ = record;
//   while (rowCount() > 1) {
//     removeRow(1);
//   }

//   // 获取元数据
//   QJsonObject metadata = record_.value("__metadata").toObject();
//   int totalRows = metadata.value("totalRows").toInt(0);

//   // qDebug() << "准备还原" << totalRows << "行数据";
//   // 创建临时存储，按行号排序
//   QMap<int, QJsonArray> sortedRows;

//   // 遍历JSON对象中的所有键，每个键应该代表一行(除了__metadata)
//   for (auto it = record_.begin(); it != record_.end(); ++it) {
//     QString key = it.key();
//     if (key == "__metadata") {
//       continue;
//     }
//     bool ok;
//     int rowIndex = key.toInt(&ok);
//     if (!ok) {
//       // qWarning() << "无效的行索引:" << key;
//       continue;
//     }

//     QJsonArray rowData = it.value().toArray();
//     if (rowData.isEmpty()) {
//       // qWarning() << "行" << rowIndex << "数据为空";
//       continue;
//     }
//     // 将行数据添加到排序映射
//     sortedRows[rowIndex] = rowData;
//   }

//   for (auto it = sortedRows.begin(); it != sortedRows.end(); ++it) {
//     int rowIndex = it.key();
//     // json 行数据
//     QJsonArray rowData = it.value();
//     // 添加新行
//     int newRow = rowCount();  // 首行为 1, 插入的位置是该处
//     insertRow(newRow);        // 插入新行到最后的位置
//     setupRow(newRow);
//     if (record_.isEmpty()) {
//       if (!isRowVisible(newRow)) {
//         scrollToItem(item(newRow, 0));
//         setupRow(newRow);  // 再次尝试创建
//       }
//       if (record_.isEmpty()) {
//         continue;
//       }
//     }

//     // 获取刚添加的行数据引用
//     TableRow &row = record_.last();
//     // 获取实际数据索引 (跳过行号)
//     int dataOffset = 1;  // 因为第一个元素是行索引
//     // 设置复选框状态
//     if (rowData.size() > dataOffset && row.checkBtn) {
//       row.checkBtn->setChecked(rowData.at(dataOffset).toBool());
//     }
//     dataOffset++;
//     // 设置名称
//     if (rowData.size() > dataOffset && row.nameEdit) {
//       // row.address->setText(rowData.at(dataOffset).toString());
//       QString addrText = rowData.at(dataOffset).toString();
//       row.nameEdit->setText(addrText);
//     }
//     dataOffset++;

//     // 设置类型
//     if (rowData.size() > dataOffset && row.typeCombo) {
//       int type = rowData.at(dataOffset).toInt();
//       for (int i = 0; i < row.typeCombo->count(); ++i) {
//         if (row.typeCombo->itemData(i).toInt() == type) {
//           row.typeCombo->setCurrentIndex(i);
//           break;
//         }
//       }
//     }
//     dataOffset++;

//     // 设置内容
//     if (rowData.size() > dataOffset && row.contentEdit) {
//       // qDebug() << "descripition: " << rowData.at(dataOffset).toString();
//       row.contentEdit->setText(rowData.at(dataOffset).toString());
//     }
//     dataOffset++;

//     // 设置描述
//     // if (rowData.size() > dataOffset && row.delaySpin) {
//     //   // qDebug() << "descripition: " <<
//     rowData.at(dataOffset).toString();
//     //   row.delaySpin->setValue(rowData.at(dataOffset).toInt());
//     // }
//     if (rowData.size() > dataOffset && row.delayEdit) {
//       // qDebug() << "descripition: " << rowData.at(dataOffset).toString();
//       // row.delayEdit->setValue(rowData.at(dataOffset).toInt());
//       row.delayEdit->setText(rowData.at(dataOffset).toString());
//     }
//     dataOffset++;

//     // // 设置地址并更新映射
//     // if (rowData.size() > dataOffset && row.address) {
//     //   QString addrText = rowData.at(dataOffset).toString();
//     //   row.address->setText(addrText);

//     //   // 添加地址映射
//     //   bool ok;
//     //   int addr;

//     //   // 地址没有16进制
//     //   if (addrText.startsWith("0x", Qt::CaseInsensitive)) {
//     //     addr = addrText.mid(2).toInt(&ok, 16);
//     //   } else {
//     //     addr = addrText.toInt(&ok, 10);
//     //   }

//     //   if (ok) {
//     //     rowsData_[newRow - 1].currentAddress = addr;
//     //     address_to_row_map_.insert(addr, newRow);
//     //   }
//     // }
//   }

//   // 调整行高和列宽以适应内容
//   QTimer::singleShot(0, this, [this]() {
//     resizeRowsToContents();
//     resizeColumnsToContents();
//   });
// }
void TtTableWidget::SetupTable(const QJsonObject &record) {
  record_ = record;
  // 清空表格（保留标题行）
  while (rowCount() > 1) {
    removeRow(1);
  }
  // 解绑所有可见行，清空池引用
  for (auto it = widget_rows_.begin(); it != widget_rows_.end();) {
    int row = it.key();
    ++it;
    unbindRow(row);
  }
  widget_rows_.clear();
  model_.clear();

  if (record_.isEmpty()) {
    return;
  }

  // 元数据
  QJsonObject metadata = record_.value("__metadata").toObject();
  int totalRows = metadata.value("totalRows").toInt(0);
  if (totalRows < 0) totalRows = 0;

  // 准备模型
  model_.resize(totalRows);

  // 排序 JSON 行
  QMap<int, QJsonArray> sortedRows;
  for (auto it = record_.begin(); it != record_.end(); ++it) {
    if (it.key() == "__metadata") continue;
    bool ok = false;
    int rowIndex = it.key().toInt(&ok);
    if (!ok) continue;
    sortedRows[rowIndex] = it.value().toArray();
  }

  // 填充模型
  int maxRow = 0;
  for (auto it = sortedRows.begin(); it != sortedRows.end(); ++it) {
    int rowIndex = it.key();  // 1-based
    QJsonArray rowData = it.value();
    if (rowIndex <= 0) continue;
    if (rowIndex > totalRows) continue;

    RowData d;
    int off = 1;  // 跳过第0个（行号）
    if (rowData.size() > off) d.enabled = rowData.at(off++).toBool(d.enabled);
    if (rowData.size() > off) d.name = rowData.at(off++).toString();
    if (rowData.size() > off) d.type = rowData.at(off++).toInt(d.type);
    if (rowData.size() > off) d.content = rowData.at(off++).toString();
    if (rowData.size() > off) d.delayMs = rowData.at(off++).toString("0");

    model_[rowIndex - 1] = d;
    maxRow = std::max(maxRow, rowIndex);
  }

  // 设置表格行数（含表头）
  setRowCount(totalRows + 1);

  // 尝试创建可见行控件
  setupVisibleRows();

  // 调整布局
  QTimer::singleShot(0, this, [this]() {
    resizeRowsToContents();
    resizeColumnsToContents();
  });
}

QJsonObject TtTableWidget::GetTableRecord() {
  QJsonObject root;
  QJsonObject metadata;
  metadata["totalRows"] = rowCount() - 1;  // 减去标题行
  // metadata["type"] = static_cast<int>(type_);
  metadata["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
  root["__metadata"] = metadata;
  // qDebug() << "size: " << rowsData_.size();
  // 临时存储行数据，用于排序
  // QVector<QPair<int, QJsonArray>> rowsData;

  // for (int i = 0; i < rowsData.size(); ++i) {
  //   const auto &row = rowsData_[i];
  //   // if (!row.checkBtn || !row.address || !row.addressName) {
  //   //   qWarning() << "跳过行 " << i << ": 基本要素缺失";
  //   //   continue; // 跳过无效行
  //   // }
  //   if (!row.checkBtn || !row.nameEdit || !row.typeCombo || !row.contentEdit
  //   ||
  //       !row.delayEdit) {
  //     qWarning() << "跳过行 " << i << ": 基本要素缺失";
  //     continue;  // 跳过无效行
  //   }
  //   QJsonArray data;
  //   data.append(i + 1);
  //   data.append(row.checkBtn->isChecked());
  //   data.append(row.nameEdit->text());
  //   data.append(row.typeCombo->currentData().toInt());
  //   data.append(row.contentEdit->text());
  //   // data.append(row.delaySpin->value());
  //   data.append(row.delayEdit->text());
  //   // if (!row.description) {
  //   //   qWarning() << "跳过行 " << i << ": description 为空";
  //   //   continue;
  //   // }
  //   // data.append(row.description->text());
  //   // root[QString::number(i + 1)] = data;
  //   rowsData.append(qMakePair(i + 1, data));  // 行号从1开始
  // }
  // qDebug() << "root: " << root;
  // 按行索引排序
  // std::sort(rowsData.begin(), rowsData.end(),
  //           [](const QPair<int, QJsonArray> &a,
  //              const QPair<int, QJsonArray> &b) { return a.first < b.first;
  //              });
  // // 将排序后的数据添加到根对象
  // for (const auto &pair : rowsData) {
  //   root[QString::number(pair.first)] = pair.second;
  // }
  // 行数据，1-based key
  for (int i = 0; i < model_.size(); ++i) {
    const auto &d = model_[i];
    QJsonArray arr;
    arr.append(i + 1);      // 行号
    arr.append(d.enabled);  // 启用
    arr.append(d.name);     // 名称
    arr.append(d.type);     // 格式枚举值
    arr.append(d.content);  // 内容
    arr.append(d.delayMs);  // 延时
    root[QString::number(i + 1)] = arr;
  }
  return root;
}

void TtTableWidget::SetCellWidget(int row, int column, QWidget *widget) {
  QTableWidget::setCellWidget(row, column, widget);
  cellWidgetCache_[widget][row] = widget;  // 缓存控件
}

void TtTableWidget::SetEnabled(bool enable) {
  QTableWidget::setEnabled(true);  // 总是启用表格本身，以允许某些列可用
  QWidget *plus = cellWidget(0, 5);
  if (plus) {
    plus->setEnabled(enable);
  }
  QWidget *sent = cellWidget(0, 6);
  if (sent) {
    sent->setEnabled(!enable);
  }
  // 遍历所有行
  for (int row = 1; row < rowCount(); ++row) {
    // 仅在 rowsData_ 有效范围内处理
    // if (row - 1 < rowsData.size()) {
    //   TableRow &rowData = rowsData_[row - 1];

    //   // 设置每一列的控件启用状态
    //   if (rowData.checkBtn) rowData.checkBtn->setEnabled(enable);

    //   if (rowData.nameEdit) rowData.nameEdit->setEnabled(enable);

    //   if (rowData.typeCombo) rowData.typeCombo->setEnabled(enable);

    //   if (rowData.contentEdit) rowData.contentEdit->setEnabled(enable);

    //   // if (rowData.delaySpin)
    //   //   rowData.delaySpin->setEnabled(enable);
    //   if (rowData.delayEdit) rowData.delayEdit->setEnabled(enable);
    // }

    // 处理第七列（发送按钮列）
    // 获取第七列的单元格小部件
    QWidget *sendBtnWidget = cellWidget(row, 6);  // 假设第七列索引为6
    if (sendBtnWidget) {
      // 查找发送按钮（通常包装在一个容器内）
      QList<QPushButton *> buttons =
          sendBtnWidget->findChildren<QPushButton *>();
      for (QPushButton *btn : buttons) {
        // for (auto *btn : qAsConst(buttons)) {
        // 发送按钮在禁用模式下启用，启用模式下禁用
        btn->setEnabled(!enable);
      }
    }
  }

  // 保存当前状态，以便其他方法可以检查
  setProperty("customEnabled", enable);
  // 样式刷新
  style()->unpolish(this);
  style()->polish(this);
}

void TtTableWidget::OnAddRowButtonClicked() {
  // // 逻辑行号：model_.size() + 1
  // int new_row = rowCount();
  // insertRow(new_row);
  // setItem(new_row, 0, new QTableWidgetItem());
  // scrollToItem(item(new_row, 0));
  // setupRow(new_row);

  // // 调整大小
  // resizeRowsToContents();
  // resizeColumnsToContents();

  // // 发出行数变化信号
  // emit OnRowsChanged(newRowIndex);
  // 逻辑行号：model_.size() + 1
  int new_row = rowCount();  // 物理行：末尾插入
  insertRow(new_row);

  // 模型扩展
  RowData d;
  d.enabled = true;
  d.type = static_cast<int>(TtTextFormat::TEXT);
  d.delayMs = "0";
  model_.append(d);

  // 尝试滚动到新行并物化
  bool wasVisible = isRowVisible(new_row);
  if (!wasVisible) {
    scrollToItem(item(new_row, 0));
  }
  setupVisibleRows();

  // 调整大小
  QTimer::singleShot(0, this, [this]() {
    resizeRowsToContents();
    resizeColumnsToContents();
  });

  emit OnRowsChanged(static_cast<quint16>(model_.size()));
}

void TtTableWidget::initHeader() {
  QStringList headers = {tr("启用"), tr("名称"), tr("格式"), tr("内容"),
                         tr("延时"), "",         ""};

  for (int col = 0; col < 7; ++col) {
    QWidget *header = nullptr;
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

// void TtTableWidget::setupVisibleRows() {
//   // 计算可见行范围（1-based）
//   int first = 1;
//   int last = rowCount() - 1;
//   if (last < 1) return;

//   QModelIndex topIdx = indexAt(QPoint(0, 0));
//   QModelIndex bottomIdx = indexAt(QPoint(0, viewport()->height() - 1));
//   if (topIdx.isValid()) first = std::max(1, topIdx.row());
//   if (bottomIdx.isValid()) last = std::min(rowCount() - 1, bottomIdx.row());

//   // 预取缓冲行
//   int buffer = std::max(2, (last - first + 1) / 5);
//   first = std::max(1, first - buffer);
//   last = std::min(rowCount() - 1, last + buffer);

//   // 绑定可见区
//   for (int r = first; r <= last; ++r) {
//     if (!widget_rows_.contains(r)) {
//       bindRow(r);
//     }
//   }

//   // 解绑不可见区
//   QList<int> keys = widget_rows_.keys();
//   for (int key : keys) {
//     if (key < first || key > last) {
//       unbindRow(key);
//     }
//   }
// }

void TtTableWidget::setupVisibleRows() {
  // 计算可见行范围（1-based）
  int first = 1;
  int last = rowCount() - 1;
  if (last < 1) return;

  QModelIndex topIdx = indexAt(QPoint(0, 0));
  QModelIndex bottomIdx = indexAt(QPoint(0, viewport()->height() - 1));
  if (topIdx.isValid()) first = std::max(1, topIdx.row());
  if (bottomIdx.isValid()) last = std::min(rowCount() - 1, bottomIdx.row());

  // 预取缓冲行
  int buffer = std::max(2, (last - first + 1) / 5);
  first = std::max(1, first - buffer);
  last = std::min(rowCount() - 1, last + buffer);

  // 绑定可见区
  for (int r = first; r <= last; ++r) {
    if (!widget_rows_.contains(r)) {
      bindRow(r);
    }
  }

  // 解绑不可见区
  QList<int> keys = widget_rows_.keys();
  for (int key : keys) {
    if (key < first || key > last) {
      unbindRow(key);
    }
  }
}

void TtTableWidget::bindRow(int row) {
  if (row <= 0 || row > rowCount() - 1) return;
  if (widget_rows_.contains(row)) return;
  if (row - 1 >= model_.size()) return;  // 模型防御

  TableRow w;
  // 从池中取控件或新建
  w.checkBtn = createSwitchButton();
  w.nameEdit = createLineEdit(tr("名称"));
  w.typeCombo = createTypeComboBox();
  w.contentEdit = createLineEdit(tr("内容"));
  w.delayEdit = createLineEdit(tr("延时(ms)"));

  auto wrap = [this](QWidget *c) { return createCellWrapper(c); };

  SetCellWidget(row, 0, wrap(w.checkBtn));
  SetCellWidget(row, 1, wrap(w.nameEdit));
  SetCellWidget(row, 2, wrap(w.typeCombo));
  SetCellWidget(row, 3, wrap(w.contentEdit));
  SetCellWidget(row, 4, wrap(w.delayEdit));
  SetCellWidget(row, 5, createDeleteButton());
  SetCellWidget(row, 6, createRowSendButton());

  // 灌入模型到控件
  const RowData &d = model_[row - 1];
  if (w.checkBtn) w.checkBtn->setChecked(d.enabled);
  if (w.nameEdit) w.nameEdit->setText(d.name);
  if (w.typeCombo) {
    // 将 type 设置到对应的 data
    for (int i = 0; i < w.typeCombo->count(); ++i) {
      if (w.typeCombo->itemData(i).toInt() == d.type) {
        w.typeCombo->setCurrentIndex(i);
        break;
      }
    }
  }
  if (w.contentEdit) w.contentEdit->setText(d.content);
  if (w.delayEdit) w.delayEdit->setText(d.delayMs);

  // 双向绑定：使用 findRowIndex 动态获取当前行，避免行删除后索引失效
  if (w.checkBtn) {
    connect(w.checkBtn, &TtSwitchButton::toggled, this,
            [this, btn = w.checkBtn](bool on) {
              int r = findRowIndex(btn, 0, true);
              if (r > 0 && r - 1 < model_.size()) model_[r - 1].enabled = on;
            });
  }
  if (w.nameEdit) {
    connect(w.nameEdit, &TtLineEdit::textChanged, this,
            [this, edit = w.nameEdit](const QString &t) {
              int r = findRowIndex(edit, 1, true);
              if (r > 0 && r - 1 < model_.size()) model_[r - 1].name = t;
            });
  }
  if (w.typeCombo) {
    connect(w.typeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this, cb = w.typeCombo](int) {
              int r = findRowIndex(cb, 2, true);
              if (r > 0 && r - 1 < model_.size())
                model_[r - 1].type = cb->currentData().toInt();
            });
  }
  if (w.contentEdit) {
    connect(w.contentEdit, &TtLineEdit::textChanged, this,
            [this, edit = w.contentEdit](const QString &t) {
              int r = findRowIndex(edit, 3, true);
              if (r > 0 && r - 1 < model_.size()) model_[r - 1].content = t;
            });
  }
  if (w.delayEdit) {
    connect(w.delayEdit, &TtLineEdit::textChanged, this,
            [this, edit = w.delayEdit](const QString &t) {
              int r = findRowIndex(edit, 4, true);
              if (r > 0 && r - 1 < model_.size()) model_[r - 1].delayMs = t;
            });
  }

  // 根据当前 SetEnabled 状态更新可用性
  bool isEnabled = property("customEnabled").toBool();
  if (w.checkBtn) w.checkBtn->setEnabled(isEnabled);
  if (w.nameEdit) w.nameEdit->setEnabled(isEnabled);
  if (w.typeCombo) w.typeCombo->setEnabled(isEnabled);
  if (w.contentEdit) w.contentEdit->setEnabled(isEnabled);
  if (w.delayEdit) w.delayEdit->setEnabled(isEnabled);

  widget_rows_.insert(row, w);
}

// ...existing code...
void TtTableWidget::unbindRow(int row) {
  auto it = widget_rows_.find(row);
  if (it == widget_rows_.end()) return;

  TableRow w = it.value();

  // 通用：先从单元格 wrapper 中摘除控件并移除 cellWidget
  auto recycleWrapper = [this, row](int col, QWidget *ctrl) {
    if (!ctrl) return;
    if (QWidget *wrapper = cellWidget(row, col)) {
      if (wrapper->layout()) wrapper->layout()->removeWidget(ctrl);
      ctrl->setParent(this);  // 避免随 wrapper 一起销毁
      QTableWidget::removeCellWidget(row, col);
      wrapper->deleteLater();
    }
    ctrl->disconnect();
    ctrl->hide();
  };

  // 分类型回收到各自对象池，避免模板依赖类型问题
  auto recycleSwitch = [this, &recycleWrapper](int col, TtSwitchButton *btn) {
    if (!btn) return;
    recycleWrapper(col, btn);
    if (switch_pool_.size() < MAX_POOL_SIZE)
      switch_pool_.append(btn);
    else
      btn->deleteLater();
  };
  auto recycleCombo = [this, &recycleWrapper](int col, TtComboBox *cb) {
    if (!cb) return;
    recycleWrapper(col, cb);
    if (combo_pool_.size() < MAX_POOL_SIZE)
      combo_pool_.append(cb);
    else
      cb->deleteLater();
  };
  auto recycleLineEdit = [this, &recycleWrapper](int col, TtLineEdit *ed) {
    if (!ed) return;
    recycleWrapper(col, ed);
    if (line_edit_pool_.size() < MAX_POOL_SIZE)
      line_edit_pool_.append(ed);
    else
      ed->deleteLater();
  };

  recycleSwitch(0, w.checkBtn);
  recycleLineEdit(1, w.nameEdit);
  recycleCombo(2, w.typeCombo);
  recycleLineEdit(3, w.contentEdit);
  recycleLineEdit(4, w.delayEdit);

  // 清理操作列的 wrapper
  for (int c = 5; c <= 6; ++c) {
    if (QWidget *wrapper = cellWidget(row, c)) {
      QTableWidget::removeCellWidget(row, c);
      wrapper->deleteLater();
    }
  }

  widget_rows_.erase(it);
}

void TtTableWidget::ensureModelSize(int rows) {
  if (rows < 0) return;
  if (model_.size() < rows) model_.resize(rows);
}

void TtTableWidget::setupRow(int row) {
  if (!isRowVisible(row)) {
    qDebug() << "不可视";
    return;
  }
  TableRow data;
  data.checkBtn = createSwitchButton();
  data.checkBtn->setChecked(true);
  data.nameEdit = createLineEdit(tr("名称"));
  data.typeCombo = createTypeComboBox();
  data.contentEdit = createLineEdit(tr("内容"));
  // data.delaySpin = createDelaySpin();
  data.delayEdit = createLineEdit(tr("延时(ms)"));

  auto makeCell = [this](QWidget *content) {
    return createCellWrapper(content);
  };

  setCellWidget(row, 0, makeCell(data.checkBtn));
  setCellWidget(row, 1, makeCell(data.nameEdit));
  setCellWidget(row, 2, makeCell(data.typeCombo));
  setCellWidget(row, 3, makeCell(data.contentEdit));
  // setCellWidget(row, 4, makeCell(data.delaySpin));
  setCellWidget(row, 4, makeCell(data.delayEdit));
  setCellWidget(row, 5, createDeleteButton());
  setCellWidget(row, 6, createRowSendButton());

  // rowsData_.append(data);

  // 设置行中各控件的启用状态（根据当前表格状态）
  bool isEnabled = property("customEnabled").toBool();
  if (data.checkBtn) data.checkBtn->setEnabled(isEnabled);
  if (data.nameEdit) data.nameEdit->setEnabled(isEnabled);
  if (data.typeCombo) data.typeCombo->setEnabled(isEnabled);
  if (data.contentEdit) data.contentEdit->setEnabled(isEnabled);
  // if (data.delaySpin)
  //   data.delaySpin->setEnabled(isEnabled);
  if (data.delayEdit) data.delayEdit->setEnabled(isEnabled);
}

TtSwitchButton *TtTableWidget::createSwitchButton() {
  // TtSwitchButton *btn = new TtSwitchButton(this);
  // btn->setChecked(true);
  // return btn;
  if (!switch_pool_.isEmpty()) {
    auto *btn = switch_pool_.takeLast();
    btn->show();
    btn->setParent(this);
    return btn;
  }
  auto *btn = new TtSwitchButton(this);
  btn->setChecked(true);
  return btn;
}

TtComboBox *TtTableWidget::createTypeComboBox() {
  if (!combo_pool_.isEmpty()) {
    auto *cb = combo_pool_.takeLast();
    cb->show();
    cb->setParent(this);
    cb->clear();
    cb->addItem(tr("TEXT"), TtTextFormat::TEXT);
    cb->addItem(tr("HEX"), TtTextFormat::HEX);
    return cb;
  }
  auto *combo = new TtComboBox(this);
  combo->addItem(tr("TEXT"), TtTextFormat::TEXT);
  combo->addItem(tr("HEX"), TtTextFormat::HEX);
  return combo;
}

QSpinBox *TtTableWidget::createDelaySpin() {
  if (!spin_pool_.isEmpty()) {
    auto *sp = spin_pool_.takeLast();
    sp->show();
    sp->setParent(this);
    return sp;
  }
  auto *spin = new QSpinBox(this);
  spin->setMinimum(0);
  return spin;
}

TtLineEdit *TtTableWidget::createLineEdit(const QString &placeholderText) {
  if (!line_edit_pool_.isEmpty()) {
    auto *ed = line_edit_pool_.takeLast();
    ed->show();
    ed->setParent(this);
    ed->clear();
    ed->setPlaceholderText(placeholderText);
    return ed;
  }
  auto *lineEdit = new TtLineEdit(this);
  lineEdit->setPlaceholderText(placeholderText);
  return lineEdit;
}

QWidget *TtTableWidget::createCellWrapper(QWidget *content) {
  QWidget *wrapper = new QWidget(this);
  Ui::TtVerticalLayout *layout = new Ui::TtVerticalLayout(wrapper);
  layout->setContentsMargins(2, 2, 2, 2);
  layout->addWidget(content);
  wrapper->setVisible(true);
  return wrapper;
}

int TtTableWidget::findRowIndex(QWidget *context, const int &col,
                                bool deep) const {
  if (!context) {
    return -1;
  }
  QWidget *parent = context->parentWidget();
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
    QWidget *grandparent = parent->parentWidget();
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

bool TtTableWidget::isRowVisible(int row) {
  if (row < 0 || row > rowCount()) {
    // 这里退出了
    qDebug() << "this exit";
    return false;
  }

  // // 当前垂直滚动条的位置
  // int scrollPos = verticalScrollBar()->value();
  // // 当前可视行数
  // int calculatedVisibleRows = visibleRowCount();
  // // 动态调整预加载行数，基于当前可见行数
  // int extraRows = qMax(2, calculatedVisibleRows / 5);
  // int visibleCount = calculatedVisibleRows + extraRows;
  // // qDebug() << "Row:" << row << "ScrollPos:" << scrollPos;
  // // qDebug() << "Visible rows:" << calculatedVisibleRows;  // 可见行数 11
  // // qDebug() << "Extra rows:" << extraRows;  // 额外预加载行数 2
  // // qDebug() << "Total visible count:" << visibleCount;  // 前者相加

  // // qDebug() << "return: "
  // //          << (row >= scrollPos && row <= (scrollPos + visibleCount));
  // return (row >= scrollPos && row <= (scrollPos + visibleCount));
  QRect vp = viewport()->rect();
  QModelIndex idx = model()->index(row, 0);
  QRect rect = visualRect(idx);
  return vp.intersects(rect);
}

QWidget *TtTableWidget::createHeaderCell(const QString &text, bool border) {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel *label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->SetPaintRightBorder(border);
  return container;
}

QWidget *TtTableWidget::createAddButton() {
  auto *btn = new QPushButton(QIcon(":/sys/plus-circle.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this,
          [this] { OnAddRowButtonClicked(); });
  return createCellWrapper(btn);
}

QWidget *TtTableWidget::createSendButton() {
  auto *btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  // 群发
  connect(btn, &QPushButton::clicked, this, [this]() {
    qDebug() << "clicked";
    std::vector<Data::MsgInfo> msgs;
    // 遍历存放的数组中
    msgs.reserve(static_cast<size_t>(model_.size()));
    // for (int i = 1; i < rowCount(); ++i) {
    //   if (rowsData_.at(i - 1).checkBtn->isChecked()) {
    //     msgs.emplace_back(
    //         rowsData_.at(i - 1).contentEdit->text(),
    //         static_cast<TtTextFormat::Type>(
    //             rowsData_.at(i - 1).typeCombo->currentData().toInt()),
    //         // rowsData_.at(i - 1).delaySpin->value());
    //         rowsData_.at(i - 1).delayEdit->text().toULong());
    //   }
    // }
    for (const auto &d : model_) {
      if (!d.enabled) continue;
      Data::MsgInfo info(d.content, static_cast<TtTextFormat::Type>(d.type),
                         d.delayMs.toULong());
      msgs.emplace_back(std::move(info));
    }
    // BUG 修复延时问题, 转化成整数
    emit SendRowsMsg(msgs);
  });
  return createCellWrapper(btn);
}

QWidget *TtTableWidget::createDeleteButton() {
  auto *deleteBtn = new QPushButton(QIcon(":/sys/trash.svg"), "");
  deleteBtn->setFlat(true);
  // connect(deleteBtn, &QPushButton::clicked, this, [this] {
  //   if (auto *btn = qobject_cast<QPushButton *>(sender())) {
  //     int row = findRowIndex(btn, 5);
  //     if (row > 0) {
  //       qDebug() << "删除行 " << row;
  //       // 直接处理 rowsData_ 数组
  //       if (row - 1 < rowsData_.size()) {
  //         // 从 rowsData_ 中移除该行数据
  //         rowsData_.remove(row - 1);
  //       }
  //       // 最后删除行
  //       removeRow(row);

  //       // 发出信号
  //       emit OnRowsChanged(row - 1);
  //     }
  //   }
  // });
  connect(deleteBtn, &QPushButton::clicked, this, [this] {
    if (auto *btn = qobject_cast<QPushButton *>(sender())) {
      int row = findRowIndex(btn, 5, true);
      if (row > 0 && row < rowCount()) {
        // 先解绑该行控件并回收
        unbindRow(row);
        // 删除表格行
        removeRow(row);
        // 删除模型数据
        if (row - 1 < model_.size()) model_.remove(row - 1);

        // 调整 widget_rows_ 的键（行号整体上移）
        QHash<int, TableRow> newMap;
        for (auto it = widget_rows_.begin(); it != widget_rows_.end(); ++it) {
          int key = it.key();
          TableRow val = it.value();
          if (key < row) {
            newMap.insert(key, val);
          } else if (key > row) {
            newMap.insert(key - 1, val);
          }
        }
        widget_rows_.swap(newMap);

        emit OnRowsChanged(static_cast<quint16>(model_.size()));

        // 重新布局
        QTimer::singleShot(0, this, [this]() {
          setupVisibleRows();
          resizeRowsToContents();
          resizeColumnsToContents();
        });
      }
    }
  });
  return createCellWrapper(deleteBtn);
}

QWidget *TtTableWidget::createRowSendButton() {
  auto *btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto *btn = qobject_cast<QPushButton *>(sender())) {
      // 发送了, 但是那边没有接受到
      qDebug() << "发送信息";
      int row = findRowIndex(btn, 6, true);
      if (row > 0 && row - 1 < model_.size()) {
        const auto &d = model_[row - 1];
        emit SendRowMsg(d.content, static_cast<TtTextFormat::Type>(d.type),
                        d.delayMs.toULong());
        // emit SendRowMsg(
        //     rowsData_[row - 1].contentEdit->text(),
        //     static_cast<TtTextFormat::Type>(
        //         rowsData_[row - 1].typeCombo->currentData().toInt()),
        //     rowsData_[row - 1].delayEdit->text().toULong());
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget *TtTableWidget::createHeaderWidget(const QString &text,
                                           bool paintBorder) {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel *label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->SetPaintRightBorder(paintBorder);
  return container;
}

QWidget *TtTableWidget::createHeaderAddRowWidget() {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *addSendBtn = new TtImageButton(":/sys/plus-circle.svg");
  addSendBtn->setFixedSize(22, 22);

  connect(addSendBtn, &TtImageButton::clicked, this,
          &TtTableWidget::OnAddRowButtonClicked);

  layout->addWidget(addSendBtn);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget *TtTableWidget::createHeaderSendMsgWidget() {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *sendBtn = new TtImageButton(":/sys/send.svg");
  sendBtn->setFixedSize(22, 22);
  layout->addWidget(sendBtn);
  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->SetPaintRightBorder(false);
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget *TtTableWidget::createFirstColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  TtSwitchButton *isEnableBtn = new TtSwitchButton(container);
  isEnableBtn->setObjectName("isEnableBtn");  // 设置对象名称
  layout->addWidget(isEnableBtn);

  return container;
}

QWidget *TtTableWidget::createSecondColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit *lineEdit = new TtLineEdit(tr("名称"), container);
  lineEdit->setObjectName("name");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget *TtTableWidget::createThirdColumnWidget() {
  // QWidget* container = new QWidget(this);
  // QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);

  // // 创建下拉框和数字输入框
  // Ui::TtComboBox* comboBox = new Ui::TtComboBox(container);
  // comboBox->setObjectName("type");  // 设置对象名称
  // comboBox->addItems({tr("TEXT"), tr("HEX")});

  // layout->addWidget(comboBox);

  // return container;
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(0, 0, 0, 0);

  // Ui::TtComboBox *comboBox = nullptr;
  // if (!comboPool_.isEmpty()) {
  //   comboBox = comboPool_.takeLast(); // 从池中取出
  // } else {
  //   comboBox = new Ui::TtComboBox(this); // 池为空时新建
  //   comboBox->addItems({tr("TEXT"), tr("HEX")});
  // }

  Ui::TtComboBox *comboBox = new Ui::TtComboBox(this);  // 池为空时新建
  comboBox->addItems({tr("TEXT"), tr("HEX")});

  comboBox->setObjectName("type");
  layout->addWidget(comboBox);
  return container;
}

QWidget *TtTableWidget::createFourthColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit *lineEdit = new TtLineEdit(tr("内容"), container);
  lineEdit->setObjectName("content");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget *TtTableWidget::createFifthColumnWidget() {
  QWidget *container = new QWidget(this);
  container->setMinimumHeight(32);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  // QSpinBox *spinBox = new QSpinBox(container);
  // spinBox->setObjectName("delay"); // 设置对象名称
  // layout->addWidget(spinBox);
  // 创建下拉框和数字输入框
  TtLineEdit *lineEdit = new TtLineEdit(tr("延时"), container);
  lineEdit->setObjectName("content");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget *TtTableWidget::createSixthColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *deleteBtn = new TtImageButton(":/sys/trash.svg", container);
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

QWidget *TtTableWidget::createSeventhColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *sendBtn = new TtImageButton(":/sys/send.svg", container);
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

TtTableWidget::HeaderWidget::~HeaderWidget() {}

void TtTableWidget::HeaderWidget::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);

  if (paint_) {
    QPainter painter(this);
    painter.setPen(QPen(QColor("#c6c6c6")));  // 设置边框颜色
    // 画一个右边框，只在自定义区域内
    // qDebug() << this->height();
    painter.drawLine(width() - 1, 4, width() - 1, this->height() - 4);
  }
}

TtModbusTableWidget::TtModbusTableWidget(TtModbusRegisterType::Type type,
                                         QWidget *parent)
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

  // 设置表头行高
  // setRowHeight(0, 10);
}

TtModbusTableWidget::~TtModbusTableWidget() {}

void TtModbusTableWidget::setRowValue(int row, int col, const QString &data) {}

QVector<int> TtModbusTableWidget::getAddressValue() {
  auto values = getRowValue(1);
  QVector<int> results;
  for (auto it = values.cbegin(); it != values.cend(); ++it) {
    results.append(it->toInt());
  }
  // qDebug() << "get address: " << results;
  return results;
}

void TtModbusTableWidget::setValue(const QString &data) {
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget *widget = cellWidget(i, 3);
    if (widget) {
      TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
      if (lineEdit) {
        lineEdit->setText(data);
      }
    }
  }
}

void TtModbusTableWidget::setValue(const int &addr,
                                   const QVector<quint16> &data) {
  // 手动更新标志
  programmatic_update_ = true;

  // // BUG 待修改为 QMutilMap 方式匹配
  // // 某一个地址映射的全部可能行
  // QList<int> rows = address_to_row_map_.values(addr);

  // // 遍历行号
  // for (int row : rows) {
  //   if (row < 1 || row > rowCount() || row -1 >= rowsData_.size()) {
  //     qDebug() << "行号不合法: " << row;
  //     continue;
  //   }
  //   switch (type_) {
  //     case TtModbusRegisterType::Coils:
  //     case TtModbusRegisterType::DiscreteInputs:
  //       if (rowsData_[row].valueButton) {
  //         // 对 button 赋值
  //         rowsData_[row].valueButton->setChecked(data.isEmpty() ? false :
  //         data[0] != 0);
  //       }
  //     break;
  //     case TtModbusRegisterType::HoldingRegisters:
  //     case TtModbusRegisterType::InputRegisters:
  //     if (rowsData_[row].value) {
  //       rowsData_[row].value->setText(data.isEmpty() ? "0" :
  //       QString::number(data[0]));
  //     }
  //     break;
  //   }
  // }
  // programmatic_update_ = false;

  switch (type_) {
    case TtModbusRegisterType::Coils: {  // Coils
      for (int i = 1; i < this->rowCount(); ++i) {
        QWidget *widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
          // 对比当前的地址与目标设置地址
          // 如果存在多个相同地址呢, 需要兼容
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget *widget = cellWidget(i, 3);
            if (widget) {
              TtSwitchButton *btn = widget->findChild<TtSwitchButton *>();
              if (btn) {
                // 触发 toggled 信号
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
        QWidget *widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget *widget = cellWidget(i, 3);
            if (widget) {
              TtSwitchButton *btn = widget->findChild<TtSwitchButton *>();
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
        QWidget *widget = cellWidget(i, 1);
        if (widget) {
          TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
          if (lineEdit && lineEdit->text() == QString::number(addr)) {
            QWidget *widget = cellWidget(i, 3);
            if (widget) {
              TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
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
  programmatic_update_ = false;
}

void TtModbusTableWidget::setValue(const QVector<quint16> &data) {
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget *widget = cellWidget(i, 3);
    if (widget) {
      TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
      if (lineEdit) {
        lineEdit->setText(QString::number((data[i - 1])));
      }
    }
  }
}

void TtModbusTableWidget::setTable(const QJsonObject &record) {
  record_ = record;
  if (record_.isEmpty()) {
    qDebug() << "表格数据空";
    return;
  }
  qDebug() << "还原表格数据: ";
  while (rowCount() > 1) {
    removeRow(1);
  }
  // 空操作
  rowsData_.clear();

  // 获取元数据
  QJsonObject metadata = record_.value("__metadata").toObject();
  int totalRows = metadata.value("totalRows").toInt(0);

  qDebug() << "准备还原" << totalRows << "行数据";
  // 创建临时存储，按行号排序
  QMap<int, QJsonArray> sortedRows;

  // 遍历JSON对象中的所有键，每个键应该代表一行(除了__metadata)
  for (auto it = record_.begin(); it != record_.end(); ++it) {
    QString key = it.key();
    if (key == "__metadata") {
      continue;
    }
    bool ok;
    int rowIndex = key.toInt(&ok);
    if (!ok) {
      qWarning() << "无效的行索引:" << key;
      continue;
    }

    QJsonArray rowData = it.value().toArray();
    if (rowData.isEmpty()) {
      qWarning() << "行" << rowIndex << "数据为空";
      continue;
    }
    // 将行数据添加到排序映射
    sortedRows[rowIndex] = rowData;
  }

  for (auto it = sortedRows.begin(); it != sortedRows.end(); ++it) {
    int rowIndex = it.key();
    // json 行数据
    QJsonArray rowData = it.value();
    // 添加新行
    int newRow = rowCount();  // 首行为 1, 插入的位置是该处
    insertRow(newRow);        // 插入新行到最后的位置
    // setup 需要全包 可视状态
    qDebug() << "table setup Row";
    setupRow(newRow);
    // 如果rowsData_大小不足，可能是setupRow没有正确创建控件
    // rowsData 逐渐递增的
    // 有问题, rowsData_.size() 一直是 0
    // 检查行是否成功添加到rowsData_
    // 这里还是有问题
    // rowsData_ 保存行, 初始 0 开头
    // 但是 newRow 是 1 开头的
    // if (rowsData_.isEmpty() || rowsData_.size() < newRow) {
    //   qDebug() << "行数据未正确创建，跳过行" << rowIndex;
    //   // 还是进入了

    //   // 强制创建控件，确保行显示
    //   if (!isRowVisible(newRow)) {
    //     scrollToItem(item(newRow, 0));
    //     setupRow(newRow);  // 再次尝试创建
    //   }

    //   // 如果仍然无法创建，跳过此行
    //   if (rowsData_.isEmpty() || rowsData_.size() < newRow) {
    //     continue;
    //   }
    // }
    if (rowsData_.isEmpty()) {
      qDebug() << "行数据未正确创建，跳过行" << rowIndex;
      // 还是进入了
      // 强制创建控件，确保行显示
      if (!isRowVisible(newRow)) {
        scrollToItem(item(newRow, 0));
        setupRow(newRow);  // 再次尝试创建
      }

      // 如果仍然无法创建，跳过此行
      if (rowsData_.isEmpty()) {
        continue;
      }
    }
    // if (rowsData_.size() < newRow) {
    //   qWarning() << "行数据未正确创建，跳过行" << rowsData_.size() <<
    //   rowIndex; continue;
    // }

    // 获取刚添加的行数据引用
    TableRow &row = rowsData_.last();
    // 获取实际数据索引 (跳过行号)
    int dataOffset = 1;  // 因为第一个元素是行索引
    // 设置复选框状态
    if (rowData.size() > dataOffset && row.checkBtn) {
      row.checkBtn->setChecked(rowData.at(dataOffset).toBool());
    }
    dataOffset++;
    // 设置地址 更新地址映射
    if (rowData.size() > dataOffset && row.address) {
      // row.address->setText(rowData.at(dataOffset).toString());
      QString addrText = rowData.at(dataOffset).toString();
      row.address->setText(addrText);
      // 添加地址映射
      // bool ok;
      // int addr;
      // if (addrText.startsWith("0x", Qt::CaseInsensitive)) {
      //   addr = addrText.mid(2).toInt(&ok, 16);
      // } else {
      //   addr = addrText.toInt(&ok, 10);
      // }

      // if (ok) {
      //   rowsData_[newRow - 1].currentAddress = addr;
      //   address_to_row_map_.insert(addr, newRow);
      // }
    }
    dataOffset++;

    // 设置地址名称
    if (rowData.size() > dataOffset && row.addressName) {
      row.addressName->setText(rowData.at(dataOffset).toString());
    }
    dataOffset++;

    // 从 json 恢复时, 没有值列
    // // 根据表格类型设置值
    // if (rowData.size() > dataOffset) {
    //   switch (type_) {
    //   case TtModbusRegisterType::Coils:
    //   case TtModbusRegisterType::DiscreteInputs:
    //     if (row.valueButton) {
    //       row.valueButton->setChecked(rowData.at(dataOffset).toBool());
    //     }
    //     break;

    //   case TtModbusRegisterType::HoldingRegisters:
    //   case TtModbusRegisterType::InputRegisters:
    //     if (row.value) {
    //       row.value->setText(rowData.at(dataOffset).toString());
    //       row.originalValue = rowData.at(dataOffset).toString(); //
    //       保存初始值
    //     }
    //     break;
    //   }
    // }
    // dataOffset++;
    // qDebug() << "test";
    // 设置描述
    if (rowData.size() > dataOffset && row.description) {
      qDebug() << "descripition: " << rowData.at(dataOffset).toString();
      row.description->setText(rowData.at(dataOffset).toString());
    }

    // // 设置地址并更新映射
    // if (rowData.size() > dataOffset && row.address) {
    //   QString addrText = rowData.at(dataOffset).toString();
    //   row.address->setText(addrText);

    //   // 添加地址映射
    //   bool ok;
    //   int addr;

    //   // 地址没有16进制
    //   if (addrText.startsWith("0x", Qt::CaseInsensitive)) {
    //     addr = addrText.mid(2).toInt(&ok, 16);
    //   } else {
    //     addr = addrText.toInt(&ok, 10);
    //   }

    //   if (ok) {
    //     rowsData_[newRow - 1].currentAddress = addr;
    //     address_to_row_map_.insert(addr, newRow);
    //   }
    // }
  }

  // 调整行高和列宽以适应内容
  QTimer::singleShot(0, this, [this]() {
    resizeRowsToContents();
    resizeColumnsToContents();
  });
}

QJsonObject TtModbusTableWidget::getTableRecord() {
  QJsonObject root;
  // 添加表格元数据
  QJsonObject metadata;
  metadata["totalRows"] = rowCount() - 1;  // 减去标题行
  metadata["type"] = static_cast<int>(type_);
  metadata["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
  root["__metadata"] = metadata;
  // qDebug() << "size: " << rowsData_.size();
  // 临时存储行数据，用于排序
  QVector<QPair<int, QJsonArray>> rowsData;

  for (int i = 0; i < rowsData_.size(); ++i) {
    const auto &row = rowsData_[i];
    if (!row.checkBtn || !row.address || !row.addressName) {
      qWarning() << "跳过行 " << i << ": 基本要素缺失";
      continue;  // 跳过无效行
    }
    QJsonArray data;
    data.append(i + 1);
    data.append(row.checkBtn->isChecked());
    data.append(row.address->text());
    data.append(row.addressName->text());
    // switch (type_) {
    //   case TtModbusRegisterType::Coils:
    //   case TtModbusRegisterType::DiscreteInputs:
    //     // 线圈和离散输入
    //     if (!row.valueButton) {
    //       qWarning() << "跳过行 " << i << ": valueButton 为空";
    //       continue;
    //     }
    //     // // FIXME 数据选项不需要保存
    //     // data.append(row.valueButton->isChecked());
    //     break;
    //   case TtModbusRegisterType::HoldingRegisters:
    //   case TtModbusRegisterType::InputRegisters:
    //     // 保持寄存器和输入寄存器
    //     if (!row.value) {
    //       qWarning() << "跳过行 " << i << ": value 为空";
    //       continue;
    //     }
    //     // data.append(row.value->text());
    //     break;
    // }
    if (!row.description) {
      qWarning() << "跳过行 " << i << ": description 为空";
      continue;
    }
    data.append(row.description->text());
    // root[QString::number(i + 1)] = data;
    rowsData.append(qMakePair(i + 1, data));  // 行号从1开始
  }
  // qDebug() << "root: " << root;
  // 按行索引排序
  std::sort(rowsData.begin(), rowsData.end(),
            [](const QPair<int, QJsonArray> &a,
               const QPair<int, QJsonArray> &b) { return a.first < b.first; });
  // 将排序后的数据添加到根对象
  for (const auto &pair : rowsData) {
    root[QString::number(pair.first)] = pair.second;
  }
  return root;
}

void TtModbusTableWidget::setCellWidget(int row, int column, QWidget *widget) {
  // 不是居中显示的
  QTableWidget::setCellWidget(row, column, widget);
  cellWidgetCache_[widget][row] = widget;  // 缓存控件
}

void TtModbusTableWidget::setEnable(bool enable) {
  // 链接成功设置为 false
  qDebug() << "SetEnable" << enable;
  // 设置表头行的状态
  if (check_state_) {
    check_state_->setEnabled(enable);
  }
  if (data_format_) {
    data_format_->setEnabled(enable);  // 地址格式选择
  }
  // 设置数据行的状态
  for (int i = 0; i < rowsData_.size(); ++i) {
    TableRow &row = rowsData_[i];

    // 第一列：选择框
    if (row.checkBtn) {
      row.checkBtn->setEnabled(enable);
    }

    // 第二列：地址
    if (row.address) {
      row.address->setReadOnly(!enable);
      row.address->setEnabled(enable);
    }

    // 第三列：地址名称
    if (row.addressName) {
      row.addressName->setReadOnly(!enable);
      row.addressName->setEnabled(enable);
    }

    // 第四列：值（根据类型不同，可能是 valueButton 或 value）
    if (row.valueButton) {
      row.valueButton->setEnabled(!enable);
    }

    if (row.value) {
      row.value->setReadOnly(!enable);
      row.value->setEnabled(!enable);

      // 编辑按钮相关控件
      if (row.editButton) {
        row.editButton->setEnabled(!enable);
      }
      if (row.confirmButton) {
        row.confirmButton->setEnabled(!enable);
      }
      if (row.cancelButton) {
        row.cancelButton->setEnabled(!enable);
      }
    }

    // 第五列：描述
    if (row.description) {
      row.description->setReadOnly(!enable);
      row.description->setEnabled(enable);
    }
  }
  // 修复样式表 - 分离各个部分，确保语法正确
  QString styleSheet;
  // 禁用输入框样式
  styleSheet +=
      "QLineEdit:disabled { background-color: #f0f0f0; color: #505050; }\n";

  // 启用输入框样式
  styleSheet += "QLineEdit:enabled { background-color: #ffffff; }\n";

  // 列样式 - 注意这里是关键修复点
  if (enable) {
    // 使用正确的选择器语法
    styleSheet += "QTableWidget::item:column(3) { background-color: #f0f0f0; }";
  } else {
    styleSheet += "QTableWidget::item:column(3) { background-color: #e6f7ff; }";
  }

  // 处理特殊的第六列按钮
  for (int row = 0; row < rowCount(); ++row) {
    QWidget *widget = cellWidget(row, 5);
    if (widget) {
      // 绘图按钮在使能状态下可用，删除按钮在非使能状态下可用
      QList<TtSvgButton *> buttons = widget->findChildren<TtSvgButton *>();
      for (TtSvgButton *button : buttons) {
        // 第一个按钮是绘图，第二个是删除
        if (buttons.size() >= 2) {
          buttons[0]->setEnabled(!enable);  // 绘图按钮
          buttons[1]->setEnabled(enable);   // 删除按钮
        }
      }
    }
  }
  // 保存当前状态以供其他方法参考
  // is_enabled_ = enable;

  // 更新表格视图
  viewport()->update();
}

QVector<QString> TtModbusTableWidget::getRowValue(int col) {
  QVector<QString> result;
  // 只会读取输入了地址的
  for (int i = 1; i < this->rowCount(); ++i) {
    QWidget *widget = cellWidget(i, col);
    if (widget) {
      TtLineEdit *lineEdit = widget->findChild<TtLineEdit *>();
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
      QWidget *cellWidget = this->cellWidget(row, 0);
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

void TtModbusTableWidget::showEvent(QShowEvent *event) {
  QTableWidget::showEvent(event);
  // 确保在显示时设置可见行
  QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
}

void TtModbusTableWidget::resizeEvent(QResizeEvent *event) {
  QTableWidget::resizeEvent(event);
  // 调整 size 后, 重新设置可见行
  QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
}

void TtModbusTableWidget::onValueChanged() {
  TtLineEdit *valueEdit = qobject_cast<TtLineEdit *>(sender());
  if (!valueEdit) return;

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
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn) return;

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
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn) return;

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
  if (programmatic_update_) {
    // 读取值的时候, 可能会改变 switchButton 的状态, 需要无视
    return;
  }
  // 点击 valueButton, 发出修改的指令
  // BUG 读取值的时候, switcbutton 也会发送状态改变, 进而判断是写
  TtSwitchButton *btn = qobject_cast<TtSwitchButton *>(sender());
  if (!btn) {
    return;
  }
  // rowsData 的索引从 0 开始
  for (int i = 0; i < rowsData_.size(); ++i) {
    if (rowsData_[i].valueButton == btn) {
      int addr = rowsData_[i].currentAddress;
      // qDebug() << "find addr" << addr << rowsData_[i].address->text();
      if (addr >= 0) {
        // 发出信号, 通知外部 switchbutton 状态改变
        emit valueConfirmed(addr, toggled ? 1 : 0);
      }
      break;
    }
  }
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
      this, &QTableWidget::windowTitleChanged, this, [this](const QString &) {
        // 使用windowTitleChanged作为一个安全的信号，以便在UI完全加载后触发一次
        QTimer::singleShot(0, this, &TtModbusTableWidget::setupVisibleRows);
        // 调整行高
        QTimer::singleShot(10, this, &TtModbusTableWidget::adjustRowHeights);
      });
  connect(data_format_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int index) {
            // 格式切换
            // 16 进制
            bool isHex = (index == 0);
            // 不需要切换格式吧
            // 遍历所有行，更新地址显示格式
            qDebug() << "切换格式";
            for (int i = 0; i < rowsData_.size(); ++i) {
              if (rowsData_[i].address) {
                QString currentText = rowsData_[i].address->text();
                bool ok;
                int value;

                // 解析当前文本为整数，不管是十六进制还是十进制
                if (currentText.startsWith("0x", Qt::CaseInsensitive)) {
                  value = currentText.mid(2).toInt(&ok, 16);
                } else {
                  value = currentText.toInt(&ok, 10);
                }

                if (ok) {
                  // 根据新格式转换回文本
                  if (isHex) {
                    rowsData_[i].address->setText(
                        QString("%1").arg(value, 2, 16, QChar('0')).toUpper());
                  } else {
                    rowsData_[i].address->setText(QString::number(value));
                  }
                }
              }
            }
          });
}

bool TtModbusTableWidget::isRowVisible(int row) {
  // 1 >= rowCount() 导致有问题, 因为首行作为了标题行
  // 为什么第一次 isVisible() 是 false, 尚未初始化完成
  // qDebug() << row << rowCount() << isVisible();
  // if (row < 0 || row > rowCount() || !isVisible()) {
  //   // 这里退出了
  //   qDebug() << "this exit";
  //   return false;
  // }
  // qDebug() << row << rowCount();
  // 取消 isVisible(), 但逻辑上还是保留了当前位于滚动条不可视区域不创建的规则
  if (row < 0 || row > rowCount()) {
    // 这里退出了
    qDebug() << "this exit";
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
  // qDebug() << "return: "
  //          << (row >= scrollPos && row <= (scrollPos + visibleCount));
  return (row >= scrollPos && row <= (scrollPos + visibleCount));
}

void TtModbusTableWidget::initHeader() {
  QStringList headers = {"", "", tr("名称"), tr("值"), tr("描述"), ""};

  // 全选或者全都不选
  // QWidget *header = createCheckButton();
  // auto *controlHeaderCheck = createCheckButton();
  check_state_ = createCheckButton();
  setCellWidget(0, 0, check_state_);

  // 记录当前的 hex 还是 DEC
  data_format_ =
      createTypeComboBox(QStringList{tr("地址(HEX)"), tr("地址(DEC)")});
  // comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  setCellWidget(0, 1, createCellWrapper(data_format_));

  for (int col = 2; col < 5; ++col) {
    QWidget *header = nullptr;
    header = createHeaderCell(headers[col], col != 4);
    setCellWidget(0, col, header);
  }
  // 刷新
  // header = createRefreshButton();
  // setCellWidget(0, 5, header);
  // setRowHeight(0, 24);
}

void TtModbusTableWidget::setupRow(int row) {
  if (!isRowVisible(row)) {
    // 当前行不可视状态
    qDebug() << "不可视";
    // 可见时才创建控件
    return;
  }

  TableRow data;
  // qDebug() << "this "
  //          << this; // 往下移动的时候, 才会动态创建, 创建之后, 就固定存在了
  switch (type_) {
    case TtModbusRegisterType::Coils: {
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.valueButton = new TtSwitchButton(this);
      data.description = new TtLineEdit(this);

      auto makeCell = [this](QWidget *content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(data.valueButton));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());
      // 添加到了 rowsData 中
      qDebug() << "append in coils";
      rowsData_.append(data);

      // 获取刚刚添加的行的引用，而不是使用临时变量
      TableRow &newRow = rowsData_.last();

      // 状态触发改变
      // 手动触发改变
      // 怎么会找不到
      // 链接信号
      connect(data.valueButton, &Ui::TtSwitchButton::toggled, this,
              &TtModbusTableWidget::onSwitchButtonToggle);

      // 地址变化信号
      connect(data.address, &TtLineEdit::textChanged, this,
              [this, row](const QString &text) {
                // 设置的行
                qDebug() << "address current row: " << row;
                // 当前存在的个数是 1
                qDebug() << "rowsData.size() " << rowsData_.size();
                if (row < 1 || row - 1 >= rowsData_.size()) {
                  qDebug() << "行号不合法: " << row;
                  return;
                }
                bool ok;
                int newAddr;
                // BUG 这里需要判断是否是十六进制
                // 16 进制不需要以 0x 开头, lineedit
                // 中输入符合十六进制格式的字符串即可
                newAddr = text.toInt(&ok, 10);
                // 越界
                // int oldAddr = rowsData_[row].currentAddress;
                int oldAddr = rowsData_[row - 1].currentAddress;

                // 更新映射
                if (ok) {
                  // 旧地址有效, 移出旧的映射
                  if (oldAddr >= 0) {
                    // 删除特定键值对
                    address_to_row_map_.remove(oldAddr, row);
                  }
                  // 添加新映射
                  address_to_row_map_.insert(newAddr, row);
                  rowsData_[row - 1].currentAddress = newAddr;
                } else {
                  // 无效地址, 移出旧映射
                  if (oldAddr >= 0) {
                    // 删除特定键值对
                    address_to_row_map_.remove(oldAddr, row);
                    // rowsData_[row].currentAddress = -1;
                    rowsData_[row - 1].currentAddress = -1;
                  }
                }
                qDebug() << "地址映射更新: 行=" << row << ", 地址=" << newAddr
                         << ", 总映射数=" << address_to_row_map_.size();
              });
      break;
    }
    case TtModbusRegisterType::DiscreteInputs: {
      // TableRow data;
      data.checkBtn = createCheckButton();
      data.address = new TtLineEdit(this);
      data.addressName = new TtLineEdit(this);
      data.valueButton = new TtSwitchButton(this);
      data.description = new TtLineEdit(this);
      auto makeCell = [this](QWidget *content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(data.valueButton));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());
      qDebug() << "append in discre";
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
      QWidget *valueContainer = new QWidget(this);
      QHBoxLayout *valueLayout = new QHBoxLayout(valueContainer);
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

      auto makeCell = [this](QWidget *content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(valueContainer));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());

      connect(data.editButton, &QPushButton::clicked, this, [this, data]() {
        auto btn = qobject_cast<QPushButton *>(sender());
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
      qDebug() << "append in hold";
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
      QWidget *valueContainer = new QWidget(this);
      QHBoxLayout *valueLayout = new QHBoxLayout(valueContainer);
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

      auto makeCell = [this](QWidget *content) {
        return createCellWrapper(content);
      };

      setCellWidget(row, 0, makeCell(data.checkBtn));
      setCellWidget(row, 1, makeCell(data.address));
      setCellWidget(row, 2, makeCell(data.addressName));
      setCellWidget(row, 3, makeCell(valueContainer));
      setCellWidget(row, 4, makeCell(data.description));
      setCellWidget(row, 5, createGraphAndDeleteButton());

      connect(data.editButton, &QPushButton::clicked, this, [this, data]() {
        auto btn = qobject_cast<QPushButton *>(sender());
        if (!btn) return;

        data.cancelButton->setVisible(true);
        data.confirmButton->setVisible(true);
        data.editButton->setVisible(false);
        data.value->setReadOnly(false);
      });

      connect(data.confirmButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onConfirmClicked);
      connect(data.cancelButton, &QPushButton::clicked, this,
              &TtModbusTableWidget::onCancelClicked);
      qDebug() << "append in input";
      rowsData_.append(data);
      break;
    }
  }
}

void TtModbusTableWidget::recycleRow(TableRow &row) {
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

void TtModbusTableWidget::deleteRow(int row) {
  if (row < 1 || row >= rowCount() || row - 1 >= rowsData_.size()) {
    qWarning() << "行号不合法: " << row;
    return;
  }
  int addr = rowsData_[row - 1].currentAddress;
  qDebug() << "delete addr" << addr;
  if (addr >= 0) {
    address_to_row_map_.remove(addr, row);
  }
  recycleRow(rowsData_[row - 1]);
  removeRow(row);
  rowsData_.remove(row - 1);  // 删除对应的数据行

  // 更新所有大于此行的行号在映射中的引用
  QMultiMap<int, int> updatedMap;
  for (auto it = address_to_row_map_.begin(); it != address_to_row_map_.end();
       ++it) {
    int currentRow = it.value();
    if (currentRow > row) {
      // 减少大于已删除行的行号
      updatedMap.insert(it.key(), currentRow - 1);
    } else if (currentRow < row) {
      // 保持小于已删除行的行号不变
      updatedMap.insert(it.key(), currentRow);
    }
    // 相等的行已经在前面移除了
  }

  // 替换为更新后的映射
  address_to_row_map_ = updatedMap;
}

TtCheckBox *TtModbusTableWidget::createCheckButton() {
  if (!switchPool_.isEmpty()) {
    auto btn = switchPool_.takeLast();
    btn->show();
    return btn;
  }
  return new TtCheckBox(this);
}

TtSwitchButton *TtModbusTableWidget::createSwitchButton() {
  return new TtSwitchButton(this);
}

TtComboBox *TtModbusTableWidget::createTypeComboBox(const QStringList &strs) {
  TtComboBox *combo =
      comboPool_.isEmpty() ? new TtComboBox(this) : comboPool_.takeLast();
  combo->clear();
  combo->addItems(strs);
  return combo;
}

TtSvgButton *TtModbusTableWidget::createRefreshButton() {
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

QWidget *TtModbusTableWidget::createCellWrapper(QWidget *content) {
  // QWidget *wrapper =
  //     widgetPool_.isEmpty() ? new QWidget : widgetPool_.takeLast();
  QWidget *wrapper = new QWidget(this);
  // 有问题
  Ui::TtHorizontalLayout *layout = new Ui::TtHorizontalLayout(wrapper);
  layout->setContentsMargins(8, 4, 8, 4);
  layout->addWidget(content);
  return wrapper;
}

int TtModbusTableWidget::findRowIndex(QWidget *context, bool deep) const {
  if (!context) {
    return -1;
  }
  QWidget *parent = context->parentWidget();
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
    QWidget *grandparent = parent->parentWidget();
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

QWidget *TtModbusTableWidget::createHeaderCell(const QString &text,
                                               bool border) {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins(2, 2, 2, 2));

  QLabel *label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(border);
  return container;
}

QWidget *TtModbusTableWidget::createAddButton() {
  // 添加按钮
  auto *btn = new QPushButton(QIcon(":/sys/plus-circle.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] { addRow(); });
  return createCellWrapper(btn);
}

QWidget *TtModbusTableWidget::createSendButton() {
  auto *btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this]() {});
  return createCellWrapper(btn);
}

QWidget *TtModbusTableWidget::createGraphAndDeleteButton() {
  QWidget *buttonGroup = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(buttonGroup);
  auto graphBtn = new TtSvgButton(":/sys/graph-up.svg", buttonGroup);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  connect(graphBtn, &TtSvgButton::toggled, this, [this](bool toggle) {
    if (auto *btn = qobject_cast<TtSvgButton *>(sender())) {
      int row = findRowIndex(btn, true);
      if (row > 0) {
        TtLineEdit *edit = rowsData_[row - 1].address;
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
    if (auto *btn = qobject_cast<TtSvgButton *>(sender())) {
      int row = findRowIndex(btn, true);
      if (row > 0) {
        // recycleRow(rowsData_[row - 1]);
        // removeRow(row);
        // rowsData_.remove(row - 1);
        deleteRow(row);
      }
    }
  });
  layout->addWidget(graphBtn);
  layout->addWidget(deleteBtn);
  return createCellWrapper(buttonGroup);
}

QWidget *TtModbusTableWidget::createDeleteButton() {
  auto *btn = new QPushButton(QIcon(":/sys/trash.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto *btn = qobject_cast<QPushButton *>(sender())) {
      int row = findRowIndex(btn);
      if (row > 0) {
        // 回收控件
        // recycleRow(rowsData_[row - 1]);
        // // 移除行
        // removeRow(row);
        // rowsData_.remove(row - 1);
        deleteRow(row);
      }
    }
  });
  return createCellWrapper(btn);
}

QWidget *TtModbusTableWidget::createRowSendButton() {
  auto *btn = new QPushButton(QIcon(":/sys/send.svg"), "");
  btn->setFlat(true);
  connect(btn, &QPushButton::clicked, this, [this] {
    if (auto *btn = qobject_cast<QPushButton *>(sender())) {
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

QWidget *TtModbusTableWidget::createHeaderWidget(const QString &text,
                                                 bool paintBorder) {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  QLabel *label = new QLabel(text, container);
  label->setStyleSheet("border: none;");
  layout->addWidget(label, 0, Qt::AlignCenter);

  container->setStyleSheet("background-color: #f0f0f0;");
  container->setPaintRightBorder(paintBorder);
  return container;
}

QWidget *TtModbusTableWidget::createHeaderAddRowWidget() {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *addSendBtn = new TtImageButton(":/sys/plus-circle.svg");
  addSendBtn->setFixedSize(22, 22);

  connect(addSendBtn, &TtImageButton::clicked, this,
          &TtModbusTableWidget::addRow);

  layout->addWidget(addSendBtn);

  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget *TtModbusTableWidget::createHeaderSendMsgWidget() {
  HeaderWidget *container = new HeaderWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *sendBtn = new TtImageButton(":/sys/send.svg");
  sendBtn->setFixedSize(22, 22);
  layout->addWidget(sendBtn);
  // container->setStyleSheet(
  //     "background-color: #f0f0f0; border-right: 1px solid #c6c6c6;");
  container->setPaintRightBorder(false);
  container->setStyleSheet("background-color: #f0f0f0;");

  return container;
}

QWidget *TtModbusTableWidget::createFirstColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  TtSwitchButton *isEnableBtn = new TtSwitchButton(container);
  isEnableBtn->setObjectName("isEnableBtn");  // 设置对象名称
  layout->addWidget(isEnableBtn);

  return container;
}

QWidget *TtModbusTableWidget::createSecondColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit *lineEdit = new TtLineEdit(tr("名称"), container);
  lineEdit->setObjectName("name");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget *TtModbusTableWidget::createThirdColumnWidget() {
  // QWidget* container = new QWidget(this);
  // QHBoxLayout* layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);

  // // 创建下拉框和数字输入框
  // Ui::TtComboBox* comboBox = new Ui::TtComboBox(container);
  // comboBox->setObjectName("type");  // 设置对象名称
  // comboBox->addItems({tr("TEXT"), tr("HEX")});

  // layout->addWidget(comboBox);

  // return container;
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  Ui::TtComboBox *comboBox = nullptr;
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

QWidget *TtModbusTableWidget::createFourthColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  // layout->setContentsMargins(5, 2, 5, 2);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  // 创建下拉框和数字输入框
  TtLineEdit *lineEdit = new TtLineEdit(tr("内容"), container);
  lineEdit->setObjectName("content");  // 设置对象名称
  layout->addWidget(lineEdit, 0, Qt::AlignLeft);

  return container;
}

QWidget *TtModbusTableWidget::createFifthColumnWidget() {
  QWidget *container = new QWidget(this);
  container->setMinimumHeight(32);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(5, 2, 5, 2);

  QSpinBox *spinBox = new QSpinBox(container);
  spinBox->setObjectName("delay");  // 设置对象名称
  layout->addWidget(spinBox);

  return container;
}

QWidget *TtModbusTableWidget::createSixthColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *deleteBtn = new TtImageButton(":/sys/trash.svg", container);
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

QWidget *TtModbusTableWidget::createSeventhColumnWidget() {
  QWidget *container = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(container);
  layout->setContentsMargins(QMargins());

  TtImageButton *sendBtn = new TtImageButton(":/sys/send.svg", container);
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

inline void TtModbusTableWidget::HeaderWidget::paintEvent(QPaintEvent *event) {
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
