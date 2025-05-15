/** QModbus 只能读取小于 10(DEX) 地址的保持寄存器
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef CORE_MODBUS_CLIENT_H
#define CORE_MODBUS_CLIENT_H

#include <QModbusClient>
#include <QObject>
#include <QQueue>
#include <QSerialPort>

#include "Def.h"

namespace Core {

struct ModbusMasterConfiguration {
  TtModbusProcotol::Type type;
  QString com;
  QSerialPort::BaudRate baud_rate;
  QSerialPort::DataBits data_bits;
  QSerialPort::Parity parity;
  QSerialPort::StopBits stop_bits; // 停止位
  QString device_id;               // 设备 id

  QString address;
  int port;

  ModbusMasterConfiguration(TtModbusProcotol::Type type, const QString &com,
                            QSerialPort::BaudRate baudRate,
                            QSerialPort::DataBits dataBits,
                            QSerialPort::Parity parity,
                            QSerialPort::StopBits stopBits,
                            const QString &deviceId, const QString &address,
                            const int &port)
      : type(type), com(com), baud_rate(baudRate), data_bits(dataBits),
        parity(parity), stop_bits(stopBits), device_id(deviceId),
        address(address), port(port) {}
};

class ModbusMaster : public QObject {
  Q_OBJECT
public:
  ModbusMaster(QObject *parent = nullptr);
  ~ModbusMaster();

  void setupConfiguration(const ModbusMasterConfiguration &config);
  bool connectModbusDevice(bool reconnect);

  bool isConnected();
  void toDisconnect();

  void readModbusData(const QModbusDataUnit::RegisterType &dataType,
                      const int &startAddr, const quint16 &size,
                      const int &serverAddr);
  bool writeModbusData(const QModbusDataUnit::RegisterType &dataType,
                       const int &startAddr, const QVector<quint16> &values,
                       const int &serverAddr);

  ///
  /// @brief readCoilsData
  /// @param addrs
  /// @param serverAddr
  /// 读取地址对应的值
  void readCoilsData(const QVector<int> &addrs, const int &serverAddr);

  void readCoilsData(const int &startAddr, const quint16 &size,
                     const int &serverAddr);

  void readDiscreteInputsData(const QVector<int> &addrs,
                              const int &serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::DiscreteInputs, *it, 1, serverAddr);
    }
  }

  void readHoldingData(const QVector<int> &addrs, const int &serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::HoldingRegisters, *it, 1, serverAddr);
    }
  }

  void readHoldingData(const int &startAddr, const quint16 &size,
                       const int &serverAddr) {
    readModbusData(QModbusDataUnit::HoldingRegisters, startAddr, size,
                   serverAddr);
  }

  void readInputRegistersData(const QVector<int> &addrs,
                              const int &serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::InputRegisters, *it, 1, serverAddr);
    }
  }

  void writeCoilsData(const int &startAddr, const QVector<quint16> &values,
                      const int &serverAddr) {
    writeModbusData(QModbusDataUnit::Coils, startAddr, values, serverAddr);
  }

  void writeDiscreteInputsData(const int &startAddr,
                               const QVector<quint16> &values,
                               const int &serverAddr) {
    writeModbusData(QModbusDataUnit::DiscreteInputs, startAddr, values,
                    serverAddr);
  }

  void writeHoldingData(const int &startAddr, const QVector<quint16> &values,
                        const int &serverAddr) {
    writeModbusData(QModbusDataUnit::HoldingRegisters, startAddr, values,
                    serverAddr);
  }

  void writeInputRegistersData(const int &startAddr,
                               const QVector<quint16> &values,
                               const int &serverAddr) {
    writeModbusData(QModbusDataUnit::InputRegisters, startAddr, values,
                    serverAddr);
  }

signals:
  // void dataReceived(const int& startAddr, const QVector<quint16>& data);
  void dataReceived(const QModbusDataUnit &dataUnit);
  void errorOccurred(const QString &error);

private slots:
  void stateChanged(QModbusDevice::State state);

private:
  void init();
  void configRtuParam();
  void configTcpParam(QString addr = "127.0.0.1", int port = 502);
  void configUdpParam();

  QModbusClient *modbusDevice = nullptr;

  QQueue<QModbusDataUnit> request_queue_;
  QSet<QModbusReply *> active_replies_; // 追踪活跃请求

  bool is_processing_ = false;

  void processNextRequest();
};

} // namespace Core

#endif // CORE_MODBUS_CLIENT_H
