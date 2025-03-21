#ifndef UI_CONTROL_TABLEWIDGET_TTHEADERVIEW_H
#define UI_CONTROL_TABLEWIDGET_TTHEADERVIEW_H

#include <QHeaderView>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtHeaderView : public QHeaderView {
  Q_OBJECT
 public:
  explicit TtHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

  void setHeaderViewWidget(QWidget* widget, int logicalIndex);  //设置表头控件

  /*函数重写*/
  void setSectionHidden(int logicalIndex,
                        bool hide);  //设置表头某一个section隐藏/显示

 protected:
  virtual void paintSection(QPainter* painter, const QRect& rect,
                            int logicalIndex) const;

 private:
  /*表头控件参数*/
  QMap<int, QWidget*> headerViewWidgetMap;

 signals:

 public slots:
};

}  // namespace Ui

#endif  // UI_CONTROL_TABLEWIDGET_TTHEADERVIEW_H
