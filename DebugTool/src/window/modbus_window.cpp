#include "window/modbus_window.h"

#include <ui/control/TableWidget/TtBaseTableWidget.h>
#include <ui/control/TableWidget/TtHeaderView.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/controls/TtModbusDelegate.h>
#include <ui/controls/TtQCustomPlot.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include "core/modbus_client.h"
#include "ui/controls/TtModbusPlot.h"
#include "ui/controls/TtTableView.h"
#include "ui/widgets/message_bar.h"

// #include "ui/controls/TtModbusDelegate.h"
#include "Def.h"
#include "widget/modbus_setting.h"

namespace Window {

ModbusWindow::ModbusWindow(TtProtocolType::ProtocolRole role, QWidget* parent)
    : FrameWindow(parent),
      role_(role),
      modbus_plot_(new Ui::TtModbusPlot),
      modbus_master_(new Core::ModbusMaster) {
  init();
  connectSignals();

  QTimer::singleShot(0, this, [this]() {
    coil_table_->resizeColumnsToContents();
    coil_table_->resizeRowsToContents();
    discrete_inputs_table_->resizeColumnsToContents();
    discrete_inputs_table_->resizeRowsToContents();
    holding_registers_table_->resizeColumnsToContents();
    holding_registers_table_->resizeRowsToContents();
    input_registers_table_->resizeColumnsToContents();
    input_registers_table_->resizeRowsToContents();
  });
}

ModbusWindow::~ModbusWindow() {
  qDebug() << "delete ModbusWindow";

  worker_thread_.quit();
  worker_thread_.wait();
  qDebug() << "delete ModbusWindow";
}

QString ModbusWindow::getTitle() const {
  return title_->text();
}

QJsonObject ModbusWindow::getConfiguration() const {
  return config_;
}

bool ModbusWindow::workState() const {
  return opened_;
}

bool ModbusWindow::saveState() {
  return saved_;
}

void ModbusWindow::setSaveState(bool state) {
  saved_ = state;
}

void ModbusWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitle", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("ModbusClientSetting",
                   modbus_client_setting_->getModbusClientSetting());
  } else if (role_ == TtProtocolType::Server) {
  }
  // 循环插入所有 function_table_ 中的表格项
  for (auto it = function_table_.constBegin(); it != function_table_.constEnd();
       ++it) {
    auto* widget = qobject_cast<Ui::TtModbusTableWidget*>(it.value());
    config_.insert("InstructionTable+" + TYPE_NAMES.value(it.key()),
                   widget->getTableRecord());
  }
  // saved_ = true;
  setSaveStatus(true);
  emit requestSaveConfig();
}

void ModbusWindow::setSetting(const QJsonObject& config) {
  if (config.isEmpty()) {
    return;
  }
  // 设置后
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  modbus_client_setting_->setOldSettings(
      config.value("ModbusClientSetting").toObject(QJsonObject()));
  if (coil_table_ && config.contains("InstructionTable+Coil")) {
    QJsonObject coilTableData =
        config.value("InstructionTable+Coil").toObject();
    if (!coilTableData.isEmpty()) {
      qDebug() << "还原线圈表格数据";
      coil_table_->setTable(coilTableData);
    }
  }
  if (discrete_inputs_table_ &&
      config.contains("InstructionTable+DiscreteInputs")) {
    QJsonObject discreteInputsTableData =
        config.value("InstructionTable+DiscreteInputs").toObject();
    if (!discreteInputsTableData.isEmpty()) {
      qDebug() << "还原离散输入表格数据";
      discrete_inputs_table_->setTable(discreteInputsTableData);
    }
  }
  if (holding_registers_table_ &&
      config.contains("InstructionTable+HoldingRegisters")) {
    QJsonObject holdingRegistersTableData =
        config.value("InstructionTable+HoldingRegisters").toObject();
    if (!holdingRegistersTableData.isEmpty()) {
      qDebug() << "还原保持寄存器表格数据";
      holding_registers_table_->setTable(holdingRegistersTableData);
    }
  }
  if (input_registers_table_ &&
      config.contains("InstructionTable+InputRegisters")) {
    QJsonObject inputRegistersTableData =
        config.value("InstructionTable+InputRegisters").toObject();
    if (!inputRegistersTableData.isEmpty()) {
      qDebug() << "还原输入寄存器表格数据";
      input_registers_table_->setTable(inputRegistersTableData);
    }
  }
  // 在所有表格加载完成后调整大小
  QTimer::singleShot(0, this, [this]() {
    coil_table_->resizeColumnsToContents();
    coil_table_->resizeRowsToContents();
    discrete_inputs_table_->resizeColumnsToContents();
    discrete_inputs_table_->resizeRowsToContents();
    holding_registers_table_->resizeColumnsToContents();
    holding_registers_table_->resizeRowsToContents();
    input_registers_table_->resizeColumnsToContents();
    input_registers_table_->resizeRowsToContents();
  });
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
  setSaveStatus(true);
}

void ModbusWindow::switchToEditMode() {
  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  title_edit_->setText(title_->text());
  stack_->setCurrentWidget(edit_widget_);
  title_edit_->setFocus();
}

void ModbusWindow::switchToDisplayMode() {
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

// void ModbusWindow::sloveDataReceived(const int& addr,
//                                      const QVector<quint16>& data) {
void ModbusWindow::sloveDataReceived(const QModbusDataUnit& dataUnit) {
  // 小于 100 ms
  // 修复发送的逻辑
  auto type = dataUnit.registerType();
  switch (type) {
    case QModbusDataUnit::Coils: {
      // 处理
      coil_table_->setValue(dataUnit.startAddress(), dataUnit.values());
      // 显示在 plot 上
      updatePlot(TtModbusRegisterType::Coils, dataUnit.startAddress(),
                 dataUnit.values().at(0));

      // 发送
      break;
    }
    case QModbusDataUnit::DiscreteInputs: {
      discrete_inputs_table_->setValue(dataUnit.startAddress(),
                                       dataUnit.values());
      updatePlot(TtModbusRegisterType::DiscreteInputs, dataUnit.startAddress(),
                 dataUnit.values().at(0));
      break;
    }
    case QModbusDataUnit::HoldingRegisters: {
      holding_registers_table_->setValue(dataUnit.startAddress(),
                                         dataUnit.values());
      updatePlot(TtModbusRegisterType::HoldingRegisters,
                 dataUnit.startAddress(), dataUnit.values().at(0));
      break;
    }
    case QModbusDataUnit::InputRegisters: {
      qDebug() << "input";
      input_registers_table_->setValue(dataUnit.startAddress(),
                                       dataUnit.values());
      updatePlot(TtModbusRegisterType::InputRegisters, dataUnit.startAddress(),
                 dataUnit.values().at(0));
      break;
    }
    case QModbusDataUnit::Invalid: {
      // 无效类型
      break;
    }
  }
}

void ModbusWindow::timerRefreshValue() {
  // 定时器刷新
  if (!modbus_master_->isConnected()) {
    return;
  }
  getCoilValue();
  getDiscreteInputsValue();
  getHoldingRegisterValue();
  getInputRegistersValue();
}

void ModbusWindow::getSpecificValue() {}

void ModbusWindow::getHoldingRegisterValue() {
  if (!modbus_master_->isConnected()) {
    return;
  }
  if (holding_registers_table_->rowCount() > 1) {
    auto values = holding_registers_table_->getAddressValue();
    modbus_master_->readHoldingData(values, 1);
  }
}

void ModbusWindow::getCoilValue() {
  if (!modbus_master_->isConnected()) {
    qDebug() << "modbus 为链接";
    return;
  }
  if (coil_table_->rowCount() > 1) {
    // 获取地址长度
    // base::DetectRunningTime test;
    auto values = coil_table_->getAddressValue();
    // 输出为 0
    // qDebug() << "get address time: " << test.elapseMilliseconds();
    // qDebug() << "get address: " << values;
    modbus_master_->readCoilsData(values, 1);
  }
}

void ModbusWindow::getDiscreteInputsValue() {
  if (!modbus_master_->isConnected()) {
    return;
  }
  if (discrete_inputs_table_->rowCount() > 1) {
    auto values = discrete_inputs_table_->getAddressValue();
    modbus_master_->readDiscreteInputsData(values, 1);
  }
}

void ModbusWindow::getInputRegistersValue() {
  if (!modbus_master_->isConnected()) {
    return;
  }
  if (input_registers_table_->rowCount() > 1) {
    auto values = input_registers_table_->getAddressValue();
    modbus_master_->readInputRegistersData(values, 1);
  }
}

void ModbusWindow::onDisconnectStarted() {
  // 创建并显示进度对话框
  if (!disconnect_progress_) {
    disconnect_progress_ = new QProgressDialog(tr("正在断开Modbus连接..."),
                                               QString(), 0, 100, this);
    disconnect_progress_->setWindowModality(Qt::WindowModal);
    disconnect_progress_->setMinimumDuration(200);   // 200ms后再显示
    disconnect_progress_->setCancelButton(nullptr);  // 不允许取消
    disconnect_progress_->setAutoClose(true);
    disconnect_progress_->setWindowTitle(tr("断开连接"));
  }

  disconnect_progress_->setValue(0);
  disconnect_progress_->show();
}

void ModbusWindow::updateDisconnectProgress(int percent) {
  if (disconnect_progress_) {
    disconnect_progress_->setValue(percent);
  }
}

void ModbusWindow::onDisconnectFinished() {
  // 关闭进度对话框
  if (disconnect_progress_) {
    disconnect_progress_->setValue(100);
    disconnect_progress_->close();
  }

  // 更新UI状态
  setControlState(true);
  opened_ = false;
  on_off_btn_->setChecked(false);
}

void ModbusWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);
  if (role_ == TtProtocolType::Client) {
    title_ = new Ui::TtNormalLabel(tr("未命名的 Modbus 主机"));
  } else {
    title_ = new Ui::TtNormalLabel(tr("未命名的 Modbus 设备模拟服务"));
  }

  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout* originalWidgetLayout =
      new Ui::TtHorizontalLayout(original_widget_);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(title_, 0, Qt::AlignLeft);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(modify_title_btn_);
  originalWidgetLayout->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout* edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  // qDebug() << "TEST1";

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &ModbusWindow::switchToEditMode);

  Ui::TtHorizontalLayout* tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout* tmpAll = new Ui::TtHorizontalLayout;

  // 保存 lambda 表达式
  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout* tmpl2 = new Ui::TtHorizontalLayout;
  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);
  // 默认显示蓝色
  save_btn_->setColors(QColor("#2196F3"), QColor("#2196F3"));

  // 开关按钮
  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setSvgSize(18, 18);
  on_off_btn_->setColors(Qt::black, Qt::red);

  tmpl2->addWidget(save_btn_);
  tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  tmpAll->addLayout(tmpP1);
  tmpAll->addLayout(tmpl2);

  main_layout_->addLayout(tmpAll);

  // 左右分隔器
  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上下分隔器
  QSplitter* VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins(0, 0, 0, 0));

  mainSplitter->addWidget(VSplitter);

  QWidget* modbusRegisterWidget = new QWidget;
  Ui::TtVerticalLayout* modbusRegisterWidgetLayout =
      new Ui::TtVerticalLayout(modbusRegisterWidget);

  // 多标签选择
  function_selection_ = new QTabWidget;

  refresh_btn_ = new Ui::TtSvgButton(":/sys/refresh-normal.svg");
  refresh_btn_->setColors(Qt::black, Qt::blue);
  refresh_btn_->setEnableHoldToCheck(true);
  refresh_btn_->setSvgSize(18, 18);
  function_selection_->setCornerWidget(refresh_btn_, Qt::BottomRightCorner);

  // 对应的 wiget 处于不可选择状态
  // 内部创建多个 widget
  function_selection_->addTab(createCoilWidget(), tr("线圈"));
  function_selection_->addTab(createDiscreteInputsWidget(), tr("离散输入"));
  function_selection_->addTab(createHoldingRegisterWidget(), tr("保持寄存器"));
  function_selection_->addTab(createInputRegisterWidget(), tr("输入寄存器"));

  modbusRegisterWidgetLayout->addWidget(function_selection_);

  // 接收来自 tab 中的 widget
  QStackedWidget* graphWidget = new QStackedWidget();
  graphWidget->addWidget(modbus_plot_.data());

  VSplitter->addWidget(modbusRegisterWidget);
  VSplitter->addWidget(graphWidget);
  VSplitter->setStretchFactor(0, 3);
  VSplitter->setStretchFactor(1, 2);
  VSplitter->setCollapsible(1, false);

  VSplitter->setSizes(QList({400, 200}));

  // 根据不同的角色，选择添加不同的窗口
  if (role_ == TtProtocolType::Client) {
    modbus_client_setting_ = new Widget::ModbusClientSetting;
    mainSplitter->addWidget(modbus_client_setting_);
  } else {
  }

  // qDebug() << "TEST2";

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);
  mainSplitter->setStretchFactor(0, 2);
  mainSplitter->setStretchFactor(1, 1);
  mainSplitter->setCollapsible(0, false);

  // 设置初始尺寸
  QList<int> initialSizes;
  initialSizes << mainSplitter->width() - 200 << 200;
  mainSplitter->setSizes(initialSizes);

  refresh_timer_.setTimerType(Qt::TimerType::PreciseTimer);
  refresh_timer_.setInterval(100);
  setControlState(true);

  // 初始化工作线程
  modbus_worker_ = new Core::ModbusWorker();
  modbus_worker_->moveToThread(&worker_thread_);
  modbus_worker_->setModbusMaster(modbus_master_.data());

  // 连接信号
  connect(&worker_thread_, &QThread::finished, modbus_worker_,
          &QObject::deleteLater);
  connect(modbus_worker_, &Core::ModbusWorker::disconnectStarted, this,
          &ModbusWindow::onDisconnectStarted);
  connect(modbus_worker_, &Core::ModbusWorker::disconnectFinished, this,
          &ModbusWindow::onDisconnectFinished);
  connect(modbus_worker_, &Core::ModbusWorker::disconnectProgress, this,
          &ModbusWindow::updateDisconnectProgress);

  // 启动工作线程
  worker_thread_.start();
}

void ModbusWindow::connectSignals() {
  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this] {
    // 为什么每次点击都会卡顿, 是否需要移动到线程总执行操作
    if (modbus_master_->isConnected()) {
      base::DetectRunningTime timer;
      qDebug() << "关闭链接";
      // 关闭
      // 链接长时间, 导致卡顿界面
      // modbus_master_->toDisconnect();
      // 后台更新
      QMetaObject::invokeMethod(modbus_worker_, "disconnectModbus",
                                Qt::QueuedConnection);
      qDebug() << timer.elapseMilliseconds();
      // 这里卡顿
      setControlState(true);
      opened_ = false;
      // 912ms
      return;
    } else {
      qDebug() << "开启链接";
      // 设置参数
      modbus_master_->setupConfiguration(
          modbus_client_setting_->getModbusClientConfiguration());
      bool result = modbus_master_->connectModbusDevice(false);
      if (result) {
        qDebug() << "链接成功";
        on_off_btn_->setChecked(true);
        setControlState(false);
        opened_ = true;
      } else {
        qDebug() << "链接失败";
        on_off_btn_->setChecked(false);
        setControlState(true);
        opened_ = false;
      }
    }
  });

  connect(save_btn_, &Ui::TtSvgButton::clicked, this,
          &ModbusWindow::saveSetting);

  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {});

  QObject::connect(&refresh_timer_, &QTimer::timeout, this,
                   &Window::ModbusWindow::timerRefreshValue);
  // 设定刷新的时间间隔
  if (modbus_client_setting_) {
    connect(modbus_client_setting_,
            &Widget::ModbusClientSetting::refreshIntervalChanged, this,
            [this](uint32_t interval) {
              setSaveStatus(false);
              refresh_timer_.setInterval(interval);
            });

    connect(modbus_client_setting_,
            &Widget::ModbusClientSetting::autoRefreshStateChanged, this,
            [this](bool enable) {
              setSaveStatus(false);
              if (enable) {
                refresh_timer_.start();
              } else {
                refresh_timer_.stop();
              }
            });
    connect(modbus_client_setting_,
            &Widget::ModbusClientSetting::graphNumsChanged, this,
            [this](quint16 nums) {
              // 设置图标数量
              setSaveStatus(false);
              modbus_plot_->setGraphsPointCapacity(nums);
            });
    connect(modbus_client_setting_,
            &Widget::ModbusClientSetting::settingChanged, this,
            [this] { setSaveStatus(false); });

  } else {
    qDebug() << "modbus_client_setting_ is nullptr";
  }

  connect(refresh_btn_, &Ui::TtSvgButton::clicked, this, [this]() {
    // BUG 根据当前的页面，调用对应的 寄存器读取函数
    // 都是通过信号获取, 如何拆分
    auto objName = function_selection_->currentWidget()->objectName();
    if (objName == "Coil") {
      // 写线圈
      getCoilValue();
    } else if (objName == "DiscreteInputs") {
      getDiscreteInputsValue();
    } else if (objName == "HoldingRegisters") {
      getHoldingRegisterValue();
    } else if (objName == "InputRegisters") {
      getInputRegistersValue();
    }
  });
  connect(modbus_master_.data(), &Core::ModbusMaster::errorOccurred, this,
          [this](const QString& error) {
            Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error, 1500,
                                    this);
            setSaveStatus(false);
            setControlState(true);
          });
  QObject::connect(modbus_master_.data(), &Core::ModbusMaster::dataReceived,
                   this, &Window::ModbusWindow::sloveDataReceived);

  connect(this, &FrameWindow::savedChanged, this, [this](bool saved) {
    if (saved) {
      save_btn_->setColors(QColor(33, 150, 243), QColor(33, 150, 243));
      save_btn_->setToolTip(tr("配置已保存"));
    } else {
      save_btn_->setColors(QColor(244, 64, 54), QColor(244, 64, 54));
      save_btn_->setToolTip(tr("有未保存的更改，点击保存"));
    }
  });
}

void ModbusWindow::setControlState(bool state) {
  // 设置为 true
  if (role_ == TtProtocolType::Client) {
    modbus_client_setting_->setEnabled(state);
  }
  refresh_btn_->setEnable(!state);
  setFunctionTableState(state);
  for (auto* address : origin_address_) {
    address->setEnabled(state);
  }
  for (auto* quantity : quantity_) {
    quantity->setEnabled(state);
  }
  for (auto* btn : plus_btn_) {
    btn->setEnabled(state);
  }
  if (state) {
    refresh_timer_.stop();
  } else {
    refresh_timer_.start();
  }
}

void ModbusWindow::updatePlot(TtModbusRegisterType::Type type, const int& addr,
                              const double& value) {
  // BUG 如果存在相同的地址, 会被当作同一条曲线, 出了地址, 还有所在行
  // 同一地址
  if (modbus_plot_) {
    // qDebug() << addr << value;
    // 接收到数据, 会添加图表
    // modbus_plot_->addGraphs(type, addr);
    modbus_plot_->addData(type, addr, value);
    modbus_plot_->setAutoScaleY(true);
    modbus_plot_->setShowTooltip(true);
  }
}

void ModbusWindow::setFunctionTableState(bool state) {
  for (auto it = function_table_.constBegin(); it != function_table_.constEnd();
       ++it) {
    Ui::TtModbusTableWidget* widget = it.value();
    widget->setEnable(state);
  }
}

QWidget* ModbusWindow::createCoilWidget() {
  // 上部操作窗口
  QWidget* coilsWidget = new QWidget;
  Ui::TtVerticalLayout* coilsWidgetLayout =
      new Ui::TtVerticalLayout(coilsWidget);
  // 底部的按钮操作栏
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  origin_address_.append(new Ui::TtLabelLineEdit(tr("起始地址"), this));
  quantity_.append(new Ui::TtLabelLineEdit(tr("数量"), this));
  plus_btn_.append(new QPushButton(tr("添加"), this));
  plus_btn_.at(0)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plus_btn_.at(0)->setIcon(QIcon(":/sys/plus-circle.svg"));

  bottomWidgetLayout->addWidget(origin_address_.at(0));
  bottomWidgetLayout->addWidget(quantity_.at(0));
  bottomWidgetLayout->addWidget(plus_btn_.at(0));

  coil_table_ =
      new Ui::TtModbusTableWidget(TtModbusRegisterType::Coils, coilsWidget);
  coil_table_->setObjectName("Coil");
  function_table_.insert(TtModbusRegisterType::Coils, coil_table_);

  coilsWidgetLayout->addWidget(coil_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  // 数据请求写入
  connect(coil_table_, &Ui::TtModbusTableWidget::valueConfirmed, this,
          [this](const int& address, const int& value) {
            if (!modbus_client_setting_) {
              qWarning()
                  << "modbus_client_setting_ is null, cannot write coils data";
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr("错误"),
                                      tr("设备未配置或不支持此操作"), 2000,
                                      this);
              return;
            }
            if (modbus_master_->isConnected()) {
              // 设备 id 当主机的 id ???
              modbus_master_->writeCoilsData(
                  address, QVector<quint16>(1, value),
                  modbus_client_setting_->getModbusDeviceId());
            }
          });
  // 不同通道的数据显示在图标上
  connect(
      coil_table_, &Ui::TtModbusTableWidget::requestShowGraph, this,
      [this](TtModbusRegisterType::Type type, const int& addr, bool enable) {
        // qDebug() << "show grah";
        if (!enable) {
          modbus_plot_->removeGraphs(type, addr);
        } else {
          modbus_plot_->addGraphs(type, addr);
        }
      });

  connect(plus_btn_.at(0), &QPushButton::clicked, this,
          [this]() { coil_table_->addRow(); });

  return coilsWidget;
}

QWidget* ModbusWindow::createDiscreteInputsWidget() {
  QWidget* discreteInputsWidget = new QWidget;
  Ui::TtVerticalLayout* discreteInputsWidgetLayout =
      new Ui::TtVerticalLayout(discreteInputsWidget);
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();

  origin_address_.append(new Ui::TtLabelLineEdit(tr("起始地址"), this));
  quantity_.append(new Ui::TtLabelLineEdit(tr("数量"), this));
  plus_btn_.append(new QPushButton(tr("添加"), this));
  plus_btn_.at(1)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plus_btn_.at(1)->setIcon(QIcon(":/sys/plus-circle.svg"));

  bottomWidgetLayout->addWidget(origin_address_.at(1));
  bottomWidgetLayout->addWidget(quantity_.at(1));
  bottomWidgetLayout->addWidget(plus_btn_.at(1));

  discrete_inputs_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::DiscreteInputs, discreteInputsWidget);
  discrete_inputs_table_->setObjectName("DiscreteInputs");

  function_table_.insert(TtModbusRegisterType::DiscreteInputs,
                         discrete_inputs_table_);

  discreteInputsWidgetLayout->addWidget(discrete_inputs_table_, 1);
  discreteInputsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(discrete_inputs_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          this, [this](const int& address, const int& value) {
            modbus_master_->writeDiscreteInputsData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      discrete_inputs_table_, &Ui::TtModbusTableWidget::requestShowGraph, this,
      [this](TtModbusRegisterType::Type type, const int& addr, bool enable) {
        if (!enable) {
          modbus_plot_->removeGraphs(type, addr);
        } else {
          modbus_plot_->addGraphs(type, addr);
        }
      });

  connect(plus_btn_.at(1), &QPushButton::clicked, this,
          [this]() { discrete_inputs_table_->addRow(); });
  return discreteInputsWidget;
}

QWidget* ModbusWindow::createHoldingRegisterWidget() {
  QWidget* holdingRegistersWidget = new QWidget;
  Ui::TtVerticalLayout* coilsWidgetLayout =
      new Ui::TtVerticalLayout(holdingRegistersWidget);
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();

  origin_address_.append(new Ui::TtLabelLineEdit(tr("起始地址"), this));
  quantity_.append(new Ui::TtLabelLineEdit(tr("数量"), this));
  plus_btn_.append(new QPushButton(tr("添加"), this));
  plus_btn_.at(2)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plus_btn_.at(2)->setIcon(QIcon(":/sys/plus-circle.svg"));

  bottomWidgetLayout->addWidget(origin_address_.at(2));
  bottomWidgetLayout->addWidget(quantity_.at(2));
  bottomWidgetLayout->addWidget(plus_btn_.at(2));

  holding_registers_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::HoldingRegisters, holdingRegistersWidget);
  holding_registers_table_->setObjectName("HoldingRegisters");
  function_table_.insert(TtModbusRegisterType::HoldingRegisters,
                         holding_registers_table_);

  // auto table = new QTableView;
  // 创建模型和委托实例
  // Ui::TableModel* model = new Ui::TableModel(this);
  // 创建模型并设置给表格
  // auto model = new QStandardItemModel(this);

  // 添加示例数据
  // model->addRow(Ui::TableModel::RowData{true, "命令1", "HEX", "55AA", 100});
  // model->addRow(
  //     Ui::TableModel::RowData{true, "测试命令", "TEXT", "Hello", 500});

  // Ui::TableDelegate* delegate = new Ui::TableDelegate();
  // table->setEditTriggers(QAbstractItemView::DoubleClicked |
  //                        QAbstractItemView::EditKeyPressed |
  //                        QAbstractItemView::AnyKeyPressed);

  // // 设置模型和委托
  // // table->setItemDelegate(delegate);
  // table->setModel(model);
  // table->setItemDelegate(new Ui::TableDelegate(this));
  // table->setModel(new Ui::TableModel);

  // 写入
  connect(holding_registers_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          this, [this](const int& address, const int& value) {
            modbus_master_->writeHoldingData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      holding_registers_table_, &Ui::TtModbusTableWidget::requestShowGraph,
      this,
      [this](TtModbusRegisterType::Type type, const int& addr, bool enable) {
        if (!enable) {
          modbus_plot_->removeGraphs(type, addr);
        } else {
          modbus_plot_->addGraphs(type, addr);
        }
      });

  coilsWidgetLayout->addWidget(holding_registers_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plus_btn_.at(2), &QPushButton::clicked, this,
          [this]() { holding_registers_table_->addRow(); });
  return holdingRegistersWidget;
}

QWidget* ModbusWindow::createInputRegisterWidget() {
  QWidget* inputRegistersWidget = new QWidget;
  Ui::TtVerticalLayout* coilsWidgetLayout =
      new Ui::TtVerticalLayout(inputRegistersWidget);
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();

  origin_address_.append(new Ui::TtLabelLineEdit(tr("起始地址"), this));
  quantity_.append(new Ui::TtLabelLineEdit(tr("数量"), this));
  plus_btn_.append(new QPushButton(tr("添加"), this));
  plus_btn_.at(3)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plus_btn_.at(3)->setIcon(QIcon(":/sys/plus-circle.svg"));

  bottomWidgetLayout->addWidget(origin_address_.at(3));
  bottomWidgetLayout->addWidget(quantity_.at(3));
  bottomWidgetLayout->addWidget(plus_btn_.at(3));

  input_registers_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::InputRegisters, inputRegistersWidget);
  input_registers_table_->setObjectName("InputRegisters");
  function_table_.insert(TtModbusRegisterType::InputRegisters,
                         input_registers_table_);

  coilsWidgetLayout->addWidget(input_registers_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(input_registers_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          this, [this](const int& address, const int& value) {
            modbus_master_->writeInputRegistersData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      input_registers_table_, &Ui::TtModbusTableWidget::requestShowGraph, this,
      [this](TtModbusRegisterType::Type type, const int& addr, bool enable) {
        if (!enable) {
          modbus_plot_->removeGraphs(type, addr);
        } else {
          modbus_plot_->addGraphs(type, addr);
        }
      });

  connect(plus_btn_.at(3), &QPushButton::clicked, this,
          [this]() { input_registers_table_->addRow(); });

  return inputRegistersWidget;
}

}  // namespace Window
