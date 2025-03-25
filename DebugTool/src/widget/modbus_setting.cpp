#include "widget/modbus_setting.h"

#include <QSerialPortInfo>

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

#include "core/modbus_client.h"

namespace Widget {

ModbusClientSetting::ModbusClientSetting(QWidget* parent) {
  init();
}

ModbusClientSetting::~ModbusClientSetting() {}

Core::ModbusMasterConfiguration
ModbusClientSetting::getModbusClientConfiguration() {
  return Core::ModbusMasterConfiguration(
      link_type_->currentData().value<TtModbusProcotol::Type>(),
      path_->currentData().toString(),
      baud_->currentData().value<QSerialPort::BaudRate>(),
      data_bit_->currentData().value<QSerialPort::DataBits>(),
      parity_bit_->currentData().value<QSerialPort::Parity>(),
      stop_bit_->currentData().value<QSerialPort::StopBits>(),
      device_id_->currentText());
}

const QJsonObject& ModbusClientSetting::getModbusClientSetting() {
  auto config = getModbusClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("LinkType", QJsonValue(config.type));
  linkSetting.insert("Path", QJsonValue(config.com));
  linkSetting.insert("BaudRate", QJsonValue(config.baud_rate));
  linkSetting.insert("DataBit", QJsonValue(config.data_bits));
  linkSetting.insert("ParityBit", QJsonValue(config.parity));
  linkSetting.insert("StopBit", QJsonValue(config.stop_bits));
  linkSetting.insert("Host", QJsonValue(host_->currentText()));
  linkSetting.insert("Port", QJsonValue(port_->currentText()));
  linkSetting.insert("DeviceId", QJsonValue(device_id_->currentText()));
  linkSetting.insert("Timeout", QJsonValue(timeout_->currentText()));
  linkSetting.insert("EnableAutoRefresh",
                     QJsonValue(auto_refresh_->isChecked()));
  linkSetting.insert("RefreshInterval",
                     QJsonValue(refresh_interval_->currentText()));
  modbus_client_save_config_.insert("LinkSetting", QJsonValue(linkSetting));
  return modbus_client_save_config_;
}

TtModbusProcotol::Type ModbusClientSetting::getModbusLinkType() const {
  return link_type_->currentData().value<TtModbusProcotol::Type>();
}

int ModbusClientSetting::getModbusDeviceId() const {
  return device_id_->currentText().toInt();
}

void ModbusClientSetting::setLinkType() {
  link_type_->addItem(tr("RTU"), TtModbusProcotol::RTU);
  link_type_->addItem(tr("RTU-ASCll"), TtModbusProcotol::RTU_ASCLL);
  link_type_->addItem(tr("TCP"), TtModbusProcotol::TCP);
  link_type_->addItem(tr("UDP"), TtModbusProcotol::UDP);
}

void ModbusClientSetting::setSerialPortsName() {
  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  path_->body()->clear();

  for (const QSerialPortInfo& portInfo : serialPortInfos) {
    QString portName = (portInfo.portName() + "-" + portInfo.description());
    path_->addItem(portName, portInfo.portName());
  }
  path_->body()->model()->sort(0);
}

void ModbusClientSetting::setSerialPortsBaudRate() {
  foreach (int64 baud_rate, QSerialPortInfo::standardBaudRates()) {
    baud_->addItem(QString::number(baud_rate), baud_rate);
  }
  baud_->body()->setCurrentText("9600");
}

void ModbusClientSetting::setSerialPortsDataBit() {
  data_bit_->addItem(QString::number(QSerialPort::DataBits::Data5),
                     QSerialPort::DataBits::Data5);
  data_bit_->addItem(QString::number(QSerialPort::DataBits::Data6),
                     QSerialPort::DataBits::Data6);
  data_bit_->addItem(QString::number(QSerialPort::DataBits::Data7),
                     QSerialPort::DataBits::Data7);
  data_bit_->addItem(QString::number(QSerialPort::DataBits::Data8),
                     QSerialPort::DataBits::Data8);
  data_bit_->body()->model()->sort(0, Qt::SortOrder::DescendingOrder);
  data_bit_->setCurrentItem(0);
}

void ModbusClientSetting::setSerialPortsParityBit() {
  parity_bit_->addItem(tr("无校验"), QSerialPort::NoParity);
  parity_bit_->addItem(tr("偶校验"), QSerialPort::EvenParity);
  parity_bit_->addItem(tr("奇校验"), QSerialPort::OddParity);
  parity_bit_->addItem(tr("0 校验"), QSerialPort::SpaceParity);
  parity_bit_->addItem(tr("1 校验"), QSerialPort::MarkParity);
}

void ModbusClientSetting::setSerialPortsStopBit() {
  stop_bit_->addItem(tr("1 位"), QSerialPort::OneStop);
  stop_bit_->addItem(tr("1.5 位"), QSerialPort::OneAndHalfStop);
  stop_bit_->addItem(tr("2 位"), QSerialPort::TwoStop);
}

void ModbusClientSetting::setControlState(bool state) {
  link_type_->setEnabled(state);
  path_->setEnabled(state);
  port_->setEnabled(state);
  baud_->setEnabled(state);
  data_bit_->setEnabled(state);
  parity_bit_->setEnabled(state);
  stop_bit_->setEnabled(state);
  device_id_->setEnabled(state);
  timeout_->setEnabled(state);
  auto_refresh_->setEnabled(state);
  refresh_interval_->setEnabled(state);
}

quint32 ModbusClientSetting::getRefreshInterval() {
  return refresh_interval_->currentText().toULong();
}

void ModbusClientSetting::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  QWidget* linkSettingWidget = new QWidget(this);
  Ui::TtVerticalLayout* linkSettingWidgetLayout =
      new Ui::TtVerticalLayout(linkSettingWidget);
  link_type_ = new Ui::TtLabelComboBox(tr("连接类型:"), linkSettingWidget);

  path_ = new Ui::TtLabelBtnComboBox(tr("路径:"), linkSettingWidget);
  baud_ = new Ui::TtLabelComboBox(tr("波特率:"), linkSettingWidget);
  data_bit_ = new Ui::TtLabelComboBox(tr("数据位:"), linkSettingWidget);
  parity_bit_ = new Ui::TtLabelComboBox(tr("校验位:"), linkSettingWidget);
  stop_bit_ = new Ui::TtLabelComboBox(tr("停止位:"), linkSettingWidget);

  host_ = new Ui::TtLabelLineEdit(tr("主机:"), linkSettingWidget);
  port_ = new Ui::TtLabelLineEdit(tr("端口:"), linkSettingWidget);

  device_id_ = new Ui::TtLabelLineEdit(tr("设备 ID:"), linkSettingWidget);
  timeout_ = new Ui::TtLabelLineEdit(tr("超时时间"), linkSettingWidget);
  auto_refresh_ = new Ui::TtSwitchButton(tr("自动刷新"), linkSettingWidget);
  refresh_interval_ =
      new Ui::TtLabelLineEdit(tr("刷新间隔"), linkSettingWidget);

  linkSettingWidgetLayout->addWidget(link_type_);

  linkSettingWidgetLayout->addWidget(path_);
  linkSettingWidgetLayout->addWidget(baud_);
  linkSettingWidgetLayout->addWidget(data_bit_);
  linkSettingWidgetLayout->addWidget(parity_bit_);
  linkSettingWidgetLayout->addWidget(stop_bit_);

  linkSettingWidgetLayout->addWidget(host_);
  linkSettingWidgetLayout->addWidget(port_);

  linkSettingWidgetLayout->addWidget(device_id_);
  linkSettingWidgetLayout->addWidget(timeout_);
  linkSettingWidgetLayout->addWidget(auto_refresh_);
  linkSettingWidgetLayout->addWidget(refresh_interval_);
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), linkSettingWidget);

  // 界面切换
  connect(link_type_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawer1](int index) {
            if (index == 0 || index == 1) {
              path_->setVisible(true);
              baud_->setVisible(true);
              data_bit_->setVisible(true);
              parity_bit_->setVisible(true);
              stop_bit_->setVisible(true);

              host_->setVisible(false);
              port_->setVisible(false);
            } else {
              path_->setVisible(false);
              baud_->setVisible(false);
              data_bit_->setVisible(false);
              parity_bit_->setVisible(false);
              stop_bit_->setVisible(false);

              host_->setVisible(true);
              port_->setVisible(true);
            }
            const auto event =
                new QResizeEvent(drawer1->size(), drawer1->size());
            QCoreApplication::postEvent(drawer1, event);
          });
  link_type_->setCurrentItem(0);

  QWidget* graphSettingWidget = new QWidget(this);
  Ui::TtVerticalLayout* graphSettingWidgetLayout =
      new Ui::TtVerticalLayout(graphSettingWidget);
  graph_capacity_ = new Ui::TtLabelLineEdit(tr("容量:"), graphSettingWidget);
  connect(graph_capacity_, &Ui::TtLabelLineEdit::currentTextChanged,
          [this](const QString& text) {
            qDebug() << "nums: " << text.toUShort();
            emit graphNumsChanged(text.toUShort());
          });
  graphSettingWidgetLayout->addWidget(graph_capacity_);
  Ui::Drawer* drawer2 = new Ui::Drawer(tr("图表"), graphSettingWidget);

  setLinkType();
  setSerialPortsName();
  setSerialPortsBaudRate();
  setSerialPortsDataBit();
  setSerialPortsParityBit();
  setSerialPortsStopBit();

  QScrollArea* scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget* scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout* scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2, 0);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);

  connect(auto_refresh_, &Ui::TtSwitchButton::toggled, this,
          &Widget::ModbusClientSetting::autoRefreshStateChanged);

  connect(
      refresh_interval_->body(), &Ui::TtLineEdit::editingFinished, [this]() {
        emit refreshIntervalChanged(refresh_interval_->currentText().toULong());
      });
}

}  // namespace Widget
