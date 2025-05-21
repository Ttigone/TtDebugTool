#include "mqtt_setting.h"

#include <ui/control/TtCheckBox.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtDrawer.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

#include "core/mqtt_client.h"

namespace Widget {

// MqttClientSetting::MqttClientSetting(QWidget *parent) : QWidget(parent) {
MqttClientSetting::MqttClientSetting(QWidget *parent) : FrameSetting(parent) {
  init();
}

MqttClientSetting::~MqttClientSetting() {}

Core::MqttClientConfiguration MqttClientSetting::getMqttClientConfiguration() {
  return Core::MqttClientConfiguration(
      link_->currentText(), port_->currentText(), client_id_->currentText(),
      protocol_version_->body()->currentData().value<TtMqttProcotol::Version>(),
      user_->currentText(), password_->currentText(),
      link_timeout_->currentText(), hold_timeout_->currentText(),
      reconnection_period_->currentText(), clear_conversation_->isChecked());
}

const QJsonObject &MqttClientSetting::getMqttClientSetting() {
  auto config = getMqttClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("Link", QJsonValue(config.link));
  linkSetting.insert("Port", QJsonValue(config.port));
  linkSetting.insert("ClientId", QJsonValue(config.clien_id));
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
  mqtt_client_save_config_.insert("Testament", QJsonValue(testament));
  return mqtt_client_save_config_;
}

void MqttClientSetting::setOldSettings(const QJsonObject &config) {
  if (config.isEmpty()) {
    return;
  }
  QJsonObject linkSetting = config.value("LinkSetting").toObject();
  QString link = linkSetting.value("Link").toString();
  QString port = linkSetting.value("Port").toString();
  QString clientId = linkSetting.value("ClientId").toString();
  int version = linkSetting.value("Version").toInt();
  QString user = linkSetting.value("User").toString();
  QString passWord = linkSetting.value("PassWord").toString();
  QString linkTimeout = linkSetting.value("LinkTimeout").toString();
  QString holdTimeout = linkSetting.value("HoldTimeout").toString();
  QString reconnectionPeriod =
      linkSetting.value("ReconnectionPeriod").toString();
  bool clearConversation = linkSetting.value("ClearConversation").toBool();

  QJsonObject testament = config.value("Testament").toObject();
  QString topic = testament.value("Topic").toString();
  QString load = testament.value("Load").toString();
  int qos = testament.value("QoS").toInt();
  bool retain = testament.value("Retain").toBool();

  link_->setText(link);
  port_->setText(port);
  client_id_->setText(clientId);
  for (int i = 0; i < protocol_version_->body()->count(); ++i) {
    if (protocol_version_->body()->itemData(i).toInt() == version) {
      protocol_version_->body()->setCurrentIndex(i);
    }
  }
  user_->setText(user);
  password_->setText(passWord);
  link_timeout_->setText(linkTimeout);
  hold_timeout_->setText(holdTimeout);
  reconnection_period_->setText(reconnectionPeriod);
  clear_conversation_->setChecked(clearConversation);
  topic_->setText(topic);
  load_->setText(load);
  for (int i = 0; i < qos_->body()->count(); ++i) {
    if (qos == qos_->body()->itemData(i).toInt()) {
      qos_->setCurrentItem(i);
    }
  }
  retain_->setChecked(retain);
}

void MqttClientSetting::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  QWidget *linkConfig = new QWidget(this);
  Ui::TtVerticalLayout *linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  link_ = new Ui::TtLabelLineEdit(tr("链接:"), linkConfig);
  port_ = new Ui::TtLabelLineEdit(tr("端口:"), linkConfig);
  client_id_ = new Ui::TtLabelLineEdit(tr("客户端 ID:"), linkConfig);
  protocol_version_ = new Ui::TtLabelComboBox(tr("协议版本:"), linkConfig);
  user_ = new Ui::TtLabelLineEdit(tr("用户名:"), linkConfig);
  password_ = new Ui::TtLabelLineEdit(tr("密码:"), linkConfig);
  link_timeout_ = new Ui::TtLabelLineEdit(tr("链接超时:"), linkConfig);
  hold_timeout_ = new Ui::TtLabelLineEdit(tr("保持链接:"), linkConfig);
  reconnection_period_ = new Ui::TtLabelLineEdit(tr("重连周期:"), linkConfig);
  clear_conversation_ = new Ui::TtCheckBox(tr("清楚会话"), linkConfig);

  // lineEdits_ << link_->body();
  // lineEdits_ << port_->body();
  // lineEdits_ << client_id_->body();
  // lineEdits_ << user_->body();
  // lineEdits_ << password_->body();
  // lineEdits_ << link_timeout_->body();
  // lineEdits_ << hold_timeout_->body();
  // lineEdits_ << reconnection_period_->body();
  // comboBoxes_ << protocol_version_->body();
  // checkBox_ << clear_conversation_;
  addLineEdit(link_->body());
  addLineEdit(port_->body());
  addLineEdit(client_id_->body());
  addLineEdit(user_->body());
  addLineEdit(password_->body());
  addLineEdit(link_timeout_->body());
  addLineEdit(hold_timeout_->body());
  addLineEdit(reconnection_period_->body());
  addComboBox(protocol_version_->body());
  addCheckBox(clear_conversation_);

  linkConfigLayout->addWidget(link_);
  linkConfigLayout->addWidget(port_);
  linkConfigLayout->addWidget(client_id_);
  linkConfigLayout->addWidget(protocol_version_);
  linkConfigLayout->addWidget(user_);
  linkConfigLayout->addWidget(password_);
  linkConfigLayout->addWidget(link_timeout_);
  linkConfigLayout->addWidget(hold_timeout_);
  linkConfigLayout->addWidget(reconnection_period_);
  linkConfigLayout->addWidget(clear_conversation_);
  setProtocolVersion();
  Ui::TtDrawer *drawerLinkSetting = new Ui::TtDrawer(
      tr("连接设置"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", linkConfig, false, this);

  QWidget *testamentWidget = new QWidget(this);
  Ui::TtVerticalLayout *testamentWidgetLayout =
      new Ui::TtVerticalLayout(testamentWidget);
  topic_ = new Ui::TtLabelLineEdit(tr("主题:"), testamentWidget);
  load_ = new Ui::TtLabelLineEdit(tr("负载:"), testamentWidget);
  qos_ = new Ui::TtLabelComboBox(tr("QoS:"), testamentWidget);
  retain_ = new Ui::TtCheckBox(tr("Retain"), testamentWidget);

  // lineEdits_ << topic_->body();
  // lineEdits_ << load_->body();
  // comboBoxes_ << qos_->body();
  // checkBox_ << retain_;
  addLineEdit(topic_->body());
  addLineEdit(load_->body());
  addComboBox(qos_->body());
  addCheckBox(retain_);

  testamentWidgetLayout->addWidget(topic_);
  testamentWidgetLayout->addWidget(load_);
  testamentWidgetLayout->addWidget(qos_);
  testamentWidgetLayout->addWidget(retain_);
  setQoS();
  Ui::TtDrawer *drawerTestament = new Ui::TtDrawer(
      tr("遗嘱"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", testamentWidget, false, this);

  // 滚动区域
  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);

  // lascr->addWidget(drawer1, 0, Qt::AlignTop);
  // lascr->addWidget(drawer2);
  // lascr->addStretch();
  // scrollContent->setLayout(lascr);
  scrollContentLayout->addWidget(drawerLinkSetting);
  scrollContentLayout->addWidget(drawerTestament);
  scrollContentLayout->addStretch();

  scrollContent->setLayout(scrollContentLayout);

  scroll->setWidget(scrollContent);
  scroll->setWidgetResizable(true);

  main_layout_->addWidget(scroll);

  connect(protocol_version_, &Ui::TtLabelComboBox::currentIndexChanged, this,
          [this](int index) {
            QVariant data = protocol_version_->currentData();
            TtMqttProcotol::Version version =
                static_cast<TtMqttProcotol::Version>(data.toInt());
            emit mqttVersionChanged(version);
          });
  protocol_version_->setCurrentItem(2);

  // for (const auto *comboBox : comboBoxes_) {
  //   if (comboBox) {
  //     connect(comboBox,
  //             QOverload<int>::of(&Ui::TtComboBox::currentIndexChanged), this,
  //             &ModbusClientSetting::settingChanged);
  //   }
  // }

  // for (const auto *lineEdit : lineEdits_) {
  //   if (lineEdit) { // 确保指针有效
  //     connect(lineEdit, &QLineEdit::textChanged, this,
  //             &FrameSetting::settingChanged);
  //   }
  // }

  // for (const auto *checkBox : checkBox_) {
  //   if (checkBox) { // 确保指针有效
  //     connect(checkBox, &QCheckBox::toggled, this,
  //             &FrameSetting::settingChanged);
  //   }
  // }
  link();
}

void MqttClientSetting::setProtocolVersion() {
  protocol_version_->addItem("3.1", TtMqttProcotol::Q3_1);
  protocol_version_->addItem("3.1.1", TtMqttProcotol::Q3_1_1);
  protocol_version_->addItem("5.0", TtMqttProcotol::Q5_0);
}

void MqttClientSetting::setQoS() {
  qos_->addItem("0", 0);
  qos_->addItem("1", 1);
  qos_->addItem("2", 2);
}

} // namespace Widget
