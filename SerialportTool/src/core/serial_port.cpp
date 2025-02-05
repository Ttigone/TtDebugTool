#include "core/serial_port.h"

namespace Core {

SerialPort::SerialPort() :
  serial_port_(new QSerialPort)
{
  init();
  serial_port_->setReadBufferSize(1024);  // 调整为合适的大小

  bool connected = connect(serial_port_, &QSerialPort::readyRead, this, &SerialPort::readData);
  // if (!connected) {
  //   qDebug() << "Failed to connect readyRead signal to readData slot.";
  // } else {
  //     qDebug() << "Successfully connected readyRead signal to readData slot.";
  // }
}

SerialPort::~SerialPort() {}

bool SerialPort::isOpened() {
  return serial_port_->isOpen();
}

void SerialPort::sendData(const QString& send_string) {
  // 向串口发送数据
  // 可以通过一个 QPlainText 显示当前发送数据
  // send_string."\r\n";
  QString out = send_string;
  out.append("\r\n");
  qDebug() << "write: " << send_string;
  QByteArray bts = send_string.toLocal8Bit();   //转换为8位字符数据数组
  serial_port_->write(bts);
}

void SerialPort::readData()
{
  qint64 bytesAvailable = serial_port_->bytesAvailable();
  qDebug() << "Bytes available to read:" << bytesAvailable;

  if (bytesAvailable > 0) {
    // 读取全部字节
    QByteArray data = serial_port_->readAll();
    QString str(data);
    qDebug() << "recv: " << str;
    emit recvData(data);
    // if ()
  } else {
    qDebug() << "No data available to read";
  }
  // showNowSendOrRecvSize(data.length(), false);
}

SerialPort::SerialError SerialPort::openSerialPort(
    SerialPortConfiguration cfg) {
  serial_port_->setPortName(cfg.com.left(cfg.com.indexOf('-')));
  //qDebug() << "TEST:" << cfg.com.left(cfg.com.indexOf('-'));
  serial_port_->setBaudRate(cfg.baud_rate);
  serial_port_->setDataBits(cfg.data_bits);
  serial_port_->setParity(cfg.parity);
  serial_port_->setStopBits(cfg.stop_bits);
  serial_port_->setFlowControl(cfg.flow_control);

  if (!serial_port_->open(QIODevice::ReadWrite)) {
    qDebug() << "error";
    switch (serial_port_->error()) {
      case QSerialPort::OpenError:
        qDebug() << "open";
        return SerialError::Open;
        break;
      case QSerialPort::DeviceNotFoundError:
        qDebug() << "device";
        return SerialError::DeviceNotFound;
        break;
      case QSerialPort::PermissionError:
        qDebug() << "permission";
        return SerialError::Permission;
        break;
    }
  }
  qDebug() << "succ";
  return SerialError::NoError;
}

void SerialPort::closeSerialPort() {
  serial_port_->close();
}

void SerialPort::init() {
  // foreach(int64 baud, QSerialPortInfo::standardBaudRates()) {
  //   list_baud_rate_.append(baud);
  // }

  // list_data_bits_ << QSerialPort::Data5 << QSerialPort::Data6 << QSerialPort::Data7 << QSerialPort::Data8;

  // map_parity_[QObject::tr("无校验")] = QSerialPort::NoParity;
  // map_parity_[QObject::tr("偶校验")] = QSerialPort::EvenParity;
  // map_parity_[QObject::tr("奇校验")] = QSerialPort::OddParity;
  // map_parity_[QObject::tr("0 校验")] = QSerialPort::SpaceParity;
  // map_parity_[QObject::tr("1 校验")] = QSerialPort::MarkParity;

  // map_stop_bits_[QObject::tr("1 位")] = QSerialPort::OneStop;
  // map_stop_bits_[QObject::tr("1.5 位")] = QSerialPort::OneAndHalfStop;
  // map_stop_bits_[QObject::tr("2 位")] = QSerialPort::TwoStop;

  // map_flow_control_[QObject::tr("None")] = QSerialPort::NoFlowControl;
  // map_flow_control_[QObject::tr("RTS/CTS")] = QSerialPort::HardwareControl;
  // map_flow_control_[QObject::tr("Xon/Xoff")] = QSerialPort::SoftwareControl;
}

void SerialPort::showNowSendOrRecvSize(int64 num, bool status)
{
  if (status) {
    send_size_ += num;
    emit sendSize(send_size_);

  } else {
    recv_size_ += num;
    emit recvSize(recv_size_);
  }
}

}  // namespace Core
