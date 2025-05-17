#ifndef UI_CONTROLS_TTPLOTWIDGET_H
#define UI_CONTROLS_TTPLOTWIDGET_H

#include <QWidget>

class QCustomPlot;

namespace Ui {

class TtPlotWidget : public QWidget {
  Q_OBJECT
public:
  explicit TtPlotWidget(QCustomPlot *plot, QWidget *parent = nullptr);
  explicit TtPlotWidget(QWidget *parent = nullptr);
  ~TtPlotWidget();

  // 接收一个 plot
  void setupPlot(QCustomPlot *plot);

signals:
  void updatePlot();

private:
  QCustomPlot *plot_;
};

} // namespace Ui

#endif // UI_CONTROLS_TTPLOTWIDGET_H
