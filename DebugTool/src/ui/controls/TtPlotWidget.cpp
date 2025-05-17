#include "ui/controls/TtPlotWidget.h"

namespace Ui {

TtPlotWidget::TtPlotWidget(QCustomPlot* plot, QWidget* parent) {}

TtPlotWidget::TtPlotWidget(QWidget* parent) : QWidget{parent} {}

TtPlotWidget::~TtPlotWidget() { qDebug() << "delete" << __FUNCTION__; }

void TtPlotWidget::setupPlot(QCustomPlot *plot) {
  if (plot != plot_) {
    plot_ = plot;
    emit updatePlot();
  }
}

} // namespace Ui
