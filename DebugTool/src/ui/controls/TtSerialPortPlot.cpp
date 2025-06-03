#include "ui/controls/TtSerialPortPlot.h"

#include <qelapsedtimer.h>
#include <ui/widgets/message_bar.h>
#include "qcustomplot/qcustomplot.h"
#include "ui/controls/TtPlotItem.h"

namespace Ui {

TtSerialPortPlot::TtSerialPortPlot(QWidget* parent) : QCustomPlot(parent) {
  setupPlot();
  last_replot_time_.start();

  bool hasOpenGL = false;
#ifdef QT_OPENGL_SUPPORT
  hasOpenGL = QOpenGLContext::currentContext() != nullptr;
#endif
  if (hasOpenGL) {
    // 默认是不开启 opengl 的
    // 如果开启了 opengl, 在分开窗口实现时, 不同窗口之间的 serial_plot_
    // 会造成数据显示混乱
    this->setOpenGl(true);
  }

  // this->setOpenGl(true);

  // setAntialiasedElements(QCP::aeAll);

  // 仅对线条使用抗锯齿，减少对其他元素的影响
  // setAntialiasedElements(QCP::aePlottables | QCP::aeAxes | QCP::aeScatters |
  //                        QCP::aeGrid);
  // 对于性能敏感的设备，可以完全禁用
  // setNotAntialiasedElements(QCP::aeAll);
  // 仅对线条使用抗锯齿，减少对其他元素的影响

  // 启用缩放交互
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  setNoAntialiasingOnDrag(true);

  setAntialiasedElements(QCP::aePlottables);
  // 减少抗锯齿元素
  setNotAntialiasedElements(QCP::aeAxes | QCP::aeGrid);
}

TtSerialPortPlot::~TtSerialPortPlot() {
  for (auto& curve : curves_) {
    if (curve.tag) {
      delete curve.tag;
      curve.tag = nullptr;
    }
  }
  curves_.clear();
}

void TtSerialPortPlot::addData(int channel, double value) {
  if (!curves_.contains(channel)) {
    return;
  }
  double currentTime = 0.0;

  if (m_useFixedTimeDelta) {
    if (!m_firstDataReceived) {
      m_startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
      m_firstDataReceived = true;
      m_lastTimestamp = 0.0;
    } else {
      m_lastTimestamp += m_fixedTimeDelta;
    }
    currentTime = m_lastTimestamp;
  } else {
    // 使用实际时间
    currentTime =
        m_firstDataReceived
            ? QDateTime::currentMSecsSinceEpoch() / 1000.0 - m_startTime
            : 0.0;

    if (!m_firstDataReceived) {
      m_startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
      m_firstDataReceived = true;
    }
  }
  auto& curve = curves_[channel];
  if (curve.timeData.size() > m_maxPoints) {
    // // 而不是移除一个点，每次删除一批点以提高性能
    int removeCount =
        qMin(points_nums_ / 10, curve.timeData.size() - points_nums_);
    if (removeCount > 0) {
      curve.timeData.remove(0, removeCount);
      curve.valueData.remove(0, removeCount);
    }
  }
  // 添加点信息
  curve.timeData.append(currentTime);
  curve.valueData.append(value);
  // 更新曲线
  if (curve.graph) {
    // curve.graph->setData(curve.timeData, curve.valueData);
    // curve.graph->setData(curve.timeData, curve.valueData);
    curve.graph->addData(curve.timeData, curve.valueData);
  }
  // 更新标签显示最新值
  if (curve.tag) {
    curve.tag->updatePosition(value);
    curve.tag->setText(QString::number(value, 'f', 2));
  }

  // 自动缩放Y轴
  if (m_autoScaleY) {
    // 仅在必要时进行Y轴缩放以提高性能
    static QElapsedTimer timer;
    if (!timer.isValid() || timer.elapsed() > 200) {
      // 限制缩放频率
      double minY = std::numeric_limits<double>::max();
      double maxY = std::numeric_limits<double>::lowest();

      for (const auto& c : qAsConst(curves_)) {
        if (c.valueData.isEmpty())
          continue;

        auto [min, max] =
            std::minmax_element(c.valueData.begin(), c.valueData.end());
        minY = qMin(minY, *min);
        maxY = qMax(maxY, *max);
      }

      // 添加一些边距
      double margin = (maxY - minY) * 0.1;
      if (margin < 0.1) {
        // 最小边距
        margin = 0.1;
      }

      yAxis->setRange(minY - margin, maxY + margin);
      timer.restart();
    }
  }

  // 更新X轴范围（只在非拖拽模式下）
  if (!m_dragEnabled) {
    double rangeMin =
        *std::min_element(curve.timeData.begin(), curve.timeData.end());
    double rangeMax =
        *std::max_element(curve.timeData.begin(), curve.timeData.end());
    // 添加5%的边距
    double margin = (rangeMax - rangeMin) * 0.05;
    xAxis->setRange(rangeMin - margin, rangeMax + margin);
  }

  // 性能优化：限制重绘频率
  if (last_replot_time_.isValid() && last_replot_time_.elapsed() < 50) {
    return;
  }
  replot(QCustomPlot::rpQueuedReplot);  // 使用队列模式重绘
  // 尝试使用直接重绘而非队列模式
  // replot(QCustomPlot::rpImmediateRefresh);
  last_replot_time_.restart();
}

void TtSerialPortPlot::clearData() {
  m_startTime = 0.0;
  for (auto& curve : curves_) {
    curve.timeData.clear();
    curve.valueData.clear();
  }
  this->replot();
}

bool TtSerialPortPlot::saveWaveFormData(const QString& filePath) {
  if (curves_.isEmpty()) {
    qDebug() << "没有图像";
    return false;
  }
  // 以及存在文件了, 就会直接存储
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream out(&file);

  // 写入CSV头
  out << "Time";
  for (auto it = curves_.begin(); it != curves_.end(); ++it) {
    out << ",Channel_" << it.key();
  }
  out << "\n";

  // 找出所有时间点
  QSet<double> allTimePoints;
  for (const auto& curve : qAsConst(curves_)) {
    for (const double& t : qAsConst(curve.timeData)) {
      allTimePoints.insert(t);
    }
  }

  // 转换为有序列表
  QList<double> timePoints = allTimePoints.values();
  std::sort(timePoints.begin(), timePoints.end());

  // 对每个时间点写入所有通道的值
  for (const double& t : qAsConst(timePoints)) {
    out << QString::number(t, 'f', 6);

    for (auto it = curves_.begin(); it != curves_.end(); ++it) {
      // 找到最接近该时间点的数据点
      int idx = -1;
      double minDiff = std::numeric_limits<double>::max();

      for (int i = 0; i < it->timeData.size(); ++i) {
        double diff = qAbs(it->timeData[i] - t);
        if (diff < minDiff) {
          minDiff = diff;
          idx = i;
        }
      }

      // 写入该通道在这个时间点的值
      if (idx >= 0 && minDiff < 0.001) {  // 允许小误差
        out << "," << QString::number(it->valueData[idx], 'f', 6);
      } else {
        out << ",";  // 该通道在此时间点无值
      }
    }

    out << "\n";
  }

  file.close();
  return true;
}

void TtSerialPortPlot::setDragEnabled(bool enabled) {
  if (m_dragEnabled != enabled) {
    m_dragEnabled = enabled;
    this->setInteraction(QCP::iRangeDrag, enabled);
    emit dragEnabledChanged(enabled);
  }
}

bool TtSerialPortPlot::isDragEnabled() const {
  return m_dragEnabled;
}

void TtSerialPortPlot::setZoomEnabled(bool enabled) {
  if (m_zoomEnabled != enabled) {
    m_zoomEnabled = enabled;
    this->setInteraction(QCP::iRangeZoom, enabled);
    emit zoomEnabledChanged(enabled);
  }
}

bool TtSerialPortPlot::isZoomEnabled() const {
  return m_zoomEnabled;
}

void TtSerialPortPlot::setTimeDeltaMode(bool enabled) {
  m_useFixedTimeDelta = enabled;
  if (enabled) {
    m_lastTimestamp = 0.0;  // 重置时间戳
  }
}

bool TtSerialPortPlot::isTimeDeltaMode() const {
  return m_useFixedTimeDelta;
}

void TtSerialPortPlot::setFixedTimeDelta(int delta) {
  if (delta > 0) {
    m_fixedTimeDelta = delta;
  }
}

double TtSerialPortPlot::fixedTimeDelta() const {
  return m_fixedTimeDelta;
}

quint16 TtSerialPortPlot::graphsPointCapacity() const {
  return points_nums_;
}

void TtSerialPortPlot::addGraphs(int channel, const QColor& color) {
  if (!curves_.contains(channel)) {
    // 新曲线
    CurveData newCurve;
    // 创建图形
    newCurve.graph = this->addGraph();
    // 设置线条的宽度样式
    // newCurve.graph->setPen(QPen(color, 3));
    newCurve.graph->setPen(QPen(color, 1.5));
    // 线条平滑样式
    newCurve.graph->setLineStyle(QCPGraph::lsLine);
    // 图表名字
    newCurve.graph->setName(QString("channel@%1").arg(channel));

    // 动态改变的
    if (points_nums_ <= 50) {
      newCurve.graph->setScatterStyle(QCPScatterStyle::ssCircle);
    } else if (points_nums_ <= 200) {
      newCurve.graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDot));
    } else {
      // 数据量大时只显示线条，不显示散点
      newCurve.graph->setScatterStyle(QCPScatterStyle::ssNone);
    }

    if (points_nums_ > 50) {
      // 数据点较多时使用样条曲线平滑显示
      newCurve.graph->setAdaptiveSampling(true);  // 启用自适应采样

      // 可以考虑使用样条曲线，但需要QCustomPlot支持或自行实现
      // 这需要修改QCustomPlot源码或使用其他方法实现
      // newCurve.graph->setCurveStyle(QCPGraph::csCubicSpline);
    }

    // BUG 由外部决定是否采用数据点样式
    // 数据点样式
    // newCurve.graph->setScatterStyle(QCPScatterStyle::ssCircle);
    // newCurve.graph->setScatterStyle(QCPScatterStyle::ssDot);
    // newCurve.graph->setLineStyle()

    // 创建右侧标签
    QCPAxis* valueAxis = this->yAxis;  // 共享Y轴或为每个曲线创建右轴
    newCurve.tag = new TtAxisTag(valueAxis);
    newCurve.tag->setPen(newCurve.graph->pen());

    // 图形没有数据
    // m_tracer->setGraph(newCurve.graph);

    curves_.insert(channel, newCurve);
  }
}

void TtSerialPortPlot::removeGraphs(int channel) {
  // qDebug() << "first delete channel" << channel;
  // qDebug() << "curves Keys: " << curves_.keys();
  if (curves_.contains(channel)) {
    // qDebug() << "delete channel" << channel;
    auto& curve = curves_[channel];

    // 当前关联的图形
    if (m_tracer->graph() == curve.graph) {
      m_tracer->setGraph(nullptr);
    }

    // 移除图形
    if (curve.graph) {
      qDebug() << "graph";
      this->removeGraph(curve.graph);
      curve.graph.clear();
    }

    // 删除标签
    if (curve.tag) {
      qDebug() << "tag";
      delete curve.tag;
      curve.tag = nullptr;
    }
    curves_.remove(channel);
  }

  if (curves_.isEmpty()) {
    qDebug() << "curves Empty";
    m_tracer->setVisible(false);
    m_coordLabel->setVisible(false);
  }
  replot();
}

void TtSerialPortPlot::setGraphsPointCapacity(quint16 nums) {
  if (nums > 0 && nums != points_nums_) {
    points_nums_ = nums;
    for (auto& curve : curves_) {
      while (curve.timeData.size() > points_nums_) {
        curve.timeData.removeFirst();
        curve.valueData.removeFirst();
        if (!curve.graph->data()->isEmpty()) {
          auto dataMap = curve.graph->data();
          dataMap->remove(dataMap->constBegin()->key);  // 使用迭代器安全删除
        }
      }
    }
    replot();
  }
}

void TtSerialPortPlot::autoScale() {
  if (curves_.isEmpty()) {
    return;
  }

  double minX = std::numeric_limits<double>::max();
  double maxX = std::numeric_limits<double>::lowest();
  double minY = std::numeric_limits<double>::max();
  double maxY = std::numeric_limits<double>::lowest();

  // 找出所有曲线的最大和最小值
  for (const auto& curve : qAsConst(curves_)) {
    if (curve.timeData.isEmpty() || curve.valueData.isEmpty())
      continue;

    auto minMaxX =
        std::minmax_element(curve.timeData.begin(), curve.timeData.end());
    auto minMaxY =
        std::minmax_element(curve.valueData.begin(), curve.valueData.end());

    minX = qMin(minX, *minMaxX.first);
    maxX = qMax(maxX, *minMaxX.second);
    minY = qMin(minY, *minMaxY.first);
    maxY = qMax(maxY, *minMaxY.second);
  }

  // 添加边距
  double xMargin = (maxX - minX) * 0.05;
  double yMargin = (maxY - minY) * 0.1;

  // 避免数据点太少或完全相同时的除零问题
  if (xMargin < 0.1)
    xMargin = 0.1;
  if (yMargin < 0.1)
    yMargin = 0.1;

  // 设置轴范围
  xAxis->setRange(minX - xMargin, maxX + xMargin);
  yAxis->setRange(minY - yMargin, maxY + yMargin);

  // 重绘
  replot();
}

void TtSerialPortPlot::mouseMoveEvent(QMouseEvent* event) {
  QCustomPlot::mouseMoveEvent(event);

  // 节流鼠标移动事件处理
  if (!m_tracerUpdateTimer.isValid() ||
      m_tracerUpdateTimer.elapsed() > TRACER_UPDATE_INTERVAL) {
    // 显示/隐藏组件
    const bool show = this->rect().contains(event->pos());
    m_vLine->setVisible(show);
    m_coordLabel->setVisible(show);

    // if (show) {
    //   // 获取鼠标坐标
    //   double x = this->xAxis->pixelToCoord(event->pos().x());
    //   double y = this->yAxis->pixelToCoord(event->pos().y());

    //   // 更新竖线位置
    //   m_vLine->point1->setCoords(x, this->yAxis->range().lower);
    //   m_vLine->point2->setCoords(x, this->yAxis->range().upper);

    //   // 更新标签内容和位置
    //   updateCoordLabelPosition(event, x, y);
    // }

    updateTracerPosition(event);
    m_tracerUpdateTimer.restart();
  }

  // const bool show = viewport().contains(event->pos());
  // // // 水平线
  // m_vLine->setVisible(show);
  // m_tracer->setVisible(show);
  // m_coordLabel->setVisible(show);

  // if (!show) {
  //   replot(QCustomPlot::rpQueuedReplot);
  //   return;
  // }

  // double x = this->xAxis->pixelToCoord(event->pos().x());
  // // qDebug() << x;

  // // 同一个 x 坐标, y 值分别是最大和最小, 竖直线
  // m_vLine->point1->setCoords(x, this->yAxis->range().lower);
  // m_vLine->point2->setCoords(x, this->yAxis->range().upper);

  // // qDebug() << "test";

  // updateTracerPosition(event);

  // replot(QCustomPlot::rpQueuedReplot);
}

void TtSerialPortPlot::resizeEvent(QResizeEvent* event) {
  QCustomPlot::resizeEvent(event);

  // 关闭不必要的抗锯齿
  // setNotAntialiasedElements(QCP::aeAll);
}

void TtSerialPortPlot::wheelEvent(QWheelEvent* event) {
  if (event->modifiers() & Qt::ControlModifier) {
    // 获取鼠标在图表中的实际坐标
    double x = xAxis->pixelToCoord(event->position().x());
    double y = yAxis->pixelToCoord(event->position().y());

    // 缩放比例 - 向上滚动放大，向下滚动缩小
    double factor = (event->angleDelta().y() > 0) ? 0.8 : 1.2;

    // 以鼠标位置为中心进行缩放
    xAxis->scaleRange(factor, x);
    yAxis->scaleRange(factor, y);

    // 重绘图表
    replot(QCustomPlot::rpQueuedReplot);

    event->accept();
  } else {
    // 其他情况使用默认处理
    QCustomPlot::wheelEvent(event);
  }
}

void TtSerialPortPlot::setupPlot() {
  QCPAxis* rightAxis = axisRect()->addAxis(QCPAxis::atRight);
  // 设置右边距
  // rightAxis->setPadding(60);
  rightAxis->setPadding(0);
  rightAxis->setVisible(true);
  rightAxis->setTicks(false);        // 隐藏刻度线
  rightAxis->setTickLabels(false);   // 隐藏刻度标签
  rightAxis->setBasePen(Qt::NoPen);  // 隐藏轴线

  this->setInteraction(QCP::iRangeDrag, false);
  this->setInteraction(QCP::iRangeZoom, false);

  // 设置标签格式
  this->xAxis->setNumberFormat("f");   // 使用固定小数点格式
  this->xAxis->setNumberPrecision(0);  // 无小数位

#if 0
  this->xAxis->setVisible(false);          // 完全隐藏坐标轴
  this->xAxis->setTicks(false);            // 隐藏刻度线
  this->xAxis->setTickLabels(false);       // 隐藏刻度标签
  this->xAxis->grid()->setVisible(false);  // 隐藏网格线（如果存在）
  this->xAxis->setLabel("");               // 清空轴标签
#else
  // 保留空间但隐藏可视元素
  // this->xAxis->setBasePen(Qt::NoPen);     // 隐藏轴线
  // this->xAxis->setTicks(false);       // 隐藏刻度线
  // this->xAxis->setTickLabels(false);  // 隐藏刻度标签
  // this->xAxis->setLabel("");          // 清空轴标签文本
#endif
  this->yAxis->setTickLabels(true);  // 显示刻度标签

  // this->yAxis->setLabel("");

  // 创建跟踪器（用于鼠标悬停时显示坐标）
  m_tracer = new QCPItemTracer(this);
  // m_tracer->setGraph(m_dataGraph);
  m_tracer->setInterpolating(true);
  m_tracer->setStyle(QCPItemTracer::TracerStyle::tsPlus);
  m_tracer->setPen(QPen(Qt::red));
  m_tracer->setBrush(QBrush(Qt::red));
  m_tracer->setSize(7);
  m_tracer->setVisible(false);

  // // 创建跟踪器标签（显示坐标值）
  // m_tracerLabel = new QCPItemText(this);
  // m_tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
  // m_tracerLabel->position->setType(QCPItemPosition::ptAbsolute);
  // // m_tracerLabel->position->setType(QCPItemPosition::ptPlotCoords);
  // m_tracerLabel->setPadding(QMargins(5, 5, 5, 5));
  // m_tracerLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
  // m_tracerLabel->setPen(QPen(Qt::black));
  // m_tracerLabel->setVisible(false);

  // 创建垂直指示线
  m_vLine = new QCPItemStraightLine(this);
  m_vLine->setPen(QPen(Qt::gray, 1, Qt::DashLine));
  m_vLine->setVisible(false);

  // 创建坐标标签
  // m_coordLabel = new QCPItemText(this);
  m_coordLabel = new TtQCPItemRichText(this);
  m_coordLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);
  m_coordLabel->position->setType(QCPItemPosition::ptViewportRatio);
  m_coordLabel->position->setCoords(1.0, 0.0);  // 右上角
  m_coordLabel->setTextAlignment(Qt::AlignRight);
  m_coordLabel->setBrush(QBrush(Qt::white));
  m_coordLabel->setPadding(QMargins(5, 5, 5, 5));
  m_coordLabel->setVisible(false);

  // 启用鼠标跟踪
  this->setMouseTracking(true);

  // axisRect()->setAutoMargins(QCP::msNone);
  axisRect()->setAutoMargins(QCP::msLeft | QCP::msBottom);
  // axisRect()->setMargins(QMargins(0, 0, 0, 0));
  axisRect()->setMargins(QMargins(5, 5, 5, 5));
  xAxis->setPadding(0);
  yAxis->setPadding(0);
}

// void TtSerialPortPlot::updateTracerPosition(QMouseEvent* event) {
//   if (curves_.isEmpty()) {
//     // 没有图像, 不显示
//     // qDebug() << "empty";
//     m_tracer->setVisible(false);
//     m_vLine->setVisible(false);
//     m_coordLabel->setVisible(false);
//     return;
//   }

//   // // 当前鼠标坐标值
//   double mouseX = this->xAxis->pixelToCoord(event->pos().x());
//   double mouseY = this->xAxis->pixelToCoord(event->pos().y());

//   // 鼠标移出 x 轴外
//   if (mouseX < this->xAxis->range().lower ||
//       mouseX > this->xAxis->range().upper) {
//     // qDebug() << "move outside";
//     m_tracer->setVisible(false);
//     // m_tracerLabel->setVisible(false);
//     m_coordLabel->setVisible(false);
//     return;
//   }

//   QDateTime time =
//       QDateTime::fromMSecsSinceEpoch((m_startTime + mouseX) * 1000);
//   QStringList tooltipLines;
//   tooltipLines.append(
//       QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));
//   // 默认 false
//   bool tracerSet = false;

//   bool hasVailData = false;

//   // qDebug() << "update: " << curves_.size();
//   // 遍历每一个图像
//   for (auto it = curves_.begin(); it != curves_.end(); ++it) {
//     //
//     const auto& curve = it.value();
//     if (!curve.graph || curve.timeData.isEmpty()) {
//       // 没有数据
//       continue;
//     }
//     // qDebug() << "insit";
//     const auto& times = curve.timeData;
//     const auto& values = curve.valueData;

//     // 添加数据一致性检查
//     if (times.size() != values.size()) {
//       qWarning() << "Data inconsistency in curve" << curve.graph->name();
//       continue;
//     }

//     auto it_lower = std::lower_bound(times.begin(), times.end(), mouseX);
//     // 索引
//     int idx = it_lower - times.begin();

//     if (idx > 0) {
//       if (idx == times.size() ||
//           mouseX - times[idx - 1] < times[idx] - mouseX) {
//         --idx;
//       }
//     }

//     if (idx >= 0 && idx < times.size() && idx < values.size()) {
//       // 设置跟踪器到当前的曲线和位置
//       // if (!tracerSet) {
//       //   m_tracer->setGraph(curve.graph);
//       //   m_tracer->setGraphKey(times[idx]);
//       //   m_tracer->setVisible(true);
//       //   tracerSet = true;
//       // }
//       QColor color = curve.graph->pen().color();
//       QString name = curve.graph->name();
//       double value = values[idx];
//       tooltipLines.append(QString("<span style='color:%1;'>%2: %3</span>")
//                               .arg(color.name())
//                               .arg(name)
//                               .arg(value, 0, 'f', 2));
//       hasVailData = true;
//     }
//   }

//   // if (tooltipLines.size() > 1) {
//   if (hasVailData) {
//     // 设置显示的富文本
//     m_coordLabel->setText(tooltipLines.join("<br>"));

//     // TtQCPItemRichText* richLabel =
//     //     qobject_cast<TtQCPItemRichText*>(m_coordLabel);
//     // const QTextDocument& doc = richLabel->document();
//     const QTextDocument& doc = m_coordLabel->document();
//     qreal textWidth = doc.idealWidth() + 10;  // 增加10px边距
//     qreal textHeight = doc.size().height() + 10;

//     // 获取鼠标位置和视口边界
//     QPoint mousePos = event->pos();
//     QRect viewportRect = viewport();

//     // 初始位置：鼠标右下方15像素
//     QPoint pos = mousePos + QPoint(15, 5);

//     // 水平边界检测
//     if (pos.x() + textWidth > viewportRect.right()) {
//       // 切换到左侧显示
//       pos.setX(mousePos.x() - textWidth - 15);
//     }

//     // 垂直边界检测
//     if (pos.y() + textHeight > viewportRect.bottom()) {
//       // 切换到上方显示
//       pos.setY(mousePos.y() - textHeight - 15);
//     }

//     // 应用最终位置
//     m_coordLabel->position->setPixelPosition(pos);
//     m_coordLabel->setVisible(true);
//   } else {
//     m_coordLabel->setVisible(false);
//   }

//   // if (!tracerSet) {
//   //   // 为什么进来?
//   //   qDebug() << "none";
//   //   m_vLine->setVisible(false);
//   //   m_tracer->setVisible(false);  // 没有找到点时隐藏
//   //   // m_coordLabel->setVisible(false);
//   // }

//   replot();
// }

// 优化查找最近点的算法
void TtSerialPortPlot::updateTracerPosition(QMouseEvent* event) {
  if (curves_.isEmpty())
    return;

  // 当前鼠标X坐标
  double mouseX = this->xAxis->pixelToCoord(event->pos().x());

  // 优化：使用二分查找代替线性搜索
  auto findNearestPoint = [mouseX](const QVector<double>& timeData) -> int {
    if (timeData.isEmpty())
      return -1;

    // 二分查找
    int left = 0, right = timeData.size() - 1;
    while (left <= right) {
      int mid = left + (right - left) / 2;
      if (timeData[mid] < mouseX) {
        left = mid + 1;
      } else {
        right = mid - 1;
      }
    }

    // 找到后比较左右两点哪个更近
    int idx = qBound(0, left, timeData.size() - 1);
    if (idx > 0 && idx < timeData.size()) {
      double distLeft = mouseX - timeData[idx - 1];
      double distRight = timeData[idx] - mouseX;
      if (distLeft < distRight)
        idx--;
    }

    return idx;
  };

  // 查找所有曲线中最近的点
  double minDistance = std::numeric_limits<double>::max();
  QCPGraph* activeGraph = nullptr;
  QCPGraphData activeData;

  for (auto it = curves_.begin(); it != curves_.end(); ++it) {
    auto& curve = it.value();
    if (!curve.graph || curve.timeData.isEmpty())
      continue;

    int idx = findNearestPoint(curve.timeData);
    if (idx >= 0 && idx < curve.timeData.size()) {
      double distance = qAbs(mouseX - curve.timeData[idx]);
      if (distance < minDistance) {
        minDistance = distance;
        activeGraph = curve.graph;
        activeData = QCPGraphData(curve.timeData[idx], curve.valueData[idx]);
      }
    }
  }

  // 更新跟踪器
  if (activeGraph &&
      minDistance < (xAxis->range().upper - xAxis->range().lower) * 0.05) {
    m_tracer->setGraph(activeGraph);
    m_tracer->setGraphKey(activeData.key);
    m_tracer->setVisible(true);
  } else {
    m_tracer->setVisible(false);
  }
}

void TtSerialPortPlot::updateCoordLabelPosition(QMouseEvent* event, double x,
                                                double y) {
  // 当前没有曲线数据或超出范围时隐藏标签
  if (curves_.isEmpty() || x < this->xAxis->range().lower ||
      x > this->xAxis->range().upper) {
    m_coordLabel->setVisible(false);
    return;
  }

  // 格式化时间信息
  QDateTime time = QDateTime::fromMSecsSinceEpoch((m_startTime + x) * 1000);
  QStringList tooltipLines;
  tooltipLines.append(
      QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));

  // 添加各条曲线的数据点信息
  bool hasValidData = false;
  for (auto it = curves_.begin(); it != curves_.end(); ++it) {
    const auto& curve = it.value();
    if (!curve.graph || curve.timeData.isEmpty()) {
      continue;
    }

    // 查找最近的数据点
    int idx = -1;
    double minDist = std::numeric_limits<double>::max();

    for (int i = 0; i < curve.timeData.size(); ++i) {
      double dist = qAbs(curve.timeData[i] - x);
      if (dist < minDist) {
        minDist = dist;
        idx = i;
      }
    }

    // 如果找到了足够近的点
    if (idx >= 0 &&
        minDist < (xAxis->range().upper - xAxis->range().lower) * 0.05) {
      QColor color = curve.graph->pen().color();
      QString name = curve.graph->name();
      double value = curve.valueData[idx];
      tooltipLines.append(QString("<span style='color:%1;'>%2: %3</span>")
                              .arg(color.name())
                              .arg(name)
                              .arg(value, 0, 'f', 2));
      hasValidData = true;
    }
  }

  // 如果有有效数据，显示标签
  if (hasValidData) {
    m_coordLabel->setText(tooltipLines.join("<br>"));

    // 计算标签大小
    const QTextDocument& doc = m_coordLabel->document();
    qreal textWidth = doc.idealWidth() + 10;  // 增加10px边距
    qreal textHeight = doc.size().height() + 10;

    // 计算标签位置
    QPoint mousePos = event->pos();
    QRect viewportRect = viewport();

    // 初始位置：鼠标右下方
    QPoint pos = mousePos + QPoint(15, 5);

    // 避免超出视口右边界
    if (pos.x() + textWidth > viewportRect.right()) {
      pos.setX(mousePos.x() - textWidth - 15);
    }

    // 避免超出视口底部边界
    if (pos.y() + textHeight > viewportRect.bottom()) {
      pos.setY(mousePos.y() - textHeight - 15);
    }

    // 应用最终位置
    m_coordLabel->position->setPixelPosition(pos);
    m_coordLabel->setVisible(true);
  } else {
    m_coordLabel->setVisible(false);
  }
}

}  // namespace Ui
