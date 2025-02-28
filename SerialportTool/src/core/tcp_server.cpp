#include "core/tcp_server.h"

#include <QRandomGenerator>

namespace Core {

TcpServer::TcpServer(QObject* parent) : QTcpServer(parent) {
  // 配置线程池
  thread_pool_.setMaxThreadCount(QThread::idealThreadCount() * 2);
}

bool TcpServer::startServer(const Core::TcpServerConfiguration& config) {
  if (isListening()) {
    return true;
  }

  // qDebug() << config.port;
  if (!listen(QHostAddress::Any, config.port)) {
    // if (!listen(QHostAddress::Any, 33333)) {
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

bool TcpServer::isRunning() const {
  return isListening();
}

void TcpServer::incomingConnection(qintptr handle) {
  qDebug() << "new";
  thread_pool_.start(new SocketTask(handle));
}

SocketTask::SocketTask(qintptr handle)
    : socket_descriptor_(handle), socket_(nullptr) {
  setAutoDelete(true);
}

void SocketTask::run() {
  // QTcpSocket socket;
  socket_ = new QTcpSocket();
  if (!socket_->setSocketDescriptor(socket_descriptor_)) {
    qWarning() << "Socket error:" << socket_->error();
    delete socket_;
    return;
  }
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

  // 监听客户端输入
  QObject::connect(socket_, &QTcpSocket::readyRead, [this]() {
    QByteArray data = socket_->readAll();
    qDebug() << "收到客户端消息:" << data;
    // 处理客户端请求，例如回复响应
  });

  // 监听断开信号
  QObject::connect(socket_, &QTcpSocket::disconnected,
                   [this]() { socket_->deleteLater(); });

  // 进入事件循环
  QEventLoop loop;
  QObject::connect(socket_, &QTcpSocket::disconnected, &loop,
                   &QEventLoop::quit);
  loop.exec();
}

}  // namespace Core
