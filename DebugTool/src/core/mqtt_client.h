#ifndef CORE_MQTT_CLIENT_H
#define CORE_MQTT_CLIENT_H

#include <QObject>

// #include <qtmqtt/include/qmqttclient.h>
// #include <QtMqtt/QMqttClient>
// #include <QtMqtt/QMqttPublishProperties>
// #include "qtmqtt_mingw64/include/qmqttclient.h"
// #include "qmqttclient.h"

#if defined(__MINGW64__)
// MinGW编译器
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtMqtt/6_6_3_mingw_64/include/qmqttclient.h>
#else
// Qt 5.x with MinGW
#include <QtMqtt/5_15_mingw_64/include/qmqttclient.h>
#define MQTT_COMPILER_TYPE "Qt5 MinGW"
#endif
#elif defined(_MSC_VER)
// MSVC编译器
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Qt 6.x with MSVC
#include <QtMqtt/6_6_3_msvc2019_64/include/qmqttclient.h>
#define MQTT_COMPILER_TYPE "Qt6 MSVC"
#else
// Qt 5.x with MSVC
#include <QtMqtt/QMqttClient>
#define MQTT_COMPILER_TYPE "Qt5 MSVC"
#endif
#else
// 其他编译器，尝试标准路径
#include <QtMqtt/QMqttClient>
#define MQTT_COMPILER_TYPE "Standard"
#endif

#include "Def.h"

namespace Core {

struct MqttClientConfiguration {
  QString link = "127.0.0.1";
  QString port = "1883";
  QString clien_id; // 客户端 id
  TtMqttProcotol::Version version;
  QString user;         // 用户
  QString password;     // 密码
  QString link_timeout; //
  QString hole_timeout;
  QString reconnection_period;
  bool clear_conversation; // 开始时清除历史消息

  MqttClientConfiguration(const QString &link, const QString &port,
                          const QString &clien_id,
                          TtMqttProcotol::Version version, const QString &user,
                          const QString &password, const QString link_timeout,
                          const QString &hole_timeout,
                          const QString &reconnection_period, bool isclear)
      : link(link), port(port), clien_id(clien_id), version(version),
        user(user), password(password), link_timeout(link_timeout),
        hole_timeout(hole_timeout), reconnection_period(reconnection_period),
        clear_conversation(isclear) {}
};

class MqttClient : public QObject {
  Q_OBJECT
public:
  explicit MqttClient(QObject *parent = nullptr);
  ~MqttClient();

  bool isConnected();
  void connectToHost(const MqttClientConfiguration &config);
  void disconnectFromHost();
  void setClientPort(int port);
  void subscribe(const QString &topic, int qos = 0);
  void unsubscribe(const QString &topic);

  qint32 publishMessage(const QString &topic, const QString &message,
                        const quint8 &qos = 0, bool retain = false,
                        const QByteArray &other = QByteArray());

signals:
  void connected();
  void disconnected();
  void stateChanged();
  void dataReceived(const QString message);
  void errorOccurred(const QString &error);

private slots:
  void brokerDisconnected();

private:
  QMqttPublishProperties *properties_;
  QMqttClient *client_;
};

} // namespace Core

#endif // CORE_MQTT_CLIENT_H
