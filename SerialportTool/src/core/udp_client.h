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

  UdpClientConfiguration(TtUdpMode::Mode mode, const QString& t_ip,
                         const QString& t_port, const QString& m_ip = "",
                         const QString& m_port = 0)
      : mode(mode),
        target_ip(t_ip),
        target_port(t_port),
        self_ip(m_ip),
        self_port(m_port) {}
};

class UdpClient : public QObject {
  Q_OBJECT
 public:
  explicit UdpClient(QObject* parent = nullptr);
  ~UdpClient();

  void close();
  void sendMessage(const QByteArray& message);

 signals:
  void disconnect();

 public slots:
  void connect(TtUdpMode::Mode mode, const QString& targetIp,
               const QString& targetPort, const QString& selfIp,
               const QString& selfPort);
  void connect(const UdpClientConfiguration& config);

  void readPendingDatagrams();
  void writePendingDatagrams(const QByteArray& message);

 private:
  TtUdpMode::Mode mode_;
  QString target_ip_;
  quint16 target_port_;
  QUdpSocket* socket_;
};

}  // namespace Core

#endif  // CORE_UDP_CLIENT_H
