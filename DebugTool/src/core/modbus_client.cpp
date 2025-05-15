#include "core/modbus_client.h"
#include <qcontainerfwd.h>
#include <qmodbusdataunit.h>
#include <qmodbusdevice.h>
#include <qmodbusreply.h>
#include <qpair.h>

#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <algorithm>

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
  if (size == 0 || size > 125) {
    emit errorOccurred("Invalid request size");
    return;
  }
  // QVector<quint16> resultDatas;
  // 读数据时，没连接则尝试重连
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    emit errorOccurred("Device not connected");
    return;
  }

  // 数据单元都是以一长串地址发送
  QModbusDataUnit dataUnit(dataType, startAddr, size);
  // 构造请求单元并加入队列
  request_queue_.enqueue(dataUnit);
  // 队列串行
  // 触发队列处理
  processNextRequest();
}

/*
 *@brief:  写modbus数据(发送写请求，不需要应答，该操作是异步的)
 *@date:   2024.03.09
 *@param:
 *dataType:写的数据类型，分四类(线圈、离散输入(只读)、输入寄存器(只读)、保持寄存器)
 *@param:  startAddr:起始地址   values:数据块  serverAddr:服务器地址
 *@return: bool:true=发送请求成功 false=请求失败
 */
bool ModbusMaster::writeModbusData(
    const QModbusDataUnit::RegisterType& dataType, const int& startAddr,
    const QVector<quint16>& values, const int& serverAddr) {
  // 写数据时，没连接则尝试重连
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    modbusDevice->connectDevice();
    return false;
  }

  QModbusDataUnit dataUnit(dataType, startAddr, values.size());
  dataUnit.setValues(values);
  // 读取数据有点慢
  // 为什么这里
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
  //           // qDebug() << QString("writeModbusData error:%1(exception code =
  //           %2)")
  //           //                 .arg(reply->errorString())
  //           //                 .arg(reply->rawResult().exceptionCode(), -1,
  //           16);
  //         } else {
  //           emit errorOccurred(
  //               QString("writeModbusData
  //               error:%1").arg(reply->errorString()));
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
          // BUG 为什么读取数据时, 会调用写函数
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

void ModbusMaster::readCoilsData(const QVector<int>& addrs,
                                 const int& serverAddr) {
  // for (auto it = addrs.cbegin(); it != addrs.cend(); ++it) {
  //   readModbusData(QModbusDataUnit::Coils, *it, 1, serverAddr);
  // }
  if (addrs.isEmpty()) {
    return;
  }
  QVector<int> sortedAddrs = addrs;
  // 地址排序
  std::sort(sortedAddrs.begin(), sortedAddrs.end());
  // 最小读取地址
  int startAddr = sortedAddrs.first();
  // 起始地址
  int lastAddr = startAddr;
  // 多个范围
  QVector<QPair<int, int>> ranges;  // <startAddr, count>

  for (int i = 1; i < sortedAddrs.size(); ++i) {
    if (sortedAddrs[i] == lastAddr + 1) {
      // 地址保持连续
      lastAddr = sortedAddrs[i];
    } else {
      // 不连续，创建新的范围
      ranges.append(qMakePair(startAddr, lastAddr - startAddr + 1));
      startAddr = sortedAddrs[i];
      lastAddr = startAddr;
    }
  }
  // 添加最后一个范围
  ranges.append(qMakePair(startAddr, lastAddr - startAddr + 1));

  // 发送合并后的请求
  for (const auto& range : ranges) {
    readModbusData(QModbusDataUnit::Coils, range.first, range.second,
                   serverAddr);
  }
}

void ModbusMaster::readCoilsData(const int& startAddr, const quint16& size,
                                 const int& serverAddr) {
  readModbusData(QModbusDataUnit::Coils, startAddr, size, serverAddr);
}

// void ModbusMaster::errorOccurred(QModbusDevice::Error error) {}

void ModbusMaster::stateChanged(QModbusDevice::State state) {}

// void ModbusMaster::processNextRequest() {
//   if (request_queue_.isEmpty() || is_processing_) {
//     return;
//   }

//   if (modbusDevice->state() != QModbusDevice::ConnectedState) {
//     emit errorOccurred("Connection lost during processing");
//     request_queue_.clear();
//     return;
//   }

//   is_processing_ = true;
//   QModbusDataUnit unit = request_queue_.dequeue();

//   if (auto* reply = modbusDevice->sendReadRequest(unit, 1)) {
//     active_replies_.insert(reply);
//     connect(reply, &QModbusReply::finished, this, [this, reply, unit]() {
//       reply->deleteLater();
//       active_replies_.remove(reply);
//       is_processing_ = false;

//       // 错误处理
//       if (reply->error() != QModbusDevice::NoError) {
//         // 过滤连接关闭错误（此时应停止后续请求）
//         if (reply->error() == QModbusDevice::ConnectionError) {
//           request_queue_.clear();
//           emit errorOccurred("Connection closed by device");
//           return;
//         }

//         // 其他错误上报
//         emit errorOccurred(
//             QString("Request failed: %1").arg(reply->errorString()));
//       } else {
//         // 处理后的数据单元
//         const QModbusDataUnit result = reply->result();
//         // 原始二进制数据
//         const QModbusResponse rawResult = reply->rawResult();
//         // 原始 PDU 数据 - 低级接口
//         // 功能码 + 寄存器值
//         // qDebug() << "功能码:" << rawResult.functionCode();   // 输出 3
//         // qDebug() << "PDU数据:" << rawResult.data().toHex();  // 输出
//         // "020045" rawResult.functionCode() + rawResult.data().toHex();
//         // // 手动解析 PDU 数据
//         // QByteArray data = rawResult.data();
//         // int byteCount = data.at(0);  // 应该是 2
//         // qDebug() << "数据字节数:" << byteCount;

//         if (result.isValid()) {
//           emit dataReceived(result);
//         }
//       }

//       // 是否会导致 请求过慢? 不能精确 100 ms
//       // 处理下一个请求
//       QMetaObject::invokeMethod(this, &ModbusMaster::processNextRequest,
//                                 Qt::QueuedConnection);
//     });
//   } else {
//     is_processing_ = false;
//     emit errorOccurred("Failed to create request");
//   }
// }

void ModbusMaster::processNextRequest() {
  const int MAX_PARALLEL_REQUESTS = 5;  // 最大并行请求数
  if (request_queue_.isEmpty() ||
      active_replies_.size() >= MAX_PARALLEL_REQUESTS) {
    // 超限运行
    return;
  }
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    // emit errorOccurred("Connection lost during processing");
    request_queue_.clear();
    return;
  }
  // 拿出一个请求
  QModbusDataUnit unit = request_queue_.dequeue();
  if (auto* reply = modbusDevice->sendReadRequest(unit, 1)) {
    active_replies_.insert(reply);
    connect(reply, &QModbusReply::finished, this, [this, reply, unit]() {
      reply->deleteLater();
      active_replies_.remove(reply);
      if (reply->error() != QModbusDevice::NoError) {
        // 处理错误
      } else {
        const QModbusDataUnit result = reply->result();
        if (result.isValid()) {
          // 信号发送接收的数据
          emit dataReceived(result);
        }
      }
      // 立即处理下一个请求, 不进入时间循环
      // 是否会造成跨线程问题？
      processNextRequest();
    });
    // 队列中还有请求且未达到最大并行数, 立即处理下一个
    if (!request_queue_.isEmpty() &&
        active_replies_.size() < MAX_PARALLEL_REQUESTS) {
      processNextRequest();
    }
  } else {
    emit errorOccurred("Failed to create request");
  }
}

}  // namespace Core

// void ModbusMaster::readModbusData(const QModbusDataUnit::RegisterType&
// dataType,
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
//   // QModbusReply* reply = modbusDevice->sendReadRequest(dataUnit,
//   serverAddr);
//   // if (reply) {
//   //   if (!reply->isFinished()) {
//   //     // 等待完成
//   //     QEventLoop eventLoop;
//   //     connect(reply, &QModbusReply::finished, &eventLoop,
//   &QEventLoop::quit);
//   //     eventLoop.exec();
//   //   }
//   //   //出错提示
//   //   if (reply->error() != QModbusDevice::NoError) {
//   //     if (reply->error() == QModbusDevice::ProtocolError) {
//   //       emit errorOccurred(
//   //           QString("readModbusData error:%1(exception code = %2)")
//   //               .arg(reply->errorString())
//   //               .arg(reply->rawResult().exceptionCode(), -1, 16));
//   //       // qDebug() << QString("readModbusData error:%1(exception code =
//   %2)")
//   //       //                 .arg(reply->errorString())
//   //       //                 .arg(reply->rawResult().exceptionCode(), -1,
//   16);
//   //     } else {
//   //       // timeout
//   //       emit errorOccurred(
//   //           QString("readModbusData error:%1").arg(reply->errorString()));
//   //       // qDebug()
//   //       //     << QString("readModbusData
//   error:%1").arg(reply->errorString());
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
//   //           for (qsizetype i = 0, total = unit.valueCount(); i < total;
//   ++i) {
//   //             qDebug() << unit.startAddress() << unit.value(i);
//   //           }
//   //         }
//   //       } else if (reply->error() == QModbusDevice::ProtocolError) {
//   //         qDebug() << QString("readModbusData error:%1(exception code =
//   %2)")
//   //                         .arg(reply->errorString())
//   //                         .arg(reply->rawResult().exceptionCode(), -1,
//   16);
//   //         // 信号的问题 ?? 非
//   //         // emit errorOccurred(
//   //         //     QString("readModbusData error:%1(exception code = %2)")
//   //         //         .arg(reply->errorString())
//   //         //         .arg(reply->rawResult().exceptionCode(), -1, 16));
//   //       } else {
//   //         // 此处
//   //         // emit errorOccurred(
//   //         //     QString("readModbusData
//   error:%1").arg(reply->errorString()));
//   //         qDebug()
//   //             << QString("readModbusData
//   error:%1").arg(reply->errorString());
//   //       }
//   //       reply->deleteLater();
//   //     });
//   //   } else {
//   //     delete reply;  // broadcast replies return immediately
//   //   }
//   // } else {
//   //   emit errorOccurred(tr("Read error:
//   %1").arg(modbusDevice->errorString()));
//   // }

//   // 构造请求单元并加入队列
//   // QModbusDataUnit dataUnit(dataType, startAddr, size);
//   request_queue_.enqueue(dataUnit);
//   // m_serverAddr = serverAddr; // 更新服务器地址

//   // 触发队列处理
//   processNextRequest();
// }
