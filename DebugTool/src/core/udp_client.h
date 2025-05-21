#ifndef CORE_UDP_CLIENT_H
#define CORE_UDP_CLIENT_H

#include <QObject>
#include <QUdpSocket>

#include "Def.h"

namespace Core {

struct UdpClientConfiguration {
  TtUdpMode::Mode mode;
  QString target_ip;
  QString target_port;
  QString self_ip;
  QString self_port;

  UdpClientConfiguration(TtUdpMode::Mode mode, const QString &t_ip,
                         const QString &t_port, const QString &m_ip = "",
                         const QString &m_port = 0)
      : mode(mode), target_ip(t_ip), target_port(t_port), self_ip(m_ip),
        self_port(m_port) {}
};

class UdpClient : public QObject {
  Q_OBJECT
public:
  explicit UdpClient(QObject *parent = nullptr);
  ~UdpClient();

  ///
  /// @brief close
  /// 关闭链接
  void close();

  ///
  /// @brief sendMessage
  /// @param message
  /// 发送消息
  void sendMessage(const QByteArray &message);

  ///
  /// @brief isConnected
  /// @return
  /// 判断是否处于链接状态
  bool isConnected() const;

signals:
  void connected();
  void disconnected();
  void dataReceived(const QByteArray &data, const QHostAddress &sender,
                    quint16 senderPort);
  void errorOccurred(const QString &error);

public slots:
  ///
  /// @brief connectToOther
  /// @param mode
  /// @param targetIp
  /// @param targetPort
  /// @param selfIp
  /// @param selfPort
  /// 链接其他 udp 客户端
  void connectToOther(TtUdpMode::Mode mode, const QString &targetIp,
                      const QString &targetPort, const QString &selfIp,
                      const QString &selfPort);
  void connectToOther(const UdpClientConfiguration &config);

private:
  ///
  /// @brief readPendingDatagrams
  /// 读取数据报
  void readPendingDatagrams();

  ///
  /// @brief writePendingDatagrams
  /// @param message
  /// @return
  /// 写数据报
  bool writePendingDatagrams(const QByteArray &message);

  TtUdpMode::Mode mode_;
  QString target_ip_;
  quint16 target_port_;
  QUdpSocket *socket_;
};

} // namespace Core

#endif // CORE_UDP_CLIENT_H
