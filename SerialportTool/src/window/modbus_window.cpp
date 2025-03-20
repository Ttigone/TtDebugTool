#include "window/modbus_window.h"

#include <ui/control/TableWidget/TtBaseTableWidget.h>
#include <ui/control/TableWidget/TtHeaderView.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/controls/TtModbusDelegate.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>

#include "ui/controls/TtTableView.h"
#include "ui/widgets/message_bar.h"

#include "core/modbus_client.h"
// #include "ui/controls/TtModbusDelegate.h"
#include "widget/modbus_setting.h"

namespace Window {

ModbusWindow::ModbusWindow(TtProtocolType::ProtocolRole role, QWidget* parent) {
  init();
  modbus_master_ = new Core::ModbusMaster();
  connect(modbus_master_, &Core::ModbusMaster::errorOccurred, this,
          [this](const QString& error) {
            Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error, 1500,
                                    this);
          });
  // connect(modbus_master_, &Core::ModbusMaster::dataReceived,
  //         [this](const QVector<quint16>& data) {
  //           qDebug() << "get datas: " << data;
  //           holding_registers_table_->setValue(data);
  //         });
  // QMetaObject::connect(modbus_master_, &Core::ModbusMaster::dataReceived, this,
  //                      QOverload<const int&, const QVector<quint16>&>::of(
  //                          &Window::ModbusWindow::sloveDataReceived));
  QObject::connect(modbus_master_, &Core::ModbusMaster::dataReceived, this,
                   &Window::ModbusWindow::sloveDataReceived);
}

ModbusWindow::~ModbusWindow() {}

QString ModbusWindow::getTitle() const {
  return title_->text();
}

QJsonObject ModbusWindow::getConfiguration() const {
  return config_;
}

void ModbusWindow::switchToEditMode() {
  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  // 预先获取之前的标题
  title_edit_->setText(title_->text());
  // 显示 edit 模式
  stack_->setCurrentWidget(edit_widget_);
  // 获取焦点
  title_edit_->setFocus();  // 自动聚焦输入框
}

void ModbusWindow::switchToDisplayMode() {
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

void ModbusWindow::sloveDataReceived(const int& addr,
                                     const QVector<quint16>& data) {
  // qDebug() << addr << data[0];
  holding_registers_table_->setValue(addr, data);
}

void ModbusWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);
  if (role_ == TtProtocolType::Client) {
    title_ = new Ui::TtNormalLabel(tr("未命名的 Modbus 主机"));
  } else {
    title_ = new Ui::TtNormalLabel(tr("未命名的 Modbus 设备模拟服务"));
  }

  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit_name.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout* tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

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

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &ModbusWindow::switchToEditMode);

  Ui::TtHorizontalLayout* tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout* tmpAll = new Ui::TtHorizontalLayout;

  // 保存 lambda 表达式
  auto handleSave = [this]() {
    //qDebug() << "失去";
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

  QTabWidget* functionSelection = new QTabWidget;

  refresh_btn_ = new Ui::TtSvgButton(":/sys/refresh-normal.svg");
  refresh_btn_->setColors(Qt::black, Qt::blue);
  refresh_btn_->setEnableHoldToCheck(true);
  refresh_btn_->setSvgSize(18, 18);
  functionSelection->setCornerWidget(refresh_btn_, Qt::BottomRightCorner);

  functionSelection->addTab(createCoilWidget(), tr("线圈"));
  functionSelection->addTab(createDiscreteInputsWidget(), tr("离散输入"));
  functionSelection->addTab(createHoldingRegisterWidget(), tr("保持寄存器"));
  functionSelection->addTab(createInputRegisterWidget(), tr("输入寄存器"));

  modbusRegisterWidgetLayout->addWidget(functionSelection);

  QWidget* graphWidget = new QWidget;

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

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);
  mainSplitter->setStretchFactor(0, 2);
  mainSplitter->setStretchFactor(1, 1);

  // 设置初始尺寸
  QList<int> initialSizes;
  initialSizes << mainSplitter->width() - 200 << 200;
  mainSplitter->setSizes(initialSizes);

  connectSignals();
}

void ModbusWindow::connectSignals() {
  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    if (modbus_master_->isConnected()) {
      qDebug() << "断开";
      modbus_master_->toDisconnect();
      modbus_client_setting_->setControlState(true);
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
      } else {
        qDebug() << "链接失败";
        on_off_btn_->setChecked(false);
        modbus_client_setting_->setControlState(true);
      }
    }
  });

  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    config_.insert("WindowTitile", title_->text());
    if (role_ == TtProtocolType::Client) {
      config_.insert("ModbusClientSetting",
                     modbus_client_setting_->getModbusClientSetting());
    } else if (role_ == TtProtocolType::Server) {
      // config_.insert("UdpServerSetting",
      //                mqtt_->getUdpServerSetting());
    }
    // config_.insert("InstructionTable", instruction_table_->getTableRecord());
    emit requestSaveConfig();
  });
}

QWidget* ModbusWindow::createCoilWidget() {
  // 线圈
  QWidget* coilsWidget = new QWidget;
  Ui::TtVerticalLayout* coilsWidgetLayout =
      new Ui::TtVerticalLayout(coilsWidget);
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit* origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit* quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton* plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  auto table = new Ui::TtModbusTableWidget(coilsWidget);

  coilsWidgetLayout->addWidget(table, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plusButton, &QPushButton::clicked, this,
          [this, table]() { table->addRow(); });
  return coilsWidget;
}

QWidget* ModbusWindow::createDiscreteInputsWidget() {

  QWidget* discreteInputsWidget = new QWidget;
  Ui::TtVerticalLayout* coilsWidgetLayout =
      new Ui::TtVerticalLayout(discreteInputsWidget);
  QWidget* bottomWidget = new QWidget;
  Ui::TtHorizontalLayout* bottomWidgetLayout =
      new Ui::TtHorizontalLayout(bottomWidget);
  bottomWidgetLayout->addStretch();
  Ui::TtLabelLineEdit* origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit* quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton* plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  auto table = new Ui::TtModbusTableWidget(discreteInputsWidget);

  coilsWidgetLayout->addWidget(table, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plusButton, &QPushButton::clicked, this,
          [this, table]() { table->addRow(); });
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
  Ui::TtLabelLineEdit* origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit* quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton* plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  holding_registers_table_ =
      new Ui::TtModbusTableWidget(holdingRegistersWidget);
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

  // 读取 / 写入 修改对应框

  // refresh
  connect(refresh_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // 只能一个个读取, 然后按照行逐一返回
    // table 需要根据行, 获取地址, 获取后, 将数据写到对应行
    if (holding_registers_table_->rowCount() > 1) {
      modbus_master_->readHoldingData(
          holding_registers_table_->getAddressValue(), 1);
    }
  });

  connect(holding_registers_table_, &Ui::TtModbusTableWidget::valueConfirmed,
          [this](const int& address, const int& value) {
            qDebug() << "T: " << address << value;
            modbus_master_->writeHoldingData(address, QVector<quint16>(value),
                                             1);
          });

  coilsWidgetLayout->addWidget(holding_registers_table_, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plusButton, &QPushButton::clicked, this,
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
  Ui::TtLabelLineEdit* origin_address_ =
      new Ui::TtLabelLineEdit(tr("起始地址"));
  bottomWidgetLayout->addWidget(origin_address_);
  Ui::TtLabelLineEdit* quantity_ = new Ui::TtLabelLineEdit(tr("数量"));
  bottomWidgetLayout->addWidget(quantity_);
  QPushButton* plusButton = new QPushButton(tr("添加"));
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  plusButton->setIcon(QIcon(":/sys/plus-circle.svg"));
  bottomWidgetLayout->addWidget(plusButton);

  auto table = new Ui::TtModbusTableWidget(inputRegistersWidget);
  coilsWidgetLayout->addWidget(table, 1);
  coilsWidgetLayout->addWidget(bottomWidget, 0, Qt::AlignBottom);

  connect(plusButton, &QPushButton::clicked, this,
          [this, table]() { table->addRow(); });
  return inputRegistersWidget;
}

}  // namespace Window
