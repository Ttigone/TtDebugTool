#ifndef CORE_TCP_SERVER_H
#define CORE_TCP_SERVER_H

#include <QObject>
#include <QRunnable>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>

namespace Core {

struct TcpServerConfiguration {
  QString host;
  QString port;

  TcpServerConfiguration(const QString &host, const QString &port)
      : host(host), port(port) {}
};

class SocketTask : public QObject, public QRunnable {
  Q_OBJECT
public:
  SocketTask(qintptr handle);
  void run() override;

signals:
  void socketCreate(QTcpSocket *socket);
  void dataReceived(const QByteArray &data);

public slots:
  void sendMessage(const QByteArray &message) {
    if (socket_ && socket_->state() == QAbstractSocket::ConnectedState) {
      socket_->write(message);
    }
  }

private:
  qintptr socket_descriptor_;
  QTcpSocket *socket_;
};

class TcpServer : public QTcpServer {
  Q_OBJECT
public:
  explicit TcpServer(QObject *parent = nullptr);

  bool startServer(const Core::TcpServerConfiguration &config);
  void stopServer();
  bool isRunning() const;
  void sendMessageToClients(const QByteArray &message);

signals:
  void serverStarted();
  void serverStopped();
  void errorOccurred(const QString &error);
  void dataReceived(const QByteArray &data);

protected:
  // 重写 incomingConnection 方法, 处理新的连接
  void incomingConnection(qintptr handle) override;

private:
  Q_DISABLE_COPY(TcpServer);

  QThreadPool thread_pool_;
  QMutex client_mutex_;                // 互斥访问客户端列表
  QList<QTcpSocket *> client_sockets_; // 客户端列表
};

} // namespace Core

#endif // TCP_SERVER_H
