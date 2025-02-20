// // 自定义模型
// class CustomModel : public QAbstractTableModel {
//   Q_OBJECT
//  public:
//   CustomModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
//     // 初始化三列：按钮、文本编辑、组合框
//     dataList = {{"启用", "名称", "格式", "内容", "延时", "操作"}};
//   }

//   int rowCount(const QModelIndex& parent = QModelIndex()) const override {
//     if (parent.isValid()) {
//       return 0;
//     }
//     return dataList.size();
//   }

//   int columnCount(const QModelIndex& parent = QModelIndex()) const override {
//     // qDebug() << "count: " << dataList[0].size();
//     return dataList.isEmpty() ? 0 : dataList[0].size();
//   }

//   QVariant data(const QModelIndex& index,
//                 int role = Qt::DisplayRole) const override {
//     if (!index.isValid()) {
//       return QVariant();
//     }
//     if (index.row() >= dataList.size() || index.row() < 0) {
//       return QVariant();
//     }

//     if (role == Qt::DisplayRole || role == Qt::EditRole) {
//       // qDebug() << "1";
//       return dataList[index.row()][index.column()];
//     }
//     return QVariant();
//   }

//   bool setData(const QModelIndex& index, const QVariant& value,
//                int role = Qt::EditRole) override {
//     if (role == Qt::EditRole) {
//       dataList[index.row()][index.column()] = value.toString();
//       emit dataChanged(index, index, {role});
//       return true;
//     }
//     return false;
//   }

//   Qt::ItemFlags flags(const QModelIndex& index) const override {
//     if (!index.isValid())
//       return Qt::NoItemFlags;
//     return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
//   }

//   QVariant headerData(int section, Qt::Orientation orientation,
//                       int role) const {
//     if (role != Qt::DisplayRole)
//       return QVariant();

//     if (orientation == Qt::Horizontal) {
//       switch (section) {
//         case 0:
//           return QString("Column 1");
//         case 1:
//           return QString("Column 2");
//         case 2:
//           return QString("Column 3");
//         default:
//           return QVariant();
//       }
//     }
//     return QVariant();
//   }

//   void addRow() {
//     // qDebug() << "test";
//     // // beginInsertRows(QModelIndex(), rowCount(), rowCount());
//     // // dataList.append({"Add Row", "", "Option 1"});
//     // // endInsertRows();
//     // qDebug() << "test2";

//     // qDebug() << "test";

//     int newRow = dataList.size();
//     beginInsertRows(QModelIndex(), newRow, newRow);

//     // 假设 RowData 是一个包含三个 QString 的结构体
//     // 满足个数
//     dataList.append({"Add Row", "", "Option 1", "s", "d", "2"});

//     endInsertRows();
//     // qDebug() << "test2";
//   }

//  private:
//   QVector<QVector<QString>> dataList;
// };

// // // 按钮委托
// // class ButtonDelegate : public QStyledItemDelegate {
// //   Q_OBJECT
// //  public:
// //   ButtonDelegate(CustomModel* model, QObject* parent = nullptr)
// //       : QStyledItemDelegate(parent), model(model) {}

// //   QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
// //                         const QModelIndex& index) const override {
// //     // 第 0 列
// //     if (index.column() == 0) {  // 按钮列
// //       QPushButton* button = new QPushButton("Add Row", parent);
// //       connect(button, &QPushButton::clicked, model, &CustomModel::addRow);
// //       return button;
// //     }
// //     return QStyledItemDelegate::createEditor(parent, option, index);
// //   }

// //   void paint(QPainter* painter, const QStyleOptionViewItem& option,
// //              const QModelIndex& index) const override {
// //     if (index.column() == 0) {
// //       QPushButton button;
// //       button.setText("Add Row");
// //       QStyleOptionButton opt;
// //       opt.rect = option.rect;
// //       opt.state = QStyle::State_Enabled;
// //       // 绘制
// //       button.style()->drawControl(QStyle::CE_PushButton, &opt, painter,
// //                                   &button);
// //     } else {
// //       QStyledItemDelegate::paint(painter, option, index);
// //     }
// //   }

// //  private:
// //   CustomModel* model;
// // };

// class ButtonDelegate : public QStyledItemDelegate {
//   Q_OBJECT
//  public:
//   explicit ButtonDelegate(QObject* parent = nullptr)
//       : QStyledItemDelegate(parent) {}

//   // 绘制按钮
//   void paint(QPainter* painter, const QStyleOptionViewItem& option,
//              const QModelIndex& index) const override {
//     QPushButton button;
//     if (index.row() == 0 && index.column() == 2) {  // 首行最后一列
//       button.setText("添加");
//     } else {
//       button.setText("按钮");
//     }

//     QStyleOptionButton optionButton;
//     optionButton.rect = option.rect.adjusted(4, 4, -4, -4);
//     optionButton.text = button.text();
//     optionButton.state = QStyle::State_Enabled;

//     QApplication::style()->drawControl(QStyle::CE_PushButton, &optionButton,
//                                        painter, &button);
//   }

//   // 创建编辑器（按钮）
//   QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
//                         const QModelIndex& index) const override {
//     Q_UNUSED(option);
//     QPushButton* button = new QPushButton(parent);
//     if (index.row() == 0 && index.column() == 2) {
//       button->setText("添加");
//     } else {
//       button->setText("按钮");
//     }
//     return button;
//   }

//   // 设置编辑器的初始化内容
//   void setEditorData(QWidget* editor, const QModelIndex& index) const override {
//     QPushButton* button = static_cast<QPushButton*>(editor);
//     if (index.row() == 0 && index.column() == 2) {
//       button->setText("添加");
//     } else {
//       button->setText("按钮");
//     }
//   }

//   // 设置模型数据（触发信号）
//   void setModelData(QWidget* editor, QAbstractItemModel* model,
//                     const QModelIndex& index) const override {
//     Q_UNUSED(editor);
//     Q_UNUSED(model);
//     Q_UNUSED(index);
//   }

//  signals:
//   void buttonClicked(const QModelIndex& index);

//  protected:
//   bool editorEvent(QEvent* event, QAbstractItemModel* model,
//                    const QStyleOptionViewItem& option,
//                    const QModelIndex& index) override {
//     if (event->type() == QEvent::MouseButtonRelease) {
//       emit buttonClicked(index);
//       return true;
//     }
//     return QStyledItemDelegate::editorEvent(event, model, option, index);
//   }
// };

// // 文本编辑委托
// class LineEditDelegate : public QStyledItemDelegate {
//   Q_OBJECT
//  public:
//   LineEditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

//   QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
//                         const QModelIndex& index) const override {
//     QLineEdit* editor = new QLineEdit(parent);
//     return editor;
//   }
// };

// // 组合框委托
// class ComboBoxDelegate : public QStyledItemDelegate {
//   Q_OBJECT
//  public:
//   ComboBoxDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

//   QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
//                         const QModelIndex& index) const override {
//     QComboBox* editor = new QComboBox(parent);
//     editor->addItems({"Option 1", "Option 2", "Option 3"});
//     return editor;
//   }

//   void setEditorData(QWidget* editor, const QModelIndex& index) const override {
//     QString currentText = index.model()->data(index, Qt::EditRole).toString();
//     QComboBox* comboBox = static_cast<QComboBox*>(editor);
//     int idx = comboBox->findText(currentText);
//     if (idx >= 0) {
//       comboBox->setCurrentIndex(idx);
//     }
//   }

//   void setModelData(QWidget* editor, QAbstractItemModel* model,
//                     const QModelIndex& index) const override {
//     QComboBox* comboBox = static_cast<QComboBox*>(editor);
//     QString selected = comboBox->currentText();
//     model->setData(index, selected, Qt::EditRole);
//   }
// };

// class MultiDelegate : public QStyledItemDelegate {
//   Q_OBJECT
//  public:
//   explicit MultiDelegate(QObject* parent = nullptr)
//       : QStyledItemDelegate(parent),
//         buttonDelegate(new ButtonDelegate(this)),
//         lineEditDelegate(new LineEditDelegate(this)),
//         comboBoxDelegate(new ComboBoxDelegate(this)) {}

//   ~MultiDelegate() {}

//   // 创建编辑器，根据行和列选择委托
//   QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
//                         const QModelIndex& index) const override {
//     if (isFirstRow(index)) {
//       if (index.column() == 2) {  // 首行最后一列
//         return buttonDelegate->createEditor(parent, option, index);
//       } else {
//         return QStyledItemDelegate::createEditor(parent, option,
//                                                  index);  // 使用默认委托
//       }
//     } else {
//       switch (index.column()) {
//         case 0:
//           return buttonDelegate->createEditor(parent, option, index);
//         case 1:
//           return lineEditDelegate->createEditor(parent, option, index);
//         case 2:
//           return comboBoxDelegate->createEditor(parent, option, index);
//         default:
//           return QStyledItemDelegate::createEditor(parent, option, index);
//       }
//     }
//   }

//   // 设置编辑器的数据
//   void setEditorData(QWidget* editor, const QModelIndex& index) const override {
//     if (isFirstRow(index)) {
//       if (index.column() == 2) {  // 首行最后一列
//         buttonDelegate->setEditorData(editor, index);
//       } else {
//         QStyledItemDelegate::setEditorData(editor, index);  // 使用默认委托
//       }
//     } else {
//       switch (index.column()) {
//         case 0:
//           buttonDelegate->setEditorData(editor, index);
//           break;
//         case 1:
//           lineEditDelegate->setEditorData(editor, index);
//           break;
//         case 2:
//           comboBoxDelegate->setEditorData(editor, index);
//           break;
//         default:
//           QStyledItemDelegate::setEditorData(editor, index);
//       }
//     }
//   }

//   // 设置模型数据
//   void setModelData(QWidget* editor, QAbstractItemModel* model,
//                     const QModelIndex& index) const override {
//     if (isFirstRow(index)) {
//       if (index.column() == 2) {  // 首行最后一列
//         buttonDelegate->setModelData(editor, model, index);
//       } else {
//         QStyledItemDelegate::setModelData(editor, model,
//                                           index);  // 使用默认委托
//       }
//     } else {
//       switch (index.column()) {
//         case 0:
//           buttonDelegate->setModelData(editor, model, index);
//           break;
//         case 1:
//           lineEditDelegate->setModelData(editor, model, index);
//           break;
//         case 2:
//           comboBoxDelegate->setModelData(editor, model, index);
//           break;
//         default:
//           QStyledItemDelegate::setModelData(editor, model, index);
//       }
//     }
//   }

//   // 绘制单元格
//   void paint(QPainter* painter, const QStyleOptionViewItem& option,
//              const QModelIndex& index) const override {
//     if (isFirstRow(index)) {
//       if (index.column() == 2) {  // 首行最后一列
//         buttonDelegate->paint(painter, option, index);
//       } else {
//         QStyledItemDelegate::paint(painter, option, index);  // 使用默认委托
//       }
//     } else {
//       switch (index.column()) {
//         case 0:
//           buttonDelegate->paint(painter, option, index);
//           break;
//         case 1:
//           lineEditDelegate->paint(painter, option, index);
//           break;
//         case 2:
//           comboBoxDelegate->paint(painter, option, index);
//           break;
//         default:
//           QStyledItemDelegate::paint(painter, option, index);
//       }
//     }
//   }

//  signals:
//   void buttonClicked(const QModelIndex& index);

//  private:
//   bool isFirstRow(const QModelIndex& index) const { return index.row() == 0; }

//   ButtonDelegate* buttonDelegate;
//   LineEditDelegate* lineEditDelegate;
//   ComboBoxDelegate* comboBoxDelegate;
// };

#include <QTableWidget>

namespace Ui {

class TtTableWidget : public QTableWidget {
 public:
  explicit TtTableWidget(QWidget* parent = nullptr);
  // TtTableWidget(int rows, int columns, QWidget* parent = nullptr);
  ~TtTableWidget();

  void setupHeaderRow();

  void setupTable(const QJsonObject &record);
  QJsonObject getTableRecord();

 private slots:
  void onAddRowButtonClicked();

 private:
  class HeaderWidget : public QWidget {
    // Q_OBJECT
   public:
    HeaderWidget(QWidget* parent = nullptr) : QWidget(parent), paint_(true) {}

    void setPaintRightBorder(bool isPaint) { paint_ = isPaint; }

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    bool paint_;
  };

  void init();

  QWidget* createHeaderEnableWidget();   // 创建启用按钮
  QWidget* createHeaderNameWidget();     // 创建启用按钮
  QWidget* createHeaderFormatWidget();   // 创建启用按钮
  QWidget* createHeaderContentWidget();  // 创建启用按钮
  QWidget* createHeaderDelayWidget();    // 创建启用按钮
  QWidget* createHeaderAddRowWidget();   // 创建添加行按钮
  QWidget* createHeaderSendMsgWidget();  // 创建发送按钮

  QWidget* createFirstColumnWidget();    // 仅用于数据行
  QWidget* createSecondColumnWidget();   // 仅用于数据行
  QWidget* createThirdColumnWidget();    // 仅用于数据行
  QWidget* createFourthColumnWidget();   // 仅用于数据行
  QWidget* createFifthColumnWidget();    // 仅用于数据行
  QWidget* createSixthColumnWidget();    // 仅用于数据行
  QWidget* createSeventhColumnWidget();  // 仅用于数据行

  QJsonObject record_;
  int rows_;
  int cols_;
};

}  // namespace Ui
