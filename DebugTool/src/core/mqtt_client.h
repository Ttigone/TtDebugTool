#ifndef CORE_MQTT_CLIENT_H
#define CORE_MQTT_CLIENT_H

#include <QObject>

#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttPublishProperties>

#include "Def.h"

namespace Core {

struct MqttClientConfiguration {
  QString link = "127.0.0.1";
  QString port = "1883";
  QString clien_id;  // 客户端 id
  TtMqttProcotol::Version version;
  QString user;          // 用户
  QString password;      // 密码
  QString link_timeout;  //
  QString hole_timeout;
  QString reconnection_period;
  bool clear_conversation;  // 开始时清除历史消息

  MqttClientConfiguration(const QString& link, const QString& port,
                          const QString& clien_id,
                          TtMqttProcotol::Version version, const QString& user,
                          const QString& password, const QString link_timeout,
                          const QString& hole_timeout,
                          const QString& reconnection_period, bool isclear)
      : link(link),
        port(port),
        clien_id(clien_id),
        version(version),
        user(user),
        password(password),
        link_timeout(link_timeout),
        hole_timeout(hole_timeout),
        reconnection_period(reconnection_period),
        clear_conversation(isclear) {}
};

class MqttClient : public QObject {
  Q_OBJECT
 public:
  explicit MqttClient(QObject* parent = nullptr);
  ~MqttClient();

  bool isConnected();
  void connectToHost(const MqttClientConfiguration& config);
  void disconnectFromHost();
  void setClientPort(int port);
  void subscribe(const QString& topic, int qos = 0);
  void unsubscribe(const QString& topic);

  qint32 publishMessage(const QString& topic, const QString& message,
                        const quint8& qos = 0, bool retain = false,
                        const QByteArray& other = QByteArray());

 signals:
  void connected();
  void disconnected();
  void stateChanged();
  void dataReceived(const QString message);
  void errorOccurred(const QString& error);


 private slots:
  void brokerDisconnected();

 private:
  QMqttPublishProperties* properties_;
  QMqttClient* client_;
};

}  // namespace Core

#endif  // CORE_MQTT_CLIENT_H
