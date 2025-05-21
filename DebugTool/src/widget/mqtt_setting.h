#ifndef WIDGET_MQTT_SETTING_H
#define WIDGET_MQTT_SETTING_H

#include "Def.h"
#include <QWidget>

#include "widget/frame_setting.h"

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
class TtCheckBox;
class TtVerticalLayout;
} // namespace Ui

namespace Core {
class MqttClientConfiguration;
} // namespace Core

namespace Widget {

// class MqttClientSetting : public QWidget {
class MqttClientSetting : public FrameSetting {
  Q_OBJECT
public:
  explicit MqttClientSetting(QWidget *parent = nullptr);
  ~MqttClientSetting();

  Core::MqttClientConfiguration getMqttClientConfiguration();
  const QJsonObject &getMqttClientSetting();

  void setOldSettings(const QJsonObject &config);

signals:
  void mqttVersionChanged(TtMqttProcotol::Version version);

private:
  void init();
  void setProtocolVersion();
  void setQoS();
  Ui::TtVerticalLayout *main_layout_;

  Ui::TtLabelLineEdit *link_;
  Ui::TtLabelLineEdit *port_;
  Ui::TtLabelLineEdit *client_id_;
  Ui::TtLabelComboBox *protocol_version_;
  Ui::TtLabelLineEdit *user_;
  Ui::TtLabelLineEdit *password_;
  Ui::TtLabelLineEdit *link_timeout_;
  Ui::TtLabelLineEdit *hold_timeout_;
  Ui::TtLabelLineEdit *reconnection_period_;
  Ui::TtCheckBox *clear_conversation_;

  Ui::TtLabelLineEdit *topic_;
  Ui::TtLabelLineEdit *load_;
  Ui::TtLabelComboBox *qos_;
  Ui::TtCheckBox *retain_;

  QJsonObject mqtt_client_save_config_;
};

} // namespace Widget

#endif // WIDGET_MQTT_SETTING_H
