#include "core/udp_server.h"

namespace {

// 获取对方信息的字符串
QString PeerInfo(const QHostAddress &address, quint16 port) {
  const static QString info = QStringLiteral("(%1:%2)");
  return info.arg(address.toString()).arg(port);
}

// 获取连接信息的字符串(加密套件和协议)
QString ConnectionInfo(QDtls *connection) {
  QString info(Core::UdpServer::tr("Session cipher: "));
  info += connection->sessionCipher().name();   // 添加加密套件名称
  info += Core::UdpServer::tr("; session protocol: ");
  switch (connection->sessionProtocol()) {
    case QSsl::DtlsV1_2:
      info += Core::UdpServer::tr("DTLS 1.2."); // 添加协议版本
      break;
    default:
      info += Core::UdpServer::tr("Unknown protocol."); // 未知协议
  }
  return info;
}

} // 匿名空间


namespace Core {

UdpServer::UdpServer(QObject* parent) : QObject(parent) {
  connect(&server_socket_, &QAbstractSocket::readyRead, this, &UdpServer::readyRead);

  // 初始化 DTLS 服务器配置
  server_configuration_ = QSslConfiguration::defaultDtlsConfiguration();
  // 设置预共享密钥身份提示
  server_configuration_.setPreSharedKeyIdentityHint("Qt DTLS example server");
  // 设置不验证对等方
  server_configuration_.setPeerVerifyMode(QSslSocket::VerifyNone);
}

UdpServer::~UdpServer()
{
  shutdown();
}

bool UdpServer::listen(const QHostAddress &address, quint16 port)
{
  if (address != server_socket_.localAddress() || port != server_socket_.localPort()) {
    // 本地地址和端口不匹配
    shutdown();
    // 绑定
    listening_ = server_socket_.bind(address, port);
    if (!listening_) {
      // 绑定失败, 发送错误消息
      emit errorMessage(server_socket_.errorString());
    }
  } else {
    // 处于正在监听的状态
    listening_ = true;
  }
  // 返回监听状态
  return listening_;
}

bool UdpServer::isListening() const
{
  return listening_;
}

void UdpServer::close()
{
  listening_ = false;
}

void UdpServer::readyRead()
{
  // 获取数据包大小
  const qint64 bytesToRead = server_socket_.pendingDatagramSize();
  if (bytesToRead <= 0) {
    emit warningMessage(tr("虚假的读取通知"));
    return;
  }

  // 构造的字节数据是未初始化
  // 初始化数据报文
  QByteArray dgram(bytesToRead, Qt::Uninitialized);
  QHostAddress peerAddress;
  quint16 peerPort = 0;

  // 读取数据报
  const quint64 bytesRead = server_socket_.readDatagram(dgram.data(), dgram.size(), &peerAddress, &peerPort);
  if (bytesRead <= 0) {
    emit warningMessage(tr("Failed to read a datagram: ") + server_socket_.errorString());
    return;
  }

  // 调整数据报文大小
  dgram.resize(bytesRead);

  if (peerAddress.isNull() || !peerPort) {
    // 提取失败
    emit warningMessage(tr("Failed to extract peer info (address, port)"));
  }

  // 查找已知的客户端连接
  const auto client = std::find_if(known_clients_.begin(), known_clients_.end(), [&](const std::unique_ptr<QDtls> &connection) {
    return connection->peerAddress() == peerAddress && connection->peerPort() == peerPort;
  });

  // 客户端不存在, 处理新的连接
  if (client == known_clients_.end()) {
    return handleNewConnection(peerAddress, peerPort, dgram);
  }

  // 如果连接已经加密, 解密数据包
  if ((*client)->isConnectionEncrypted()) {
    decryptDatagram(client->get(), dgram);
    if ((*client)->dtlsError() == QDtlsError::RemoteClosedConnectionError) {
      known_clients_.erase(client);
    }
    return;
  }

  // 执行握手
  doHandShake(client->get(), dgram);

}

void UdpServer::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
  Q_ASSERT(auth);

  // 发出信息信号
  emit infoMessage(tr("PSK callback, received a client's identity: '%1'").arg(QString::fromLatin1(auth->identity())));
  auth->setPreSharedKey(QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f"));
}

void UdpServer::handleNewConnection(const QHostAddress &peerAddress, quint16 peer_port, const QByteArray &client_hello)
{
  if (!listening_) {
    return;
  }
  // 获取对等方信息
  const QString peer_info = PeerInfo(peerAddress, peer_port);
  // 验证客户端
  if (cookie_sender_.verifyClient(&server_socket_, client_hello, peerAddress, peer_port)) {
    emit infoMessage(peer_info + tr(": verified, starting a handshake"));

    // 创建新的 DTLS 连接
    std::unique_ptr<QDtls> newConnection {new QDtls{QSslSocket::SslServerMode}};
    // 设置 DTLS 配置
    newConnection->setDtlsConfiguration(server_configuration_);
    // 设置对等方
    newConnection->setPeer(peerAddress, peer_port);
    // 连接 PKS 对等方
    newConnection->connect(newConnection.get(), &QDtls::pskRequired, this, &UdpServer::pskRequired);
    // 保存新的客户端连接
    known_clients_.push_back(std::move(newConnection));
    // 执行握手
    doHandShake(known_clients_.back().get(), client_hello);

  } else if (cookie_sender_.dtlsError() != QDtlsError::NoError) {
    // 报告 DTLS 错误
    emit errorMessage(tr("DTLS error: ") + cookie_sender_.dtlsErrorString());
  } else {
    // 未验证的信息
    emit infoMessage(peer_info + tr(": not verified yet"));
  }

}

void UdpServer::doHandShake(QDtls *new_connection, const QByteArray &client_hello)
{
  // 执行握手操作
  const bool result = new_connection->doHandshake(&server_socket_, client_hello);
  if (!result) {
    // 握手失败
    emit errorMessage(new_connection->dtlsErrorString());
    return;
  }
  // 获取对等方信息
  const QString peerInfo = PeerInfo(new_connection->peerAddress(), new_connection->peerPort());

  switch (new_connection->handshakeState()) {
    case QDtls::HandshakeInProgress:
      emit infoMessage(peerInfo + tr(": handshake is in progress..."));
      break;
    case QDtls::HandshakeComplete:
      emit infoMessage(tr("Connection with %1 encrypted. %2").arg(peerInfo, ConnectionInfo(new_connection)));
    default:
      Q_UNREACHABLE();
  }
}

void UdpServer::decryptDatagram(QDtls *connection, const QByteArray &client_message)
{
  // 确保连接是加密的
  Q_ASSERT(connection->isConnectionEncrypted());
  // 获取对等方消息
  const QString peerInfo = PeerInfo(connection->peerAddress(), connection->peerPort());
  // 解密数据报
  const QByteArray dgram = connection->decryptDatagram(&server_socket_, client_message);
  if (dgram.size()) {
    emit datagramReceived(peerInfo, client_message, dgram);
    // 发出确认帧
    connection->writeDatagramEncrypted(&server_socket_, tr("to %1: ACK").arg(peerInfo).toLatin1());
  } else if (connection->dtlsError() == QDtlsError::NoError) {
    // 处理空数据报
    emit warningMessage(peerInfo + ": " + tr("0 byte dgram, could be a re-connect attempt"));
  } else {
    // 发送错误消息
    emit errorMessage(peerInfo + ": " + connection->dtlsErrorString());
  }
}

void UdpServer::shutdown()
{
  // 关闭所有已知客户端连接
  for (const auto &connection : std::exchange(known_clients_, {})) {
    connection->shutdown(&server_socket_);
  }

  // 关闭服务器 socket
  server_socket_.close();
}


} // namespace Core
