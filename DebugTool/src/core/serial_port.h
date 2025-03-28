#ifndef CORE_SERIAL_PORT_H
#define CORE_SERIAL_PORT_H

#include <QQueue>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

namespace Core {

struct SerialPortConfiguration {
  QString com;
  QSerialPort::BaudRate baud_rate;
  QSerialPort::DataBits data_bits;
  QSerialPort::Parity parity;
  QSerialPort::StopBits stop_bits;
  QSerialPort::FlowControl flow_control;

  SerialPortConfiguration(const QString& com, QSerialPort::BaudRate baudRate,
                          QSerialPort::DataBits dataBits,
                          QSerialPort::Parity parity,
                          QSerialPort::StopBits stopBits,
                          QSerialPort::FlowControl flowControl)
      : com(com),
        baud_rate(baudRate),
        data_bits(dataBits),
        parity(parity),
        stop_bits(stopBits),
        flow_control(flowControl) {}
};

class SerialPortWorker : public QObject {
  Q_OBJECT
 public:
  SerialPortWorker(QObject* parent = nullptr);
  ~SerialPortWorker();

  [[nodiscard]] bool isOpened();

 signals:
  void dataReceived(QByteArray data);
  void errorOccurred(const QString& error);
  void serialPortStatusChanged(bool isOpen);

 public slots:
  // void initSerialPort();
  void openSerialPort(SerialPortConfiguration cfg);
  void closeSerialPort();
  void sendData(const QString& send_string);
  void readData();

 private slots:
  // 实际向串口写入数据的函数
  void handleWriteRequest();

 private:
  void init();

  QSerialPort* serial_ = nullptr;
  QByteArray receive_buffer_;
  QTimer* receive_timer_;

  QQueue<QByteArray> send_queue_;  // 队列
  QMutex send_mutex_;              // 互斥锁
};

}  // namespace Core

Q_DECLARE_METATYPE(Core::SerialPortConfiguration)

#endif  // CORE_SERIAL_PORT_H
