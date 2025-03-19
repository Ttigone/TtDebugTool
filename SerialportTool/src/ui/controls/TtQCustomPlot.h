#ifndef UI_CONTROLS_TTQCUSTOMPLOT_H
#define UI_CONTROLS_TTQCUSTOMPLOT_H

#include <qcustomplot/qcustomplot.h>

#include "ui/controls/TtTracer.h"

namespace Ui {

class TtQCustomPlot : public QCustomPlot {
  Q_OBJECT
  enum RefreshActionEnum {
    eRefreshNone,
    eRefreshData,
    eRefreshMouseWhell,
    eRefreshMouseMove,
    eRefreshMousePress,
    eRefreshButtonPress,
    eRefreshSaveWaveData,
    eRefreshReadWaveData
  };

 public:
  explicit TtQCustomPlot(QWidget* parent = nullptr, bool enableGPU = true);
  ~TtQCustomPlot();

  QPointer<TtQCustomPlotTracer> plotTracer = nullptr;  //坐标跟随鼠标.使用时创建

  int GraphShowTimerSet = 20;  //波形刷新

  void startRefreshTimer();
  void stopRefreshTimer();
  void refreshGraphs();
  bool refresh(RefreshActionEnum action);

 public slots:
  void selectionChanged();  // 选择已更改
  void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);

  void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);

  void selectedGraphsColorSet();
  void moveLegend();
  void addRandomGraph(QColor color);
  void removeSelectedGraph();
  void removeAllGraphs();
  void hideSelectedGraph();
  void hideAllGraph();
  void showAllGraph();
  bool isAllGraphHide();
  bool ishaveGraphHide();
  void contextMenuRequest(QPoint pos);
  void addGraphs();
  void addGraphsData(const QVector<double>& keys,
                     const QVector<double>& values);
  void GraphRefreshSlot();
  void readWaveformData();
  void saveWaveformData();
  void saveWaveformImage();

 protected:
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  //    void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

 signals:

 private:
  void setGraphColor();

  void refreshPlotTracer();

  QDateTime startTime;
  QDateTime endTime;
  QMutex mutex;

  QVector<QColor> graphColor;
  QVector<double> lastKeys;
  double pos_x = 0;
  double pos_y = 0;
  QTimer refreshTimer;

  double GraphsxAxisLength = 0;
  double GraphsxAxisEnd = 0;

  int GraphsDataMaxLength = 10240;

  int refreshGraphsCount = 0;
  bool isGraphsyAxisAuto = false;
  bool isGraphsxAxisAuto = false;
  bool isRefreshTimerEnable = false;  //是否启动定时器
  bool isRefreshGraphs = false;       //是否已经有波形数据

  bool isRefreshreplot = false;
  RefreshActionEnum refreshAction = eRefreshNone;

  // bool RefreshGraphs = false;//是否更新了波形数据
  // bool RefreshMouseWhell = false;
  // bool RefreshMousePress = false;
  // bool RefreshMouseMove = false;
  bool keyPress_X = false;
  bool keyPress_V = false;
};
}  // namespace Ui

#endif
