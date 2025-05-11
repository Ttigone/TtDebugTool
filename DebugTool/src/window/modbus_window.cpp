#include "window/modbus_window.h"

#include "core/modbus_client.h"
#include "ui/controls/TtModbusPlot.h"
#include "ui/controls/TtTableView.h"
#include "ui/widgets/message_bar.h"
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
// #include "ui/controls/TtModbusDelegate.h"
#include "Def.h"
#include "widget/modbus_setting.h"

namespace Window {

ModbusWindow::ModbusWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    // : QWidget(parent), customPlot(new Ui::TtModbusPlot) {
    : FrameWindow(parent), customPlot(new Ui::TtModbusPlot) {
  base::DetectRunningTime runtime;
  init();
  qDebug() << runtime.elapseMilliseconds();

  connectSignals();
  modbus_master_ = new Core::ModbusMaster();
  connect(modbus_master_, &Core::ModbusMaster::errorOccurred, this,
          [this](const QString &error) {
            Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error, 1500,
                                    this);
          });
  // QObject::connect(modbus_master_, &Core::ModbusMaster::dataReceived, this,
  //                  &Window::ModbusWindow::sloveDataReceived);
  QObject::connect(modbus_master_, &Core::ModbusMaster::dataReceived, this,
                   &Window::ModbusWindow::sloveDataReceived);
  // 单独新建链接正常
  // 这个地方失效
  qDebug() << "TEST3";
}

ModbusWindow::~ModbusWindow() { qDebug() << "delete ModbusWindow"; }

QString ModbusWindow::getTitle() const { return title_->text(); }

QJsonObject ModbusWindow::getConfiguration() const { return config_; }

bool ModbusWindow::workState() const {
  // return open
  return false;
}

bool ModbusWindow::saveState() { return saved_; }

void ModbusWindow::setSaveState(bool state) {}

void ModbusWindow::saveSetting() {}

void ModbusWindow::setSetting(const QJsonObject &config) {}

void ModbusWindow::switchToEditMode() {
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
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
void ModbusWindow::sloveDataReceived(const QModbusDataUnit &dataUnit) {

  // 小于 100 ms
  // 修复发送的逻辑
  auto type = dataUnit.registerType();
  switch (type) {
  case QModbusDataUnit::Coils: {
    coil_table_->setValue(dataUnit.startAddress(), dataUnit.values());
    updatePlot(TtModbusRegisterType::Coils, dataUnit.startAddress(),
               dataUnit.values().at(0));

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
    updatePlot(TtModbusRegisterType::HoldingRegisters, dataUnit.startAddress(),
               dataUnit.values().at(0));
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
    return;
  }
  if (coil_table_->rowCount() > 1) {
    auto values = coil_table_->getAddressValue();
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
  Ui::TtHorizontalLayout *tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout *edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  qDebug() << "TEST1";

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &ModbusWindow::switchToEditMode);

  Ui::TtHorizontalLayout *tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout *tmpAll = new Ui::TtHorizontalLayout;

  // 保存 lambda 表达式
  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout *tmpl2 = new Ui::TtHorizontalLayout;
  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);

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
  QSplitter *mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上下分隔器
  QSplitter *VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins(0, 0, 0, 0));

  mainSplitter->addWidget(VSplitter);

  QWidget *modbusRegisterWidget = new QWidget;
  Ui::TtVerticalLayout *modbusRegisterWidgetLayout =
      new Ui::TtVerticalLayout(modbusRegisterWidget);

  function_selection_ = new QTabWidget;

  refresh_btn_ = new Ui::TtSvgButton(":/sys/refresh-normal.svg");
  refresh_btn_->setColors(Qt::black, Qt::blue);
  refresh_btn_->setEnableHoldToCheck(true);
  refresh_btn_->setSvgSize(18, 18);
  function_selection_->setCornerWidget(refresh_btn_, Qt::BottomRightCorner);

  function_selection_->addTab(createCoilWidget(), tr("线圈"));
  function_selection_->addTab(createDiscreteInputsWidget(), tr("离散输入"));
  function_selection_->addTab(createHoldingRegisterWidget(), tr("保持寄存器"));
  function_selection_->addTab(createInputRegisterWidget(), tr("输入寄存器"));

  modbusRegisterWidgetLayout->addWidget(function_selection_);

  // 接收来自 tab 中的 widget
  QStackedWidget *graphWidget = new QStackedWidget();
  graphWidget->addWidget(customPlot);

  VSplitter->addWidget(modbusRegisterWidget);
  VSplitter->addWidget(graphWidget);
  VSplitter->setStretchFactor(0, 3);
  VSplitter->setStretchFactor(1, 2);

  VSplitter->setSizes(QList({400, 200}));

  // 根据不同的角色，选择添加不同的窗口
  if (role_ == TtProtocolType::Client) {
    modbus_client_setting_ = new Widget::ModbusClientSetting;
    mainSplitter->addWidget(modbus_client_setting_);
  } else {
  }

  qDebug() << "TEST2";

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);
  mainSplitter->setStretchFactor(0, 2);
  mainSplitter->setStretchFactor(1, 1);

  // 设置初始尺寸
  QList<int> initialSizes;
  initialSizes << mainSplitter->width() - 200 << 200;
  mainSplitter->setSizes(initialSizes);

  refresh_timer_.setTimerType(Qt::TimerType::PreciseTimer);
}

void ModbusWindow::connectSignals() {
  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    if (modbus_master_->isConnected()) {
      qDebug() << "断开";
      modbus_master_->toDisconnect();
      qDebug() << "test1";
      modbus_client_setting_->setControlState(true);
      qDebug() << "test2";
      refresh_btn_->setEnable(false);
      return;
    }
    modbus_master_->setupConfiguration(
        modbus_client_setting_->getModbusClientConfiguration());
    if (!modbus_master_->isConnected()) {
      bool result = modbus_master_->connectModbusDevice(false);
      if (result) {
        qDebug() << "链接成功";
        on_off_btn_->setChecked(true);
        modbus_client_setting_->setControlState(false);
        refresh_btn_->setEnable(true);
      } else {
        qDebug() << "链接失败";
        on_off_btn_->setChecked(false);
        modbus_client_setting_->setControlState(true);
        refresh_btn_->setEnable(false);
      }
    }
  });

  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    config_.insert("WindowTitile", title_->text());
    if (role_ == TtProtocolType::Client) {
      config_.insert("ModbusClientSetting",
                     modbus_client_setting_->getModbusClientSetting());
    } else if (role_ == TtProtocolType::Server) {
    }
    // config_.insert("InstructionTable", instruction_table_->getTableRecord());
    emit requestSaveConfig();
  });

  QObject::connect(&refresh_timer_, &QTimer::timeout, this,
                   &Window::ModbusWindow::timerRefreshValue);

  connect(modbus_client_setting_,
          &Widget::ModbusClientSetting::refreshIntervalChanged,
          [this](const quint32 &interval) {
            refresh_timer_.setInterval(interval);
          });

  connect(modbus_client_setting_,
          &Widget::ModbusClientSetting::autoRefreshStateChanged,
          [this](bool enable) {
            if (enable) {
              refresh_timer_.start();
            } else {
              refresh_timer_.stop();
            }
          });
  connect(modbus_client_setting_,
          &Widget::ModbusClientSetting::graphNumsChanged, [this](quint16 nums) {
            // 设置图标数量
            customPlot->setGraphsPointCapacity(nums);
          });

  connect(refresh_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // 根据当前的页面，调用对应的 寄存器读取函数
    // 都是通过信号获取, 如何拆分
    auto objName = function_selection_->currentWidget()->objectName();
    if (objName == "Coil") {
      // 写线圈
      getCoilValue();
    } else if (objName == "DiscreteInputs") {

    } else if (objName == "HoldingRegisters") {
      getHoldingRegisterValue();
    } else if (objName == "InputRegisters") {
    }
  });
}

void ModbusWindow::updatePlot(TtModbusRegisterType::Type type, const int &addr,
                              const double &value) {
  if (customPlot) {
    // qDebug() << addr << value;

    // 接收到数据, 会添加图表
    // customPlot->addGraphs(type, addr);
    customPlot->addData(type, addr, value);
    customPlot->setAutoScaleY(true);
    customPlot->setShowTooltip(true);
  }
}

QWidget *ModbusWindow::createCoilWidget() {
  // 线圈
  QWidget *coilsWidget = new QWidget;
  Ui::TtVerticalLayout *coilsWidgetLayout =
      new Ui::TtVerticalLayout(coilsWidget);
  QWidget *bottomWidget = new QWidget;
  Ui::TtHorizontalLayout *bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit *origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit *quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton *plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  coil_table_ =
      new Ui::TtModbusTableWidget(TtModbusRegisterType::Coils, coilsWidget);
  coil_table_->setObjectName("Coil");

  coilsWidgetLayout->addWidget(coil_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  // 写入
  connect(coil_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          [this](const int &address, const int &value) {
            // 写线圈
            modbus_master_->writeCoilsData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      coil_table_, &Ui::TtModbusTableWidget::requestShowGraph,
      [this](TtModbusRegisterType::Type type, const int &addr, bool enable) {
        // qDebug() << "show grah";
        if (!enable) {
          // 清空后, 仍然有残留 plot
          customPlot->removeGraphs(type, addr);
        } else {
          customPlot->addGraphs(type, addr);
        }
      });

  connect(plusButton, &QPushButton::clicked, this,
          [this]() { coil_table_->addRow(); });

  return coilsWidget;
}

QWidget *ModbusWindow::createDiscreteInputsWidget() {

  QWidget *discreteInputsWidget = new QWidget;
  Ui::TtVerticalLayout *discreteInputsWidgetLayout =
      new Ui::TtVerticalLayout(discreteInputsWidget);
  QWidget *bottomWidget = new QWidget;
  Ui::TtHorizontalLayout *bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit *origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit *quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton *plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  discrete_inputs_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::DiscreteInputs, discreteInputsWidget);
  discrete_inputs_table_->setObjectName("DiscreteInputs");

  discreteInputsWidgetLayout->addWidget(discrete_inputs_table_, 1);
  discreteInputsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(discrete_inputs_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          [this](const int &address, const int &value) {
            modbus_master_->writeDiscreteInputsData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      discrete_inputs_table_, &Ui::TtModbusTableWidget::requestShowGraph,
      [this](TtModbusRegisterType::Type type, const int &addr, bool enable) {
        if (!enable) {
          customPlot->removeGraphs(type, addr);
        } else {
          customPlot->addGraphs(type, addr);
        }
      });

  connect(plusButton, &QPushButton::clicked, this,
          [this]() { discrete_inputs_table_->addRow(); });

  return discreteInputsWidget;
}

QWidget *ModbusWindow::createHoldingRegisterWidget() {
  QWidget *holdingRegistersWidget = new QWidget;
  Ui::TtVerticalLayout *coilsWidgetLayout =
      new Ui::TtVerticalLayout(holdingRegistersWidget);

  QWidget *bottomWidget = new QWidget;
  Ui::TtHorizontalLayout *bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit *origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit *quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton *plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  holding_registers_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::HoldingRegisters, holdingRegistersWidget);
  holding_registers_table_->setObjectName("HoldingRegisters");

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
          [this](const int &address, const int &value) {
            modbus_master_->writeHoldingData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      holding_registers_table_, &Ui::TtModbusTableWidget::requestShowGraph,
      [this](TtModbusRegisterType::Type type, const int &addr, bool enable) {
        if (!enable) {
          customPlot->removeGraphs(type, addr);
        } else {
          customPlot->addGraphs(type, addr);
        }
      });

  coilsWidgetLayout->addWidget(holding_registers_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plusButton, &QPushButton::clicked, this,
          [this]() { holding_registers_table_->addRow(); });
  return holdingRegistersWidget;
}

QWidget *ModbusWindow::createInputRegisterWidget() {
  QWidget *inputRegistersWidget = new QWidget;
  Ui::TtVerticalLayout *coilsWidgetLayout =
      new Ui::TtVerticalLayout(inputRegistersWidget);
  QWidget *bottomWidget = new QWidget;
  Ui::TtHorizontalLayout *bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit *origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit *quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton *plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  input_registers_table_ = new Ui::TtModbusTableWidget(
      TtModbusRegisterType::InputRegisters, inputRegistersWidget);
  input_registers_table_->setObjectName("InputRegisters");

  coilsWidgetLayout->addWidget(input_registers_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(input_registers_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          [this](const int &address, const int &value) {
            modbus_master_->writeInputRegistersData(
                address, QVector<quint16>(1, value),
                modbus_client_setting_->getModbusDeviceId());
          });

  connect(
      input_registers_table_, &Ui::TtModbusTableWidget::requestShowGraph,
      [this](TtModbusRegisterType::Type type, const int &addr, bool enable) {
        if (!enable) {
          customPlot->removeGraphs(type, addr);
        } else {
          customPlot->addGraphs(type, addr);
        }
      });

  connect(plusButton, &QPushButton::clicked, this,
          [this]() { input_registers_table_->addRow(); });

  return inputRegistersWidget;
}

} // namespace Window
