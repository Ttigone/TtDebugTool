#ifndef TTMODBUSPLOT_H
#define TTMODBUSPLOT_H

#include "qcustomplot/qcustomplot.h"

// y 方向的图标
class AxisTag : public QObject {
  Q_OBJECT
 public:
  explicit AxisTag(QCPAxis* parentAxis);
  virtual ~AxisTag();

  // setters:
  void setPen(const QPen& pen);
  void setBrush(const QBrush& brush);
  void setText(const QString& text);

  // getters:
  QPen pen() const { return mLabel->pen(); }
  QBrush brush() const { return mLabel->brush(); }
  QString text() const { return mLabel->text(); }

  // other methods:
  void updatePosition(double value);

 protected:
  QCPAxis* mAxis;
  QPointer<QCPItemTracer> mDummyTracer;
  QPointer<QCPItemLine> mArrow;
  QPointer<QCPItemText> mLabel;
};

class ModbusPlot : public QCustomPlot {
  Q_OBJECT

 public:
  explicit ModbusPlot(QWidget* parent = nullptr);
  ~ModbusPlot();

  // 添加新的数据点
  void addData(double value);

  // 清除所有数据
  void clearData();

  // 设置是否显示坐标提示
  void setShowTooltip(bool show) { m_showTooltip = show; }

  // 设置是否自动调整Y轴范围
  void setAutoScaleY(bool autoScale) { m_autoScaleY = autoScale; }

 private:
  bool m_firstDataReceived = false;
  QCPGraph* m_dataGraph;        // 数据线图
  QVector<double> m_timeData;   // X轴数据 (时间)
  QVector<double> m_valueData;  // Y轴数据 (值)

  // 鼠标悬停显示组件
  QCPItemTracer* m_tracer;     // 数据点跟踪器
  QCPItemText* m_tracerLabel;  // 显示坐标的标签

  bool m_showTooltip = true;  // 是否显示坐标提示
  bool m_autoScaleY = true;   // 是否自动调整Y轴范围
  int m_maxPoints = 100;      // 最大保留点数
  double m_startTime = 0.0;   // 记录第一个数据点的时间

  AxisTag* tag_;                 // 右侧标签
  QCPItemStraightLine* m_vLine;  // 垂直指示线
  QCPItemText* m_coordLabel;     // 坐标标签

  void setupPlot();                               // 初始化图表
  void updateTracerPosition(QMouseEvent* event);  // 更新跟踪器位置

 protected:
  // 鼠标移动事件
  void mouseMoveEvent(QMouseEvent* event) override;
};

#endif  // TTMODBUSPLOT_H
