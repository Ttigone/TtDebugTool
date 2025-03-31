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

  void setupTable(const QJsonObject &record);
  QJsonObject getTableRecord();

  void setCellWidget(int row, int column, QWidget* widget);

 signals:
  void sendRowMsg(const QString& msg);
  void sendRowsMsg(const QVector<QString>& msg);

 private slots:
  void onAddRowButtonClicked();

 private:
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
  QWidget* createCellWrapper(QWidget* content);

  int findRowIndex(QWidget* context, const int& col, bool deep = false) const {
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

  // UI 创建
  QWidget* createHeaderCell(const QString& text, bool border = true) {
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

  QWidget* createAddButton();
  QWidget* createSendButton();
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

  QJsonObject record_;
  int rows_;
  int cols_;
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

  void setCellWidget(int row, int column, QWidget* widget);

 signals:
  void valueConfirmed(const int& addr, const int& value);  // 值被确认保存
  void requestShowGraph(TtModbusRegisterType::Type type, const int& addr,
                        bool enabled);

 public slots:
  void addRow();

 private slots:
  void onValueChanged();
  void onConfirmClicked();
  void onCancelClicked();
  void onSwitchButtonToggle(bool toggled);

 private:
  QVector<QString> getRowValue(int col);

  struct TableRow {
    TtCheckBox* checkBtn = nullptr;
    TtLineEdit* address = nullptr;
    TtLineEdit* addressName = nullptr;
    TtLineEdit* value = nullptr;
    TtSwitchButton* valueButton = nullptr;
    QPushButton* editButton = nullptr;     // 新增
    QPushButton* confirmButton = nullptr;  // 新增
    QPushButton* cancelButton = nullptr;   // 新增
    QString originalValue;                 // 新增
    TtLineEdit* description = nullptr;
    bool fromPool = false;
  };

  QList<TtComboBox*> comboPool_;
  QList<TtCheckBox*> switchPool_;
  QList<QSpinBox*> spinPool_;
  QList<QWidget*> widgetPool_;

  QVector<TableRow> rowsData_;

  void initHeader();
  void setupRow(int row);
  void recycleRow(TableRow& row);

  // 控件管理
  TtCheckBox* createCheckButton();
  TtSwitchButton* createSwitchButton();
  TtComboBox* createTypeComboBox(const QStringList& strs);
  TtSvgButton* createRefreshButton();
  QWidget* createCellWrapper(QWidget* content);

  int findRowIndex(QWidget* context, bool deep = false) const {
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

  // UI 创建
  QWidget* createHeaderCell(const QString& text, bool border = true) {
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

  QWidget* createAddButton() {
    auto* btn = new QPushButton(QIcon(":/sys/plus-circle.svg"), "");
    btn->setFlat(true);
    connect(btn, &QPushButton::clicked, this, [this] { addRow(); });
    return createCellWrapper(btn);
  }
  QWidget* createSendButton() {
    auto* btn = new QPushButton(QIcon(":/sys/send.svg"), "");
    btn->setFlat(true);
    connect(btn, &QPushButton::clicked, this, [this]() {});
    return createCellWrapper(btn);
  }

  QWidget* createGraphAndDeleteButton();

  QWidget* createDeleteButton() {
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

  QWidget* createRowSendButton() {
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

  QJsonObject record_;
  int rows_;
  int cols_;

  TtModbusRegisterType::Type type_;
};

}  // namespace Ui
