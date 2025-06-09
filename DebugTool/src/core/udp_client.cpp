#include "udp_client.h"

#include <QAbstractSocket>
#include <QNetworkDatagram>

namespace Core {

UdpClient::UdpClient(QObject *parent) : QObject(parent) {
  socket_ = new QUdpSocket(this);
  QObject::connect(socket_, &QUdpSocket::readyRead, this,
                   &UdpClient::readPendingDatagrams);

  QObject::connect(
      socket_, &QUdpSocket::errorOccurred, this,
      [this](QAbstractSocket::SocketError error) {
        if (error == QAbstractSocket::ConnectionRefusedError) {
          // 正常
          qDebug() << "UDP信息：目标端口可能没有应用在监听，但数据包已发送";
          return;
        }
        emit errorOccurred(QString("Socket error: %1").arg(error));
      });
}

UdpClient::~UdpClient() {}

void UdpClient::close() { socket_->close(); }

void UdpClient::sendMessage(const QByteArray &message) {
  writePendingDatagrams(message);
}

bool UdpClient::isConnected() const {
  // return socket_ && socket_->state() == QAbstractSocket::BoundState;
  return socket_ && socket_->state() != QAbstractSocket::UnconnectedState &&
         !target_ip_.isEmpty() && target_port_ > 0;
}

void UdpClient::connectToOther(TtUdpMode::Mode mode, const QString &targetIp,
                               const QString &targetPort, const QString &selfIp,
                               const QString &selfPort) {
  qDebug() << "connect";
  if (socket_->state() != QAbstractSocket::UnconnectedState) {
    socket_->close();
  }
  mode_ = mode;
  // 目的端口
  target_ip_ = targetIp;
  target_port_ = targetPort.toInt();

  // 验证目标地址和端口
  if (target_ip_.isEmpty() || target_port_ <= 0) {
    emit errorOccurred("Invalid target address or port");
    return;
  }

  // 如果没有自定自己的 ip, 将会使用目标地址相同的 ip
  bool bindSuccess = false;
  QHostAddress bindAddress;
  // 处理未指定IP的情况
  if (selfIp.isEmpty()) {
    bindAddress = QHostAddress(targetIp); // 默认使用目标IP作为本地IP
  } else {
    bindAddress = QHostAddress(selfIp);
  }
  // 处理端口：如果selfPort为空，传入0让Qt自动分配
  int port = selfPort.isEmpty() ? 0 : selfPort.toInt();
  if (port == 0) {
    // qDebug() << "No port specified, Qt will assign a random port";
  }
  switch (mode) {
  case TtUdpMode::Unicast: { // 单播
    bindSuccess = socket_->bind(bindAddress, port);
    if (!bindSuccess) {
      emit errorOccurred(
          QString("Failed to bind socket: %1").arg(socket_->errorString()));
    } else {
      // qDebug() << "success bind ip";
    }
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    // 和单播一样, 不同之处只在目的地址
    // 多播模式 - 绑定本地地址和端口，并加入多播组
    qDebug() << "UDP Multicast Mode";
    qDebug() << "Binding to" << selfIp << ":" << selfPort;
    qDebug() << "Joining multicast group" << targetIp;
    // 首先绑定到指定端口
    // bindSuccess = socket_->bind(
    //     selfIp.isEmpty() ? QHostAddress::Any : QHostAddress(selfIp),
    //     selfPort.toInt(),
    // QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    bindSuccess = socket_->bind(bindAddress, port,
                                QAbstractSocket::ShareAddress |
                                    QAbstractSocket::ReuseAddressHint);

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
    // bindSuccess = socket_->bind(
    //     selfIp.isEmpty() ? QHostAddress::Any : QHostAddress(selfIp),
    //     selfPort.toInt(),
    //     QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    bindSuccess = socket_->bind(bindAddress, port,
                                QAbstractSocket::ShareAddress |
                                    QAbstractSocket::ReuseAddressHint);

    if (!bindSuccess) {
      emit errorOccurred(
          QString("Failed to bind socket: %1").arg(socket_->errorString()));
    }
    break;
  }
  }
  // 连接状态报告
  if (bindSuccess) {
    // qDebug() << "Socket successfully bound to port" << socket_->localPort();
    emit connected();
  } else {
    // 执行了
    qDebug() << "Socket binding failed";
    emit disconnected();
  }
}

void UdpClient::connectToOther(const UdpClientConfiguration &config) {
  qDebug() << config.target_ip << config.target_port;
  // 这里没有问题
  UdpClient::connectToOther(config.mode, config.target_ip, config.target_port,
                            config.self_ip, config.self_port);
}

void UdpClient::readPendingDatagrams() {
  while (socket_->hasPendingDatagrams()) {
    QNetworkDatagram datagram = socket_->receiveDatagram();
    if (!datagram.isValid()) {
      // BUG 读取, 这里有问题, 会接收到对应的无效数据报, 发送怎么
      // 发送数据会读取对应的数据包
      // emit errorOccurred("Invalid datagram received");
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
  // if (!socket_ || socket_->state() != QAbstractSocket::BoundState) {
  if (!socket_) {
    emit errorOccurred("Socket not initialized");
    return false;
  }
  qDebug() << "test";
  // qDebug() << "Sending data, size:" << message.size() << "bytes";
  bool success = false;
  qint64 bytesSent = 0;
  switch (mode_) {
  case TtUdpMode::Unicast: { // 单播
    // qDebug() << "Sending unicast to" << target_ip_ << ":" << target_port_;
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
    // qDebug() << "Sending multicast to" << target_ip_ << ":" << target_port_;
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
    // qDebug() << "Sending broadcast to port" << target_port_;
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
  return success;
}

} // namespace Core
