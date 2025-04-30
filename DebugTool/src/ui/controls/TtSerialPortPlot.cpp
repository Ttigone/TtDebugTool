#include "ui/controls/TtSerialPortPlot.h"

#include <ui/widgets/message_bar.h>
#include "ui/controls/TtPlotItem.h"

namespace Ui {

TtSerialPortPlot::TtSerialPortPlot(QWidget* parent) : QCustomPlot(parent) {
  setupPlot();
  this->setOpenGl(true);

  setAntialiasedElements(QCP::aeAll);
}

TtSerialPortPlot::~TtSerialPortPlot() {}

void TtSerialPortPlot::addData(int channel, double value) {
  if (!curves_.contains(channel)) {
    return;
  }

  if (!m_firstDataReceived) {
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
      yAxis->setRange(yMin, yMax);
    }
  }

  // 自动缩放X轴
  if (true) {
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();

    for (auto& c : curves_) {
      if (c.graph && c.graph->data()->size() > 0) {  // 直接使用图形数据
        auto data = c.graph->data();
        xMin = qMin(xMin, data->begin()->key);
        xMax = qMax(xMax, (data->end() - 1)->key);
      }
    }

    if (xMin < xMax) {
      xAxis->setRange(xMin, xMax);  // 无额外边距
    } else if (xMin == xMax) {
      xAxis->setRange(xMin - 0.1, xMax + 0.1);
    }
  }
  replot();
}

void TtSerialPortPlot::clearData() {
  m_startTime = 0.0;
  for (auto& curve : curves_) {
    curve.timeData.clear();
    curve.valueData.clear();
  }
  this->replot();
}

void TtSerialPortPlot::saveWaveFormData() {
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

void TtSerialPortPlot::addGraphs(int channel, const QColor& color) {
  if (!curves_.contains(channel)) {
    // 新曲线
    CurveData newCurve;
    // 创建图形
    newCurve.graph = this->addGraph();
    newCurve.graph->setPen(QPen(color, 3));
    newCurve.graph->setName(QString("channel@%1").arg(channel));
    newCurve.graph->setScatterStyle(QCPScatterStyle::ssCircle);
    // newCurve.graph->setScatterStyle(QCPScatterStyle::ssDot);
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

void TtSerialPortPlot::mouseMoveEvent(QMouseEvent* event) {
  QCustomPlot::mouseMoveEvent(event);
  // if (curves_.isEmpty()) {
  //   return;
  // }

  const bool show = viewport().contains(event->pos());
  // // 水平线
  m_vLine->setVisible(show);
  m_tracer->setVisible(show);
  m_coordLabel->setVisible(show);

  // for (auto* label : m_hoverLabels) {
  //   if (label) {
  //     label->setVisible(false);
  //     this->removeItem(label);
  //   }
  // }
  // m_hoverLabels.clear();

  if (!show) {
    replot();
    return;
  }

  double x = this->xAxis->pixelToCoord(event->pos().x());
  // qDebug() << x;

  // 同一个 x 坐标, y 值分别是最大和最小, 竖直线
  m_vLine->point1->setCoords(x, this->yAxis->range().lower);
  m_vLine->point2->setCoords(x, this->yAxis->range().upper);

  // qDebug() << "test";

  updateTracerPosition(event);

  replot();
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
  this->yAxis->setTickLabels(false);  // 隐藏刻度标签

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

void TtSerialPortPlot::updateTracerPosition(QMouseEvent* event) {
  if (curves_.isEmpty()) {
    // 没有图像, 不显示
    // qDebug() << "empty";
    m_tracer->setVisible(false);
    m_vLine->setVisible(false);
    m_coordLabel->setVisible(false);
    return;
  }

  // // 当前鼠标坐标值
  double mouseX = this->xAxis->pixelToCoord(event->pos().x());
  double mouseY = this->xAxis->pixelToCoord(event->pos().y());

  // 鼠标移出 x 轴外
  if (mouseX < this->xAxis->range().lower ||
      mouseX > this->xAxis->range().upper) {
    // qDebug() << "move outside";
    m_tracer->setVisible(false);
    // m_tracerLabel->setVisible(false);
    m_coordLabel->setVisible(false);
    return;
  }

  QDateTime time =
      QDateTime::fromMSecsSinceEpoch((m_startTime + mouseX) * 1000);
  QStringList tooltipLines;
  tooltipLines.append(
      QString("<b>时间: %1</b>").arg(time.toString("HH:mm:ss")));
  // 默认 false
  bool tracerSet = false;

  bool hasVailData = false;

  // qDebug() << "update: " << curves_.size();
  // 遍历每一个图像
  for (auto it = curves_.begin(); it != curves_.end(); ++it) {
    //
    const auto& curve = it.value();
    if (!curve.graph || curve.timeData.isEmpty()) {
      // 没有数据
      continue;
    }
    // qDebug() << "insit";
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
      hasVailData = true;
    }
  }

  // if (tooltipLines.size() > 1) {
  if (hasVailData) {
    // 设置显示的富文本
    m_coordLabel->setText(tooltipLines.join("<br>"));

    // TtQCPItemRichText* richLabel =
    //     qobject_cast<TtQCPItemRichText*>(m_coordLabel);
    // const QTextDocument& doc = richLabel->document();
    const QTextDocument& doc = m_coordLabel->document();
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

  // if (!tracerSet) {
  //   // 为什么进来?
  //   qDebug() << "none";
  //   m_vLine->setVisible(false);
  //   m_tracer->setVisible(false);  // 没有找到点时隐藏
  //   // m_coordLabel->setVisible(false);
  // }

  replot();
}

}  // namespace Ui
