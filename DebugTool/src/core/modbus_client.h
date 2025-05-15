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

#if 1

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

  void readDiscreteInputsData(const QVector<int> &addrs, const int &serverAddr);

  void readHoldingData(const QVector<int> &addrs, const int &serverAddr);

  void readHoldingData(const int &startAddr, const quint16 &size,
                       const int &serverAddr);

  void readInputRegistersData(const QVector<int> &addrs, const int &serverAddr);

  void writeCoilsData(const int &startAddr, const QVector<quint16> &values,
                      const int &serverAddr);

  void writeDiscreteInputsData(const int &startAddr,
                               const QVector<quint16> &values,
                               const int &serverAddr);

  void writeHoldingData(const int &startAddr, const QVector<quint16> &values,
                        const int &serverAddr);

  void writeInputRegistersData(const int &startAddr,
                               const QVector<quint16> &values,
                               const int &serverAddr);

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

#elif

#include "modbus.h"

class TtModbusRtuClient : public QObject {
  Q_OBJECT
public:
  explicit TtModbusRtuClient(QObject *parent = nullptr);
  ~TtModbusRtuClient();

  ///
  /// @brief connect
  /// @param port
  /// @param baud
  /// @param parity
  /// @param dataBit
  /// @param stopBit
  /// @return
  /// 链接 Rtu 设备
  bool connect(const QString &port, int baud, char parity, int dataBit,
               int stopBit);

  ///
  /// @brief disconnect
  /// 断开 Rtu 设备
  void disconnect();

  ///
  /// @brief isConnected
  /// @return
  /// 链接状态
  bool isConnected();

  ///
  /// @brief setSlaveAddress
  /// @param address
  /// 设置从站地址
  void setSlaveAddress(int address);

  ///
  /// @brief setTimeout
  /// @param msec
  /// @return
  /// 设置超时时间
  bool setTimeout(int msec);

  ///
  /// @brief readCoils
  /// @param addr
  /// @param nb
  /// @return
  /// 读线圈
  QVector<bool> readCoils(int addr, int nb);

  ///
  /// @brief readDiscreteInputs
  /// @param addr
  /// @param nb
  /// @return
  /// 读取离散输入
  QVector<bool> readDiscreteInputs(int addr, int nb);

  ///
  /// @brief readHoldingRegisters
  /// @param addr
  /// @param nb
  /// @return
  /// 读取保持寄存器
  QVector<uint16_t> readHoldingRegisters(int addr, int nb);

  ///
  /// @brief readInputRegisters
  /// @param addr
  /// @param nb
  /// @return
  /// 读取输入寄存器
  QVector<uint16_t> readInputRegisters(int addr, int nb);

  ///
  /// @brief
  /// @param addr
  /// @param value
  /// @return
  /// 写单个线圈
  bool writeCoil(int addr, bool value);

  ///
  /// @brief writeCoils
  /// @param addr
  /// @param values
  /// @return
  /// 写多个线圈
  bool writeCoils(int addr, const QVector<bool> &values);

  ///
  /// @brief writeRegister
  /// @param addr
  /// @param value
  /// @return
  /// 写单个寄存器
  bool writeRegister(int addr, uint16_t value);

  ///
  /// @brief writeRegisters
  /// @param addr
  /// @param values
  /// @return
  /// 写多个寄存器
  bool writeRegisters(int addr, const QVector<uint16_t> &values);

  ///
  /// @brief lastError
  /// @return
  /// 获取最后的错误信息
  QString lastError() const;

signals:
  ///
  /// @brief connectionChanged
  /// @param connected
  /// 链接状态改变
  void connectionChanged(bool connected);

  ///
  /// @brief communicationError
  /// @param errorMessage
  /// 通信错误信号
  void communicationError(const QStrin &errorMessage);

private:
  modbus_t *ctx_;         // modbus 上下文
  bool connected_;        // 链接状态
  int slave_address_;     // 从站地址
  QString error_message_; // 错误信息
};

#endif

#endif // CORE_MODBUS_CLIENT_H
