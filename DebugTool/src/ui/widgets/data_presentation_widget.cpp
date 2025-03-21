#include "data_presentation_widget.h"

namespace Ui {

// DataPresentationWidget::DataPresentationWidget(QWidget* parent)
//     : QWidget(parent) {
//   init();
// }

// DataPresentationWidget::~DataPresentationWidget() {}

// void DataPresentationWidget::init() {
//   // 创建一个内容窗口 ProtocolWaveform 父元素为 pProtocolWaveformCDockWidget
//   QVBoxLayout* main_layout_ = new QVBoxLayout(this);
//   // QWidget* ProtocolWaveform = new QWidget(this);
//   // customplot_ = new QCustomPlot(this);
//   customplot_ = new TtQCustomPlot(this);
//   // customplot_->showMaximized();
//   main_layout_->addWidget(customplot_);
//   customplot_->addGraph();
//   customplot_->graph(0)->setPen(
//       QPen(Qt::blue));  // line color blue for first graph
//   customplot_->graph(0)->setBrush(QBrush(QColor(
//       0, 0, 255, 20)));  // first graph will be filled with translucent blue
//   customplot_->addGraph();
//   customplot_->graph(1)->setPen(
//       QPen(Qt::red));  // line color red for second graph

//   QVector<double> x(250), y0(250), y1(250);
//   for (int i = 0; i < 250; ++i) {
//     x[i] = i;
//     y0[i] = qExp(-i / 150.0) * qCos(i / 10.0);  // exponentially decaying cosine
//     y1[i] = qExp(-i / 150.0);                   // exponential envelope
//   }
//   // customplot_->xAxis2->setVisible(true);
//   // customplot_->xAxis2->setTickLabels(false);
//   // customplot_->yAxis2->setVisible(true);
//   // customplot_->yAxis2->setTickLabels(false);
//   // make left and bottom axes always transfer their ranges to right and top axes:
//   connect(customplot_->xAxis, SIGNAL(rangeChanged(QCPRange)),
//           customplot_->xAxis2, SLOT(setRange(QCPRange)));
//   connect(customplot_->yAxis, SIGNAL(rangeChanged(QCPRange)),
//           customplot_->yAxis2, SLOT(setRange(QCPRange)));

//   // pass data points to graphs:
//   customplot_->graph(0)->setData(x, y0);
//   customplot_->graph(1)->setData(x, y1);
//   // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
//   customplot_->graph(0)->rescaleAxes();
//   // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
//   customplot_->graph(1)->rescaleAxes(true);
//   // Note: we could have also just called customPlot->rescaleAxes(); instead
//   // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:

//   // customplot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
//   //                              QCP::iSelectPlottables);

//   // cusotmplot_ = new TtQCustomPlot(ProtocolWaveform, true);
//   QVector<double> readKeys;
//   QVector<double> readValues;
//   for (int i = 0; i < 4; ++i) {
//     readKeys.append(1);
//     readValues.append(2);
//   }
//   customplot_->addGraphsData(readKeys, readValues);
// }

}  // namespace Ui
