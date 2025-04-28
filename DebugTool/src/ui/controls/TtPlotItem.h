#ifndef UI_CONTROLS_TTPLOTITEM_H
#define UI_CONTROLS_TTPLOTITEM_H

#include "qcustomplot/qcustomplot.h"

namespace Ui {

class TtAxisTag : public QObject {
  Q_OBJECT
 public:
  explicit TtAxisTag(QCPAxis* parentAxis);
  virtual ~TtAxisTag();

  void setPen(const QPen& pen);
  void setBrush(const QBrush& brush);
  void setText(const QString& text);

  QPen pen() const { return mLabel->pen(); }
  QBrush brush() const { return mLabel->brush(); }
  QString text() const { return mLabel->text(); }

  void updatePosition(double value);

 protected:
  QCPAxis* mAxis;
  QPointer<QCPItemTracer> mDummyTracer;
  QPointer<QCPItemLine> mArrow;
  QPointer<QCPItemText> mLabel;
};

class TtTracer : public QCPItemTracer {
 public:
  TtTracer(QCustomPlot* parentPlot);

 protected:
  bool event(QEvent* event) override {
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseMove) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      QApplication::sendEvent(mParentPlot, mouseEvent);
      return true;
    }
    return QCPItemTracer::event(event);
  }
};

class TtQCustomPlotTracer : public QObject {
  Q_OBJECT
 public:
  enum TracerShowType {
    showTracerPoint = 0X01,
    showTracerLabel = 0X02,
    showTracerArrow = 0X04,
    showTracerLine = 0X08,
    showTracerTop = 0X10,
    showTracerBottom = 0X20,
    showTracerAll =
        showTracerPoint | showTracerLabel | showTracerArrow | showTracerLine
  };
  explicit TtQCustomPlotTracer(QCustomPlot* _plot);
  ~TtQCustomPlotTracer();
  void setLabelFont(QFont font);
  void setPen(const QPen& pen);
  void setBrush(const QBrush& brush);
  void setText(const QString& text);
  void setLabelPen(const QPen& pen);
  void updatePosition(TracerShowType type, double xValue, double yValue);
  void setVisible(bool visible);

 protected:
  QCustomPlot* plot;                  // 传入实例化的QcustomPlot
  QPointer<TtTracer> tracerPoint;     // 跟踪的点
  QPointer<QCPItemText> tracerLabel;  // 显示的数值
  QPointer<QCPItemLine> tracerArrow;  // 箭头
  QPointer<QCPItemLine> tracerLine;   // 线段
  TracerShowType currentType;
  bool visible;
 signals:
 public slots:
};

}  // namespace Ui

#endif  // UI_CONTROLS_TTTRACER_H
