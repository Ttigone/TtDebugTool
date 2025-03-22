#include "TtModbusPlot.h"

AxisTag::AxisTag(QCPAxis* parentAxis) : QObject(parentAxis), mAxis(parentAxis) {
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

  mLabel = new QCPItemText(mAxis->parentPlot());
  mLabel->setLayer("overlay");
  mLabel->setClipToAxisRect(false);
  mLabel->setPadding(QMargins(3, 0, 3, 0));
  // mLabel->setPadding(QMargins(6, 0, 6, 0));
  // mLabel->setBrush(QBrush(Qt::white));
  mLabel->setPen(QPen(Qt::blue, 1));
  mLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  mLabel->position->setParentAnchor(mArrow->start);

  // 增加文本可视化设置
  mLabel->setColor(Qt::black);  // 文字颜色
  // mLabel->setBrush(QBrush(Qt::white));       // 背景填充
  mLabel->setBrush(QBrush(QColor(255, 255, 255, 220)));  // 背景填充
  // mLabel->setPadding(QMargins(4, 2, 4, 2));  // 增加内边距
  mLabel->setFont(QFont("Arial", 8));  // 明确设置字体
}

AxisTag::~AxisTag() {
  if (mDummyTracer)
    mDummyTracer->parentPlot()->removeItem(mDummyTracer);
  if (mArrow)
    mArrow->parentPlot()->removeItem(mArrow);
  if (mLabel)
    mLabel->parentPlot()->removeItem(mLabel);
}

void AxisTag::setPen(const QPen& pen) {
  mArrow->setPen(pen);
  mLabel->setPen(pen);
}

void AxisTag::setBrush(const QBrush& brush) {
  mLabel->setBrush(brush);
}

void AxisTag::setText(const QString& text) {
  mLabel->setText(text);
}

void AxisTag::updatePosition(double value) {
  mDummyTracer->position->setCoords(1, value);
  mArrow->end->setCoords(mAxis->offset(), 0);
}

ModbusPlot::ModbusPlot(QWidget* parent) : QCustomPlot(parent) {
  setupPlot();
}

ModbusPlot::~ModbusPlot() {}

void ModbusPlot::setupPlot() {
  // 创建图表和数据容器
  m_dataGraph = this->addGraph();
  m_dataGraph->setPen(QPen(Qt::blue, 2));

  QCPAxis* rightAxis = axisRect()->addAxis(QCPAxis::atRight);
  rightAxis->setPadding(60);
  rightAxis->setVisible(true);
  rightAxis->setTicks(false);        // 隐藏刻度线
  rightAxis->setTickLabels(false);   // 隐藏刻度标签
  rightAxis->setBasePen(Qt::NoPen);  // 隐藏轴线

  // 右侧垂直标签 附着值
  tag_ = new AxisTag(m_dataGraph->valueAxis());
  tag_->setPen(m_dataGraph->pen());

  // 允许用户交互
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
  m_tracer->setGraph(m_dataGraph);
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
  m_coordLabel = new QCPItemText(this);
  m_coordLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);
  // m_coordLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
  m_coordLabel->position->setType(QCPItemPosition::ptViewportRatio);
  m_coordLabel->position->setCoords(1.0, 0.0);  // 右上角
  m_coordLabel->setTextAlignment(Qt::AlignRight);
  m_coordLabel->setBrush(QBrush(Qt::white));
  m_coordLabel->setPadding(QMargins(5, 5, 5, 5));
  m_coordLabel->setVisible(false);

  // 启用鼠标跟踪
  this->setMouseTracking(true);
}

void ModbusPlot::addData(double value) {
  if (!m_firstDataReceived && !m_valueData.isEmpty()) {
    // 当首次收到数据时执行
    this->yAxis->setTicks(true);
    this->yAxis->setTickLabels(true);
    m_firstDataReceived = true;
  }

  // 添加时间戳(秒)
  double timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;

  // 首次数据时初始化起始时间
  if (m_timeData.isEmpty()) {
    m_startTime = timestamp;
  }

  // 计算相对于起始时间的偏移
  double relativeTime = timestamp - m_startTime;

  // 存储时间数据
  m_timeData.append(relativeTime);  // 或直接存储 timestamp
  m_valueData.append(value);

  // // 添加数据点, x 是数据的大小个数
  // qDebug() << relativeTime << value;
  m_dataGraph->addData(relativeTime, value);

  xAxis->rescale();
  m_dataGraph->rescaleValueAxis(false, true);
  // xAxis->setRange(xAxis->range().upper, 100, Qt::AlignRight);

  if (!m_timeData.isEmpty()) {
    double latestTime = m_timeData.last();
    // 最近 10的数据
    this->xAxis->setRange(latestTime - 10, latestTime);
  }

  // 从数据池获取数据
  double graph1Value = m_dataGraph->dataMainValue(m_dataGraph->dataCount() - 1);
  // 动态更新显示的值 (y)
  tag_->updatePosition(graph1Value);
  /// 文本格式
  tag_->setText(QString::number(graph1Value, 'f', 2));

  // m_valueData.append(value);
  // qDebug() << "m_VDa" << m_valueData;

  if (m_autoScaleY && !m_valueData.isEmpty()) {
    // 获取当前X轴显示的时间范围
    const double xMin = this->xAxis->range().lower;
    const double xMax = this->xAxis->range().upper;
    // qDebug() << xMin << xMax;

    // 仅统计当前可见区域内的数据点
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    int validPoints = 0;

    for (int i = 0; i < m_timeData.size(); ++i) {
      if (m_timeData[i] >= xMin && m_timeData[i] <= xMax) {
        // 处于当前的时间之内
        const double yVal = m_valueData[i];
        // qDebug() << "get y: " << yVal;
        // 获取对应的 y 值
        minY = qMin(minY, yVal);
        maxY = qMax(maxY, yVal);
        validPoints++;
      }
    }
    // 如果没有可见点，使用全局范围
    if (validPoints == 0) {
      minY = *std::min_element(m_valueData.begin(), m_valueData.end());
      maxY = *std::max_element(m_valueData.begin(), m_valueData.end());
    }
    qDebug() << "maxY: " << maxY << "minY" << minY;

    // 处理所有值相同的情况
    const double yRange = maxY - minY;
    if (yRange < 1e-6) {
      // minY -= 1.0;
      // maxY += 1.0;
      // 以当前值为中心，创建对称范围
      const double center = minY;  // 或 maxY（两者相同）
      const double margin = qMax(center * 0.1, 0.5);  // 至少保留5%的值或0.5单位
      minY = center - margin;
      maxY = center + margin;
    } else {
      // 添加动态边距（至少5%的范围或固定值）
      // const double margin = qMax(yRange * 0.1, 0.5);
      const double margin = qMax(yRange * 0.2, qMin(0.5, minY * 0.05));
      qDebug() << "margin: " << margin;
      minY -= margin;
      maxY += margin;
    }

    this->yAxis->setRange(minY, maxY);
  }

  // 重绘图表
  this->replot();
}

void ModbusPlot::clearData() {
  m_timeData.clear();
  m_valueData.clear();
  m_dataGraph->setData(m_timeData, m_valueData);
  this->replot();
}

void ModbusPlot::mouseMoveEvent(QMouseEvent* event) {
  QCustomPlot::mouseMoveEvent(event);

  // 显示/隐藏组件
  const bool show = this->rect().contains(event->pos());
  m_vLine->setVisible(show);
  m_coordLabel->setVisible(show);

  if (show) {
    // 获取鼠标坐标
    double x = this->xAxis->pixelToCoord(event->pos().x());
    double y = this->yAxis->pixelToCoord(event->pos().y());

    // 更新竖线位置
    m_vLine->point1->setCoords(x, this->yAxis->range().lower);
    m_vLine->point2->setCoords(x, this->yAxis->range().upper);

    // 更新标签内容
    m_coordLabel->setText(
        QString("X: %1\nY: %2").arg(x, 0, 'f', 1).arg(y, 0, 'f', 2));
  }

  updateTracerPosition(event);
}

void ModbusPlot::updateTracerPosition(QMouseEvent* event) {
  if (m_dataGraph->data()->size() == 0)
    return;

  // 获取鼠标位置对应的X坐标
  double mouseX = this->xAxis->pixelToCoord(event->pos().x());

  // 找到最近的数据点
  int closestIndex = 0;
  double closestDistance = std::numeric_limits<double>::max();

  for (int i = 0; i < m_timeData.size(); ++i) {
    double distance = qAbs(m_timeData[i] - mouseX);
    if (distance < closestDistance) {
      closestDistance = distance;
      closestIndex = i;
    }
  }

  // 如果找到足够近的点，显示跟踪器和标签
  if (closestIndex >= 0 && closestIndex < m_timeData.size()) {
    double dataX = m_timeData[closestIndex];
    double dataY = m_valueData[closestIndex];

    // 设置跟踪器位置
    m_tracer->setGraphKey(dataX);
    m_tracer->setVisible(true);

    QDateTime baseTime = QDateTime::fromMSecsSinceEpoch(m_startTime * 1000);
    QDateTime actualTime = baseTime.addSecs(static_cast<qint64>(dataX));
    QString timeStr = actualTime.toString("yyyy-MM-dd hh:mm:ss");

    // 更新标签
    m_tracerLabel->setText(
        QString("时间: %1\n值: %2").arg(timeStr).arg(dataY, 0, 'f', 2));

    // 动态计算标签位置（关键修改）
    QFontMetrics fm(m_tracerLabel->font());
    int textWidth = fm.horizontalAdvance(m_tracerLabel->text());
    int viewportRight = this->viewport().width();

    // 判断是否需要左侧显示
    if (event->pos().x() + textWidth + 20 > viewportRight) {
      m_tracerLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignTop);
      m_tracerLabel->position->setCoords(event->pos().x() - 10,
                                         event->pos().y() - 10);
    } else {
      m_tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
      m_tracerLabel->position->setCoords(event->pos().x() + 10,
                                         event->pos().y() - 10);
    }
    m_tracerLabel->setVisible(true);

    this->replot();
  } else {
    // 如果没有足够近的点，隐藏跟踪器和标签
    m_tracer->setVisible(false);
    m_tracerLabel->setVisible(false);
  }
}
