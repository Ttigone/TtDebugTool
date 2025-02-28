#include "widget/serial_setting.h"

#include "core/serial_port.h"

#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/window/combobox.h>

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
  select_serial_port_->setCurrentItem(1);
}

Core::SerialPortConfiguration SerialSetting::getSerialPortConfiguration() {
  // 使用构造函数初始化
  Core::SerialPortConfiguration cfg(
      select_serial_port_->body()->itemText(
          select_serial_port_->body()->currentIndex()),
      static_cast<QSerialPort::BaudRate>(
          select_baud_rate_->body()
              ->itemData(select_baud_rate_->body()->currentIndex())
              .toInt()),
      static_cast<QSerialPort::DataBits>(
          select_data_bit_->body()
              ->itemData(select_data_bit_->body()->currentIndex())
              .toInt()),
      static_cast<QSerialPort::Parity>(
          select_parity_bit_->body()
              ->itemData(select_parity_bit_->body()->currentIndex())
              .toInt()),
      static_cast<QSerialPort::StopBits>(
          select_stop_bit_->body()
              ->itemData(select_stop_bit_->body()->currentIndex())
              .toInt()),
      static_cast<QSerialPort::FlowControl>(
          select_fluid_control_->body()
              ->itemData(select_fluid_control_->body()->currentIndex())
              .toInt()));

  //qDebug() << cfg.com << cfg.baud_rate << cfg.data_bits << cfg.parity
  //         << cfg.stop_bits << cfg.flow_control;
  return cfg;
}

Core::SerialPortConfiguration SerialSetting::defaultSerialPortConfiguration() {
  // 为每个 box 设置对应的 cfg 参数
  // 可删
  DefaultSetting.com = matchingSerialCOMx(select_serial_port_->currentText());
  qDebug() << DefaultSetting.parity;
  // 串口名转换成对应的配置项
  if (DefaultSetting.com.isEmpty()) {
    // 显示错误
  }
  // TODO baud 有问题
  qDebug() << DefaultSetting.com << DefaultSetting.baud_rate
           << DefaultSetting.data_bits << DefaultSetting.parity
           << DefaultSetting.stop_bits << DefaultSetting.flow_control;

  return DefaultSetting;
}

void SerialSetting::displayDefaultSetting() {
  for (int i = 0; i < select_baud_rate_->count(); ++i) {
    if (select_baud_rate_->itemText(i).contains(
            QString::number(DefaultSetting.baud_rate))) {
      select_baud_rate_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < select_data_bit_->count(); ++i) {
    if (select_data_bit_->itemText(i).contains(
            QString::number(DefaultSetting.data_bits))) {
      select_data_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < select_parity_bit_->count(); ++i) {
    // bug
    // qDebug() << select_parity_bit_->itemText(i);
    if (select_parity_bit_->itemText(i).contains(
            QString::number(DefaultSetting.parity))) {
      select_parity_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < select_stop_bit_->count(); ++i) {
    if (select_stop_bit_->itemText(i).contains(
            QString::number(DefaultSetting.stop_bits))) {
      select_stop_bit_->setCurrentItem(i);
      break;
    }
  }
  for (int i = 0; i < select_fluid_control_->count(); ++i) {
    if (select_fluid_control_->itemText(i).contains(
            QString::number(DefaultSetting.flow_control))) {
      select_fluid_control_->setCurrentItem(i);
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
  select_serial_port_->body()->clear();
  for (const QSerialPortInfo& portInfo : serialPortInfos) {
    QString portName = (portInfo.portName() + "-" + portInfo.description());
    select_serial_port_->addItem(portName);
  }
}

void SerialSetting::setSerialPortsBaudRate() {
  foreach (int64 baud_rate, QSerialPortInfo::standardBaudRates()) {
    select_baud_rate_->addItem(QString::number(baud_rate), baud_rate);
  }
}

void SerialSetting::setSerialPortsDataBit() {
  list_data_bits_ << QSerialPort::Data5 << QSerialPort::Data6
                  << QSerialPort::Data7 << QSerialPort::Data8;
  for (auto& data_bit : list_data_bits_) {
    select_data_bit_->addItem(QString::number(data_bit), data_bit);
  }
}

void SerialSetting::setSerialPortsParityBit() {
  map_parity_[QObject::tr("无校验")] = QSerialPort::NoParity;
  map_parity_[QObject::tr("偶校验")] = QSerialPort::EvenParity;
  map_parity_[QObject::tr("奇校验")] = QSerialPort::OddParity;
  map_parity_[QObject::tr("0 校验")] = QSerialPort::SpaceParity;
  map_parity_[QObject::tr("1 校验")] = QSerialPort::MarkParity;
  for (auto it = map_parity_.begin(); it != map_parity_.end(); ++it) {
    select_parity_bit_->addItem(it.key(), it.value());
  }
}

void SerialSetting::setSerialPortsStopBit() {
  map_stop_bits_[QObject::tr("1 位")] = QSerialPort::OneStop;
  map_stop_bits_[QObject::tr("1.5 位")] = QSerialPort::OneAndHalfStop;
  map_stop_bits_[QObject::tr("2 位")] = QSerialPort::TwoStop;
  for (auto it = map_stop_bits_.begin(); it != map_stop_bits_.end(); ++it) {
    select_stop_bit_->addItem(it.key(), it.value());
  }
}

void SerialSetting::setSerialPortsFluidControl() {
  map_flow_control_[QObject::tr("None")] = QSerialPort::NoFlowControl;
  map_flow_control_[QObject::tr("RTS/CTS")] = QSerialPort::HardwareControl;
  map_flow_control_[QObject::tr("Xon/Xoff")] = QSerialPort::SoftwareControl;
  for (auto it = map_flow_control_.begin(); it != map_flow_control_.end();
       ++it) {
    select_fluid_control_->addItem(it.key(), it.value());
  }
}

void SerialSetting::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  QWidget* serialConfigWidget = new QWidget(this);
  select_serial_port_ =
      new Ui::TtLabelBtnComboBox(tr("串口:"), serialConfigWidget);
  select_baud_rate_ =
      new Ui::TtLabelComboBox(tr("波特率:"), serialConfigWidget);
  select_data_bit_ = new Ui::TtLabelComboBox(tr("数据位:"), serialConfigWidget);
  select_parity_bit_ =
      new Ui::TtLabelComboBox(tr("校验位:"), serialConfigWidget);
  select_stop_bit_ = new Ui::TtLabelComboBox(tr("停止位:"), serialConfigWidget);
  select_fluid_control_ =
      new Ui::TtLabelComboBox(tr("流控:"), serialConfigWidget);

  Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(serialConfigWidget);

  layout->addWidget(select_serial_port_);
  layout->addWidget(select_baud_rate_);
  layout->addWidget(select_data_bit_);
  layout->addWidget(select_parity_bit_);
  layout->addWidget(select_stop_bit_);
  layout->addWidget(select_fluid_control_);

  connect(select_serial_port_, &Ui::TtLabelBtnComboBox::clicked,
          [this]() { setSerialPortsName(); });

  // 初始 app 时, 随机地设置串口配置
  // 选择可以选择全部, 但是显示的时候, 只显示 COMx
  setSerialPortsName();
  setSerialPortsBaudRate();
  setSerialPortsDataBit();
  setSerialPortsParityBit();
  setSerialPortsStopBit();
  setSerialPortsFluidControl();
  displayDefaultSetting();

  QWidget* linkSettingWidget = new QWidget;
  QVBoxLayout* linkSettingWidgetLayout = new QVBoxLayout(linkSettingWidget);
  linkSettingWidgetLayout->setSpacing(0);
  linkSettingWidgetLayout->setContentsMargins(QMargins());
  linkSettingWidgetLayout->addWidget(serialConfigWidget);
  linkSettingWidget->adjustSize();  // 确保大小正确
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), linkSettingWidget);

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

  Ui::Drawer* drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);

  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawer2](int index) {
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
                new QResizeEvent(drawer2->size(), drawer2->size());
            QCoreApplication::postEvent(drawer2, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  line_break_ = new Ui::TtLabelComboBox(tr("换行符: "));
  line_break_->addItem("\\r");
  line_break_->addItem("\\n");
  line_break_->addItem("\\r\\n");
  Ui::Drawer* drawer3 = new Ui::Drawer(tr("换行"), line_break_);

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
  Ui::Drawer* drawer4 = new Ui::Drawer(tr("心跳"), heartbeatWidget);

  connect(heartbeat_send_type_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, heartbeatWidget, drawer4](int index) {
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
            const auto event =
                new QResizeEvent(drawer4->size(), drawer4->size());
            QCoreApplication::postEvent(drawer4, event);
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

  lascr->addWidget(drawer1, 0, Qt::AlignTop);
  //lascr->addWidget(sds, 0, Qt::AlignTop);
  lascr->addWidget(drawer2);
  lascr->addWidget(drawer3);
  lascr->addWidget(drawer4);
  lascr->addStretch();
  scrollContent->setLayout(lascr);

  scr->setWidget(scrollContent);
  scr->setWidgetResizable(true);

  main_layout_->addWidget(scr);
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
