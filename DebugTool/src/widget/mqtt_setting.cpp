#include "mqtt_setting.h"

#include <ui/control/TtCheckBox.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

#include "core/mqtt_client.h"

namespace Widget {

MqttClientSetting::MqttClientSetting(QWidget* parent) : QWidget(parent) {
  init();
}

MqttClientSetting::~MqttClientSetting() {}

Core::MqttClientConfiguration MqttClientSetting::getMqttClientConfiguration() {
  return Core::MqttClientConfiguration(
      link_->currentText(), port_->currentText(), clien_id_->currentText(),
      protocol_version_->body()->currentData().value<TtMqttProcotol::Version>(),
      user_->currentText(), password_->currentText(),
      link_timeout_->currentText(), hold_timeout_->currentText(),
      reconnection_period_->currentText(), clear_conversation_->isChecked());
}

const QJsonObject& MqttClientSetting::getMqttClientSetting() {
  auto config = getMqttClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("Link", QJsonValue(config.link));
  linkSetting.insert("Port", QJsonValue(config.port));
  linkSetting.insert("ClienId", QJsonValue(config.clien_id));
  linkSetting.insert("Version", QJsonValue(config.version));
  linkSetting.insert("User", QJsonValue(config.user));
  linkSetting.insert("PassWord", QJsonValue(config.password));
  linkSetting.insert("LinkTimeout", QJsonValue(config.link_timeout));
  linkSetting.insert("HoldTimeout", QJsonValue(config.hole_timeout));
  linkSetting.insert("ReconnectionPeriod",
                     QJsonValue(config.reconnection_period));
  linkSetting.insert("ClearConversation",
                     QJsonValue(config.clear_conversation));
  mqtt_client_save_config_.insert("LinkSetting", QJsonValue(linkSetting));
  QJsonObject testament;
  testament.insert("Topic", QJsonValue(topic_->currentText()));
  testament.insert("Load", QJsonValue(load_->currentText()));
  testament.insert("QoS", QJsonValue(qos_->currentText()));
  testament.insert("Retain", QJsonValue(retain_->isCheckable()));
  mqtt_client_save_config_.insert("Testment", QJsonValue(testament));
  return mqtt_client_save_config_;
}

void MqttClientSetting::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  QWidget* linkConfig = new QWidget(this);
  Ui::TtVerticalLayout* linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  link_ = new Ui::TtLabelLineEdit(tr("链接:"), linkConfig);
  port_ = new Ui::TtLabelLineEdit(tr("端口:"), linkConfig);
  clien_id_ = new Ui::TtLabelLineEdit(tr("客户端 ID:"), linkConfig);
  protocol_version_ = new Ui::TtLabelComboBox(tr("协议版本:"), linkConfig);
  user_ = new Ui::TtLabelLineEdit(tr("用户名:"), linkConfig);
  password_ = new Ui::TtLabelLineEdit(tr("密码:"), linkConfig);
  link_timeout_ = new Ui::TtLabelLineEdit(tr("链接超时:"), linkConfig);
  hold_timeout_ = new Ui::TtLabelLineEdit(tr("保持链接:"), linkConfig);
  reconnection_period_ = new Ui::TtLabelLineEdit(tr("重连周期:"), linkConfig);
  clear_conversation_ = new Ui::TtCheckBox(tr("清楚会话"), linkConfig);
  linkConfigLayout->addWidget(link_);
  linkConfigLayout->addWidget(port_);
  linkConfigLayout->addWidget(clien_id_);
  linkConfigLayout->addWidget(protocol_version_);
  linkConfigLayout->addWidget(user_);
  linkConfigLayout->addWidget(password_);
  linkConfigLayout->addWidget(link_timeout_);
  linkConfigLayout->addWidget(hold_timeout_);
  linkConfigLayout->addWidget(reconnection_period_);
  linkConfigLayout->addWidget(clear_conversation_);
  setProtocolVersion();
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), linkConfig);

  QWidget* testamentWidget = new QWidget;
  Ui::TtVerticalLayout* testamentWidgetLayout =
      new Ui::TtVerticalLayout(testamentWidget);
  topic_ = new Ui::TtLabelLineEdit(tr("主题:"), testamentWidget);
  load_ = new Ui::TtLabelLineEdit(tr("负载:"), testamentWidget);
  qos_ = new Ui::TtLabelComboBox(tr("QoS:"), testamentWidget);
  retain_ = new Ui::TtCheckBox(tr("Retain"), testamentWidget);
  testamentWidgetLayout->addWidget(topic_);
  testamentWidgetLayout->addWidget(load_);
  testamentWidgetLayout->addWidget(qos_);
  testamentWidgetLayout->addWidget(retain_);
  setQoS();
  Ui::Drawer* drawer2 = new Ui::Drawer(tr("遗嘱"), testamentWidget);

  // 滚动区域
  QScrollArea* scr = new QScrollArea(this);
  scr->setFrameStyle(QFrame::NoFrame);
  QWidget* scrollContent = new QWidget(scr);

  Ui::TtVerticalLayout* lascr = new Ui::TtVerticalLayout(scrollContent);

  lascr->addWidget(drawer1, 0, Qt::AlignTop);
  lascr->addWidget(drawer2);
  lascr->addStretch();
  scrollContent->setLayout(lascr);

  scr->setWidget(scrollContent);
  scr->setWidgetResizable(true);

  main_layout_->addWidget(scr);

  QObject::connect(protocol_version_, &Ui::TtLabelComboBox::currentIndexChanged,
                   this, [this](int index) {
                     QVariant data = protocol_version_->currentData();
                     TtMqttProcotol::Version version =
                         static_cast<TtMqttProcotol::Version>(data.toInt());
                     emit mqttVersionChanged(version);
                   });
  protocol_version_->setCurrentItem(2);
}

void MqttClientSetting::setProtocolVersion() {
  protocol_version_->addItem("3.1", TtMqttProcotol::Q3_1);
  protocol_version_->addItem("3.1.1", TtMqttProcotol::Q3_1_1);
  protocol_version_->addItem("5.0", TtMqttProcotol::Q5_0);
}

void MqttClientSetting::setQoS() {
  qos_->addItem("0");
  qos_->addItem("1");
  qos_->addItem("2");
}

}  // namespace Widget
