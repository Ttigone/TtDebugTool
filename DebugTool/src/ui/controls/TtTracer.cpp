#include "ui/controls/TtTracer.h"

namespace Ui {

TtTracer::TtTracer(QCustomPlot* parentPlot) : QCPItemTracer(parentPlot) {}

TtQCustomPlotTracer::TtQCustomPlotTracer(QCustomPlot* _plot) {}

TtQCustomPlotTracer::~TtQCustomPlotTracer() {}

void TtQCustomPlotTracer::setLabelFont(QFont font) {}

void TtQCustomPlotTracer::setPen(const QPen& pen) {}

void TtQCustomPlotTracer::setBrush(const QBrush& brush) {}

void TtQCustomPlotTracer::setText(const QString& text) {}

void TtQCustomPlotTracer::setLabelPen(const QPen& pen) {}

void TtQCustomPlotTracer::updatePosition(TracerShowType type, double xValue,
                                         double yValue) {}

void TtQCustomPlotTracer::setVisible(bool visible) {}

}  // namespace Ui
