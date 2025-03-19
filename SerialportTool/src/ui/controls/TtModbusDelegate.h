#ifndef TTMODBUSDELEGATE_H
#define TTMODBUSDELEGATE_H

#include <QApplication>
#include <QSpinBox>
#include <QStyledItemDelegate>

#include <ui/control/TtSwitchButton.h>

namespace Ui {

class TableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  // 自定义数据结构（替代原有的 TableRow）
  struct RowData {
    // bool enabled = false;
    bool enabled;
    QString name;
    QString format;  // "TEXT" 或 "HEX"
    QString content;
    // int delay = 0;
    int delay;
  };

  explicit TableModel(QObject* parent = nullptr)
      : QAbstractTableModel(parent) {}

  // 必须实现的虚函数
  int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    return m_data.size();
  }

  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 7;  // 对应7列
  }

  QVariant data(const QModelIndex& index, int role) const override {
    if (!index.isValid())
      return QVariant();
    const auto& row = m_data[index.row()];
    switch (index.column()) {
      case 0:
        return row.enabled;  // 启用状态
      case 1:
        return row.name;  // 名称
      case 2:
        return row.format;  // 格式
      case 3:
        return row.content;  // 内容
      case 4:
        return row.delay;  // 延时
                           // 5、6列为操作按钮，不需要存储数据
    }
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override {
    if (!index.isValid())
      return false;
    auto& row = m_data[index.row()];
    switch (index.column()) {
      case 0:
        row.enabled = value.toBool();
        break;
      case 1:
        row.name = value.toString();
        break;
      case 2:
        row.format = value.toString();
        break;
      case 3:
        row.content = value.toString();
        break;
      case 4:
        row.delay = value.toInt();
        break;
      default:
        return false;
    }
    emit dataChanged(index, index);
    return true;
  }

  // 添加行
  void addRow(const RowData& data = RowData()) {
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(data);
    endInsertRows();
  }

  // 删除行
  void removeRow(int row) {
    if (row < 0 || row >= m_data.size())
      return;
    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    endRemoveRows();
  }

 private:
  QVector<RowData> m_data;
};

class TableDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  TableDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

  // 绘制单元格
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    // if (index.column() == 0) {
    //   // 绘制开关按钮
    //   drawSwitchButton(painter, option, index.data().toBool());
    // } else {
    //   QStyledItemDelegate::paint(painter, option, index);
    // }
    // 仅当不在编辑状态时绘制开关按钮
    if (index.column() == 0 && !(option.state & QStyle::State_Editing)) {
      drawSwitchButton(painter, option, index.data().toBool());
    } else {
      QStyledItemDelegate::paint(painter, option, index);
    }
  }

  // 创建编辑器（实际控件）
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override {
    qDebug() << "test";
    switch (index.column()) {
      case 0:
        return createSwitchButton(parent, index);  // 开关按钮
      case 2:
        return createFormatComboBox(parent, index);  // 格式下拉框
      case 4:
        return createDelaySpinBox(parent, index);  // 延时输入框
      default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    switch (index.column()) {
      case 0: {
        TtSwitchButton* btn = qobject_cast<TtSwitchButton*>(editor);
        btn->setChecked(index.data().toBool());
        break;
      }
      case 2: {
        QComboBox* combo = qobject_cast<QComboBox*>(editor);
        combo->addItems({"TEXT", "HEX"});
        combo->setCurrentText(index.data().toString());
        break;
      }
      case 4: {
        QSpinBox* spin = qobject_cast<QSpinBox*>(editor);
        spin->setRange(0, 9999);
        spin->setValue(index.data().toInt());
        break;
      }
      default:
        QStyledItemDelegate::setEditorData(editor, index);
    }
  }

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override {
    // 设置编辑器的几何形状，使其覆盖单元格
    editor->setGeometry(option.rect);
  }

  // 同步编辑器数据到模型
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override {
    switch (index.column()) {
      case 0: {
        auto btn = qobject_cast<TtSwitchButton*>(editor);
        model->setData(index, btn->isChecked());
        break;
      }
      case 2: {
        auto combo = qobject_cast<QComboBox*>(editor);
        model->setData(index, combo->currentText());
        break;
      }
      case 4: {
        auto spin = qobject_cast<QSpinBox*>(editor);
        model->setData(index, spin->value());
        break;
      }
      default:
        QStyledItemDelegate::setModelData(editor, model, index);
    }
  }

 private:
  // 绘制开关按钮
  void drawSwitchButton(QPainter* painter, const QStyleOptionViewItem& option,
                        bool checked) const {
    QStyleOptionButton opt;
    opt.rect = option.rect.adjusted(4, 4, -4, -4);  // 边距
    opt.state = QStyle::State_Enabled |
                (checked ? QStyle::State_On : QStyle::State_Off);
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &opt, painter);
  }

  // 创建控件
  TtSwitchButton* createSwitchButton(QWidget* parent,
                                     const QModelIndex& index) const {
    auto btn = new TtSwitchButton(parent);
    btn->setChecked(index.data().toBool());
    return btn;
  }

  QComboBox* createFormatComboBox(QWidget* parent,
                                  const QModelIndex& index) const {
    auto combo = new QComboBox(parent);
    qDebug() << "Test";
    combo->addItems({"TEXT", "HEX"});
    combo->setCurrentText(index.data().toString());
    return combo;
  }
  QSpinBox* createDelaySpinBox(QWidget* parent,
                               const QModelIndex& index) const {
    auto spin = new QSpinBox(parent);
    spin->setRange(0, 9999);
    spin->setValue(index.data().toInt());
    return spin;
  }
};

class TtModbusDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  //测试表格委托列索引
  enum DelegateColIndex {
    // NO_COL = 0,           //编号列(文本列)
    // BATCH_CHECK_COL = 1,  //批量选择(图标列)
    // LOVE_COL = 5          //喜欢(图标列)
    CHECK_COL = 0,         //编号列(文本列)
    ADDRESS_COL = 1,       //批量选择(图标列)
    NAME_COL = 2,          //喜欢(图标列)
    VALUE_COL = 3,         //批量选择(图标列)
    DESCRIPITION_COL = 4,  //批量选择(图标列)
    OPEARION_COL = 4,      //批量选择(图标列)
  };

  explicit TtModbusDelegate(QObject* parent = 0);

 protected:
  /*view 表项外观渲染*/
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;  //绘制渲染view item(表项)外观

 private:
  QPixmap uncheckedPixmap;  //批量未选中图标
  QPixmap checkedPixmap;    //批量选中图标

  QPixmap lovingPixmap;  //将会喜欢
  QPixmap lovedPixmap;   //喜欢

 signals:

 public slots:
};

}  // namespace Ui

#endif  // TTMODBUSDELEGATE_H
