#ifndef UI_CONTROL_TTLOADINGWIDGET_H
#define UI_CONTROL_TTLOADINGWIDGET_H

#include <QDialog>

namespace Ui {

class TtLoadingWidget : public QDialog {
  Q_OBJECT
public:
  TtLoadingWidget(QWidget *parent = nullptr);
  ~TtLoadingWidget();

private:
};

} // namespace Ui

#endif // UI_CONTROL_TTLOADINGWIDGET_H
