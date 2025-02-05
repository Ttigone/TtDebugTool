#ifndef CORE_TCP_SERVER_H
#define CORE_TCP_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

namespace Core {

class TcpSocketThread : public QThread {
  Q_OBJECT

 public:
  TcpSocketThread(qintptr socket_descriptor, const QString& fortune,
                  QObject* parent);

  // 重写 run() 方法, 线程执行入口
  void run() override;

 signals:
  void error(QTcpSocket::SocketError error);

 private:
  // socket 描述符
  qintptr socket_descriptor_;
  QString text_;
};

class TcpServer : public QTcpServer {
  Q_OBJECT
 public:
  explicit TcpServer(QObject* parent = nullptr);

 protected:
  // 重写 incomingConnection 方法, 处理新的连接
  void incomingConnection(qintptr socket_descriptor) override;

 private:
  QStringList fortunes_;
};

}  // namespace Core

#endif  // TCP_SERVER_H
