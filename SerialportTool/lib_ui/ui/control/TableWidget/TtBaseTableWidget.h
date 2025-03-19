#ifndef UI_CONTROL_TABLEWIDGET_TTBASETABLEWIDGET_H
#define UI_CONTROL_TABLEWIDGET_TTBASETABLEWIDGET_H

#include <QHeaderView>
#include <QScrollBar>
#include <QTableWidget>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtBaseTableWidget : public QTableWidget {
  Q_OBJECT
 public:
  explicit TtBaseTableWidget(QWidget* parent = nullptr);
  explicit TtBaseTableWidget(int row, int column, QWidget* parent = nullptr);

  /*设置/获取单元格数据(文本、图标、部件以及用户自定义数据等)*/
  void setItemText(int row, int column, QString text,
                   int alignment = Qt::AlignCenter);
  QString getItemText(int row, int column);
  void setItemIcon(int row, int column, QIcon& icon);
  void setItemWidget(int row, int column, QWidget* widget,
                     bool isCenteredLayout = false);
  QWidget* getItemWidget(int row, int column, bool isCenteredLayout = false);
  void setItemData(int row, int column, QVariant variant,
                   int role = Qt::UserRole);
  QVariant getItemData(int row, int column, int role = Qt::UserRole);

  /*其他常用功能接口*/
  void setColumnAutoNumber(int column, int firstNumber = 1);  //设置某列自动编号
  void setColWidthRowHeight(int columnWidth, int rowHeight);  //设置列宽行高
  //设置表格垂直滚动条悬浮显示
  void setVerScrollBarSuspension(int verScrollBarActualWidth = 56,
                                 int verScrollBarVisibleWidth = 9,
                                 bool verScrollBarAsNeeded = false);

 protected:
  virtual void resizeEvent(QResizeEvent* event);

 private:
  void initTableProperty();  //初始化设置一些表格的默认属性

  bool verScrollBarSuspensionFlag;  //垂直滚动条悬浮标记

 signals:

 public slots:
  void verticalScrollBarRangeChangedSlot(
      int min, int max);  //表格垂直滚动条范围改变信号响应槽
};

}  // namespace Ui

#endif  // UI_CONTROL_TABLEWIDGET_TTBASETABLEWIDGET_H
