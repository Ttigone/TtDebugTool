#ifndef UI_CONTROL_TTLISTVIEW_H
#define UI_CONTROL_TTLISTVIEW_H

#include <QListView>

#include "ui/ui_pch.h"

namespace Ui {

class TtListViewPrivate;

class Tt_EXPORT TtListView : public QListView {
  Q_OBJECT
  Q_Q_CREATE(TtListView)
  Q_PROPERTY_CREATE_Q_H(int, ItemHeight)
  Q_PROPERTY_CREATE_Q_H(bool, IsTransparent)
 public:
  explicit TtListView(QWidget* parent = nullptr);
  ~TtListView();
};

}  // namespace Ui

#endif  // UI_CONTROL_TTLISTVIEW_H
