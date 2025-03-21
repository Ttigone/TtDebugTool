#include "core/mqtt_client.h"

#include <QMessageBox>

namespace Core {

MqttClient::MqttClient(QObject* parent) : QObject{parent} {
  client_ = new QMqttClient(this);
  connect(client_, &QMqttClient::stateChanged, this, &MqttClient::stateChanged);
  connect(client_, &QMqttClient::connected, this, &MqttClient::connected);
  connect(client_, &QMqttClient::disconnected, this, &MqttClient::disconnected);
  connect(client_, &QMqttClient::messageReceived, this,
          [this](const QByteArray& message, const QMqttTopicName& topic) {
            const QString content = QDateTime::currentDateTime().toString() +
                                    " Received Topic: " + topic.name() +
                                    " Message: " + message + u'\n';

            emit dataReceived(content);
          });

  connect(client_, &QMqttClient::stateChanged,
          [this](QMqttClient::ClientState state) {
            if (state == QMqttClient::Disconnected) {
              emit disconnected();
            }
          });
  connect(
      client_, &QMqttClient::errorChanged,
      [this](QMqttClient::ClientError error) {
        qDebug() << "test Error";
        switch (error) {
          case QMqttClient::NoError:
            break;
          case QMqttClient::InvalidProtocolVersion:
            emit errorOccurred(tr("服务器不支持客户端请求的MQTT协议版本"));
            break;
          case QMqttClient::IdRejected:
            emit errorOccurred(tr("服务器拒绝了客户端ID (可能无效或已被使用)"));
            break;
          case QMqttClient::ServerUnavailable:
            emit errorOccurred(tr("服务器当前不可用 (可能超载或维护中)"));
            break;
          case QMqttClient::BadUsernameOrPassword:
            emit errorOccurred(tr("错误的用户名或者密码"));
            break;
          case QMqttClient::NotAuthorized:
            emit errorOccurred(tr("客户端未获授权连接到此服务器 (权限不足)"));
            break;
          case QMqttClient::TransportInvalid:
            emit errorOccurred(tr("底层传输层无效"));
            break;
          case QMqttClient::ProtocolViolation:
            emit errorOccurred(tr("检测到MQTT协议违规情况"));
            break;
          case QMqttClient::UnknownError:
            emit errorOccurred(tr("发生未知错误"));
            break;
          case QMqttClient::Mqtt5SpecificError:
            emit errorOccurred(tr("MQTT 5.0特有的错误"));
            break;
        }
      });
}

MqttClient::~MqttClient() {}

bool MqttClient::isConnected() {
  if (client_) {
    return client_->state() == QMqttClient::Connected;
  }
  return false;
}

void MqttClient::connectToHost(const MqttClientConfiguration& config) {
  bool ok;
  if (config.link.isEmpty()) {
    emit errorOccurred(tr("连接参数无效"));
    return;
  }
  int port = config.port.toInt(&ok);
  if (!ok || port <= 0 || port > 65535) {
    emit errorOccurred(tr("无效的端口号: ") + config.port);
    return;
  }
  client_->setHostname(config.link);  // broker 地址
  client_->setPort(port);             // 端口
  client_->setClientId(config.clien_id);
  client_->setProtocolVersion(QMqttClient::ProtocolVersion(config.version));
  client_->connectToHost();
}

void MqttClient::disconnectFromHost() {
  if (client_->state() == QMqttClient::ClientState::Connected) {
    client_->disconnectFromHost();
  }
}

void MqttClient::setClientPort(int port) {
  client_->setPort(port);
}

void MqttClient::subscribe(const QString& topic, int qos) {
  auto subscription = client_->subscribe(topic, qos);
  if (!subscription) {
    qDebug() << "Error", "Could not subscribe. Is there a valid connection?";
    return;
  }
}

void MqttClient::unsubscribe(const QString& topic) {
  client_->unsubscribe(topic);
}

qint32 MqttClient::publishMessage(const QString& topic, const QString& message,
                                  const quint8& qos, bool retain,
                                  const QByteArray& other) {
  if (other.isEmpty()) {
    return client_->publish(topic, message.toUtf8(), qos, retain);
  }
  /**
   * 解析
   * 
  */
  QDataStream stream(other);
  // 1. 恢复固定字段（6个组合框）
  QString contentType, msgExpiry, topicAlias, respTopic, corrData, subsId,
      payload;
  stream >> contentType >> msgExpiry >> topicAlias >> respTopic >> corrData >>
      subsId >> payload;

  QMqttPublishProperties property;
  property.setContentType(contentType);
  property.setMessageExpiryInterval(msgExpiry.toULong());
  // bug 5.0 指定时 会崩溃
  // qDebug() << "topic alias: " << topicAlias;
  // qDebug() << "topic alias to Ushort: " << topicAlias.toUShort();
  // property.setTopicAlias(2133);
  // property.setTopicAlias(topicAlias.toUShort());
  property.setResponseTopic(respTopic);
  property.setCorrelationData(corrData.toUtf8());
  if (subsId.isEmpty()) {
    property.setSubscriptionIdentifiers(QList<quint32>());
  } else {
    property.setSubscriptionIdentifiers(QList<quint32>(subsId.toULong()));
  }
  property.setPayloadFormatIndicator(
      (payload.compare("1")) ? QMqtt::PayloadFormatIndicator::UTF8Encoded
                             : QMqtt::PayloadFormatIndicator::Unspecified);

  QMqttUserProperties userProps;
  // 读取动态属性
  QString key, value;
  while (!stream.atEnd()) {
    stream >> key >> value;
    userProps.append({key, value});
  }
  property.setUserProperties(userProps);

  return client_->publish(topic, property, message.toUtf8(), qos, retain);
}

void MqttClient::brokerDisconnected() {}

}  // namespace Core
