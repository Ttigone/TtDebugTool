#include "ui/control/TtLoadingWidget.h"

namespace Ui {

TtLoadingWidget::TtLoadingWidget(QWidget *parent) : QDialog(parent) {
  setAttribute(Qt::WA_DeleteOnClose);
}

TtLoadingWidget::~TtLoadingWidget() { qDebug() << "delete" << __FUNCTION__; }

} // namespace Ui
