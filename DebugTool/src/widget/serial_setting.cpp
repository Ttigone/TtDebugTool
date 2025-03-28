#include "widget/serial_setting.h"

#include "core/serial_port.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

namespace Widget {

Core::SerialPortConfiguration DefaultSetting = {
    // 默认的端口配置, 可以在从配置文件中读取上一次留存的记录
    QString(""),           QSerialPort::Baud9600, QSerialPort::Data8,
    QSerialPort::NoParity, QSerialPort::OneStop,  QSerialPort::NoFlowControl};

SerialSetting::SerialSetting(QWidget* parent)
    : QWidget(parent)
{
  init();
}

SerialSetting::~SerialSetting() {}

void SerialSetting::setOldSettings() {
  serial_port_->setCurrentItem(1);
}

Core::SerialPortConfiguration SerialSetting::getSerialPortConfiguration() {
  // 使用构造函数初始化
  Core::SerialPortConfiguration config(
      serial_port_->currentData().value<QString>(),
      baud_rate_->currentData().value<QSerialPort::BaudRate>(),
      data_bit_->currentData().value<QSerialPort::DataBits>(),
      parity_bit_->currentData().value<QSerialPort::Parity>(),
      stop_bit_->currentData().value<QSerialPort::StopBits>(),
      flow_control_->currentData().value<QSerialPort::FlowControl>());
  //qDebug() << cfg.com << cfg.baud_rate << cfg.data_bits << cfg.parity
  //         << cfg.stop_bits << cfg.flow_control;
  return config;
}

Core::SerialPortConfiguration SerialSetting::defaultSerialPortConfiguration() {
  // 为每个 box 设置对应的 cfg 参数
  // 可删
  DefaultSetting.com = matchingSerialCOMx(serial_port_->currentText());
  qDebug() << DefaultSetting.parity;
  // 串口名转换成对应的配置项
  if (DefaultSetting.com.isEmpty()) {
  }
  qDebug() << DefaultSetting.com << DefaultSetting.baud_rate
           << DefaultSetting.data_bits << DefaultSetting.parity
           << DefaultSetting.stop_bits << DefaultSetting.flow_control;

  return DefaultSetting;
}

void SerialSetting::displayDefaultSetting() {
  for (int i = 0; i < baud_rate_->count(); ++i) {
    if (baud_rate_->itemText(i).contains(
            QString::number(DefaultSetting.baud_rate))) {
      baud_rate_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < data_bit_->count(); ++i) {
    if (data_bit_->itemText(i).contains(
            QString::number(DefaultSetting.data_bits))) {
      data_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < parity_bit_->count(); ++i) {
    // bug
    // qDebug() << select_parity_bit_->itemText(i);
    if (parity_bit_->itemText(i).contains(
            QString::number(DefaultSetting.parity))) {
      parity_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < stop_bit_->count(); ++i) {
    if (stop_bit_->itemText(i).contains(
            QString::number(DefaultSetting.stop_bits))) {
      stop_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < flow_control_->count(); ++i) {
    if (flow_control_->itemText(i).contains(
            QString::number(DefaultSetting.flow_control))) {
      flow_control_->setCurrentItem(i);
      break;
    }
  }
}

const QJsonObject& SerialSetting::getSerialSetting() {
  auto cfg = getSerialPortConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("PortName", QJsonValue(cfg.com));
  linkSetting.insert("BaudRate", QJsonValue(cfg.baud_rate));
  linkSetting.insert("DataBits", QJsonValue(cfg.data_bits));
  linkSetting.insert("StopBits", QJsonValue(cfg.stop_bits));
  linkSetting.insert("FlowControl", QJsonValue(cfg.flow_control));
  serial_save_config_.insert("LinkSetting", QJsonValue(linkSetting));

  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  framing.insert("Timeout",
                 QJsonValue(framing_timeout_->body()->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->body()->currentText()));
  serial_save_config_.insert("Framing", QJsonValue(framing));

  QJsonObject lineFeed;
  lineFeed.insert("LineFeed", QJsonValue(line_break_->body()->currentText()));
  serial_save_config_.insert("Framing", QJsonValue(lineFeed));

  QJsonObject heartbeat;
  heartbeat.insert("Type", QJsonValue(heartbeat_send_type_->body()->currentText()));
  heartbeat.insert("Interval",
                 QJsonValue(heartbeat_interval_->body()->currentText()));
  heartbeat.insert("Content",
                 QJsonValue(heartbeat_content_->body()->currentText()));
  serial_save_config_.insert("Heartbeat", QJsonValue(heartbeat));


  //qDebug() << "Json: " << serial_save_config_;

  return serial_save_config_;
}

void SerialSetting::setSerialPortsName() {
  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  serial_port_->body()->clear();
  for (const QSerialPortInfo& portInfo : serialPortInfos) {
    QString portName = (portInfo.portName() + "-" + portInfo.description());
    serial_port_->addItem(portName, portInfo.portName());
  }
  serial_port_->body()->model()->sort(0);

  connect(
      serial_port_->body(), &QComboBox::currentTextChanged,
      [this](const QString& text) {
        // qDebug() << text;
        qDebug() << serial_port_->body()->currentData().value<QString>();
        // serial_port_->body()->setCurrentText(
        //     serial_port_->body()->currentData().value<QString>() + "test");
      });
  serial_port_->setCurrentItem(0);
}

void SerialSetting::setSerialPortsBaudRate() {
  foreach (int64 baud_rate, QSerialPortInfo::standardBaudRates()) {
    baud_rate_->addItem(QString::number(baud_rate), baud_rate);
  }
}

void SerialSetting::setSerialPortsDataBit() {
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

void SerialSetting::setSerialPortsParityBit() {
  parity_bit_->addItem(tr("无校验"), QSerialPort::NoParity);
  parity_bit_->addItem(tr("偶校验"), QSerialPort::EvenParity);
  parity_bit_->addItem(tr("奇校验"), QSerialPort::OddParity);
  parity_bit_->addItem(tr("0 校验"), QSerialPort::SpaceParity);
  parity_bit_->addItem(tr("1 校验"), QSerialPort::MarkParity);
}

void SerialSetting::setSerialPortsStopBit() {
  stop_bit_->addItem(tr("1 位"), QSerialPort::OneStop);
  stop_bit_->addItem(tr("1.5 位"), QSerialPort::OneAndHalfStop);
  stop_bit_->addItem(tr("2 位"), QSerialPort::TwoStop);
}

void SerialSetting::setSerialPortsFluidControl() {
  flow_control_->addItem(tr("None"), QSerialPort::NoFlowControl);
  flow_control_->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
  flow_control_->addItem(tr("Xon/Xoff"), QSerialPort::SoftwareControl);
}

void SerialSetting::setControlState(bool state) {
  serial_port_->setEnabled(state);
  baud_rate_->setEnabled(state);
  data_bit_->setEnabled(state);
  parity_bit_->setEnabled(state);
  stop_bit_->setEnabled(state);
  flow_control_->setEnabled(state);
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  line_break_->setEnabled(state);
  heartbeat_send_type_->setEnabled(state);
  heartbeat_interval_->setEnabled(state);
  heartbeat_content_->setEnabled(state);
}

quint32 SerialSetting::getRefreshInterval() {
  // return refr
}

void SerialSetting::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  QWidget* serialConfigWidget = new QWidget(this);
  serial_port_ = new Ui::TtLabelBtnComboBox(tr("串口:"), serialConfigWidget);
  baud_rate_ = new Ui::TtLabelComboBox(tr("波特率:"), serialConfigWidget);
  data_bit_ = new Ui::TtLabelComboBox(tr("数据位:"), serialConfigWidget);
  parity_bit_ = new Ui::TtLabelComboBox(tr("校验位:"), serialConfigWidget);
  stop_bit_ = new Ui::TtLabelComboBox(tr("停止位:"), serialConfigWidget);
  flow_control_ = new Ui::TtLabelComboBox(tr("流控:"), serialConfigWidget);

  Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(serialConfigWidget);

  layout->addWidget(serial_port_);
  layout->addWidget(baud_rate_);
  layout->addWidget(data_bit_);
  layout->addWidget(parity_bit_);
  layout->addWidget(stop_bit_);
  layout->addWidget(flow_control_);

  connect(serial_port_, &Ui::TtLabelBtnComboBox::clicked,
          [this]() { setSerialPortsName(); });

  setSerialPortsName();
  setSerialPortsBaudRate();
  setSerialPortsDataBit();
  setSerialPortsParityBit();
  setSerialPortsStopBit();
  setSerialPortsFluidControl();
  // displayDefaultSetting();

  QWidget* linkSettingWidget = new QWidget;
  QVBoxLayout* linkSettingWidgetLayout = new QVBoxLayout(linkSettingWidget);
  linkSettingWidgetLayout->setSpacing(0);
  linkSettingWidgetLayout->setContentsMargins(QMargins());
  linkSettingWidgetLayout->addWidget(serialConfigWidget);
  linkSettingWidget->adjustSize();  // 确保大小正确
  Ui::Drawer* drawerLinkSetting =
      new Ui::Drawer(tr("连接设置"), linkSettingWidget);

  QWidget* scriptWidget = new QWidget;
  Ui::TtVerticalLayout* scriptWidgetLayout =
      new Ui::TtVerticalLayout(scriptWidget);
  script_ = new Ui::TtLabelLineEdit(tr("脚本"), scriptWidget);
  scriptWidgetLayout->addWidget(script_);

  Ui::Drawer* drawerScript = new Ui::Drawer(tr("脚本设置"), scriptWidget);

  QWidget* framingWidget = new QWidget;
  Ui::TtVerticalLayout* framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framingWidget->adjustSize();  // 确保大小正确
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelComboBox(tr("时间: "));
  framing_fixed_length_ = new Ui::TtLabelComboBox(tr("长度: "));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);

  Ui::Drawer* drawerFraming = new Ui::Drawer(tr("分帧"), framingWidget);

  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawerFraming](int index) {
            switch (index) {
              case 0: {
                framing_timeout_->setVisible(false);
                framing_fixed_length_->setVisible(false);
                break;
              }
              case 1: {
                framing_timeout_->setVisible(true);
                framing_fixed_length_->setVisible(false);
                break;
              }
              case 2: {
                framing_timeout_->setVisible(false);
                framing_fixed_length_->setVisible(true);
                break;
              }
            }
            const auto event =
                new QResizeEvent(drawerFraming->size(), drawerFraming->size());
            QCoreApplication::postEvent(drawerFraming, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  line_break_ = new Ui::TtLabelComboBox(tr("换行符: "));
  line_break_->addItem("\\r");
  line_break_->addItem("\\n");
  line_break_->addItem("\\r\\n");
  Ui::Drawer* drawerLineBreak = new Ui::Drawer(tr("换行"), line_break_);

  QWidget* heartbeatWidget = new QWidget;
  Ui::TtVerticalLayout* heartbeatWidgetLayout =
      new Ui::TtVerticalLayout(heartbeatWidget);
  heartbeatWidget->adjustSize();  // 确保大小正确
  heartbeat_send_type_ = new Ui::TtLabelComboBox(tr("类型: "));
  heartbeat_send_type_->addItem(tr("无"));
  heartbeat_send_type_->addItem(tr("文本"));
  heartbeat_send_type_->addItem(tr("HEX"));
  heartbeat_interval_ = new Ui::TtLabelComboBox(tr("间隔: "));
  heartbeat_content_ = new Ui::TtLabelComboBox(tr("内容: "));
  heartbeatWidgetLayout->addWidget(heartbeat_send_type_);
  heartbeatWidgetLayout->addWidget(heartbeat_interval_);
  heartbeatWidgetLayout->addWidget(heartbeat_content_);
  Ui::Drawer* drawerHeartBeat = new Ui::Drawer(tr("心跳"), heartbeatWidget);

  connect(heartbeat_send_type_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, heartbeatWidget, drawerHeartBeat](int index) {
            switch (index) {
              case 0: {
                heartbeat_interval_->setVisible(false);
                heartbeat_content_->setVisible(false);
                break;
              }
              case 1: {
                heartbeat_interval_->setVisible(true);
                heartbeat_content_->setVisible(true);
                break;
              }
              case 2: {
                heartbeat_interval_->setVisible(true);
                heartbeat_content_->setVisible(true);
                break;
              }
            }
            const auto event = new QResizeEvent(drawerHeartBeat->size(),
                                                drawerHeartBeat->size());
            QCoreApplication::postEvent(drawerHeartBeat, event);
          });
  heartbeat_send_type_->setCurrentItem(0);
  heartbeat_interval_->setVisible(false);
  heartbeat_content_->setVisible(false);


  // 滚动区域
  QScrollArea* scr = new QScrollArea(this);
  scr->setFrameStyle(QFrame::NoFrame);
  QWidget* scrollContent = new QWidget(scr);
  //scr->setWidget(scrollContent);
  //scr->setWidgetResizable(true);

  Ui::TtVerticalLayout* lascr = new Ui::TtVerticalLayout(scrollContent);

  lascr->addWidget(drawerLinkSetting, 0, Qt::AlignTop);
  lascr->addWidget(drawerScript);
  lascr->addWidget(drawerLineBreak);
  lascr->addWidget(drawerHeartBeat);
  lascr->addStretch();
  scrollContent->setLayout(lascr);

  scr->setWidget(scrollContent);
  scr->setWidgetResizable(true);

  main_layout_->addWidget(scr);
  heartbeat_content_->body()->setCurrentText("TEST");
}

void SerialSetting::refreshSerialCOMx() {}

QString SerialSetting::matchingSerialCOMx(const QString& name) {

  // qDebug() << "pipie: " << name;
  qDebug() << name.size();
  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo& portInfo : serialPortInfos) {
    // QRegularExpression regex(name);
    // QRegularExpressionMatch match = regex.match(portInfo.portName());
    // if (match.hasMatch()) {
    //   qDebug() << "yes" << portInfo.portName();
    //   // COMx 匹配原有字符串
    //   // qDebug() <<
    //   return portInfo.portName();
    // } else {
    //   // 没有匹配到
    //   // 有可能是串口加载时, 存在这个串口, 但过了一段时间后, 当 user 打开串口时, 串口已经不存在
    //   // error
    //   return "";
    // }

    if (portInfo.portName() == name) {
      return portInfo.portName();
    }

    // 获取有多少个 COM
    // int cnt = portInfo.portName().count(name.left(3));

    // if (cnt == 1) {
    //   // COM1 匹配到 COM10
    //   int pos = portInfo.portName().indexOf(name);
    //   if (pos != -1) {
    //     // qDebug() << "pos: " << pos + name.size();
    //     // qDebug() << "name: " << portInfo.portName();
    //     // qDebug() << "1: " << portInfo.description();
    //     if (portInfo.portName().at(pos + name.size()) != QChar('-')) {
    //       continue;
    //     }
    //     return portInfo.portName();
    //   }
    //   // if (pos +)
    // } else {
    //   int front = portInfo.portName().indexOf(name);
    //   int back = portInfo.portName().lastIndexOf(name.left(3));
    //   if (front == back) {
    //     continue;
    //   } else {
    //     return portInfo.portName();
    //   }
    // }
    // uint8 pos = portInfo.portName().indexOf(name);

    // int front = portInfo.portName().indexOf(name.left(3));
    // int back = portInfo.portName().lastIndexOf(name.left(3));
    // // 说明只有一个 COMx
    // if (front == back) {
    //   if (front != -1) {
    //     return portInfo.portName();
    //   } else {
    //     return QString();
    //   }
    // } else if (front < back) {
    //   // 虚拟COMx, 默认前一个
    //   return portInfo.portName();
    // } else {
    //   // 跳过
    //   continue;
    // }
  }
}

}  // namespace Widget
