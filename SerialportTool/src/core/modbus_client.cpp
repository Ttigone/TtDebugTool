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
  if (config.type == TtModbusProcotol::RTU ||
      config.type == TtModbusProcotol::RTU_ASCLL) {
    qDebug() << "R";
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

    qDebug() << "T";
    modbusDevice = new QModbusTcpClient(this);
  } else if (config.type == TtModbusProcotol::UDP) {
    qDebug() << "U";
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
  return modbusDevice->state() == QModbusDevice::ConnectedState;
}

void ModbusMaster::toDisconnect() {
  modbusDevice->disconnectDevice();
}

/*
 *@brief:  读modbus数据(同步不阻塞界面)
 *注:modbus采用的是异步通信，QModbusReply默认也是通过finished信号异步处理，但这种异步方式在传递数据时不太友好(除了读取的数据外，
 *还需要记录读请求的数据类型、起始地址、服务器地址等)。所以这里通过QEventLoop实现一个同步但不阻塞界面的处理，方便对读取的数据进行处理。
 *@date:   2024.03.09
 *@param:  dataType:读的数据类型，分四类(线圈、离散输入(只读)、输入寄存器(只读)、保持寄存器)
 *@param:  startAddr:起始地址   size:数据块大小  serverAddr:服务器地址
 *@return: QVector<quint16>:数据块，出错返回空数组
 */
// QVector<quint16> ModbusMaster::readModbusData
void ModbusMaster::readModbusData(const QModbusDataUnit::RegisterType& dataType,
                                  const int& startAddr, const quint16& size,
                                  const int& serverAddr) {
  // 返回的数据
  QVector<quint16> resultDatas;
  //读数据时，没连接则尝试重连
  if (modbusDevice->state() != QModbusDevice::ConnectedState) {
    // 链接设备
    modbusDevice->connectDevice();
    emit dataReceived(resultDatas);
    // return resultDatas;
  }
  // 数据单元
  QModbusDataUnit dataUnit(dataType, startAddr, size);
  // 发射读取请求
  QModbusReply* reply = modbusDevice->sendReadRequest(dataUnit, serverAddr);
  if (reply) {
    if (!reply->isFinished()) {
      // 等待完成
      QEventLoop eventLoop;
      connect(reply, &QModbusReply::finished, &eventLoop, &QEventLoop::quit);
      eventLoop.exec();
    }
    //出错提示
    if (reply->error() != QModbusDevice::NoError) {
      if (reply->error() == QModbusDevice::ProtocolError) {
        emit errorOccurred(
            QString("readModbusData error:%1(exception code = %2)")
                .arg(reply->errorString())
                .arg(reply->rawResult().exceptionCode(), -1, 16));
        // qDebug() << QString("readModbusData error:%1(exception code = %2)")
        //                 .arg(reply->errorString())
        //                 .arg(reply->rawResult().exceptionCode(), -1, 16);
      } else {
        emit errorOccurred(
            QString("readModbusData error:%1").arg(reply->errorString()));
        // qDebug()
        //     << QString("readModbusData error:%1").arg(reply->errorString());
      }
    } else {
      //处理应答
      const QModbusDataUnit unit = reply->result();
      if (unit.isValid()) {
        resultDatas = unit.values();
      }
    }
    reply->deleteLater();
  }
  emit dataReceived(resultDatas);
  // return resultDatas;
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
  QModbusReply* reply = modbusDevice->sendWriteRequest(dataUnit, serverAddr);
  if (reply) {
    if (!reply->isFinished()) {
      //出错提示
      connect(reply, &QModbusReply::finished, this, [this, reply]() {
        if (reply->error() != QModbusDevice::NoError) {
          if (reply->error() == QModbusDevice::ProtocolError) {
            emit errorOccurred(
                QString("writeModbusData error:%1(exception code = %2)")
                    .arg(reply->errorString())
                    .arg(reply->rawResult().exceptionCode(), -1, 16));
            // qDebug() << QString("writeModbusData error:%1(exception code = %2)")
            //                 .arg(reply->errorString())
            //                 .arg(reply->rawResult().exceptionCode(), -1, 16);
          } else {
            emit errorOccurred(
                QString("writeModbusData error:%1").arg(reply->errorString()));
            // qDebug() << QString("writeModbusData error:%1")
            //                 .arg(reply->errorString());
          }
        }
        reply->deleteLater();
      });
    } else {
      reply->deleteLater();
    }
    return true;
  } else {
    return false;
  }
}

// void ModbusMaster::errorOccurred(QModbusDevice::Error error) {}

void ModbusMaster::stateChanged(QModbusDevice::State state) {}

}  // namespace Core
