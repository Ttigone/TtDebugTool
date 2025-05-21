#include "udp_client.h"

#include <QAbstractSocket>
#include <QNetworkDatagram>

namespace Core {

UdpClient::UdpClient(QObject *parent) : QObject(parent) {
  socket_ = new QUdpSocket(this);
  QObject::connect(socket_, &QUdpSocket::readyRead, this,
                   &UdpClient::readPendingDatagrams);

  QObject::connect(socket_, &QUdpSocket::errorOccurred, this,
                   [this](QAbstractSocket::SocketError error) {
                     emit errorOccurred(QString("Socket error: %1").arg(error));
                   });
}

UdpClient::~UdpClient() {}

void UdpClient::close() { socket_->close(); }

void UdpClient::sendMessage(const QByteArray &message) {
  writePendingDatagrams(message);
}

bool UdpClient::isConnected() const {
  return socket_ && socket_->state() == QAbstractSocket::BoundState;
}

void UdpClient::connectToOther(TtUdpMode::Mode mode, const QString &targetIp,
                               const QString &targetPort, const QString &selfIp,
                               const QString &selfPort) {
  if (socket_->state() != QAbstractSocket::UnconnectedState) {
    // 关闭之前的链接
    socket_->close();
  }
  mode_ = mode;
  target_ip_ = targetIp;
  target_port_ = targetPort.toInt();
  bool bindSuccess = false;

  switch (mode) {
  case TtUdpMode::Unicast: { // 单播
    qDebug() << "UDP Unicast Mode";
    qDebug() << "Binding to" << selfIp << ":" << selfPort;
    bindSuccess = socket_->bind(selfIp.isEmpty() ? QHostAddress::Any
                                                 : QHostAddress(selfIp),
                                selfPort.toInt());
    if (!bindSuccess) {
      emit errorOccurred(
          QString("Failed to bind socket: %1").arg(socket_->errorString()));
    }
    // socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    // 和单播一样, 不同之处只在目的地址
    // 多播模式 - 绑定本地地址和端口，并加入多播组
    qDebug() << "UDP Multicast Mode";
    qDebug() << "Binding to" << selfIp << ":" << selfPort;
    qDebug() << "Joining multicast group" << targetIp;
    // 首先绑定到指定端口
    bindSuccess = socket_->bind(
        selfIp.isEmpty() ? QHostAddress::Any : QHostAddress(selfIp),
        selfPort.toInt(),
        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if (bindSuccess) {
      // 然后加入多播组
      QHostAddress multicastAddress(targetIp);
      if (multicastAddress.isMulticast()) {
        bindSuccess = socket_->joinMulticastGroup(multicastAddress);
        if (!bindSuccess) {
          emit errorOccurred(QString("Failed to join multicast group: %1")
                                 .arg(socket_->errorString()));
        }
      } else {
        emit errorOccurred(
            QString("Invalid multicast address: %1").arg(targetIp));
        bindSuccess = false;
      }
    } else {
      emit errorOccurred(
          QString("Failed to bind socket: %1").arg(socket_->errorString()));
    }
    // socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    break;
  }
  case TtUdpMode::Broadcast: { // 广播/组播
    // socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    // 广播模式 - 绑定本地地址和端口，启用广播
    qDebug() << "UDP Broadcast Mode";
    qDebug() << "Binding to" << selfIp << ":" << selfPort;
    // 加入多播
    // 启用广播
    // socket_->setSocketOption(QAbstractSocket::BroadcastFlag, true);
    // socket_->setSocketOption(QAbstractSocket::BroadcastSocketOption, true);
    // Qt 缺少广播的代码

    bindSuccess = socket_->bind(
        selfIp.isEmpty() ? QHostAddress::Any : QHostAddress(selfIp),
        selfPort.toInt(),
        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if (!bindSuccess) {
      emit errorOccurred(
          QString("Failed to bind socket: %1").arg(socket_->errorString()));
    }
    break;
  }
  }
  // 连接状态报告
  if (bindSuccess) {
    // emit connected();
  } else {
    // emit disconnected();
  }
}

void UdpClient::connectToOther(const UdpClientConfiguration &config) {
  // UdpClient::connectToOther(config.mode, config.target_ip,
  // config.target_port,
  //                           config.self_ip, config.target_port);
  UdpClient::connectToOther(config.mode, config.target_ip, config.target_port,
                            config.self_ip, config.self_port);
}

void UdpClient::readPendingDatagrams() {
  while (socket_->hasPendingDatagrams()) {
    QNetworkDatagram datagram = socket_->receiveDatagram();
    if (!datagram.isValid()) {
      emit errorOccurred("Invalid datagram received");
      continue;
    }
    QByteArray data = datagram.data();
    QHostAddress sender = datagram.senderAddress();
    quint16 senderPort = datagram.senderPort();

    // 记录接收信息
    qDebug() << "Received datagram from" << sender.toString() << ":"
             << senderPort << "Size:" << data.size() << "bytes";

    // 读取数据
    switch (mode_) {
    case TtUdpMode::Unicast: { // 单播
      // 一对一通信
      emit dataReceived(data, sender, senderPort);
      break;
    }
    case TtUdpMode::Multicast: { // 多播
      // 地址不同
      // 多播模式 - 验证数据包是否来自预期的多播组
      QHostAddress targetAddress(target_ip_);

      // 检查是否是多播地址范围 (224.0.0.0 - 239.255.255.255)
      if (targetAddress.isMulticast()) {
        // 即使数据包不是来自多播地址，我们也接收它
        // 因为多播发送方通常使用普通unicast地址
        emit dataReceived(data, sender, senderPort);
      } else {
        qDebug() << "Warning: Not configured with a proper multicast address";
        // 仍然发送接收到的数据
        emit dataReceived(data, sender, senderPort);
      }
      break;
    }
    case TtUdpMode::Broadcast: { // 广播/组播
      // 广播模式 - 通常接收所有目的广播地址的包

      // 对于广播，我们需要验证它是否发送到广播地址
      // 但由于UDP接收不区分广播和单播接收，所以这里只是记录
      if (datagram.destinationAddress() == QHostAddress::Broadcast) {
        qDebug() << "Received broadcast packet";
      } else {
        qDebug() << "Received packet to address:"
                 << datagram.destinationAddress().toString();
      }

      emit dataReceived(data, sender, senderPort);
      break;
    }
    }
  }
}

bool UdpClient::writePendingDatagrams(const QByteArray &message) {
  // qDebug() << "Send" << message;
  // QNetworkDatagram datagram;
  // datagram.setData(message);
  // datagram.setDestination(QHostAddress(target_ip_), target_port_);
  // switch (mode_) {
  //   case TtUdpMode::Unicast: {  // 单播
  //     qDebug() << "SU";
  //     socket_->writeDatagram(datagram);
  //     break;
  //   }
  //   case TtUdpMode::Multicast: {  // 多播
  //     qDebug() << "SM";
  //     socket_->writeDatagram(datagram);
  //     break;
  //   }
  //   case TtUdpMode::Broadcast: {  // 广播/组播
  //     qDebug() << "SB";
  //     socket_->writeDatagram(datagram);
  //     break;
  //   }
  // }
  if (!socket_ || socket_->state() != QAbstractSocket::BoundState) {
    emit errorOccurred("Socket not bound, cannot send data");
    return false;
  }

  qDebug() << "Sending data, size:" << message.size() << "bytes";

  bool success = false;
  qint64 bytesSent = 0;

  switch (mode_) {
  case TtUdpMode::Unicast: { // 单播
    qDebug() << "Sending unicast to" << target_ip_ << ":" << target_port_;
    QNetworkDatagram datagram(message, QHostAddress(target_ip_), target_port_);
    bytesSent = socket_->writeDatagram(datagram);
    success = (bytesSent == message.size());
    if (!success) {
      emit errorOccurred(
          QString("Failed to send unicast: %1").arg(socket_->errorString()));
    }
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    qDebug() << "Sending multicast to" << target_ip_ << ":" << target_port_;
    QHostAddress multicastAddress(target_ip_);
    if (multicastAddress.isMulticast()) {
      QNetworkDatagram datagram(message, multicastAddress, target_port_);
      bytesSent = socket_->writeDatagram(datagram);
      success = (bytesSent == message.size());
      if (!success) {
        emit errorOccurred(QString("Failed to send multicast: %1")
                               .arg(socket_->errorString()));
      }
    } else {
      emit errorOccurred(
          QString("Invalid multicast address: %1").arg(target_ip_));
    }
    break;
  }
  case TtUdpMode::Broadcast: { // 广播
    qDebug() << "Sending broadcast to port" << target_port_;
    QNetworkDatagram datagram(message, QHostAddress::Broadcast, target_port_);
    bytesSent = socket_->writeDatagram(datagram);
    success = (bytesSent == message.size());
    if (!success) {
      emit errorOccurred(
          QString("Failed to send broadcast: %1").arg(socket_->errorString()));
    }
    break;
  }
  }

  // return success;
}

} // namespace Core
