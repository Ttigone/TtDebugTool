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
  quint16 port;

  TcpServerConfiguration(const QString& host, quint16 port)
      : host(host), port(port) {}
};

class SocketTask : public QRunnable {
 public:
  SocketTask(qintptr handle);
  void run() override;
  
 private:
  qintptr socket_descriptor_;
  QTcpSocket* socket_;
};

class TcpServer : public QTcpServer {
  Q_OBJECT
 public:
  explicit TcpServer(QObject* parent = nullptr);

  bool startServer(const Core::TcpServerConfiguration& config);
  void stopServer();
  bool isRunning() const;

 signals:
  void serverStarted();
  void serverStopped();
  void errorOccurred(const QString& error);

 protected:
  // 重写 incomingConnection 方法, 处理新的连接
  void incomingConnection(qintptr handle) override;

 private:
  QStringList fortunes_;
  QThreadPool thread_pool_;
};

}  // namespace Core

#endif  // TCP_SERVER_H
