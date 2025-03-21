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
#include <QSerialPort>

#include "Def.h"

namespace Core {

struct ModbusMasterConfiguration {
  TtModbusProcotol::Type type;
  QString com;
  QSerialPort::BaudRate baud_rate;
  QSerialPort::DataBits data_bits;
  QSerialPort::Parity parity;
  QSerialPort::StopBits stop_bits;  // 停止位
  QString device_id;                // 设备 id

  ModbusMasterConfiguration(TtModbusProcotol::Type type, const QString& com,
                            QSerialPort::BaudRate baudRate,
                            QSerialPort::DataBits dataBits,
                            QSerialPort::Parity parity,
                            QSerialPort::StopBits stopBits,
                            const QString& deviceId)
      : type(type),
        com(com),
        baud_rate(baudRate),
        data_bits(dataBits),
        parity(parity),
        stop_bits(stopBits),
        device_id(deviceId) {}
};

class ModbusMaster : public QObject {
  Q_OBJECT
 public:
  ModbusMaster(QObject* parent = nullptr);
  ~ModbusMaster();

  void setupConfiguration(const ModbusMasterConfiguration& config);
  bool connectModbusDevice(bool reconnect);

  bool isConnected();
  void toDisconnect();

  // QVector<quint16> readModbusData(const QModbusDataUnit::RegisterType& dataType,
  //                                 const int& startAddr, const quint16& size,
  //                                 const int& serverAddr);
  // bool writeModbusData(const QModbusDataUnit::RegisterType& dataType,
  //                      const int& startAddr, const QVector<quint16>& values,
  //                      const int& serverAddr);

  void readModbusData(const QModbusDataUnit::RegisterType& dataType,
                      const int& startAddr, const quint16& size,
                      const int& serverAddr);
  bool writeModbusData(const QModbusDataUnit::RegisterType& dataType,
                       const int& startAddr, const QVector<quint16>& values,
                       const int& serverAddr);
  // 起始地址, 读取个数
  // QVector<quint16> readCoilsData(const int& startAddr, const quint16& size) {
  //   return readModbusData(QModbusDataUnit::Coils, startAddr, size,
  //                         server_);  //读线圈
  // }
  // QVector<quint16> readHoldingData(const int& startAddr, const quint16& size) {
  //   return readModbusData(QModbusDataUnit::HoldingRegisters, startAddr, size,
  //                         server_);  //读保持寄存器
  // }
  // bool writeCoilsData(const int& startAddr, const QVector<quint16>& values) {
  //   return writeModbusData(QModbusDataUnit::Coils, startAddr, values,
  //                          server_);  //写线圈
  // }
  // bool writeHoldingData(const int& startAddr, const QVector<quint16>& values) {
  //   return writeModbusData(QModbusDataUnit::HoldingRegisters, startAddr, values,
  //                          server_);  //写保持寄存器
  // }

  void readCoilsData(const int& startAddr, const quint16& size,
                     const int& serverAddr) {
    readModbusData(QModbusDataUnit::Coils, startAddr, size,
                   serverAddr);  //读线圈
  }

  void readHoldingData(const QVector<int>& addrs, const int& serverAddr) {
    // 直接传入一系列地址, 接收只能一个一个值接收, 因为 1 字节请求, 需要拆分
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::HoldingRegisters, *it, 1, serverAddr);
    }
    // readModbusData(QModbusDataUnit::HoldingRegisters, startAddr, size,
    //                serverAddr);  //读保持寄存器
  }

  void readHoldingData(const int& startAddr, const quint16& size,
                       const int& serverAddr) {
    readModbusData(QModbusDataUnit::HoldingRegisters, startAddr, size,
                   serverAddr);  //读保持寄存器
  }
  void writeCoilsData(const int& startAddr, const QVector<quint16>& values,
                      const int& serverAddr) {
    writeModbusData(QModbusDataUnit::Coils, startAddr, values,
                    serverAddr);  //写线圈
  }
  void writeHoldingData(const int& startAddr, const QVector<quint16>& values,
                        const int& serverAddr) {
    writeModbusData(QModbusDataUnit::HoldingRegisters, startAddr, values,
                    serverAddr);  //写保持寄存器
  }

 signals:
  void dataReceived(const int& startAddr, const QVector<quint16>& data);
  // void dataReceived(const QVector<quint16>& data);
  void errorOccurred(const QString& error);

 private slots:
  // void errorOccurred(QModbusDevice::Error error);
  void stateChanged(QModbusDevice::State state);

 private:
  void init();
  void configRtuParam();
  void configTcpParam(QString addr = "127.0.0.1", int port = 502);
  void configUdpParam();

  QModbusClient* modbusDevice = nullptr;
};

}  // namespace Core

#endif  // CORE_MODBUS_CLIENT_H
