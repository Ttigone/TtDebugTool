#include "ui/controls/TtPlotItem.h"

namespace Ui {

TtAxisTag::TtAxisTag(QCPAxis* parentAxis)
    : QObject(parentAxis), mAxis(parentAxis) {
  mDummyTracer = new QCPItemTracer(mAxis->parentPlot());
  mDummyTracer->setVisible(false);
  mDummyTracer->position->setTypeX(
      QCPItemPosition::ptAxisRectRatio);  // x 是固定的
  mDummyTracer->position->setTypeY(
      QCPItemPosition::ptPlotCoords);  // y 值动态改变
  mDummyTracer->position->setAxisRect(mAxis->axisRect());
  mDummyTracer->position->setAxes(0, mAxis);
  mDummyTracer->position->setCoords(1, 0);  // 解释 x: 1 右下角, y: 值

  // 箭头
  mArrow = new QCPItemLine(mAxis->parentPlot());
  mArrow->setLayer("overlay");
  mArrow->setClipToAxisRect(false);
  mArrow->setHead(QCPLineEnding::esSpikeArrow);
  mArrow->start->setParentAnchor(mArrow->end);  // 起点跟随终点
  mArrow->start->setCoords(15, 0);
  mArrow->end->setParentAnchor(
      mDummyTracer->position);  // 终点跟随 tracer 的位置
  mArrow->setVisible(true);

  mLabel = new QCPItemText(mAxis->parentPlot());
  mLabel->setLayer("overlay");
  mLabel->setClipToAxisRect(false);
  mLabel->setPadding(QMargins(3, 0, 3, 0));
  mLabel->setPen(QPen(Qt::blue, 1));
  mLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  mLabel->position->setParentAnchor(mArrow->start);
  mLabel->setVisible(true);

  // 增加文本可视化设置
  mLabel->setColor(Qt::black);  // 文字颜色
  // mLabel->setBrush(QBrush(Qt::white));       // 背景填充
  mLabel->setBrush(QBrush(QColor(255, 255, 255, 220)));  // 背景填充
  // mLabel->setPadding(QMargins(4, 2, 4, 2));  // 增加内边距
  mLabel->setFont(QFont("Arial", 8));  // 明确设置字体
}

TtAxisTag::~TtAxisTag() {
  if (mDummyTracer)
    mDummyTracer->parentPlot()->removeItem(mDummyTracer);
  if (mArrow)
    mArrow->parentPlot()->removeItem(mArrow);
  if (mLabel)
    mLabel->parentPlot()->removeItem(mLabel);
}

void TtAxisTag::setPen(const QPen& pen) {
  mArrow->setPen(pen);
  mLabel->setPen(pen);
}

void TtAxisTag::setBrush(const QBrush& brush) {
  mLabel->setBrush(brush);
}

void TtAxisTag::setText(const QString& text) {
  mLabel->setText(text);
}

void TtAxisTag::updatePosition(double value) {
  mDummyTracer->position->setCoords(1, value);
  mArrow->end->setCoords(mAxis->offset(), 0);
}

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
