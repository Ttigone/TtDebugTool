#include "core/tcp_server.h"

#include <QRandomGenerator>

namespace Core {

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent) {
  // 默认启动了多客户端模式
  // 关闭服务端监听套接字时, 与客户端的通讯的套接字不会关闭,
  // 只有服务端进程关闭时, 才会全部关闭 配置线程池
  thread_pool_.setMaxThreadCount(QThread::idealThreadCount() * 2);
}

bool TcpServer::startServer(const Core::TcpServerConfiguration &config) {
  if (isListening()) {
    return true;
  }

  // 特定地址与端口
  if (!listen(QHostAddress(config.host), config.port)) {
    emit errorOccurred(tr("无法启动服务: ") + errorString());
    return false;
  }
  emit serverStarted();
  return true;
}

void TcpServer::stopServer() {
  if (isListening()) {
    close();
    emit serverStopped();
  }
}

bool TcpServer::isRunning() const { return isListening(); }

void TcpServer::sendMessageToClients(const QByteArray &message) {
  QMutexLocker locker(&client_mutex_);
  for (QTcpSocket *client : client_sockets_) {
    if (client && client->state() == QAbstractSocket::ConnectedState) {
      // 是否跨线程调用
      // client->write(message);
      // 通过信号触发发送操作，确保线程安全
      QMetaObject::invokeMethod(
          client, [client, message]() { client->write(message); },
          Qt::QueuedConnection);
    }
  }
}

void TcpServer::incomingConnection(qintptr handle) {
  // 增加一个套接字用于处理与客户端的连接通讯
  auto task = new SocketTask(handle);
  connect(task, &SocketTask::socketCreate, this, [this](QTcpSocket *socket) {
    QMutexLocker locker(&client_mutex_);
    client_sockets_.append(socket);
  });
  connect(task, &SocketTask::dataReceived, this, &TcpServer::dataReceived);
  thread_pool_.start(task);
}

SocketTask::SocketTask(qintptr handle)
    : socket_descriptor_(handle), socket_(nullptr) {
  setAutoDelete(true);
}

void SocketTask::run() {
  qDebug() << "new socket thread: " << QThread::currentThread();
  // QTcpSocket socket;
  socket_ = new QTcpSocket();
  if (!socket_->setSocketDescriptor(socket_descriptor_)) {
    qWarning() << "Socket error:" << socket_->error();
    delete socket_;
    return;
  }

  emit socketCreate(socket_);

#if 0
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setByteOrder(QDataStream::LittleEndian);  // 设置与接收端一致的字节序
  // out.setVersion(QDataStream::Qt_6_4);
  // out << message_;
  // 前三个字节是 0
  // out << "TEST";
  // bug 头部有多余的字节
  QString m_message("TEST");
  out.writeBytes(m_message.toUtf8().constData(), m_message.size());

  // 异步写入
  socket_->write(block);
#else
  // 直接写入消息, 而非采用 QDataStream, 涉及内部自定义消息数据
  // socket_->write(QString("Test").toUtf8());
#endif

  // 监听客户端输入
  QObject::connect(socket_, &QTcpSocket::readyRead, this, [this]() {
    QByteArray data = socket_->readAll();
    // qDebug() << "收到客户端消息:" << data;
    // 处理客户端请求，例如回复响应
    emit dataReceived(data);
  });

  // 监听断开信号
  QObject::connect(socket_, &QTcpSocket::disconnected, this,
                   [this]() { socket_->deleteLater(); });

  // 进入事件循环
  QEventLoop loop;
  QObject::connect(socket_, &QTcpSocket::disconnected, &loop,
                   &QEventLoop::quit);
  loop.exec();
}

} // namespace Core
