#ifndef UI_CONTROL_TTDIALOGPRIVATE_H
#define UI_CONTROL_TTDIALOGPRIVATE_H

namespace Ui {

class TtDialog;

class TtDialogPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtDialog)
 public:
  explicit TtDialogPrivate(QObject* parent = nullptr);
  ~TtDialogPrivate();
};

}  // namespace Ui

#endif  // UI_CONTROL_TTDIALOGPRIVATE_H
