#include "core/tcp_client.h"

namespace Core {

/*
TcpClient::TcpClient(QObject* parent) : QObject(parent) {
  qDebug() << "TCP Thread: " << QThread::currentThread();
  // 主线程中创建 error
  // socket = new QTcpSocket();

  client_thread_ = new QThread(this);

  connect(
      client_thread_, &QThread::started, this,
      [this]() {
        qDebug() << "Socket Thread: " << QThread::currentThread();
        socket = new QTcpSocket();
        // 连接信号槽
        connect(socket, &QTcpSocket::connected, this,
                &TcpClient::onSocketConnected);
        connect(socket, &QTcpSocket::disconnected, this,
                &TcpClient::onSocketDisconnected);
        connect(socket, &QTcpSocket::readyRead, this,
                &TcpClient::onSocketReadyRead);

        // 错误上报
        connect(socket,
                QOverload<QAbstractSocket::SocketError>::of(
                    &QTcpSocket::errorOccurred),
                this, &TcpClient::onSocketError);
      },
      Qt::QueuedConnection);

  // 移动到工作线程
  // socket->moveToThread(client_thread_);
  // connect(client_thread_, &QThread::finished, &QThread::deleteLater);

  // 启动线程
  client_thread_->start();
}

TcpClient::~TcpClient() {
  // 无响应
  qDebug() << "tcp clien delete";
  client_thread_->quit();
  client_thread_->wait();
  // 主线程中删除
  // delete socket;

  // 在工作线程中删除socket
  QMetaObject::invokeMethod(
      client_thread_, [this]() { delete socket; },
      Qt::BlockingQueuedConnection);
}

bool TcpClient::connectToServer(const QString& serverIp, quint16 serverPort,
                                const QString& localIp, quint16 localPort) {
  // 检查本地地址合法性
  if (!localIp.isEmpty()) {
    QHostAddress localAddress;
    if (!localAddress.setAddress(localIp)) {
      qDebug() << "本地IP地址格式错误";
      // qDebug() << "本地IP地址格式错误";
      return false;
    }

    if (!checkPortAvailability(localIp, localPort)) {
      qDebug() << "端口被占用或不可用";
      return false;
    }

    // 绑定本地地址和端口
    QMetaObject::invokeMethod(
        socket,
        [this, localAddress, localPort]() {
          if (!socket->bind(localAddress, localPort)) {
            qDebug() << "绑定本地地址失败:" << socket->errorString();
          }
        },
        Qt::BlockingQueuedConnection);
  }

  // 连接服务端
  QMetaObject::invokeMethod(
      socket,
      [this, serverIp, serverPort]() {
        // 异步调用
        socket->connectToHost(QHostAddress(serverIp), serverPort);
      },
      Qt::BlockingQueuedConnection);
  // 连接超时
  return socket->waitForConnected(1500);
}

bool TcpClient::connectToServer(const TcpClientConfiguration& config) {
  // QMetaObject::invokeMethod(
  //     socket,
  //     [this, config]() {
  //       // 异步调用
  //       socket->connectToHost(QHostAddress(config.target_host),
  //                             config.target_port);
  //       //
  //       qDebug() << "connect Thread: " << QThread::currentThread();
  //     },
  //     // Qt::BlockingQueuedConnection);
  //     Qt::QueuedConnection);
  // // 连接超时
  // return socket->waitForConnected(1500);
  return connectToServer(config.target_host, config.target_port,
                         config.self_host, config.self_port);
}

void TcpClient::disconnectFromServer() {
  // 都是异步调用 socket 的函数
  qDebug() << "disconnect";
  QMetaObject::invokeMethod(socket, &QTcpSocket::disconnectFromHost);
}

void TcpClient::sendMessage(const QByteArray& message) {
  QMetaObject::invokeMethod(
      socket,
      [this, message]() {
        if (socket->state() == QTcpSocket::ConnectedState) {
          socket->write(message);
        }
      },
      Qt::QueuedConnection);
}

void TcpClient::onSocketConnected() {
  emit connected();
}

void TcpClient::onSocketDisconnected() {
  emit disconnected();
}

void TcpClient::onSocketReadyRead() {
  QByteArray data = socket->readAll();
  emit dataReceived(data);
}

void TcpClient::onSocketError(QAbstractSocket::SocketError error) {
  Q_UNUSED(error);
  emit errorOccurred(socket->errorString());
}

bool TcpClient::isSameSubnet(const QString& clientIp, const QString& serverIp) {
  QHostAddress clientAddr(clientIp);
  QHostAddress serverAddr(serverIp);
  QHostAddress mask("255.255.255.0");  // 假设子网掩码为24位

  quint32 clientSubnet = clientAddr.toIPv4Address() & mask.toIPv4Address();
  quint32 serverSubnet = serverAddr.toIPv4Address() & mask.toIPv4Address();

  return clientSubnet == serverSubnet;
}

bool TcpClient::checkPortAvailability(const QString& ip, quint16 port) {
  QTcpSocket testSocket;
  QHostAddress address(ip);
  // 直接测试绑定
  return testSocket.bind(address, port);
}
*/

TcpClient::TcpClient(QObject* parent) : QThread(parent) {}

TcpClient::~TcpClient() {
  if (isRunning()) {
    quit();
    wait(1000);
  }
  if (socket_) {
    delete socket_;
    socket_ = nullptr;
  }
}

void TcpClient::run() {
  // 创建事件循环
  QEventLoop loop;


  // 在子线程中创建 socket
  socket_ = new QTcpSocket();

  // 连接信号槽
  connect(socket_, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
  connect(socket_, &QTcpSocket::disconnected, this,
          &TcpClient::onSocketDisconnected);
  connect(socket_, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);
  connect(
      socket_,
      QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
      this, &TcpClient::onSocketError);

  // 端口复用
  /*
   * 操作系统会保留处于TIME_WAIT状态的套接字一段时间（通常为2分钟），
   * 以防止旧数据包干扰新连接。
   * 启用SO_REUSEADDR选项可以允许立即重用端口
   */
  // 有端口没 ip, 没端口有 ip, 没端口没 ip
  if (!local_ip_.isEmpty() && local_port_ != 0) {
    socket_->bind(QHostAddress(local_ip_), local_port_);
    // socket_->bind(QHostAddress(local_ip_), local_port_, QAbstractSocket::ReuseAddressHint); // 启用 SO_REUSEADDR
  }
  // 连接服务端
  socket_->connectToHost(server_ip_, server_port_);

  // 启动事件循环
  exec();

  qDebug() << "终止事件循环";

  delete socket_;
  socket_ = nullptr;
}

void TcpClient::connectToServer(const QString& serverIp,
                                const QString& serverPort,
                                const QString& localIp,
                                const QString& localPort) {
  // 验证端口号
  if (!serverPort.isEmpty() && !isValidPort(serverPort)) {
    emit errorOccurred(tr("服务器端口无效，端口范围必须为0-65535"));
    return;
  }

  if (!localPort.isEmpty() && !isValidPort(localPort)) {
    emit errorOccurred(tr("本地端口无效，端口范围必须为0-65535"));
    return;
  }
  // 验证本地 ip
  qDebug() << "local ip" << localIp;
  if (!localIp.isEmpty() && !isValidHostAddress(localIp)) {
    emit errorOccurred(tr("本地地址无效"));
    return;
  }

  server_ip_ = serverIp;
  server_port_ = serverPort.isEmpty() ? 0 : serverPort.toUShort();
  local_ip_ = localIp;
  local_port_ = localPort.isEmpty() ? 0 : localPort.toUShort();

  // qDebug() << "localport" << local_port_;

  // if (localPort)

  if (!local_ip_.isEmpty() && !isPortAvailable(local_ip_, local_port_)) {
    emit errorOccurred(tr("本地端口 %1 已被占用").arg(local_port_));
    return;
  }

  if (isRunning()) {
    emit errorOccurred(tr("客户端正在运行，请先断开"));
    return;
  }
  // 启动线程
  start();
}

void TcpClient::connectToServer(const TcpClientConfiguration& config) {
  connectToServer(config.target_host, config.target_port, config.self_host,
                  config.self_port);
}

void TcpClient::disconnectFromServer() {
  // // 直接调用是在主线程中运行, 有关 socket 的操作都需要在 run 方法中运行
  // qDebug() << currentThread();
  // if (socket_ && socket_->state() == QTcpSocket::ConnectedState) {
  //   socket_->disconnectFromHost();
  // }

  // 发送信号
  // emit disconnectRequested();

  QMetaObject::invokeMethod(
      socket_,
      [this]() {
        if (socket_) {
          qDebug() << "disconnect";
          // // 异步调用
          // socket_->disconnectFromHost();
          socket_->abort();  // 直接关闭, 丢弃缓冲区数据
          delete socket_;
          socket_ = nullptr;
        }
      },
      Qt::QueuedConnection);
}

void TcpClient::sendMessage(const QByteArray& message) {
  // if (socket_ && socket_->state() == QTcpSocket::ConnectedState) {
  //   socket_->write(message);
  // }
  // 不能直接写入
  // 通过信号触发发送操作，确保线程安全
  // emit sendMessageRequested(message);
  // 异步调用方式, 放入 socket_ 的事件队列
  // 对象一定是 socket_, 而不能是 this
  QMetaObject::invokeMethod(
      socket_,
      [this, message]() {
        if (socket_ && socket_->state() == QTcpSocket::ConnectedState) {
          socket_->write(message);
        } else {
          emit errorOccurred(tr("无法发送消息：连接未建立"));
        }
      },
      Qt::QueuedConnection);
}

// 槽函数实现
void TcpClient::onSocketConnected() {
  emit connected();
}

void TcpClient::onSocketDisconnected() {
  emit disconnected();
  quit();  // 退出事件循环
}

void TcpClient::onSocketReadyRead() {
  while (socket_->bytesAvailable() > 0) {
    QByteArray data = socket_->readAll();
    emit dataReceived(data);
  }
}

void TcpClient::onSocketError(QAbstractSocket::SocketError error) {
  Q_UNUSED(error);
  emit errorOccurred(socket_->errorString());
  quit();  // 发生错误时退出线程
}

bool TcpClient::isValidPort(const QString& portStr) {
  bool ok;
  quint32 port = portStr.toUInt(&ok);

  // 检查是否成功转换为数字且在有效范围内
  if (!ok || port > 65535) {
    return false;
  }

  return true;
}

bool TcpClient::isValidHostAddress(const QString& hostAddress) {
  QHostAddress testHost;
  auto test = testHost.setAddress(hostAddress);
  qDebug() << "test: " << test;
  return test;
}

}  // namespace Core
