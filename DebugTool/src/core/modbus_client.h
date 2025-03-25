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

  void readModbusData(const QModbusDataUnit::RegisterType& dataType,
                      const int& startAddr, const quint16& size,
                      const int& serverAddr);
  bool writeModbusData(const QModbusDataUnit::RegisterType& dataType,
                       const int& startAddr, const QVector<quint16>& values,
                       const int& serverAddr);

  void readDiscreteInputsData(const QVector<int>& addrs,
                              const int& serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::DiscreteInputs, *it, 1, serverAddr);
    }
  }

  void readCoilsData(const QVector<int>& addrs, const int& serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::Coils, *it, 1, serverAddr);
    }
  }

  void readCoilsData(const int& startAddr, const quint16& size,
                     const int& serverAddr) {
    readModbusData(QModbusDataUnit::Coils, startAddr, size, serverAddr);
  }

  void readHoldingData(const QVector<int>& addrs, const int& serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::HoldingRegisters, *it, 1, serverAddr);
    }
  }

  void readHoldingData(const int& startAddr, const quint16& size,
                       const int& serverAddr) {
    readModbusData(QModbusDataUnit::HoldingRegisters, startAddr, size,
                   serverAddr);
  }

  void readInputRegistersData(const QVector<int>& addrs,
                              const int& serverAddr) {
    for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
      readModbusData(QModbusDataUnit::InputRegisters, *it, 1, serverAddr);
    }
  }

  void writeCoilsData(const int& startAddr, const QVector<quint16>& values,
                      const int& serverAddr) {
    writeModbusData(QModbusDataUnit::Coils, startAddr, values, serverAddr);
  }
  void writeHoldingData(const int& startAddr, const QVector<quint16>& values,
                        const int& serverAddr) {
    writeModbusData(QModbusDataUnit::HoldingRegisters, startAddr, values,
                    serverAddr);
  }

 signals:
  // void dataReceived(const int& startAddr, const QVector<quint16>& data);
  void dataReceived(const QModbusDataUnit& dataUnit);
  void errorOccurred(const QString& error);

 private slots:
  void stateChanged(QModbusDevice::State state);

 private:
  void init();
  void configRtuParam();
  void configTcpParam(QString addr = "127.0.0.1", int port = 502);
  void configUdpParam();

  QModbusClient* modbusDevice = nullptr;

  QQueue<QModbusDataUnit> request_queue_;
  QSet<QModbusReply*> active_replies_;  // 追踪活跃请求

  bool is_processing_ = false;

  void processNextRequest() {
    if (request_queue_.isEmpty() || is_processing_) {
      return;
    }

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
      emit errorOccurred("Connection lost during processing");
      request_queue_.clear();
      return;
    }

    is_processing_ = true;
    QModbusDataUnit unit = request_queue_.dequeue();

    if (auto* reply = modbusDevice->sendReadRequest(unit, 1)) {
      active_replies_.insert(reply);
      connect(reply, &QModbusReply::finished, this, [this, reply, unit]() {
        reply->deleteLater();
        active_replies_.remove(reply);
        is_processing_ = false;

        // 错误处理
        if (reply->error() != QModbusDevice::NoError) {
          // 过滤连接关闭错误（此时应停止后续请求）
          if (reply->error() == QModbusDevice::ConnectionError) {
            request_queue_.clear();
            emit errorOccurred("Connection closed by device");
            return;
          }

          // 其他错误上报
          emit errorOccurred(
              QString("Request failed: %1").arg(reply->errorString()));
        } else {
          // 成功处理数据
          const QModbusDataUnit result = reply->result();
          if (result.isValid()) {
            // emit dataReceived(result.startAddress(), result.values());
            emit dataReceived(result);
          }
        }

        // 是否会导致 请求过慢? 不能精确 100 ms
        // 处理下一个请求
        QMetaObject::invokeMethod(this, &ModbusMaster::processNextRequest,
                                  Qt::QueuedConnection);
      });
    } else {
      is_processing_ = false;
      emit errorOccurred("Failed to create request");
    }
  }
};

}  // namespace Core

#endif  // CORE_MODBUS_CLIENT_H
