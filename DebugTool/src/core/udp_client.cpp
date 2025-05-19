#include "udp_client.h"

#include <QAbstractSocket>
#include <QNetworkDatagram>

namespace Core {

UdpClient::UdpClient(QObject *parent) : QObject(parent) {
  socket_ = new QUdpSocket(this);
  QObject::connect(socket_, &QUdpSocket::readyRead, this,
                   &UdpClient::readPendingDatagrams);
}

UdpClient::~UdpClient() {}

void UdpClient::close() { socket_->close(); }

void UdpClient::sendMessage(const QByteArray &message) {
  writePendingDatagrams(message);
}

void UdpClient::connect(TtUdpMode::Mode mode, const QString &targetIp,
                        const QString &targetPort, const QString &selfIp,
                        const QString &selfPort) {
  mode_ = mode;
  switch (mode) {
  case TtUdpMode::Unicast: { // 单播
    // 一对一通信
    qDebug() << "U";
    socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    // 和单播一样, 不同之处只在目的地址
    qDebug() << "M";
    socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    break;
  }
  case TtUdpMode::Broadcast: { // 广播/组播
    // socket_->setSocketOption(QAbstractSocket::SocketOption::,
    // socket_->setSocketOption()
    // 1);  // 启用广播
    qDebug() << "B";
    // socket_->writeDatagram("Broadcast Data", QHostAddress::Broadcast, 12345);
    // QHostAddress groupAddress(targetIp);
    socket_->bind(QHostAddress(selfIp), selfPort.toInt());
    break;
  }
  }
  target_ip_ = targetIp;
  target_port_ = targetPort.toInt();
}

void UdpClient::connect(const UdpClientConfiguration &config) {
  UdpClient::connect(config.mode, config.target_ip, config.target_port,
                     config.self_ip, config.target_port);
}

void UdpClient::readPendingDatagrams() {
  switch (mode_) {
  case TtUdpMode::Unicast: { // 单播
    // 一对一通信
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    // 地址不同
    break;
  }
  case TtUdpMode::Broadcast: { // 广播/组播
    break;
  }
  }
}

void UdpClient::writePendingDatagrams(const QByteArray &message) {
  // 获取是空的
  qDebug() << "Send" << message;
  QNetworkDatagram datagram;
  datagram.setData(message);
  datagram.setDestination(QHostAddress(target_ip_), target_port_);
  switch (mode_) {
  case TtUdpMode::Unicast: { // 单播
    qDebug() << "SU";
    socket_->writeDatagram(datagram);
    break;
  }
  case TtUdpMode::Multicast: { // 多播
    qDebug() << "SM";
    socket_->writeDatagram(datagram);
    break;
  }
  case TtUdpMode::Broadcast: { // 广播/组播
    qDebug() << "SB";
    socket_->writeDatagram(datagram);
    break;
  }
  }
}

} // namespace Core
