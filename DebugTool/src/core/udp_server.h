#ifndef CORE_UDP_SERVER_H
#define CORE_UDP_SERVER_H

#include <QObject>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
class QHostAddress;
QT_END_NAMESPACE

namespace Core {

struct UdpServerConfiguration {
  QString self_host;
  QString self_port;

  UdpServerConfiguration(const QString &host = "", const QString &port = 0)
      : self_host(host), self_port(port) {}
};

class UdpServer : public QObject {
  Q_OBJECT
public:
  explicit UdpServer(QObject *parent = nullptr);
  ~UdpServer();

  bool listen(const QHostAddress &address, quint16 port);
  bool listen(const UdpServerConfiguration &config);
  bool isListening() const;
  void sendMessage(const QByteArray &message);
  void sendTo(const QByteArray &message, const QHostAddress &address,
              quint16 port);
  void close();

  // 广播
  void sendBroadcast(const QByteArray &message, quint16 targetPort);
  // 组播
  void sendMulticast(const QByteArray &message, const QString &multicastGroup,
                     quint16 targetPort);
  void sendToSubnet(const QByteArray &message, const QString &subnet,
                    quint16 targetPort);

  // 实现服务发现协议
  void startServiceDiscovery(quint16 discoveryPort);

signals:
  void datagramReceived(const QString &peerInfo, const quint16 &peerPort,
                        const QByteArray &message);
  void errorOccurred(const QString &error);

private slots:
  void readPendingDatagrams();
  void writePendingDatagrams(const QByteArray &message);

private:
  void shutdown();

private:
  bool listening_ = false;
  QUdpSocket *server_socket_;

  QSet<QPair<QHostAddress, quint16>> known_clients_;
};

// class UdpServer : public QObject {
//   Q_OBJECT
//  public:
//   explicit UdpServer(QObject* parent = nullptr);
//   ~UdpServer();

//   ///
//   /// @brief listen 监听指定地址和端口
//   /// @param address
//   /// @param port
//   /// @return
//   ///
//   bool listen(const QHostAddress &address, quint16 port);
//   ///
//   /// @brief isListening 检查服务器是否在监听状态
//   /// @return
//   ///
//   bool isListening() const;
//   ///
//   /// @brief close 关闭监听
//   ///
//   void close();

//  signals:
//   void errorMessage(const QString &message);
//   void warningMessage(const QString &message);
//   void infoMessage(const QString &message);

//   void datagramReceived(const QString &peer_info, const QByteArray
//   &peer_port, const QByteArray &client_hello);

//  private slots:
//   ///
//   /// @brief readyRead 读取数据
//   ///
//   void readyRead(); // 处理读取准备就绪的槽函数

//   ///
//   /// @brief pskRequired PSK(预共享密钥) 请求处理
//   /// @param auth
//   ///
//   void pskRequired(QSslPreSharedKeyAuthenticator *auth);

//  private:
//   ///
//   /// @brief handleNewConnection 处理新连接
//   /// @param peerAddress
//   /// @param peer_port
//   /// @param client_hello
//   ///
//   void handleNewConnection(const QHostAddress &peerAddress, quint16
//   peer_port, const QByteArray &client_hello);

//   ///
//   /// @brief doHandShake 握手操作
//   /// @param new_connection
//   /// @param client_hello
//   ///
//   void doHandShake(QDtls *new_connection, const QByteArray &client_hello);

//   ///
//   /// @brief decryptDatagram 解密数据报
//   /// @param connection
//   /// @param client_message
//   ///
//   void decryptDatagram(QDtls *connection, const QByteArray &client_message);

//   ///
//   /// @brief shutdown 关闭服务器
//   ///
//   void shutdown();

//   bool listening_ = false;
//   QUdpSocket server_socket_;

//   QSslConfiguration server_configuration_;
//   QDtlsClientVerifier cookie_sender_;
//   std::vector<std::unique_ptr<QDtls>> known_clients_;

//   Q_DISABLE_COPY(UdpServer)
// };

} // namespace Core

#endif // CORE_UDP_SERVER_H
