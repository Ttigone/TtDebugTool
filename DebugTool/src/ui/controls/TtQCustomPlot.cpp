#include "ui/controls/TtQCustomPlot.h"

namespace Ui {

TtQCustomPlot::TtQCustomPlot(QWidget* parent, bool enableGPU) {
  setGraphColor();
  // 拖拽时不启用抗拒齿
  setNoAntialiasingOnDrag(false);

  qDebug() << "opengle=" << this->openGl();
  // 启用 opengl
  setOpenGl(true);

  qDebug() << "opengle=" << this->openGl();

  setAntialiasedElements(QCP::aeAll);

  setInteractions(
      QCP::
          iRangeDrag |  // 允许用户拖动图表中的可视区域，以改变 x 和 y 轴的范围。
      QCP::
          iRangeZoom |  // 允许用户使用鼠标滚轮或拖动来缩放图表的可视区域，以改变 x 和 y 轴的范围。
      QCP::iSelectAxes |    // 允许用户选择坐标轴。
      QCP::iSelectLegend |  // 允许用户选择图例项（如果图表中有图例的话）。
      QCP::
          iSelectPlottables);  // 允许用户选择图表中的数据（如曲线、柱状图等）。

  // 设置QCustomPlot控件的坐标轴框，让坐标轴框围满整个绘图区域
  axisRect()->setupFullAxesBox();
  // 将QCustomPlot控件的第二个x轴设置为可见
  xAxis2->setVisible(true);
  // 将QCustomPlot控件的第二个x轴的刻度标签设置为不可见，也就是不显示刻度标签
  xAxis2->setTickLabels(false);
  // 将QCustomPlot控件的第二个y轴设置为可见
  yAxis2->setVisible(true);
  // 将QCustomPlot控件的第二个y轴的刻度标签设置为不可见，也就是不显示刻度标签
  yAxis2->setTickLabels(false);

  // 设置x轴的小数位数为2
  xAxis->setNumberFormat("f");   // 使用浮点数格式
  xAxis->setNumberPrecision(3);  // 设置小数精度为2

  // 设置y轴的小数位数为3
  yAxis->setNumberFormat("f");   // 使用浮点数格式
  yAxis->setNumberPrecision(3);  // 设置小数精度为3

  // 使上下两个X轴的范围总是相等，使左右两个Y轴的范围总是相等
  connect(xAxis, SIGNAL(rangeChanged(QCPRange)), this->xAxis2,
          SLOT(setRange(QCPRange)));
  connect(yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2,
          SLOT(setRange(QCPRange)));
  // 选中图例显示
  connect(this, &QCustomPlot::selectionChangedByUser, this,
          &TtQCustomPlot::selectionChanged);

  // 双击修改坐标轴名称
  connect(this, &QCustomPlot::axisDoubleClick, this,
          &TtQCustomPlot::axisLabelDoubleClick);

  // 双击修改图例名称
  connect(this, &QCustomPlot::legendDoubleClick, this,
          &TtQCustomPlot::legendDoubleClick);

  // 单击图形时在状态栏中显示消息的连接槽：
  // connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
  // 显示图例名称
  this->legend->setVisible(true);
  QFont legendFont = font();
  legendFont.setPointSize(10);
  this->legend->setFont(legendFont);
  this->legend->setSelectedFont(legendFont);
  this->legend->setSelectableParts(QCPLegend::spItems);

  // 追踪光标
  plotTracer = new TtQCustomPlotTracer(this);

  this->xAxis->setRange(0, 5);
  this->yAxis->setRange(0, 20);

  this->xAxis->ticker()->setTickCount(8);
  this->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);
  this->yAxis->ticker()->setTickCount(8);
  this->yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);

  // 刷新
  connect(&refreshTimer, &QTimer::timeout, this,
          &TtQCustomPlot::GraphRefreshSlot);

  // 清除图标
  clearGraphs();
  refreshTimer.setTimerType(Qt::PreciseTimer);
  stopRefreshTimer();
  startRefreshTimer();

  // 鼠标右键逻辑
  setContextMenuPolicy(Qt::CustomContextMenu);
  // 展示右键弹出框
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(contextMenuRequest(QPoint)));
  // 获取当前时间
  startTime = QDateTime::currentDateTime();
}

TtQCustomPlot::~TtQCustomPlot() {
  qDebug() << "delete";
  refresh(eRefreshNone);
  isRefreshTimerEnable = false;
  isRefreshTimerEnable = false;  // 是否启动定时器
  isRefreshGraphs = false;       // 是否有波形
  keyPress_X = false;
  keyPress_V = false;
  // 停止刷新
  stopRefreshTimer();
  // 移除图标
  removeAllGraphs();
  if (!plotTracer.isNull()) {
    delete plotTracer;
    plotTracer = nullptr;
  }
}

void TtQCustomPlot::startRefreshTimer() {
  // 开启定时器刷新
  isRefreshTimerEnable = true;
  if (!refreshTimer.isActive()) {
    // 20 ms 定时
    refreshTimer.start(GraphShowTimerSet);
  }
}

void TtQCustomPlot::stopRefreshTimer() {
  isRefreshTimerEnable = false;
  if (refreshTimer.isActive()) {
    refreshTimer.stop();
  }
}

void TtQCustomPlot::refreshGraphs() {
  // 刷新波形
  endTime = QDateTime::currentDateTime();
  // 计算时间差
  qint64 elapsedTime = startTime.msecsTo(endTime);
  // 打印时间差（以毫秒为单位）
  if (elapsedTime >= 10) {
    mutex.lock();
    if (isRefreshreplot == false) {
      if (isGraphsyAxisAuto == true) {
        this->yAxis->rescale(true);
      }
      if (isGraphsxAxisAuto == true) {
        // 实时显示将其对齐到最大值处
        GraphsxAxisLength =
            this->xAxis->range().upper - this->xAxis->range().lower;
        this->xAxis->setRange(GraphsxAxisEnd, GraphsxAxisLength,
                              Qt::AlignRight);
      }
      refreshPlotTracer();
    } else {
      // 刷新动作
      switch (refreshAction) {
        case eRefreshNone: {
          break;
        }
        case eRefreshData: {
          this->replot(QCustomPlot::rpQueuedReplot);
          refreshAction = eRefreshNone;
          break;
        }
        case eRefreshMouseWhell: {
          refreshAction = eRefreshNone;
          break;
        }
        case eRefreshMouseMove: {
          this->replot(QCustomPlot::rpQueuedReplot);
          refreshAction = eRefreshNone;
          break;
        }
        case eRefreshMousePress: {
          refreshAction = eRefreshNone;
          break;
        }
        case eRefreshButtonPress: {
          refreshAction = eRefreshNone;
          break;
        }
        case eRefreshSaveWaveData: {
          break;
        }
        case eRefreshReadWaveData: {
          break;
        }
      }
    }
    mutex.unlock();
    isRefreshreplot = !isRefreshreplot;
    startTime = QDateTime::currentDateTime();
  }
}

bool TtQCustomPlot::refresh(RefreshActionEnum action) {
  if (refreshAction == eRefreshNone) {
    refreshAction = action;
    return true;
  } else {
    return false;
  }
}

void TtQCustomPlot::selectionChanged() {}

void TtQCustomPlot::axisLabelDoubleClick(QCPAxis* axis,
                                         QCPAxis::SelectablePart part) {}

void TtQCustomPlot::legendDoubleClick(QCPLegend* legend,
                                      QCPAbstractLegendItem* item) {}

void TtQCustomPlot::selectedGraphsColorSet() {}

void TtQCustomPlot::moveLegend() {}

void TtQCustomPlot::addRandomGraph(QColor color) {
  // 添加波形图，随机颜色
  addGraph();
  // 设置名称
  // if (this->pName[this->graphCount() - 1] != nullptr)
  // {
  // 	// this->graph()->setName(this->pName[this->graphCount() - 1]);
  // }

  // 设置数据线形状为直线
  graph()->setLineStyle((QCPGraph::LineStyle::lsLine));
  // 设置数据点为圆形，填充为画笔的颜色
  graph()->setScatterStyle(
      QCPScatterStyle(QCPScatterStyle::ScatterShape::ssDisc,
                      1));  // 数据点
  // 初始清除数据
  graph()->data()->clear();
  QPen graphPen;
  graphPen.setColor(color);
  graphPen.setWidthF(3);
  // 设置颜色
  graph()->setPen(graphPen);
  refresh(eRefreshData);
}

void TtQCustomPlot::removeSelectedGraph() {}

void TtQCustomPlot::removeAllGraphs() {}

void TtQCustomPlot::hideSelectedGraph() {}

void TtQCustomPlot::hideAllGraph() {
  // 隐藏所有图形
  for (int index = 0; index < this->graphCount(); ++index) {
    // 不可视
    this->graph(index)->setVisible(false);
    // 灰色
    this->legend->item(index)->setTextColor(Qt::gray);
  }
  refresh(eRefreshData);
}

void TtQCustomPlot::showAllGraph() {
  // 显示所有图表
  for (int index = 0; index < this->graphCount(); index++) {
    this->graph(index)->setVisible(true);
    this->legend->item(index)->setTextColor(Qt::black);
  }
  refresh(eRefreshData);
}

bool TtQCustomPlot::isAllGraphHide() {
  // 是否所有图形都隐藏
  for (int index = 0; index < this->graphCount(); index++) {
    if (this->graph(index)->visible()) {
      return false;
    }
  }
  return true;
}

bool TtQCustomPlot::ishaveGraphHide() {
  for (int index = 0; index < this->graphCount(); index++) {
    if (!this->graph(index)->visible()) {
      return true;
    }
  }
  return false;
}

void TtQCustomPlot::contextMenuRequest(QPoint pos) {}

void TtQCustomPlot::addGraphs() {
  // 添加图表
  if (refreshGraphsCount > 0) {
    // 先清楚
    clearGraphs();
    lastKeys.resize(refreshGraphsCount);
    for (int igraph = 0; igraph < refreshGraphsCount; igraph++) {
      // 随机添加一个颜色
      addRandomGraph(graphColor[igraph]);

      lastKeys[igraph] = 0;
    }
    // 开始刷新
    isRefreshGraphs = true;
  }
}

void TtQCustomPlot::addGraphsData(const QVector<double>& keys,
                                  const QVector<double>& values) {
  // 向图表添加数据
  // 更新数据长度
  refreshGraphsCount = qMin(keys.size(), values.size());
  if (isRefreshGraphs == false) {
    // 没有处于更新的时候
    return;
  }
  if (refreshGraphsCount > 0) {
    // 保存和导入数据时不刷新
    if ((refreshAction != eRefreshSaveWaveData)) {
      for (int iGraph = 0;
           (iGraph < qMin(this->graphCount(), refreshGraphsCount)); iGraph++) {
        // 更新结尾
        if (keys[iGraph] > lastKeys[iGraph]) {
          if (keys[iGraph] > GraphsxAxisEnd) {
            GraphsxAxisEnd = keys[iGraph];
          }
          this->graph(iGraph)->addData(keys[iGraph], values[iGraph]);
        }
      }
      // 从文件读取，则不使用数据刷新
      if (refreshAction != eRefreshReadWaveData) {
        refresh(eRefreshData);
      }
    }
    lastKeys = keys;
  }
}

void TtQCustomPlot::GraphRefreshSlot() {
  // 定时刷新槽函数
  if (isRefreshTimerEnable == false) {
    return;
  }
  refreshGraphs();
  // if (RefreshGraphs == true)
  // {
  // 	// 删除超过容量的数据
  // 	for (uint32_t iGraph = 0; iGraph < this->graphCount(); iGraph++)
  // 	{
  // 		int length = this->graph(iGraph)->data()->size();
  // 		if (length > GraphsDataMaxLength)
  // 		{
  // 			double removeBefore = this->graph(iGraph)->data()->at(length - GraphsDataMaxLength)->mainKey();
  // 			this->graph(iGraph)->data()->removeBefore(removeBefore);
  // 		}
  // 	}
  // }
}

void TtQCustomPlot::readWaveformData() {
  // 读取波形数据
  // 无波形数据才可以导入
  if (isRefreshGraphs == false) {
    if (refresh(eRefreshReadWaveData)) {
      // 从本地读取数据文件
      QString dirpath = QFileDialog::getOpenFileName(
          this, QStringLiteral("导入波形数据"),
          qApp->applicationDirPath() + "/plot.csv", QString(tr("*.csv")));
      if (!dirpath.isEmpty()) {
        QFile file(dirpath);
        if (!file.open(QIODevice::ReadOnly)) {
          QMessageBox::critical(NULL, tr("提示"), tr("无法打开该文件！"));
        } else {
          QStringList list;
          list.clear();
          // 文本流
          QTextStream csvStream(&file);
          this->clearGraphs();
          // 遍历行 读取一行文本
          QString fileHeadLine = csvStream.readLine();
          // ',' 号分隔 跳过空白符
          // list = fileHeadLine.split(",", QString::SkipEmptyParts);
          list = fileHeadLine.split(",", Qt::SkipEmptyParts);
          // 刷新的图表数目
          refreshGraphsCount = list.size() - 1;
          // 添加曲线
          addGraphs();
          for (int iGraphsData = 0; !csvStream.atEnd(); iGraphsData++) {
            // 读取一行
            QString fileDataLine = csvStream.readLine();
            // ',' 分隔 保留空白符
            QStringList listData =
                // fileDataLine.split(",", QString::KeepEmptyParts);
                fileDataLine.split(",", Qt::KeepEmptyParts);

            // 根据","开分隔开每行的列
            QVector<double> key;
            QVector<double> value;
            // 调整大小 列数大小
            key.resize(listData.size() - 1);
            value.resize(listData.size() - 1);
            for (int iGraphs = 1; iGraphs < listData.size(); iGraphs++) {
              // 遍历列
              // (x, y) ?
              key[iGraphs - 1] = listData[0].toFloat();
              value[iGraphs - 1] = listData[iGraphs].toFloat();
            }
            // 一副图表的数据添加完成
            addGraphsData(key, value);
          }
          // 关闭文件
          file.close();
          // 加载完成
          refreshAction = eRefreshData;
          return;
        }
        // 无法打开文件
        refreshAction = eRefreshNone;
      } else {
        // 不存在文件
        refreshAction = eRefreshNone;
      }
    } else {
      QMessageBox::critical(NULL, tr("提示"), tr("请停止数据刷新操作！"));
      refreshAction = eRefreshNone;
    }
  } else {
    QMessageBox::critical(NULL, tr("提示"),
                          tr("已经存在波形数据，无法导入，请清空波形后导入！"));
    refreshAction = eRefreshNone;
  }
  refreshAction = eRefreshNone;
}

void TtQCustomPlot::saveWaveformData() {
  // 存储波形数据
  // 有无波形数据
  if (isRefreshGraphs == true) {
    // 如果无动作
    if (refresh(eRefreshSaveWaveData)) {
      // 需要操作
      QString dirpath = QFileDialog::getSaveFileName(
          this, QStringLiteral("保存波形数据"),
          qApp->applicationDirPath() + "/plot.csv", QString(tr("*.csv")));
      if (!dirpath.isEmpty()) {
        QFile file(dirpath);
        // 方式：Append为追加，WriteOnly，ReadOnly
        if (!file.open(QIODevice::WriteOnly)) {
          QMessageBox::critical(NULL, tr("提示"), tr("无法创建文件！"));
        } else {
          QTextStream stream(&file);
          {
            // 添加标题栏
            {
              stream << (tr("time(s),"));
              int iGraphIndex = 0;
              // 图表个数
              for (iGraphIndex = 0; iGraphIndex < this->graphCount() - 1;
                   iGraphIndex++) {
                stream << (this->graph(iGraphIndex)->name() + ",");
              }
              // 行
              stream << (this->graph(iGraphIndex)->name() + "\n");
            }
          }
          // 遍历数据
          for (int iGraphData = 0; iGraphData < this->graph(0)->dataCount();
               iGraphData++) {
            // 添加时间轴
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
            stream << (QString::number((this->graph(iGraphIndex)
                                            ->data()
                                            ->at(iGraphData)
                                            ->value),
                                       'f', 6) +
                       "\n");
          }
          refreshAction = eRefreshNone;
          file.close();
          return;
        }
        refreshAction = eRefreshNone;
      } else {
        refreshAction = eRefreshNone;
      }
    } else {
      QMessageBox::critical(NULL, tr("提示"), tr("请停止数据刷新操作！"));
      refreshAction = eRefreshNone;
    }
  } else {
    QMessageBox::critical(NULL, tr("提示"), tr("无任何波形可供保存！"));
    refreshAction = eRefreshNone;
  }
  refreshAction = eRefreshNone;
}

void TtQCustomPlot::saveWaveformImage() {
  if (isRefreshGraphs == true) {
    if (refresh(eRefreshSaveWaveData)) {
      QString dirpath = QFileDialog::getSaveFileName(
          this, QStringLiteral("保存为图片"),
          qApp->applicationDirPath() + "/plot.png", QString(tr("*.png")));
      if (!dirpath.isEmpty()) {
        savePng(dirpath);
        refreshAction = eRefreshNone;
        return;
      } else {
        refreshAction = eRefreshNone;
      }
    } else {
      QMessageBox::critical(NULL, tr("提示"), tr("请停止数据刷新操作！"));
      refreshAction = eRefreshNone;
    }
  } else {
    QMessageBox::critical(NULL, tr("提示"), tr("无任何波形可供保存！"));
    refreshAction = eRefreshNone;
  }
  refreshAction = eRefreshNone;
}

void TtQCustomPlot::wheelEvent(QWheelEvent* event) {}

void TtQCustomPlot::mousePressEvent(QMouseEvent* event) {}

void TtQCustomPlot::mouseMoveEvent(QMouseEvent* event) {}

void TtQCustomPlot::keyPressEvent(QKeyEvent* event) {}

void TtQCustomPlot::keyReleaseEvent(QKeyEvent* event) {}

void TtQCustomPlot::setGraphColor() {
  // 添加32种优化的颜色值到graphColor向量
  graphColor.append(QColor("#1f77b4"));  // 蓝色
  graphColor.append(QColor("#ff7f0e"));  // 橙色
  graphColor.append(QColor("#2ca02c"));  // 绿色
  graphColor.append(QColor("#d62728"));  // 红色
  graphColor.append(QColor("#9467bd"));  // 紫色
  graphColor.append(QColor("#8c564b"));  // 棕色
  graphColor.append(QColor("#e377c2"));  // 粉红色
  graphColor.append(QColor("#7f7f7f"));  // 灰色
  graphColor.append(QColor("#bcbd22"));  // 黄绿色
  graphColor.append(QColor("#17becf"));  // 青色
  graphColor.append(QColor("#9edae5"));  // 淡青色
  graphColor.append(QColor("#98df8a"));  // 淡绿色
  graphColor.append(QColor("#ff9896"));  // 淡红色
  graphColor.append(QColor("#c5b0d5"));  // 淡紫色
  graphColor.append(QColor("#c49c94"));  // 淡棕色
  graphColor.append(QColor("#f7b6d2"));  // 淡粉红色
  graphColor.append(QColor("#dbdb8d"));  // 淡黄绿色
  graphColor.append(QColor("#aec7e8"));  // 淡蓝色
  graphColor.append(QColor("#ffbb78"));  // 淡橙色
  graphColor.append(QColor("#2ca089"));  // 海绿色
  graphColor.append(QColor("#ff4500"));  // 橙红色
  graphColor.append(QColor("#32cd32"));  // 酸橙绿色
  graphColor.append(QColor("#0000ff"));  // 纯蓝色
  graphColor.append(QColor("#daa520"));  // 金色
  graphColor.append(QColor("#808080"));  // 深灰色
  graphColor.append(QColor("#8a2be2"));  // 蓝紫色
  graphColor.append(QColor("#fa8072"));  // 浅肉色
  graphColor.append(QColor("#4682b4"));  // 钢蓝色
  graphColor.append(QColor("#708090"));  // 石板灰
  graphColor.append(QColor("#6a5acd"));  // 石板蓝
  graphColor.append(QColor("#228b22"));  // 森林绿
  graphColor.append(QColor("#d2691e"));  // 巧克力色
}

void TtQCustomPlot::refreshPlotTracer() {
  // 刷新图标跟踪
}

} // namespace Ui
