#ifndef UI_CONTROLS_TTTABLEVIEW_H
#define UI_CONTROLS_TTTABLEVIEW_H

#include <QHeaderView>
#include <QScrollBar>
#include <QTableWidget>

#include "Def.h"
#include "data/communication_metadata.h"
#include <ui/control/TtLineEdit.h>

class QSpinBox;

namespace Ui {

class TtSwitchButton;
class TtComboBox;
class TtLineEdit;
class TtCheckBox;
class TtSvgButton;

class TtTableWidget : public QTableWidget {
  Q_OBJECT
public:
  explicit TtTableWidget(QWidget *parent = nullptr);
  ~TtTableWidget();

  void setupHeaderRow();
  void setupTable(const QJsonObject &record);
  QJsonObject getTableRecord();
  void setCellWidget(int row, int column, QWidget *widget);
  void setEnabled(bool enable);

signals:
  void rowsChanged(quint16 rows);
  void sendRowMsg(const QString &msg, TtTextFormat::Type type, uint32_t times);
  void sendRowsMsg(const std::vector<Data::MsgInfo> &msgs);

private slots:
  void onAddRowButtonClicked();

private:
  class HeaderWidget : public QWidget {
  public:
    HeaderWidget(QWidget *parent = nullptr) : QWidget(parent), paint_(true) {}
    ~HeaderWidget();

    void setPaintRightBorder(bool isPaint) { paint_ = isPaint; }

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    bool paint_;
  };
  // 每一行固定的显示控件
  struct TableRow {
    // TtSwitchButton *enableBtn = nullptr;
    TtSwitchButton *checkBtn{nullptr};
    TtLineEdit *nameEdit{nullptr};
    TtComboBox *typeCombo{nullptr};
    TtLineEdit *contentEdit{nullptr};
    QSpinBox *delaySpin{nullptr};
    bool fromPool{false};
  };

  // 每个对象池的最大大小
  static const int MAX_POOL_SIZE = 100;

  // 对象池
  QList<TtComboBox *> comboPool_;
  QList<TtSwitchButton *> switchPool_;
  QList<QSpinBox *> spinPool_;
  QList<TtLineEdit *> lineEditPool_;
  QList<QWidget *> widgetPool_;

  QVector<TableRow> rowsData_;

  void initHeader();
  void setupRow(int row);
  void recycleRow(TableRow &row);

  // 控件管理
  TtSwitchButton *createSwitchButton();
  TtComboBox *createTypeComboBox();
  QSpinBox *createDelaySpin();
  TtLineEdit *createLineEdit(const QString &placeholderText = "");

  /// @brief 创建一个单元格的包装器
  /// @param content 包装的内容
  /// @return 包装后的QWidget
  QWidget *createCellWrapper(QWidget *content);

  int findRowIndex(QWidget *context, const int &col, bool deep = false) const;

  bool isRowVisible(int row);

  // UI 创建
  QWidget *createHeaderCell(const QString &text, bool border = true);

  QWidget *createAddButton();
  QWidget *createSendButton();
  QWidget *createDeleteButton();
  QWidget *createRowSendButton();

  QWidget *createHeaderWidget(const QString &text, bool paintBorder);

  QWidget *createHeaderAddRowWidget();  // 创建添加行按钮
  QWidget *createHeaderSendMsgWidget(); // 创建发送按钮

  QWidget *createFirstColumnWidget();   // 仅用于数据行
  QWidget *createSecondColumnWidget();  // 仅用于数据行
  QWidget *createThirdColumnWidget();   // 仅用于数据行
  QWidget *createFourthColumnWidget();  // 仅用于数据行
  QWidget *createFifthColumnWidget();   // 仅用于数据行
  QWidget *createSixthColumnWidget();   // 仅用于数据行
  QWidget *createSeventhColumnWidget(); // 仅用于数据行
  // 在类中添加控件缓存
  QMap<QWidget *, QHash<int, QWidget *>> cellWidgetCache_;

  QJsonObject record_;
  int rows_;
  int cols_;
  int visibleRowCount();
};

class TtModbusTableWidget : public QTableWidget {
  Q_OBJECT
public:
  explicit TtModbusTableWidget(TtModbusRegisterType::Type type,
                               QWidget *parent = nullptr);
  ~TtModbusTableWidget();

  void setRowValue(int row, int col, const QString &data);

  ///
  /// @brief getAddressValue
  /// @return
  /// 获取地址所在列的全部数据
  QVector<int> getAddressValue();

  ///
  /// @brief setValue
  /// @param data
  /// 单独设置某个寄存器的值
  void setValue(const QString &data);

  ///
  /// @brief setValue
  /// @param addr 一系列寄存器地址
  /// @param data 对应的寄存器值
  /// 设置一列寄存器的值
  void setValue(const int &addr, const QVector<quint16> &data);

  ///
  /// @brief setValue
  /// @param data
  ///
  void setValue(const QVector<quint16> &data);

  ///
  /// @brief setTable
  /// @param record
  /// 设置表格内容
  void setTable(const QJsonObject &record);

  ///
  /// @brief getTableRecord
  /// @return
  /// 获取表格设置的内容
  QJsonObject getTableRecord();

  ///
  /// @brief setCellWidget
  /// @param row
  /// @param column
  /// @param widget
  /// 为特定行列设定 cellWidget
  void setCellWidget(int row, int column, QWidget *widget);

  void setEnable(bool enable);

signals:
  ///
  /// @brief valueConfirmed
  /// @param addr
  /// @param value
  /// 发送值, 请求写入
  void valueConfirmed(const int &addr, const int &value);

  ///
  /// @brief requestShowGraph
  /// @param type 寄存器类型
  /// @param addr 寄存器机制
  /// @param enabled 是否使能在图标上
  /// 显示在表格上
  void requestShowGraph(TtModbusRegisterType::Type type, const int &addr,
                        bool enabled);

public slots:
  ///
  /// @brief addRow
  /// 表格添加一行
  void addRow();

protected:
  void showEvent(QShowEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void onValueChanged();
  void onConfirmClicked();
  void onCancelClicked();
  ///
  /// @brief onSwitchButtonToggle
  /// @param toggled
  /// 线圈或离散寄存器 请求写入值
  void onSwitchButtonToggle(bool toggled);
  void adjustRowHeights();

private:
  void connectSignals();

  ///
  /// @brief isRowVisible
  /// @param row
  /// @return
  /// 当前行是否处于可见区域
  bool isRowVisible(int row);
  ///
  /// @brief getRowValue
  /// @param col
  /// @return
  /// 遍历行, 从 1 ~ rowCount()
  QVector<QString> getRowValue(int col);
  void setupVisibleRows();

  struct TableRow {
    TtCheckBox *checkBtn{nullptr};
    TtLineEdit *address{nullptr};
    TtLineEdit *addressName{nullptr};
    TtLineEdit *value{nullptr};
    TtSwitchButton *valueButton{nullptr};
    // 编辑当前 lineedit value 时使用
    QPushButton *editButton{nullptr};    // 新增
    QPushButton *confirmButton{nullptr}; // 新增
    QPushButton *cancelButton{nullptr};  // 新增
    QString originalValue;               // 新增
    TtLineEdit *description{nullptr};
    bool fromPool{false};
    int currentAddress = -1; // 存储当前地址值
  };

  QList<TtComboBox *> comboPool_;
  QList<TtCheckBox *> switchPool_;
  QList<QSpinBox *> spinPool_;
  QList<QWidget *> widgetPool_;

  QVector<TableRow> rowsData_;

  ///
  /// @brief initHeader
  /// 设置行头
  void initHeader();
  void setupRow(int row);
  void recycleRow(TableRow &row);
  void deleteRow(int row);

  // 控件管理
  TtCheckBox *createCheckButton();
  TtSwitchButton *createSwitchButton();
  TtComboBox *createTypeComboBox(const QStringList &strs);
  TtSvgButton *createRefreshButton();
  QWidget *createCellWrapper(QWidget *content);

  int findRowIndex(QWidget *context, bool deep = false) const;

  // UI 创建
  QWidget *createHeaderCell(const QString &text, bool border = true);
  QWidget *createAddButton();
  QWidget *createSendButton();
  QWidget *createGraphAndDeleteButton();
  QWidget *createDeleteButton();
  QWidget *createRowSendButton();

  class HeaderWidget : public QWidget {
  public:
    HeaderWidget(QWidget *parent = nullptr) : QWidget(parent), paint_(true) {}

    void setPaintRightBorder(bool isPaint) { paint_ = isPaint; }

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    bool paint_;
  };

  QWidget *createHeaderWidget(const QString &text, bool paintBorder);

  QWidget *createHeaderAddRowWidget();  // 创建添加行按钮
  QWidget *createHeaderSendMsgWidget(); // 创建发送按钮

  QWidget *createFirstColumnWidget();   // 仅用于数据行
  QWidget *createSecondColumnWidget();  // 仅用于数据行
  QWidget *createThirdColumnWidget();   // 仅用于数据行
  QWidget *createFourthColumnWidget();  // 仅用于数据行
  QWidget *createFifthColumnWidget();   // 仅用于数据行
  QWidget *createSixthColumnWidget();   // 仅用于数据行
  QWidget *createSeventhColumnWidget(); // 仅用于数据行

  int visibleRowCount();

  // 在类中添加控件缓存
  QMap<QWidget *, QHash<int, QWidget *>> cellWidgetCache_;

  Ui::TtCheckBox *check_state_{nullptr}; // 控制 check 列
  Ui::TtComboBox *data_format_{nullptr}; // 控制 address 列

  QJsonObject record_;
  int rows_;
  int cols_;

  TtModbusRegisterType::Type type_;
  bool programmatic_update_{false};

  // // 地址到行号的映射
  // QMap<int, int> address_to_row_map_;

  // 一个地址可能对应多行
  QMultiMap<int, int> address_to_row_map_;
};

} // namespace Ui

#endif // UI_CONTROLS_TTTABLEVIEW_H
