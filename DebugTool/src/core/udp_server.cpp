#include "core/udp_server.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QNetworkDatagram>

namespace Core {

UdpServer::UdpServer(QObject *parent) : QObject(parent) {
  server_socket_ = new QUdpSocket(this);
  connect(server_socket_, &QUdpSocket::readyRead, this,
          &UdpServer::readPendingDatagrams);
}

UdpServer::~UdpServer() { shutdown(); }

bool UdpServer::listen(const QHostAddress &address, quint16 port) {
  // if (server_socket_->state() != QAbstractSocket::BoundState) {
  server_socket_->close(); // 先关闭已有的绑定
  // }
  listening_ = server_socket_->bind(address, port);
  if (!listening_) {
    emit errorOccurred("Bind Failed: " + server_socket_->errorString());
  }
  return listening_;
}

bool UdpServer::listen(const UdpServerConfiguration &config) {
  qDebug() << config.self_host << config.self_port;
  return listen(QHostAddress(config.self_host), config.self_port.toInt());
}

bool UdpServer::isListening() const { return listening_; }

void UdpServer::sendMessage(const QByteArray &message) {
  writePendingDatagrams(message);
}

void UdpServer::sendTo(const QByteArray &message, const QHostAddress &address,
                       quint16 port) {
  server_socket_->writeDatagram(message, address, port);
}

void UdpServer::close() {
  listening_ = false;
  server_socket_->close();
}

void UdpServer::sendBroadcast(const QByteArray &message, quint16 targetPort) {
  QHostAddress broadcastAddress = QHostAddress::Broadcast;
  qint64 bytesWritten =
      server_socket_->writeDatagram(message, broadcastAddress, targetPort);
  if (bytesWritten == -1) {
    emit errorOccurred("Broadcast failed: " + server_socket_->errorString());
  }
}

void UdpServer::sendMulticast(const QByteArray &message,
                              const QString &multicastGroup,
                              quint16 targetPort) {
  QHostAddress multicastAddress(multicastGroup);
  if (!multicastAddress.isMulticast()) {
    emit errorOccurred("Invalid multicast address: " + multicastGroup);
    return;
  }
  qint64 bytesWritten =
      server_socket_->writeDatagram(message, multicastAddress, targetPort);
  if (bytesWritten == -1) {
    qDebug() << "Multicast failed:" << server_socket_->errorString();
    emit errorOccurred("Multicast failed: " + server_socket_->errorString());
  } else {
    qDebug() << "Multicast sent:" << bytesWritten << "bytes to"
             << multicastGroup << ":" << targetPort;
  }
}

void UdpServer::sendToSubnet(const QByteArray &message, const QString &subnet,
                             quint16 targetPort) {
  // 发送到子网中的所有地址 (例如: 192.168.1.0/24)
  QStringList parts = subnet.split('/');
  if (parts.size() != 2) {
    emit errorOccurred("Invalid subnet format. Use: 192.168.1.0/24");
    return;
  }

  QHostAddress networkAddr(parts[0]);
  int prefixLength = parts[1].toInt();

  if (prefixLength < 24 || prefixLength > 30) {
    emit errorOccurred("Prefix length should be between 24-30 for safety");
    return;
  }

  // 计算子网中的所有主机地址
  quint32 network = networkAddr.toIPv4Address();
  quint32 hostMask = (1 << (32 - prefixLength)) - 1;

  int successCount = 0;
  for (quint32 host = 1; host < hostMask; ++host) {
    QHostAddress hostAddr(network | host);
    qint64 bytesWritten =
        server_socket_->writeDatagram(message, hostAddr, targetPort);
    if (bytesWritten > 0) {
      successCount++;
    }
  }

  qDebug() << "Subnet broadcast completed. Sent to" << successCount
           << "addresses";
}

void Core::UdpServer::UdpServer::startServiceDiscovery(quint16 discoveryPort) {
  // 监听服务发现请求
  QUdpSocket *discoverySocket = new QUdpSocket(this);
  if (discoverySocket->bind(QHostAddress::Any, discoveryPort)) {
    connect(discoverySocket, &QUdpSocket::readyRead, this,
            [this, discoverySocket]() {
              while (discoverySocket->hasPendingDatagrams()) {
                QNetworkDatagram datagram = discoverySocket->receiveDatagram();
                QByteArray data = datagram.data();

                // 检查是否为服务发现请求
                if (data == "DISCOVER_SERVICE") {
                  // 回复服务信息
                  QJsonObject serviceInfo;
                  serviceInfo["service"] = "UDP_DEBUG_TOOL";
                  serviceInfo["port"] = server_socket_->localPort();
                  serviceInfo["version"] = "1.0";

                  QJsonDocument doc(serviceInfo);
                  QByteArray response = doc.toJson(QJsonDocument::Compact);

                  discoverySocket->writeDatagram(response,
                                                 datagram.senderAddress(),
                                                 datagram.senderPort());
                  qDebug() << "Service discovery "
                              "response sent to"
                           << datagram.senderAddress().toString();
                }
              }
            });
    qDebug() << "Service discovery started on port" << discoveryPort;
  }
}

void UdpServer::readPendingDatagrams() {
  /*
  while (server_socket_->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(server_socket_->pendingDatagramSize());
    QHostAddress peerAddress;
    quint16 peerPort = 0;

    // 读取数据报
    qint64 bytesRead = server_socket_->readDatagram(
        datagram.data(), datagram.size(), &peerAddress, &peerPort);

    if (bytesRead > 0) {
      // 记录新客户端
      known_clients_.insert(qMakePair(peerAddress, peerPort));
      emit datagramReceived(peerAddress.toString(), peerPort, datagram);
    } else {
      // qWarning() << "Failed to read datagram:" <<
  server_socket_.errorString(); emit errorOccurred("Failed to read datagram:" +
                         server_socket_->errorString());
    }
  }
*/
  while (server_socket_->hasPendingDatagrams()) {
    // 数据报
    QNetworkDatagram datagram = server_socket_->receiveDatagram();
    // 封装消息
    QByteArray data = datagram.data();
    // 发送方地址
    QHostAddress sender = datagram.senderAddress();
    // 发送方端口
    quint16 sendPort = datagram.senderPort();

    // 确保地址格式正确
    if (sender.protocol() == QAbstractSocket::IPv4Protocol) {
      // 对于IPv4地址，确保使用正确的格式
      sender = QHostAddress(sender.toIPv4Address());
    }
    // 扩展消息
    // QHostAddress destinationAddress = datagram.destinationAddress();
    // int hopLimit = datagram.hopLimit();  // 跳数限制 TTL
    auto clientPair = qMakePair(sender, sendPort);
    if (!known_clients_.contains(clientPair)) {
      known_clients_.insert(clientPair);
      // 如果是新客户端, 记录
      qDebug() << "New client connected:" << sender.toString() << sendPort;
    } else {
      // 已经存在的客户端
      // 新的客户端
      qDebug() << "Known client reconnected:" << sender.toString() << sendPort;
    }
    emit datagramReceived(sender.toString(), sendPort, data);
  }
}

void UdpServer::writePendingDatagrams(const QByteArray &message) {
  if (known_clients_.isEmpty()) {
    emit errorOccurred(tr("没有客户端链接"));
    return;
  }
  int successCount = 0;
  for (const auto &client : known_clients_) {
    const QHostAddress &addr = client.first;
    quint16 port = client.second;

    qint64 bytesWritten = server_socket_->writeDatagram(message, addr, port);
    if (bytesWritten == -1) {
      qDebug() << "Failed to send to" << addr.toString() << ":" << port
               << "Error:" << server_socket_->errorString();
    } else if (bytesWritten != message.size()) {
      qDebug() << "Partial send to" << addr.toString() << ":" << port
               << "Sent:" << bytesWritten << "Expected:" << message.size();
    } else {
      qDebug() << "Successfully sent" << bytesWritten << "bytes to"
               << addr.toString() << ":" << port;
      successCount++;
    }
  }

  if (successCount == 0) {
    emit errorOccurred("Failed to send message to any client");
  }
}

void UdpServer::shutdown() {
  // 清空客户端列表并关闭 Socket
  known_clients_.clear();
  server_socket_->close();
  listening_ = false;
}

} // namespace Core

// namespace {

// // 获取对方信息的字符串
// QString PeerInfo(const QHostAddress &address, quint16 port) {
//   const static QString info = QStringLiteral("(%1:%2)");
//   return info.arg(address.toString()).arg(port);
// }

// // 获取连接信息的字符串(加密套件和协议)
// QString ConnectionInfo(QDtls *connection) {
//   QString info(Core::UdpServer::tr("Session cipher: "));
//   info += connection->sessionCipher().name();   // 添加加密套件名称
//   info += Core::UdpServer::tr("; session protocol: ");
//   switch (connection->sessionProtocol()) {
//     case QSsl::DtlsV1_2:
//       info += Core::UdpServer::tr("DTLS 1.2."); // 添加协议版本
//       break;
//     default:
//       info += Core::UdpServer::tr("Unknown protocol."); // 未知协议
//   }
//   return info;
// }

// } // 匿名空间

// UdpServer::UdpServer(QObject* parent) : QObject(parent) {
//   // qDebug() << " SSL Backend" << QSslSocket::sslLibraryVersionString();
//   // 在 main() 或初始化代码中检查 OpenSSL 是否可用
//   if (!QSslSocket::supportsSsl()) {
//     qDebug() << "OpenSSL not available!";
//   }

//   connect(&server_socket_, &QAbstractSocket::readyRead, this,
//   &UdpServer::readyRead);

//   // 初始化 DTLS 服务器配置
//   server_configuration_ = QSslConfiguration::defaultDtlsConfiguration();
//   // 设置预共享密钥身份提示
//   /*
//    * erro qt.network.ssl: The backend "schannel" does not support DTLS
//    cookies
//    * Windows 默认的 SSL 后端 schannel（微软的安全通道）不支持 DTLS Cookies
//    功能。
//   */
//   server_configuration_.setPreSharedKeyIdentityHint("Qt DTLS example
//   server");
//   // 设置不验证对等方

//   /*
//    * Error: "DTLS error: No TLS backend is available, no client verification"
//    * Qt 未找到支持 DTLS 的 TLS 后端，或后端未正确配置（如缺少 OpenSSL 库）。
//    * 配置 Qt 使用 OpenSSL, 将 OpenSSL 的 DLL 文件（libcrypto-1_1-x64.dll 和
//    libssl-1_1-x64.dll）复制到：
//    * Qt 的 bin 目录（如 C:\Qt\6.5.0\mingw_64\bin）。
//    * 或应用程序的输出目录（如 build/release）。
//   */
//   server_configuration_.setPeerVerifyMode(QSslSocket::VerifyNone);
// }

// UdpServer::~UdpServer()
// {
//   shutdown();
// }

// bool UdpServer::listen(const QHostAddress &address, quint16 port)
// {
//   if (address != server_socket_.localAddress() || port !=
//   server_socket_.localPort()) {
//     // 本地地址和端口不匹配
//     shutdown();
//     // 绑定
//     listening_ = server_socket_.bind(address, port);
//     if (!listening_) {
//       // 绑定失败, 发送错误消息
//       emit errorMessage(server_socket_.errorString());
//     }
//   } else {
//     // 处于正在监听的状态
//     listening_ = true;
//   }
//   qDebug() << listening_;
//   // 返回监听状态
//   return listening_;
// }

// bool UdpServer::isListening() const
// {
//   return listening_;
// }

// void UdpServer::close()
// {
//   listening_ = false;
// }

// void UdpServer::readyRead()
// {
//   // 获取数据包大小
//   const qint64 bytesToRead = server_socket_.pendingDatagramSize();
//   if (bytesToRead <= 0) {
//     emit warningMessage(tr("虚假的读取通知"));
//     return;
//   }

//   // 构造的字节数据是未初始化
//   // 初始化数据报文
//   QByteArray dgram(bytesToRead, Qt::Uninitialized);
//   QHostAddress peerAddress;
//   quint16 peerPort = 0;

//   // 读取数据报
//   const quint64 bytesRead = server_socket_.readDatagram(dgram.data(),
//   dgram.size(), &peerAddress, &peerPort); if (bytesRead <= 0) {
//     emit warningMessage(tr("Failed to read a datagram: ") +
//     server_socket_.errorString()); return;
//   }

//   // 调整数据报文大小
//   dgram.resize(bytesRead);

//   if (peerAddress.isNull() || !peerPort) {
//     // 提取失败
//     emit warningMessage(tr("Failed to extract peer info (address, port)"));
//   }

//   // 查找已知的客户端连接
//   const auto client = std::find_if(known_clients_.begin(),
//   known_clients_.end(), [&](const std::unique_ptr<QDtls> &connection) {
//     return connection->peerAddress() == peerAddress && connection->peerPort()
//     == peerPort;
//   });

//   // 客户端不存在, 处理新的连接
//   if (client == known_clients_.end()) {
//     return handleNewConnection(peerAddress, peerPort, dgram);
//   }

//   // 如果连接已经加密, 解密数据包
//   if ((*client)->isConnectionEncrypted()) {
//     decryptDatagram(client->get(), dgram);
//     if ((*client)->dtlsError() == QDtlsError::RemoteClosedConnectionError) {
//       known_clients_.erase(client);
//     }
//     return;
//   }

//   // 执行握手
//   doHandShake(client->get(), dgram);

// }

// void UdpServer::pskRequired(QSslPreSharedKeyAuthenticator *auth)
// {
//   Q_ASSERT(auth);

//   // 发出信息信号
//   emit infoMessage(tr("PSK callback, received a client's identity:
//   '%1'").arg(QString::fromLatin1(auth->identity())));
//   auth->setPreSharedKey(QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f"));
// }

// void UdpServer::handleNewConnection(const QHostAddress &peerAddress, quint16
// peer_port, const QByteArray &client_hello)
// {
//   if (!listening_) {
//     return;
//   }
//   // 获取对等方信息
//   const QString peer_info = PeerInfo(peerAddress, peer_port);
//   // 验证客户端
//   if (cookie_sender_.verifyClient(&server_socket_, client_hello, peerAddress,
//   peer_port)) {
//     emit infoMessage(peer_info + tr(": verified, starting a handshake"));

//     // 创建新的 DTLS 连接
//     std::unique_ptr<QDtls> newConnection {new
//     QDtls{QSslSocket::SslServerMode}};
//     // 设置 DTLS 配置
//     newConnection->setDtlsConfiguration(server_configuration_);
//     // 设置对等方
//     newConnection->setPeer(peerAddress, peer_port);
//     // 连接 PKS 对等方
//     newConnection->connect(newConnection.get(), &QDtls::pskRequired, this,
//     &UdpServer::pskRequired);
//     // 保存新的客户端连接
//     known_clients_.push_back(std::move(newConnection));
//     // 执行握手
//     doHandShake(known_clients_.back().get(), client_hello);

//   } else if (cookie_sender_.dtlsError() != QDtlsError::NoError) {
//     // 报告 DTLS 错误
//     emit errorMessage(tr("DTLS error: ") + cookie_sender_.dtlsErrorString());
//   } else {
//     // 未验证的信息
//     emit infoMessage(peer_info + tr(": not verified yet"));
//   }

// }

// void UdpServer::doHandShake(QDtls *new_connection, const QByteArray
// &client_hello)
// {
//   // 执行握手操作
//   const bool result = new_connection->doHandshake(&server_socket_,
//   client_hello); if (!result) {
//     // 握手失败
//     emit errorMessage(new_connection->dtlsErrorString());
//     return;
//   }
//   // 获取对等方信息
//   const QString peerInfo = PeerInfo(new_connection->peerAddress(),
//   new_connection->peerPort());

//   switch (new_connection->handshakeState()) {
//     case QDtls::HandshakeInProgress:
//       emit infoMessage(peerInfo + tr(": handshake is in progress..."));
//       break;
//     case QDtls::HandshakeComplete:
//       emit infoMessage(tr("Connection with %1 encrypted. %2").arg(peerInfo,
//       ConnectionInfo(new_connection)));
//     default:
//       Q_UNREACHABLE();
//   }
// }

// void UdpServer::decryptDatagram(QDtls *connection, const QByteArray
// &client_message)
// {
//   // 确保连接是加密的
//   Q_ASSERT(connection->isConnectionEncrypted());
//   // 获取对等方消息
//   const QString peerInfo = PeerInfo(connection->peerAddress(),
//   connection->peerPort());
//   // 解密数据报
//   const QByteArray dgram = connection->decryptDatagram(&server_socket_,
//   client_message); if (dgram.size()) {
//     emit datagramReceived(peerInfo, client_message, dgram);
//     // 发出确认帧
//     connection->writeDatagramEncrypted(&server_socket_, tr("to %1:
//     ACK").arg(peerInfo).toLatin1());
//   } else if (connection->dtlsError() == QDtlsError::NoError) {
//     // 处理空数据报
//     emit warningMessage(peerInfo + ": " + tr("0 byte dgram, could be a
//     re-connect attempt"));
//   } else {
//     // 发送错误消息
//     emit errorMessage(peerInfo + ": " + connection->dtlsErrorString());
//   }
// }

// void UdpServer::shutdown()
// {
//   // 关闭所有已知客户端连接
//   for (const auto &connection : std::exchange(known_clients_, {})) {
//     connection->shutdown(&server_socket_);
//   }

//   // 关闭服务器 socket
//   server_socket_.close();
// }
