#ifndef CORE_SERIAL_PORT_H
#define CORE_SERIAL_PORT_H

#include <QSerialPort>
#include <QSerialPortInfo>


namespace Core {

struct SerialPortConfiguration {
  QString com;
  QSerialPort::BaudRate baud_rate;
  QSerialPort::DataBits data_bits;
  QSerialPort::Parity parity;
  QSerialPort::StopBits stop_bits;
  QSerialPort::FlowControl flow_control;

  // 构造函数
  SerialPortConfiguration(const QString& com, QSerialPort::BaudRate baudRate,
                          QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
                          QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl)
      : com(com), baud_rate(baudRate), data_bits(dataBits),
        parity(parity), stop_bits(stopBits), flow_control(flowControl) {}
};

class SerialPort : public QObject {
  Q_OBJECT
 public:
  SerialPort();
  ~SerialPort();

  [[nodiscard]] bool isOpened();

  void sendData(const QString& send_string);
  void readData();

  enum SerialError {
    NoError = 0x0001,
    DeviceNotFound = 0x0002,
    Permission = 0x0003,
    Open = 0x0004,
  };

  Q_ENUMS(SerialError)

 signals:
  void sendSize(int64);
  void recvSize(int64);
  void recvData(QByteArray data);

 public Q_SLOTS:
  [[nodiscard]] SerialError openSerialPort(SerialPortConfiguration cfg);
  void closeSerialPort();

 private:
  void init();
  void showNowSendOrRecvSize(int64 num, bool status);

  QSerialPort* serial_port_;

  int64 send_size_ = 0;
  int64 recv_size_ = 0;
};

}  // namespace Core

#endif  // CORE_SERIAL_PORT_H
