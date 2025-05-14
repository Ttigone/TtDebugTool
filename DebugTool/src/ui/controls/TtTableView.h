#include <QHeaderView>
#include <QScrollBar>
#include <QTableWidget>

#include <ui/control/TtLineEdit.h>
#include "Def.h"

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
  explicit TtTableWidget(QWidget* parent = nullptr);
  ~TtTableWidget();

  void setupHeaderRow();

  void setupTable(const QJsonObject& record);
  QJsonObject getTableRecord();

  void setCellWidget(int row, int column, QWidget* widget);

 signals:
  void rowsChanged(quint16 rows);
  void sendRowMsg(const QString& msg);
  void sendRowsMsg(const QVector<QPair<QString, int>>& msg);

 private slots:
  void onAddRowButtonClicked();

 private:
  // 每一行固定的显示控件
  struct TableRow {
    TtSwitchButton* enableBtn = nullptr;
    TtLineEdit* nameEdit = nullptr;
    TtComboBox* typeCombo = nullptr;
    TtLineEdit* contentEdit = nullptr;
    QSpinBox* delaySpin = nullptr;
    bool fromPool = false;
  };

  // 对象池
  QList<TtComboBox*> comboPool_;
  QList<TtSwitchButton*> switchPool_;
  QList<QSpinBox*> spinPool_;
  QList<QWidget*> widgetPool_;

  QVector<TableRow> rowsData_;

  void initHeader();
  void setupRow(int row);
  void recycleRow(TableRow& row);

  // 控件管理
  TtSwitchButton* createSwitchButton();
  TtComboBox* createTypeComboBox();
  QSpinBox* createDelaySpin();

  /// @brief 创建一个单元格的包装器
  /// @param content 包装的内容
  /// @return 包装后的QWidget
  QWidget* createCellWrapper(QWidget* content);

  int findRowIndex(QWidget* context, const int& col, bool deep = false) const;

  bool isRowVisible(int row) {
    return row >= verticalScrollBar()->value() &&
           row <= verticalScrollBar()->value() + visibleRowCount();
  }

  // UI 创建
  QWidget* createHeaderCell(const QString& text, bool border = true);

  QWidget* createAddButton();
  QWidget* createSendButton();
  QWidget* createDeleteButton();
  QWidget* createRowSendButton();

  class HeaderWidget : public QWidget {
   public:
    HeaderWidget(QWidget* parent = nullptr) : QWidget(parent), paint_(true) {}
    ~HeaderWidget();

    void setPaintRightBorder(bool isPaint) { paint_ = isPaint; }

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    bool paint_;
  };

  QWidget* createHeaderWidget(const QString& text, bool paintBorder);

  QWidget* createHeaderAddRowWidget();   // 创建添加行按钮
  QWidget* createHeaderSendMsgWidget();  // 创建发送按钮

  QWidget* createFirstColumnWidget();    // 仅用于数据行
  QWidget* createSecondColumnWidget();   // 仅用于数据行
  QWidget* createThirdColumnWidget();    // 仅用于数据行
  QWidget* createFourthColumnWidget();   // 仅用于数据行
  QWidget* createFifthColumnWidget();    // 仅用于数据行
  QWidget* createSixthColumnWidget();    // 仅用于数据行
  QWidget* createSeventhColumnWidget();  // 仅用于数据行
  // 在类中添加控件缓存
  QMap<QWidget*, QHash<int, QWidget*>> cellWidgetCache_;

  QJsonObject record_;
  int rows_;
  int cols_;
  int visibleRowCount();
};

class TtModbusTableWidget : public QTableWidget {
  Q_OBJECT
 public:
  explicit TtModbusTableWidget(TtModbusRegisterType::Type type,
                               QWidget* parent = nullptr);
  ~TtModbusTableWidget();

  void setRowValue(int row, int col, const QString& data);
  QVector<int> getAddressValue();
  void setValue(const QString& data);
  void setValue(const int& addr, const QVector<quint16>& data);
  void setValue(const QVector<quint16>& data);

  void setTable(const QJsonObject& record);
  QJsonObject getTableRecord();

  ///
  /// @brief setCellWidget
  /// @param row
  /// @param column
  /// @param widget
  /// 为特定行列设定 cellWidget
  void setCellWidget(int row, int column, QWidget* widget);

 signals:
  ///
  /// @brief valueConfirmed
  /// @param addr
  /// @param value
  /// 发送值, 请求写入
  void valueConfirmed(const int& addr, const int& value);
  void requestShowGraph(TtModbusRegisterType::Type type, const int& addr,
                        bool enabled);

 public slots:
  void addRow();

 protected:
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

 private slots:
  void onValueChanged();
  void onConfirmClicked();
  void onCancelClicked();
  void onSwitchButtonToggle(bool toggled);
  void adjustRowHeights();

 private:
  void connectSignals();
  bool isRowVisible(int row);
  QVector<QString> getRowValue(int col);
  void setupVisibleRows();

  struct TableRow {
    TtCheckBox* checkBtn{nullptr};
    TtLineEdit* address{nullptr};
    TtLineEdit* addressName{nullptr};
    TtLineEdit* value{nullptr};
    TtSwitchButton* valueButton{nullptr};
    QPushButton* editButton{nullptr};     // 新增
    QPushButton* confirmButton{nullptr};  // 新增
    QPushButton* cancelButton{nullptr};   // 新增
    QString originalValue;                // 新增
    TtLineEdit* description{nullptr};
    bool fromPool{false};
  };

  QList<TtComboBox*> comboPool_;
  QList<TtCheckBox*> switchPool_;
  QList<QSpinBox*> spinPool_;
  QList<QWidget*> widgetPool_;

  QVector<TableRow> rowsData_;

  ///
  /// @brief initHeader
  /// 设置行头
  void initHeader();
  void setupRow(int row);
  void recycleRow(TableRow& row);

  // 控件管理
  TtCheckBox* createCheckButton();
  TtSwitchButton* createSwitchButton();
  TtComboBox* createTypeComboBox(const QStringList& strs);
  TtSvgButton* createRefreshButton();
  QWidget* createCellWrapper(QWidget* content);

  int findRowIndex(QWidget* context, bool deep = false) const;

  // UI 创建
  QWidget* createHeaderCell(const QString& text, bool border = true);

  QWidget* createAddButton();
  QWidget* createSendButton();

  QWidget* createGraphAndDeleteButton();

  QWidget* createDeleteButton();

  QWidget* createRowSendButton();

  class HeaderWidget : public QWidget {
   public:
    HeaderWidget(QWidget* parent = nullptr) : QWidget(parent), paint_(true) {}

    void setPaintRightBorder(bool isPaint) { paint_ = isPaint; }

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    bool paint_;
  };

  QWidget* createHeaderWidget(const QString& text, bool paintBorder);

  QWidget* createHeaderAddRowWidget();   // 创建添加行按钮
  QWidget* createHeaderSendMsgWidget();  // 创建发送按钮

  QWidget* createFirstColumnWidget();    // 仅用于数据行
  QWidget* createSecondColumnWidget();   // 仅用于数据行
  QWidget* createThirdColumnWidget();    // 仅用于数据行
  QWidget* createFourthColumnWidget();   // 仅用于数据行
  QWidget* createFifthColumnWidget();    // 仅用于数据行
  QWidget* createSixthColumnWidget();    // 仅用于数据行
  QWidget* createSeventhColumnWidget();  // 仅用于数据行

  // 在类中添加控件缓存
  QMap<QWidget*, QHash<int, QWidget*>> cellWidgetCache_;

  Ui::TtCheckBox* check_state_;  // 控制 check 列
  Ui::TtComboBox* data_format_;  // 控制 address 列

  QJsonObject record_;
  int rows_;
  int cols_;

  TtModbusRegisterType::Type type_;
  int visibleRowCount();
};

}  // namespace Ui
