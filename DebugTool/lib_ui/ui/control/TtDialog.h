#ifndef UI_CONTROL_TTDIALOG_H
#define UI_CONTROL_TTDIALOG_H

#include <QDialog>

#include "ui/ui_pch.h"

namespace Ui {

class TtDialogPrivate;

class Tt_EXPORT TtDialog : public QDialog {
  Q_OBJECT
  Q_Q_CREATE(TtDialog)
 public:
  explicit TtDialog(QWidget* parent = nullptr);
  ~TtDialog();
};

}  // namespace Ui

#endif  // UI_CONTROL_TTDIALOG_H
