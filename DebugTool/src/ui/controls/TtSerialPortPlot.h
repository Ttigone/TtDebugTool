#ifndef UI_CONTROLS_TTSERIALPORTPLOT_H
#define UI_CONTROLS_TTSERIALPORTPLOT_H

#include "TtQCPItemRichText.h"
#include <QObject>

class QCustomPlot;
class QCPAxis;
class QCPItemTracer;
class QCPItemLine;
class QCPItemText;
class QCPItemTracer;

namespace Ui {

class TtAxisTag;

class TtSerialPortPlot : public QCustomPlot {
  Q_OBJECT
public:
  explicit TtSerialPortPlot(QWidget *parent = nullptr);
  ~TtSerialPortPlot();

  // 添加新的数据点
  void addData(int channel, double value);

  void setShowTooltip(bool show) { m_showTooltip = show; }
  void setAutoScaleY(bool autoScale) { m_autoScaleY = autoScale; }
  void clearData();
  bool saveWaveFormData(const QString &filePath);

  // 新增方法
  void setDragEnabled(bool enabled);
  bool isDragEnabled() const;

  void setZoomEnabled(bool enabled);
  bool isZoomEnabled() const;

  void setTimeDeltaMode(bool enabled);
  bool isTimeDeltaMode() const;
  void setFixedTimeDelta(int delta);
  double fixedTimeDelta() const;

  quint16 graphsPointCapacity() const;

signals:
  // 新增信号
  void dragEnabledChanged(bool enabled);
  void zoomEnabledChanged(bool enabled);

public slots:
  void addGraphs(int channel, const QColor &color = Qt::red);
  void removeGraphs(int channel);
  void setGraphsPointCapacity(quint16 nums);

  // 新增槽
  void autoScale();

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void setupPlot();                              // 初始化图表
  void updateTracerPosition(QMouseEvent *event); // 更新跟踪器位置
                                                 // 添加这个函数声明
  void updateCoordLabelPosition(QMouseEvent *event, double x, double y);

  bool m_firstDataReceived = false;

  QCPItemTracer *m_tracer; // 数据点跟踪器
  // QCPItemText* m_tracerLabel;  // 显示坐标的标签

  bool m_showTooltip = true; // 是否显示坐标提示
  bool m_autoScaleY = true;  // 是否自动调整Y轴范围
  int m_maxPoints = 100;     // 最大保留点数
  double m_startTime = 0.0;  // 记录第一个数据点的时间

  QCPItemStraightLine *m_vLine; // 垂直指示线
  // QCPItemText* m_coordLabel;     // 坐标标签
  TtQCPItemRichText *m_coordLabel; // 坐标标签

  quint16 points_nums_ = 100;

  // 每个图像的数据都不同
  struct CurveData {
    QVector<double> timeData;
    QVector<double> valueData;
    QPointer<QCPGraph> graph;
    QPointer<TtAxisTag> tag;
  };

  QMap<int, CurveData> curves_; // Key: Channel
  QList<QColor> m_colorPalette; // 颜色轮转列表

  QList<QCPItemText *> m_hoverLabels;

  QElapsedTimer last_replot_time_;

  // 新增成员
  bool m_dragEnabled = false;
  bool m_zoomEnabled = false;
  bool m_useFixedTimeDelta = false;
  double m_fixedTimeDelta = 0.1; // 默认时间间隔0.1秒
  double m_lastTimestamp = 0.0;  // 上次时间戳
  QElapsedTimer m_tracerUpdateTimer;
  static constexpr int TRACER_UPDATE_INTERVAL = 50; // 毫秒
};

} // namespace Ui
#endif // UI_CONTROLS_TTSERIALPORTPLOT_H
