#include "core/modbus_client.h"

#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>

namespace Core {

ModbusMaster::ModbusMaster(QObject* parent) : QObject(parent) {}

ModbusMaster::~ModbusMaster() {
  if (modbusDevice) {
    modbusDevice->disconnectDevice();
  }
}

void ModbusMaster::setupConfiguration(const ModbusMasterConfiguration& config) {
  qDebug() << config.com << config.baud_rate << config.parity
           << config.data_bits << config.stop_bits;
  if (modbusDevice) {
    modbusDevice->disconnectDevice();
    delete modbusDevice;
    modbusDevice = nullptr;
  }
  if (config.type == TtModbusProcotol::RTU ||
      config.type == TtModbusProcotol::RTU_ASCLL) {
    modbusDevice = new QModbusRtuSerialClient(this);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                         config.com);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                         config.baud_rate);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                         config.parity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                         config.data_bits);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                         config.stop_bits);
  } else if (config.type == TtModbusProcotol::TCP) {
    modbusDevice = new QModbusTcpClient(this);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter,
                                         config.address);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter,
                                         config.port);
  }
}

bool ModbusMaster::connectModbusDevice(bool reconnect) {

  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    return modbusDevice->connectDevice();
  } else {
    if (reconnect) {
      modbusDevice->disconnectDevice();
      return modbusDevice->connectDevice();
    }
    return true;
  }
}

bool ModbusMaster::isConnected() {
  if (modbusDevice) {
    return modbusDevice->state() == QModbusDevice::ConnectedState;
  }
  return false;
}

void ModbusMaster::toDisconnect() {
  if (modbusDevice) {
    modbusDevice->disconnectDevice();
    // 终止所有未完成请求
    for (QModbusReply* reply : active_replies_) {
      reply->deleteLater();
    }
    active_replies_.clear();
    request_queue_.clear();
    is_processing_ = false;
  }
}

void ModbusMaster::readModbusData(const QModbusDataUnit::RegisterType& dataType,
                                  const int& startAddr, const quint16& size,
                                  const int& serverAddr) {
  QVector<quint16> resultDatas;
  //读数据时，没连接则尝试重连
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    emit errorOccurred("Device not connected");
    return;
  }
  // 数据单元都是以一长串地址发送
  QModbusDataUnit dataUnit(dataType, startAddr, size);
  // 构造请求单元并加入队列
  request_queue_.enqueue(dataUnit);

  // 触发队列处理
  processNextRequest();
}

/*
 *@brief:  写modbus数据(发送写请求，不需要应答，该操作是异步的)
 *@date:   2024.03.09
 *@param:  dataType:写的数据类型，分四类(线圈、离散输入(只读)、输入寄存器(只读)、保持寄存器)
 *@param:  startAddr:起始地址   values:数据块  serverAddr:服务器地址
 *@return: bool:true=发送请求成功 false=请求失败
 */
bool ModbusMaster::writeModbusData(
    const QModbusDataUnit::RegisterType& dataType, const int& startAddr,
    const QVector<quint16>& values, const int& serverAddr) {
  //写数据时，没连接则尝试重连
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    modbusDevice->connectDevice();
    return false;
  }

  QModbusDataUnit dataUnit(dataType, startAddr, values.size());
  dataUnit.setValues(values);
  qDebug() << " 1: " << dataUnit.startAddress() << dataUnit.values();

  // QModbusReply* reply = modbusDevice->sendWriteRequest(dataUnit, serverAddr);
  // if (reply) {
  //   if (!reply->isFinished()) {
  //     //出错提示
  //     connect(reply, &QModbusReply::finished, this, [this, reply]() {
  //       if (reply->error() != QModbusDevice::NoError) {
  //         if (reply->error() == QModbusDevice::ProtocolError) {
  //           emit errorOccurred(
  //               QString("writeModbusData error:%1(exception code = %2)")
  //                   .arg(reply->errorString())
  //                   .arg(reply->rawResult().exceptionCode(), -1, 16));
  //           // qDebug() << QString("writeModbusData error:%1(exception code = %2)")
  //           //                 .arg(reply->errorString())
  //           //                 .arg(reply->rawResult().exceptionCode(), -1, 16);
  //         } else {
  //           emit errorOccurred(
  //               QString("writeModbusData error:%1").arg(reply->errorString()));
  //           // qDebug() << QString("writeModbusData error:%1")
  //           //                 .arg(reply->errorString());
  //         }
  //       }
  //       reply->deleteLater();
  //     });
  //   } else {
  //     reply->deleteLater();
  //   }
  //   return true;
  // } else {
  //   return false;
  // }

  if (auto* reply = modbusDevice->sendWriteRequest(dataUnit, serverAddr)) {
    if (!reply->isFinished()) {
      connect(reply, &QModbusReply::finished, this, [this, reply]() {
        const auto error = reply->error();
        if (error == QModbusDevice::ProtocolError) {
          emit errorOccurred(
              QString("writeModbusData error:%1(exception code = 0x%2)")
                  .arg(reply->errorString())
                  .arg(reply->rawResult().exceptionCode(), -1, 16));
        } else if (error != QModbusDevice::NoError) {
          emit errorOccurred(QString("Write response error: %1 (code: 0x%2)")
                                 .arg(reply->errorString())
                                 .arg(error, -1, 16));
        }
        reply->deleteLater();
      });
    } else {
      reply->deleteLater();
      return true;
    }
  } else {
    emit errorOccurred(
        QString("Write error: %1").arg(modbusDevice->errorString()));
    return false;
  }
  return true;
}

// void ModbusMaster::errorOccurred(QModbusDevice::Error error) {}

void ModbusMaster::stateChanged(QModbusDevice::State state) {}

}  // namespace Core

// void ModbusMaster::readModbusData(const QModbusDataUnit::RegisterType& dataType,
//                                   const int& startAddr, const quint16& size,
//                                   const int& serverAddr) {
//   QVector<quint16> resultDatas;
//   //读数据时，没连接则尝试重连
//   if (modbusDevice->state() != QModbusDevice::ConnectedState) {
//     // modbusDevice->connectDevice();
//     emit errorOccurred("Device not connected");
//     return;
//   }
//   // qDebug() << "2";
//   // 数据单元都是以一长串地址发送
//   QModbusDataUnit dataUnit(dataType, startAddr, size);
//   // qDebug() << "1";
//   // 发射读取请求
//   // QModbusReply* reply = modbusDevice->sendReadRequest(dataUnit, serverAddr);
//   // if (reply) {
//   //   if (!reply->isFinished()) {
//   //     // 等待完成
//   //     QEventLoop eventLoop;
//   //     connect(reply, &QModbusReply::finished, &eventLoop, &QEventLoop::quit);
//   //     eventLoop.exec();
//   //   }
//   //   //出错提示
//   //   if (reply->error() != QModbusDevice::NoError) {
//   //     if (reply->error() == QModbusDevice::ProtocolError) {
//   //       emit errorOccurred(
//   //           QString("readModbusData error:%1(exception code = %2)")
//   //               .arg(reply->errorString())
//   //               .arg(reply->rawResult().exceptionCode(), -1, 16));
//   //       // qDebug() << QString("readModbusData error:%1(exception code = %2)")
//   //       //                 .arg(reply->errorString())
//   //       //                 .arg(reply->rawResult().exceptionCode(), -1, 16);
//   //     } else {
//   //       // timeout
//   //       emit errorOccurred(
//   //           QString("readModbusData error:%1").arg(reply->errorString()));
//   //       // qDebug()
//   //       //     << QString("readModbusData error:%1").arg(reply->errorString());
//   //     }
//   //   } else {
//   //     //处理应答
//   //     const QModbusDataUnit unit = reply->result();
//   //     if (unit.isValid()) {
//   //       resultDatas = unit.values();
//   //     }
//   //   }
//   //   reply->deleteLater();
//   // }
//   // // emit dataReceived(resultDatas);
//   // if (size == 1) {
//   //   emit dataReceived(startAddr, resultDatas);
//   // } else {
//   //   // emit dataReceived(resultDatas);
//   // }
//   // return resultDatas;

//   // -----

//   // if (auto* reply = modbusDevice->sendReadRequest(dataUnit, serverAddr)) {
//   //   if (!reply->isFinished()) {
//   //     connect(reply, &QModbusReply::finished, this, [this, reply]() {
//   //       if (reply->error() == QModbusDevice::NoError) {
//   //         //处理应答
//   //         const QModbusDataUnit unit = reply->result();
//   //         if (unit.isValid()) {
//   //           // 数据
//   //           for (qsizetype i = 0, total = unit.valueCount(); i < total; ++i) {
//   //             qDebug() << unit.startAddress() << unit.value(i);
//   //           }
//   //         }
//   //       } else if (reply->error() == QModbusDevice::ProtocolError) {
//   //         qDebug() << QString("readModbusData error:%1(exception code = %2)")
//   //                         .arg(reply->errorString())
//   //                         .arg(reply->rawResult().exceptionCode(), -1, 16);
//   //         // 信号的问题 ?? 非
//   //         // emit errorOccurred(
//   //         //     QString("readModbusData error:%1(exception code = %2)")
//   //         //         .arg(reply->errorString())
//   //         //         .arg(reply->rawResult().exceptionCode(), -1, 16));
//   //       } else {
//   //         // 此处
//   //         // emit errorOccurred(
//   //         //     QString("readModbusData error:%1").arg(reply->errorString()));
//   //         qDebug()
//   //             << QString("readModbusData error:%1").arg(reply->errorString());
//   //       }
//   //       reply->deleteLater();
//   //     });
//   //   } else {
//   //     delete reply;  // broadcast replies return immediately
//   //   }
//   // } else {
//   //   emit errorOccurred(tr("Read error: %1").arg(modbusDevice->errorString()));
//   // }

//   // 构造请求单元并加入队列
//   // QModbusDataUnit dataUnit(dataType, startAddr, size);
//   request_queue_.enqueue(dataUnit);
//   // m_serverAddr = serverAddr; // 更新服务器地址

//   // 触发队列处理
//   processNextRequest();
// }
