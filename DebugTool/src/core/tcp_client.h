#ifndef CORE_TCP_CLIENT_H
#define CORE_TCP_CLIENT_H

#include <QObject>
#include <QTcpSocket>

namespace Core {

struct TcpClientConfiguration {
  QString target_host;
  QString target_port;
  QString self_host;
  QString self_port;

  TcpClientConfiguration(const QString& t_host, const QString& t_port,
                         const QString& m_host = "", const QString& m_port = 0)
      : target_host(t_host),
        target_port(t_port),
        self_host(m_host),
        self_port(m_port) {}
};

class TcpClient : public QThread {
  Q_OBJECT
 public:
  explicit TcpClient(QObject* parent = nullptr);
  ~TcpClient();

  void connectToServer(const QString& serverIp, const QString& serverPort,
                       const QString& localIp = "",
                       const QString& localPort = "");
  void connectToServer(const TcpClientConfiguration& config);
  void disconnectFromServer();
  void sendMessage(const QByteArray& message);

  bool isPortAvailable(const QString& ip, quint16 port) {
    // 如果端口为0，系统会自动分配端口
    if (port == 0) {
      return true;
    }
    QTcpSocket testSocket;
    return testSocket.bind(QHostAddress(ip), port);
  }

 signals:
  void connected();
  void disconnected();
  void dataReceived(const QByteArray& data);
  void errorOccurred(const QString& error);

 protected:
  void run() override;  // 重写线程入口

 public slots:

 private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketReadyRead();
  void onSocketError(QAbstractSocket::SocketError error);

 private:
  bool isValidPort(const QString& portStr);
  bool isValidHostAddress(const QString& hostAddress);

  QTcpSocket* socket_ = nullptr;
  QString server_ip_;
  quint16 server_port_;
  QString local_ip_;
  quint16 local_port_;
  QByteArray pending_message_;
};

}  // namespace Core

#endif  // TCP_CLIENT_H

/*
class TcpClient : public QObject {
  Q_OBJECT
 public:
  explicit TcpClient(QObject* parent = nullptr);
  ~TcpClient();

  bool connectToServer(const QString& serverIp, quint16 serverPort,
                       const QString& localIp, quint16 localPort);
  bool connectToServer(const TcpClientConfiguration& config);
  void disconnectFromServer();
  void sendMessage(const QByteArray& message);

 signals:
  void connected();
  void disconnected();
  void dataReceived(const QByteArray& data);
  void errorOccurred(const QString& error);

 private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketReadyRead();
  void onSocketError(QAbstractSocket::SocketError error);

  bool isSameSubnet(const QString& clientIp, const QString& serverIp);
  bool checkPortAvailability(const QString& ip, quint16 port);

 private:
  QTcpSocket* socket;
  QThread* client_thread_;
};
*/
