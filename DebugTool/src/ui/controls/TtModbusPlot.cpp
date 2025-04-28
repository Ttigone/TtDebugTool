#include "TtModbusPlot.h"

#include <ui/widgets/message_bar.h>
#include "qcustomplot/qcustomplot.h"
#include "ui/controls/TtPlotItem.h"

namespace Ui {

TtModbusPlot::TtModbusPlot(QWidget* parent) : QCustomPlot(parent) {
  setupPlot();
  this->setOpenGl(true);
  qDebug() << "opengle=" << this->openGl();

  setAntialiasedElements(QCP::aeAll);
}

TtModbusPlot::~TtModbusPlot() {}

void TtModbusPlot::setupPlot() {
  QCPAxis* rightAxis = axisRect()->addAxis(QCPAxis::atRight);
  rightAxis->setPadding(60);
  rightAxis->setVisible(true);
  rightAxis->setTicks(false);        // 隐藏刻度线
  rightAxis->setTickLabels(false);   // 隐藏刻度标签
  rightAxis->setBasePen(Qt::NoPen);  // 隐藏轴线

  this->setInteraction(QCP::iRangeDrag, false);
  this->setInteraction(QCP::iRangeZoom, false);

  // 配置横轴显示格式（禁用科学计数法）
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  fixedTicker->setTickStep(1.0);  // 主刻度间隔1秒
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);
  this->xAxis->setTicker(fixedTicker);
  this->xAxis->setNumberPrecision(0);  // 显示整数秒

#if 0
  this->xAxis->setVisible(false);          // 完全隐藏坐标轴
  this->xAxis->setTicks(false);            // 隐藏刻度线
  this->xAxis->setTickLabels(false);       // 隐藏刻度标签
  this->xAxis->grid()->setVisible(false);  // 隐藏网格线（如果存在）
  this->xAxis->setLabel("");               // 清空轴标签
#else
  // 保留空间但隐藏可视元素
  // this->xAxis->setBasePen(Qt::NoPen);     // 隐藏轴线
  this->xAxis->setTicks(false);       // 隐藏刻度线
  this->xAxis->setTickLabels(false);  // 隐藏刻度标签
  // this->xAxis->setLabel("");          // 清空轴标签文本
#endif
  this->yAxis->setTickLabels(false);  // 隐藏刻度标签

  // this->yAxis->setLabel("");

  // 创建跟踪器（用于鼠标悬停时显示坐标）
  m_tracer = new QCPItemTracer(this);
  // m_tracer->setGraph(m_dataGraph);
  m_tracer->setInterpolating(true);
  m_tracer->setStyle(QCPItemTracer::tsCircle);
  m_tracer->setPen(QPen(Qt::red));
  m_tracer->setBrush(QBrush(Qt::red));
  m_tracer->setSize(7);
  m_tracer->setVisible(false);

  // 创建跟踪器标签（显示坐标值）
  m_tracerLabel = new QCPItemText(this);
  m_tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
  m_tracerLabel->position->setType(QCPItemPosition::ptAbsolute);
  // m_tracerLabel->position->setType(QCPItemPosition::ptPlotCoords);
  m_tracerLabel->setPadding(QMargins(5, 5, 5, 5));
  m_tracerLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
  m_tracerLabel->setPen(QPen(Qt::black));
  m_tracerLabel->setVisible(false);

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
}

void TtModbusPlot::addData(TtModbusRegisterType::Type type, const int& addr,
                           double value) {

  auto key = qMakePair(static_cast<int>(type), addr);

  if (!m_curves.contains(key)) {
    return;
  }

  if (!m_firstDataReceived) {
    // 当首次收到数据时执行
    this->yAxis->setTicks(true);
    this->yAxis->setTickLabels(true);
    m_firstDataReceived = true;
  }

  CurveData& curve = m_curves[key];

  // 添加时间戳(秒)
  double timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
  if (curve.timeData.isEmpty()) {
    m_startTime = timestamp;  // 共用同一个起始时间
  }
  // 计算相对于起始时间的偏移
  double relativeTime = timestamp - m_startTime;

  if (curve.timeData.size() > points_nums_) {
    curve.timeData.removeFirst();
    curve.valueData.removeFirst();
  }

  curve.timeData.append(relativeTime);
  curve.valueData.append(value);
  curve.graph->addData(relativeTime, value);

  if (curve.tag) {
    double lastValue = curve.valueData.last();
    curve.tag->updatePosition(lastValue);
    curve.tag->setText(QString::number(lastValue, 'f', 2));
  }
  // 自动缩放
  if (m_autoScaleY) {
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    bool hasData = false;

    for (auto& c : m_curves) {
      if (!c.valueData.isEmpty()) {
        auto [minIt, maxIt] =
            std::minmax_element(c.valueData.begin(), c.valueData.end());
        yMin = qMin(yMin, *minIt);  // 所有图像中的最小 y
        yMax = qMax(yMax, *maxIt);
        hasData = true;
      }
    }
    if (hasData) {
      if (yMin >= yMax) {
        double center = yMin;                           // 当yMin == yMax时
        double margin = qMax(qAbs(center) * 0.2, 0.5);  // 20%或最小0.5单位
        yMin = center - margin;
        yMax = center + margin;
      } else {
        // 常规边距计算
        double margin = (yMax - yMin) * 0.1;
        yMin -= margin;
        yMax += margin;
      }
      qDebug() << yMin << yMax;
      yAxis->setRange(yMin, yMax);
    }
  }

  // 自动缩放X轴
  if (true) {
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();

    // 遍历所有曲线获取时间范围
    for (auto& c : m_curves) {
      if (!c.timeData.isEmpty()) {
        xMin = qMin(xMin, c.timeData.first());
        xMax = qMax(xMax, c.timeData.last());
      }
    }

    if (xMin < xMax) {
      // 留10%边距或固定延伸
      double margin = (xMax - xMin) * 0.1;
      xAxis->setRange(xMin - margin, xMax + margin);
    } else if (xMin == xMax) {
      // 处理单点情况
      xAxis->setRange(xMin - 1.0, xMax + 1.0);
    }
  }
  replot();
}

void TtModbusPlot::addGraphs(TtModbusRegisterType::Type type, const int& addr) {
  auto key = qMakePair(static_cast<int>(type), addr);
  if (!m_curves.contains(key)) {
    // 新曲线
    CurveData newCurve;
    static const QList<QColor> defaultColors = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow};

    QColor color = defaultColors.at(m_curves.size() % defaultColors.size());

    // 创建图形
    newCurve.graph = this->addGraph();
    newCurve.graph->setPen(QPen(color, 2));
    newCurve.graph->setName(QString("%1@%2").arg(type).arg(addr));

    // 创建右侧标签
    QCPAxis* valueAxis = this->yAxis;  // 共享Y轴或为每个曲线创建右轴
    newCurve.tag = new TtAxisTag(valueAxis);
    newCurve.tag->setPen(newCurve.graph->pen());

    m_tracer->setGraph(newCurve.graph);

    m_curves.insert(key, newCurve);
  }
}

void TtModbusPlot::removeGraphs(TtModbusRegisterType::Type type,
                                const int& addr) {
  auto key = qMakePair(static_cast<int>(type), addr);
  if (m_curves.contains(key)) {
    qDebug() << "remove";
    auto& curve = m_curves[key];

    // 移除图形
    if (curve.graph) {
      this->removeGraph(curve.graph);
      curve.graph.clear();
    }

    // 删除标签
    if (curve.tag) {
      delete curve.tag;
      curve.tag = nullptr;
    }

    m_curves.remove(key);
    replot();
  }
}

void TtModbusPlot::setGraphsPointCapacity(quint16 nums) {
  // points_nums_ = nums;
  if (nums > 0 && nums != points_nums_) {
    points_nums_ = nums;

    // 立即应用新的限制
    for (auto& curve : m_curves) {
      while (curve.timeData.size() > points_nums_) {
        curve.timeData.removeFirst();
        curve.valueData.removeFirst();
        // if (!curve.graph->data()->isEmpty()) {
        //   if (!curve.graph->data()->isEmpty()) {
        //     auto it = curve.graph->data()->constBegin();
        //     double firstKey = it->key;
        //     curve.graph->data()->remove(firstKey);
        //   }
        // }
        if (!curve.graph->data()->isEmpty()) {
          auto dataMap = curve.graph->data();
          dataMap->remove(dataMap->constBegin()->key);  // 使用迭代器安全删除
        }
      }
    }
    replot();
  }

  // clearGrphs();
}

void TtModbusPlot::clearData() {
  m_startTime = 0.0;
  for (auto& curve : m_curves) {
    curve.timeData.clear();
    curve.valueData.clear();
    // curve.graph.data();
  }
  this->replot();
}

// void TtModbusPlot::mouseMoveEvent(QMouseEvent* event) {
//   QCustomPlot::mouseMoveEvent(event);

//   // 显示/隐藏组件
//   const bool show = this->rect().contains(event->pos());
//   m_vLine->setVisible(show);
//   m_coordLabel->setVisible(show);

//   if (show) {
//     // 获取鼠标坐标
//     double x = this->xAxis->pixelToCoord(event->pos().x());
//     double y = this->yAxis->pixelToCoord(event->pos().y());

//     // 更新竖线位置
//     m_vLine->point1->setCoords(x, this->yAxis->range().lower);
//     m_vLine->point2->setCoords(x, this->yAxis->range().upper);

//     // 更新标签内容
//     m_coordLabel->setText(
//         QString("X: %1\nY: %2").arg(x, 0, 'f', 1).arg(y, 0, 'f', 2));
//   } else {
//     m_coordLabel->setVisible(false);
//   }

//   updateTracerPosition(event);
// }

void TtModbusPlot::mouseMoveEvent(QMouseEvent* event) {
  QCustomPlot::mouseMoveEvent(event);

  // 显示/隐藏组件
  const bool show = this->rect().contains(event->pos());
  m_vLine->setVisible(show);
  m_coordLabel->setVisible(show);

  if (show) {
    // 获取鼠标坐标
    double x = this->xAxis->pixelToCoord(event->pos().x());
    // double y = this->yAxis->pixelToCoord(event->pos().y());

    // 更新竖线位置
    m_vLine->point1->setCoords(x, this->yAxis->range().lower);
    m_vLine->point2->setCoords(x, this->yAxis->range().upper);

    updateTracerPosition(event);
    // 更新标签内容
    // m_coordLabel->setText(
    //     QString("X: %1\nY: %2").arg(x, 0, 'f', 1).arg(y, 0, 'f', 2));
  } else {
    m_coordLabel->setVisible(false);
  }
}

// void TtModbusPlot::updateTracerPosition(QMouseEvent* event) {
//   if (m_curves.isEmpty()) {
//     m_tracer->setVisible(false);
//     m_tracerLabel->setVisible(false);
//     return;
//   }

//   // 当前鼠标坐标值
//   double mouseX = this->xAxis->pixelToCoord(event->pos().x());

//   // 鼠标移出 x 轴外
//   if (mouseX < this->xAxis->range().lower ||
//       mouseX > this->xAxis->range().upper) {
//     m_tracer->setVisible(false);
//     m_tracerLabel->setVisible(false);
//     return;
//   }

//   QMap<double, QPair<QCPGraph*, QCPGraphData>> nearestPoints;

//   qDebug() << "=== 鼠标位置调试 ===";
//   qDebug() << "鼠标X坐标（像素）:" << event->pos().x();
//   qDebug() << "转换后X值:" << mouseX;

//   // 遍历每一个图像
//   for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
//     //
//     const auto& curve = it.value();
//     if (!curve.graph || curve.timeData.isEmpty()) {
//       continue;
//     }

//     // 获取时间 x 轴
//     auto times = curve.timeData;

//     auto it_lower = std::lower_bound(times.begin(), times.end(), mouseX);
//     int idx = it_lower - times.begin();
//     // 处理边界情况
//     if (idx == 0) {
//       // 鼠标在第一个点左侧或刚好在第一个点
//     } else if (idx == times.size()) {
//       // 鼠标在最后一个点右侧，取最后一个点
//       idx = times.size() - 1;
//     } else {
//       // 比较左右两个点，选择更近的
//       double distLeft = mouseX - times[idx - 1];
//       double distRight = times[idx] - mouseX;
//       if (distLeft < distRight)
//         idx--;
//     }

//     // 最终安全限制
//     idx = qBound(0, idx, times.size() - 1);

//     qDebug() << "计算后索引:" << idx;
//     if (idx >= 0 && idx < times.size() && idx < curve.valueData.size()) {
//       double distance = fabs(mouseX - times[idx]);
//       qDebug() << "有效数据点:" << times[idx] << curve.valueData[idx];

//       QCPGraphData dataPoint(times[idx], curve.valueData.value(idx));
//       nearestPoints.insert(distance, qMakePair(curve.graph, dataPoint));
//     }
//   }

//   // 更新跟踪器和标签
//   if (!nearestPoints.isEmpty()) {
//     auto closest = nearestPoints.first();
//     QCPGraph* activeGraph = closest.first;
//     QCPGraphData data = closest.second;

//     // 设置跟踪器
//     m_tracer->setGraph(activeGraph);
//     m_tracer->setGraphKey(data.key);
//     m_tracer->setVisible(true);

//     // 设置标签内容
//     QDateTime dt =
//         QDateTime::fromMSecsSinceEpoch((m_startTime + data.key) * 1000);
//     QString text = QString("%1\n值: %2")
//                        .arg(dt.toString("HH:mm:ss"))
//                        .arg(data.value, 0, 'f', 2);
//     m_tracerLabel->setText(text);

//     qDebug() << dt;

//     // 动态定位标签
//     QPoint cursorPos = event->pos();
//     QRect viewport = this->viewport();
//     QFontMetrics fm(m_tracerLabel->font());
//     int textWidth = fm.horizontalAdvance(text) + 20;

//     // qDebug() <<

//     if (cursorPos.x() + textWidth > viewport.right()) {
//       m_tracerLabel->position->setPixelPosition(
//           QPointF(viewport.right() - textWidth, cursorPos.y() - 20));
//     } else {
//       m_tracerLabel->position->setPixelPosition(
//           QPointF(cursorPos.x() + 15, cursorPos.y() - 20));
//     }

//     m_tracerLabel->setVisible(true);
//   } else {
//     m_tracer->setVisible(false);
//     m_tracerLabel->setVisible(false);
//   }

//   replot();
// }

void TtModbusPlot::updateTracerPosition(QMouseEvent* event) {
  if (m_curves.isEmpty()) {
    m_tracer->setVisible(false);
    m_tracerLabel->setVisible(false);
    return;
  }

  // 当前鼠标坐标值
  double mouseX = this->xAxis->pixelToCoord(event->pos().x());

  // 鼠标移出 x 轴外
  if (mouseX < this->xAxis->range().lower ||
      mouseX > this->xAxis->range().upper) {
    m_tracer->setVisible(false);
    m_tracerLabel->setVisible(false);
    return;
  }

  // 图像距离当前鼠标坐标值的最近点, 遍历每一张图, 性能消耗高
  // QMap<double, QPair<QCPGraph*, QCPGraphData>> nearestPoints;

  // qDebug() << "=== 鼠标位置调试 ===";
  // qDebug() << "鼠标X坐标（像素）:" << event->pos().x();
  // qDebug() << "转换后X值:" << mouseX;

  QDateTime time =
      QDateTime::fromMSecsSinceEpoch((m_startTime + mouseX) * 1000);
  QStringList tooltipLines;
  tooltipLines.append(
      QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));

  // 遍历每一个图像
  for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
    //
    const auto& curve = it.value();
    if (!curve.graph || curve.timeData.isEmpty()) {
      continue;
    }

    const auto& times = curve.timeData;
    const auto& values = curve.valueData;

    // 添加数据一致性检查
    if (times.size() != values.size()) {
      qWarning() << "Data inconsistency in curve" << curve.graph->name();
      continue;
    }

    auto it_lower = std::lower_bound(times.begin(), times.end(), mouseX);
    // 索引
    int idx = it_lower - times.begin();

    if (idx > 0) {
      if (idx == times.size() ||
          mouseX - times[idx - 1] < times[idx] - mouseX) {
        --idx;
      }
    }

    if (idx >= 0 && idx < times.size() && idx < values.size()) {
      QColor color = curve.graph->pen().color();
      QString name = curve.graph->name();
      double value = values[idx];
      tooltipLines.append(QString("<span style='color:%1;'>%2: %3</span>")
                              .arg(color.name())
                              .arg(name)
                              .arg(value, 0, 'f', 2));
    }
  }

  if (tooltipLines.size() > 1) {
    m_coordLabel->setText(tooltipLines.join("<br>"));

    TtQCPItemRichText* richLabel =
        qobject_cast<TtQCPItemRichText*>(m_coordLabel);
    const QTextDocument& doc = richLabel->document();
    qreal textWidth = doc.idealWidth() + 10;  // 增加10px边距
    qreal textHeight = doc.size().height() + 10;

    // 获取鼠标位置和视口边界
    QPoint mousePos = event->pos();
    QRect viewportRect = viewport();

    // 初始位置：鼠标右下方15像素
    QPoint pos = mousePos + QPoint(15, 5);

    // 水平边界检测
    if (pos.x() + textWidth > viewportRect.right()) {
      // 切换到左侧显示
      pos.setX(mousePos.x() - textWidth - 15);
    }

    // 垂直边界检测
    if (pos.y() + textHeight > viewportRect.bottom()) {
      // 切换到上方显示
      pos.setY(mousePos.y() - textHeight - 15);
    }

    // 应用最终位置
    m_coordLabel->position->setPixelPosition(pos);

    // QFontMetrics fm(m_coordLabel->font());
    // QString plainText = tooltipLines.join("\n");
    // qDebug() << plainText;

    // QRect textBound = fm.boundingRect(
    //     QRect(0, 0, 200, 100), Qt::TextDontClip | Qt::AlignLeft, plainText);
    // qDebug() << textBound;

    // QPoint pos = event->pos() + QPoint(20, 20);

    // // 获取视口边界
    // QRect viewportRect = viewport();

    // // 右侧有问题
    // if (pos.x() + textBound.width() > viewportRect.right()) {
    //   pos.setX(event->pos().x() - textBound.width() - 20);  // 左侧显示
    // }
    // // 垂直边界检测
    // if (pos.y() + textBound.height() > viewportRect.bottom()) {
    //   pos.setY(viewportRect.bottom() - textBound.height() - 5);
    // }

    // // m_coordLabel->setBrush(QBrush(QColor(Qt::cyan)));
    // m_coordLabel->setColor(QColor(Qt::cyan));
    // m_coordLabel->position->setPixelPosition(pos);
    m_coordLabel->setVisible(true);
  } else {
    m_coordLabel->setVisible(false);
  }
  replot();
}

SerialPlot::SerialPlot(QWidget* parent) : QCustomPlot(parent) {
  setupPlot();
  this->setOpenGl(true);
  // qDebug() << "opengle=" << this->openGl();

  setAntialiasedElements(QCP::aeAll);
}

SerialPlot::~SerialPlot() {}

void SerialPlot::addData(int channel, double value) {
  // auto key = qMakePair(static_cast<int>(type), addr);
  if (!curves_.contains(channel)) {
    return;
  }

  if (!m_firstDataReceived) {
    // 当首次收到数据时执行
    this->yAxis->setTicks(true);
    this->yAxis->setTickLabels(true);
    m_firstDataReceived = true;
  }

  // 对应图像
  CurveData& curve = curves_[channel];

  // 添加时间戳(秒)
  double timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
  if (curve.timeData.isEmpty()) {
    m_startTime = timestamp;  // 共用同一个起始时间
  }
  // 计算相对于起始时间的偏移
  double relativeTime = timestamp - m_startTime;

  if (curve.timeData.size() > points_nums_) {
    curve.timeData.removeFirst();
    curve.valueData.removeFirst();
  }

  curve.timeData.append(relativeTime);
  curve.valueData.append(value);
  // 相对时间作为 key 值
  curve.graph->addData(relativeTime, value);

  if (curve.tag) {
    double lastValue = curve.valueData.last();
    curve.tag->updatePosition(lastValue);
    curve.tag->setText(QString::number(lastValue, 'f', 2));
  }
  // 自动缩放
  if (m_autoScaleY) {
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    bool hasData = false;

    for (auto& c : curves_) {
      if (!c.valueData.isEmpty()) {
        auto [minIt, maxIt] =
            std::minmax_element(c.valueData.begin(), c.valueData.end());
        yMin = qMin(yMin, *minIt);  // 所有图像中的最小 y
        yMax = qMax(yMax, *maxIt);
        hasData = true;
      }
    }
    if (hasData) {
      if (yMin >= yMax) {
        double center = yMin;                           // 当yMin == yMax时
        double margin = qMax(qAbs(center) * 0.2, 0.5);  // 20%或最小0.5单位
        yMin = center - margin;
        yMax = center + margin;
      } else {
        // 常规边距计算
        double margin = (yMax - yMin) * 0.1;
        yMin -= margin;
        yMax += margin;
      }
      // qDebug() << yMin << yMax;
      yAxis->setRange(yMin, yMax);
    }
  }

  // 自动缩放X轴
  if (true) {
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();

    // // 遍历所有曲线获取时间范围
    // for (auto& c : m_curves) {
    //   if (!c.timeData.isEmpty()) {
    //     xMin = qMin(xMin, c.timeData.first());
    //     xMax = qMax(xMax, c.timeData.last());
    //   }
    // }
    // for (auto& c : m_curves) {
    // if (c.graph && c.graph.data()->size() > 0) {}
    // }
    for (auto& c : curves_) {
      if (c.graph && c.graph->data()->size() > 0) {  // 直接使用图形数据
        auto data = c.graph->data();
        xMin = qMin(xMin, data->begin()->key);
        xMax = qMax(xMax, (data->end() - 1)->key);
      }
    }

    // 控制左右边距
    // if (xMin < xMax) {
    //   // 留10%边距或固定延伸
    //   double margin = (xMax - xMin) * 0.1;
    //   xAxis->setRange(xMin - margin, xMax + margin);
    // } else if (xMin == xMax) {
    //   // 处理单点情况
    //   xAxis->setRange(xMin - 1.0, xMax + 1.0);
    // }
    if (xMin < xMax) {
      xAxis->setRange(xMin, xMax);  // 无额外边距
    } else if (xMin == xMax) {
      xAxis->setRange(xMin - 0.1, xMax + 0.1);
    }
  }
  replot();
}

void SerialPlot::clearData() {
  m_startTime = 0.0;
  for (auto& curve : curves_) {
    curve.timeData.clear();
    curve.valueData.clear();
    // curve.graph.data();
  }
  this->replot();
}

void SerialPlot::saveWaveFormData() {
  QString dirpath = QFileDialog::getSaveFileName(
      this, QStringLiteral("保存波形数据"),
      qApp->applicationDirPath() + "/plot.csv", QString(tr("*.csv")));

  if (!dirpath.isEmpty()) {
    QFile file(dirpath);
    // 方式：Append为追加，WriteOnly，ReadOnly
    if (!file.open(QIODevice::WriteOnly)) {
      Ui::TtMessageBar::warning(TtMessageBarType::Top, tr("提示"),
                                tr("无法创建文件!"), 1500);
    } else {
      QTextStream stream(&file);
      {
        // 添加标题栏
        {
          if (this->graphCount() > 0) {
            stream << (tr("time(s),"));
            int iGraphIndex = 0;
            for (iGraphIndex = 0; iGraphIndex < this->graphCount() - 1;
                 iGraphIndex++) {
              // 添加每个图表的名字(通道的名字)
              stream << (this->graph(iGraphIndex)->name() + ",");
            }
            // 添加换行符
            stream << (this->graph(iGraphIndex)->name() + "\n");
          }
        }
      }
      if (this->graphCount() > 0) {
        // 遍历数据
        for (int iGraphData = 0; iGraphData < this->graph(0)->dataCount();
             iGraphData++) {
          // 添加时间轴
          qDebug() << QString::number(
              (this->graph(0)->data()->at(iGraphData)->key), 'f', 6);

          stream << (QString::number(
                         (this->graph(0)->data()->at(iGraphData)->key), 'f',
                         6) +
                     ",");
          // 遍历曲线
          int iGraphIndex = 0;
          for (iGraphIndex = 0; iGraphIndex < this->graphCount() - 1;
               iGraphIndex++) {
            // 添加数据
            stream << (QString::number((this->graph(iGraphIndex)
                                            ->data()
                                            ->at(iGraphData)
                                            ->value),
                                       'f', 6) +
                       ",");
          }
          stream
              << (QString::number(
                      (this->graph(iGraphIndex)->data()->at(iGraphData)->value),
                      'f', 6) +
                  "\n");
        }
      }
      // refreshAction = eRefreshNone;
      file.close();
      return;
    }
    // refreshAction = eRefreshNone;
  } else {
  }

  // // 使用前面定义的函数保存CSV
  // exportTableWidgetToCsv(filePath, tableWidget);
  // QMessageBox::information(this, "保存成功", "CSV 文件已成功保存。");
}

void SerialPlot::addGraphs(int channel, const QColor& color) {
  if (!curves_.contains(channel)) {

    // 新曲线
    CurveData newCurve;
    // static const QList<QColor> defaultColors = {
    //     Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow};
    // QColor color = defaultColors.at(curves_.size() % defaultColors.size());

    qDebug() << "color: " << color;
    // 创建图形
    newCurve.graph = this->addGraph();
    newCurve.graph->setPen(QPen(color, 2));
    newCurve.graph->setName(QString("channel@%1").arg(channel));

    // 创建右侧标签
    QCPAxis* valueAxis = this->yAxis;  // 共享Y轴或为每个曲线创建右轴
    newCurve.tag = new TtAxisTag(valueAxis);
    newCurve.tag->setPen(newCurve.graph->pen());

    m_tracer->setGraph(newCurve.graph);

    curves_.insert(channel, newCurve);
  }
}

void SerialPlot::removeGraphs(int channel) {
  if (curves_.contains(channel)) {
    auto& curve = curves_[channel];

    // 移除图形
    if (curve.graph) {
      this->removeGraph(curve.graph);
      curve.graph.clear();
    }

    // 删除标签
    if (curve.tag) {
      delete curve.tag;
      curve.tag = nullptr;
    }
    curves_.remove(channel);
    replot();
  }
}

void SerialPlot::setGraphsPointCapacity(quint16 nums) {
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

void SerialPlot::mouseMoveEvent(QMouseEvent* event) {
  QCustomPlot::mouseMoveEvent(event);

  // 显示/隐藏组件
  const bool show = this->rect().contains(event->pos());
  // 水平线
  m_vLine->setVisible(show);
  m_coordLabel->setVisible(show);

  for (auto* label : m_hoverLabels) {
    if (label) {
      label->setVisible(false);
      this->removeItem(label);
    }
  }
  m_hoverLabels.clear();

  if (!show) {
    replot();
    return;
  }

  double x = this->xAxis->pixelToCoord(event->pos().x());

  // 设置垂直线位置
  m_vLine->point1->setCoords(x, this->yAxis->range().lower);
  m_vLine->point2->setCoords(x, this->yAxis->range().upper);

  updateTracerPosition(event);

  replot();
}

void SerialPlot::setupPlot() {
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

  // 配置横轴显示格式（禁用科学计数法）
  // QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  // fixedTicker->setTickStep(1.0);  // 主刻度间隔1秒
  // fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);

  // 设置标签格式
  this->xAxis->setNumberFormat("f");   // 使用固定小数点格式
  this->xAxis->setNumberPrecision(0);  // 无小数位
  // 如果需要确保刻度是整数
  // this->xAxis->setAutoTickStep(false); // 对于旧版本
  // this->xAxis->setTickStep(1.0);       // 设置整数步长

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
  this->yAxis->setTickLabels(false);  // 隐藏刻度标签

  // this->yAxis->setLabel("");

  // 创建跟踪器（用于鼠标悬停时显示坐标）
  m_tracer = new QCPItemTracer(this);
  // m_tracer->setGraph(m_dataGraph);
  m_tracer->setInterpolating(true);
  m_tracer->setStyle(QCPItemTracer::tsCircle);
  m_tracer->setPen(QPen(Qt::red));
  m_tracer->setBrush(QBrush(Qt::red));
  m_tracer->setSize(8);
  m_tracer->setVisible(false);

  // 创建跟踪器标签（显示坐标值）
  m_tracerLabel = new QCPItemText(this);
  m_tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
  m_tracerLabel->position->setType(QCPItemPosition::ptAbsolute);
  // m_tracerLabel->position->setType(QCPItemPosition::ptPlotCoords);
  m_tracerLabel->setPadding(QMargins(5, 5, 5, 5));
  m_tracerLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
  m_tracerLabel->setPen(QPen(Qt::black));
  m_tracerLabel->setVisible(false);

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

// void SerialPlot::updateTracerPosition(QMouseEvent* event) {
//   if (m_curves.isEmpty()) {
//     m_tracer->setVisible(false);
//     m_tracerLabel->setVisible(false);
//     return;
//   }

//   // 当前鼠标坐标值
//   double mouseX = this->xAxis->pixelToCoord(event->pos().x());

//   // 鼠标移出 x 轴外
//   if (mouseX < this->xAxis->range().lower ||
//       mouseX > this->xAxis->range().upper) {
//     m_tracer->setVisible(false);
//     m_tracerLabel->setVisible(false);
//     return;
//   }

//   // 图像距离当前鼠标坐标值的最近点, 遍历每一张图, 性能消耗高
//   // QMap<double, QPair<QCPGraph*, QCPGraphData>> nearestPoints;

//   // qDebug() << "=== 鼠标位置调试 ===";
//   // qDebug() << "鼠标X坐标（像素）:" << event->pos().x();
//   // qDebug() << "转换后X值:" << mouseX;

//   QDateTime time =
//       QDateTime::fromMSecsSinceEpoch((m_startTime + mouseX) * 1000);
//   QStringList tooltipLines;
//   tooltipLines.append(
//       QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));

//   // 遍历每一个图像
//   for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
//     //
//     const auto& curve = it.value();
//     if (!curve.graph || curve.timeData.isEmpty()) {
//       continue;
//     }

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
//       QColor color = curve.graph->pen().color();
//       QString name = curve.graph->name();
//       double value = values[idx];
//       tooltipLines.append(QString("<span style='color:%1;'>%2: %3</span>")
//                               .arg(color.name())
//                               .arg(name)
//                               .arg(value, 0, 'f', 2));
//     }
//   }

//   if (tooltipLines.size() > 1) {
//     m_coordLabel->setText(tooltipLines.join("<br>"));

//     TtQCPItemRichText* richLabel =
//         qobject_cast<TtQCPItemRichText*>(m_coordLabel);
//     const QTextDocument& doc = richLabel->document();
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
//   replot();
// }

void SerialPlot::updateTracerPosition(QMouseEvent* event) {
  if (curves_.isEmpty()) {
    m_tracer->setVisible(false);
    m_tracerLabel->setVisible(false);
    return;
  }

  // 当前鼠标坐标值
  double mouseX = this->xAxis->pixelToCoord(event->pos().x());
  double mouseY = this->xAxis->pixelToCoord(event->pos().y());

  // 鼠标移出 x 轴外
  if (mouseX < this->xAxis->range().lower ||
      mouseX > this->xAxis->range().upper) {
    // qDebug() << "move outside";
    m_tracer->setVisible(false);
    m_tracerLabel->setVisible(false);
    return;
  }
  // // 鼠标移出 y 轴外
  // if (mouseY < this->yAxis->range().lower ||
  //     mouseY > this->yAxis->range().upper) {
  //   qDebug() << "test";
  //   m_tracer->setVisible(false);
  //   m_tracerLabel->setVisible(false);
  //   return;
  // }

  QDateTime time =
      QDateTime::fromMSecsSinceEpoch((m_startTime + mouseX) * 1000);
  QStringList tooltipLines;
  tooltipLines.append(
      QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));
  bool tracerSet = false;

  // 遍历每一个图像
  for (auto it = curves_.begin(); it != curves_.end(); ++it) {
    //
    const auto& curve = it.value();
    if (!curve.graph || curve.timeData.isEmpty()) {
      continue;
    }

    const auto& times = curve.timeData;
    const auto& values = curve.valueData;

    // 添加数据一致性检查
    if (times.size() != values.size()) {
      qWarning() << "Data inconsistency in curve" << curve.graph->name();
      continue;
    }

    auto it_lower = std::lower_bound(times.begin(), times.end(), mouseX);
    // 索引
    int idx = it_lower - times.begin();

    if (idx > 0) {
      if (idx == times.size() ||
          mouseX - times[idx - 1] < times[idx] - mouseX) {
        --idx;
      }
    }

    if (idx >= 0 && idx < times.size() && idx < values.size()) {
      // 设置跟踪器到当前的曲线和位置
      // if (!tracerSet) {
      //   m_tracer->setGraph(curve.graph);
      //   m_tracer->setGraphKey(times[idx]);
      //   m_tracer->setVisible(true);
      //   tracerSet = true;
      // }
      QColor color = curve.graph->pen().color();
      QString name = curve.graph->name();
      double value = values[idx];
      tooltipLines.append(QString("<span style='color:%1;'>%2: %3</span>")
                              .arg(color.name())
                              .arg(name)
                              .arg(value, 0, 'f', 2));
    }
  }

  if (tooltipLines.size() > 1) {
    m_coordLabel->setText(tooltipLines.join("<br>"));

    TtQCPItemRichText* richLabel =
        qobject_cast<TtQCPItemRichText*>(m_coordLabel);
    const QTextDocument& doc = richLabel->document();
    qreal textWidth = doc.idealWidth() + 10;  // 增加10px边距
    qreal textHeight = doc.size().height() + 10;

    // 获取鼠标位置和视口边界
    QPoint mousePos = event->pos();
    QRect viewportRect = viewport();

    // 初始位置：鼠标右下方15像素
    QPoint pos = mousePos + QPoint(15, 5);

    // 水平边界检测
    if (pos.x() + textWidth > viewportRect.right()) {
      // 切换到左侧显示
      pos.setX(mousePos.x() - textWidth - 15);
    }

    // 垂直边界检测
    if (pos.y() + textHeight > viewportRect.bottom()) {
      // 切换到上方显示
      pos.setY(mousePos.y() - textHeight - 15);
    }

    // 应用最终位置
    m_coordLabel->position->setPixelPosition(pos);
    m_coordLabel->setVisible(true);
  } else {
    m_coordLabel->setVisible(false);
  }

  if (!tracerSet) {
    m_tracer->setVisible(false);  // 没有找到点时隐藏
  }
  replot();
}

}  // namespace Ui
