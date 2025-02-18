#include "widget/serial_setting.h"

#include "core/serial_port.h"

#include <ui/window/combobox.h>

namespace Widget {

Core::SerialPortConfiguration DefaultSetting = {
    // 默认的端口配置, 可以在从配置文件中读取上一次留存的记录
    QString(""),           QSerialPort::Baud9600, QSerialPort::Data8,
    QSerialPort::NoParity, QSerialPort::OneStop,  QSerialPort::NoFlowControl};

SerialSetting::SerialSetting(QWidget* parent)
    : select_serial_port_(new Ui::TtLabelBtnComboBox(tr("串口:"), this)),
      select_baud_rate_(new Ui::TtLabelComboBox(tr("波特率:"), this)),
      select_data_bit_(new Ui::TtLabelComboBox(tr("数据位:"), this)),
      select_parity_bit_(new Ui::TtLabelComboBox(tr("校验位:"), this)),
      select_stop_bit_(new Ui::TtLabelComboBox(tr("停止位:"), this)),
      select_fluid_control_(new Ui::TtLabelComboBox(tr("流控:"), this)),
      QWidget(parent) {
  QVBoxLayout* layout = new QVBoxLayout;
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());
  this->setLayout(layout);

  layout->addWidget(select_serial_port_);
  layout->addWidget(select_baud_rate_);
  layout->addWidget(select_data_bit_);
  layout->addWidget(select_parity_bit_);
  layout->addWidget(select_stop_bit_);
  layout->addWidget(select_fluid_control_);

  connect(select_serial_port_, &Ui::TtLabelBtnComboBox::clicked,
          [this]() { setSerialPortsName(); });
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

  return cfg;
}

Core::SerialPortConfiguration SerialSetting::defaultSerialPortConfiguration() {
  // 为每个 box 设置对应的 cfg 参数
  // 可删
  DefaultSetting.com = matchingSerialCOMx(select_serial_port_->currentText());
  //qDebug() << select_serial_port->currentText();
  // qDebug() << "DEFAULT: " << DefaultSetting.com;
  qDebug() << DefaultSetting.parity;
  // 串口名转换成对应的配置项
  if (DefaultSetting.com.isEmpty()) {
    // 显示错误
  }
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
    qDebug() << select_parity_bit_->itemText(i);
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

void SerialSetting::refreshSerialCOMx() {}

QString SerialSetting::matchingSerialCOMx(const QString& name) {

  qDebug() << "pipie: " << name;
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
