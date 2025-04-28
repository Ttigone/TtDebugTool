#ifndef UI_CONTROLS_TTMODBUSPLOT_H
#define UI_CONTROLS_TTMODBUSPLOT_H

#include "TtQCPItemRichText.h"

#include "Def.h"

class QCustomPlot;
class QCPAxis;
class QCPItemTracer;
class QCPItemLine;
class QCPItemText;
class QCPItemTracer;

namespace Ui {

class TtAxisTag;

class TtModbusPlot : public QCustomPlot {
  Q_OBJECT

 public:
  explicit TtModbusPlot(QWidget* parent = nullptr);
  ~TtModbusPlot();

  // 添加新的数据点
  void addData(TtModbusRegisterType::Type type, const int& addr, double value);

  void setShowTooltip(bool show) { m_showTooltip = show; }
  void setAutoScaleY(bool autoScale) { m_autoScaleY = autoScale; }
  void clearData();

 public slots:
  void addGraphs(TtModbusRegisterType::Type type, const int& addr);
  void removeGraphs(TtModbusRegisterType::Type type, const int& addr);
  void setGraphsPointCapacity(quint16 nums);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  void setupPlot();                               // 初始化图表
  void updateTracerPosition(QMouseEvent* event);  // 更新跟踪器位置

  bool m_firstDataReceived = false;

  QCPItemTracer* m_tracer;     // 数据点跟踪器
  QCPItemText* m_tracerLabel;  // 显示坐标的标签

  bool m_showTooltip = true;  // 是否显示坐标提示
  bool m_autoScaleY = true;   // 是否自动调整Y轴范围
  int m_maxPoints = 100;      // 最大保留点数
  double m_startTime = 0.0;   // 记录第一个数据点的时间

  QCPItemStraightLine* m_vLine;  // 垂直指示线
  // QCPItemText* m_coordLabel;     // 坐标标签
  TtQCPItemRichText* m_coordLabel;  // 坐标标签

  quint16 points_nums_ = 100;

  struct CurveData {
    QVector<double> timeData;
    QVector<double> valueData;
    QPointer<QCPGraph> graph;
    QPointer<TtAxisTag> tag;
  };
  QMap<QPair<int, int>, CurveData> m_curves;  // Key: (Type, Addr)
  QList<QColor> m_colorPalette;               // 颜色轮转列表
};

class SerialPlot : public QCustomPlot {
  Q_OBJECT

 public:
  explicit SerialPlot(QWidget* parent = nullptr);
  ~SerialPlot();

  // 添加新的数据点
  void addData(int channel, double value);

  void setShowTooltip(bool show) { m_showTooltip = show; }
  void setAutoScaleY(bool autoScale) { m_autoScaleY = autoScale; }
  void clearData();

  void saveWaveFormData();

 public slots:
  void addGraphs(int channel, const QColor& color = Qt::red);
  void removeGraphs(int channel);
  void setGraphsPointCapacity(quint16 nums);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  void setupPlot();                               // 初始化图表
  void updateTracerPosition(QMouseEvent* event);  // 更新跟踪器位置

  bool m_firstDataReceived = false;

  QCPItemTracer* m_tracer;     // 数据点跟踪器
  QCPItemText* m_tracerLabel;  // 显示坐标的标签

  bool m_showTooltip = true;  // 是否显示坐标提示
  bool m_autoScaleY = true;   // 是否自动调整Y轴范围
  int m_maxPoints = 100;      // 最大保留点数
  double m_startTime = 0.0;   // 记录第一个数据点的时间

  QCPItemStraightLine* m_vLine;  // 垂直指示线
  // QCPItemText* m_coordLabel;     // 坐标标签
  TtQCPItemRichText* m_coordLabel;  // 坐标标签

  quint16 points_nums_ = 100;

  struct CurveData {
    QVector<double> timeData;
    QVector<double> valueData;
    QPointer<QCPGraph> graph;
    QPointer<TtAxisTag> tag;
  };

  QMap<int, CurveData> curves_;               // Key: Channel
  QList<QColor> m_colorPalette;               // 颜色轮转列表

  QList<QCPItemText*> m_hoverLabels;
};

}  // namespace Ui

#endif  // UI_CONTROLS_TTMODBUSPLOT_H
