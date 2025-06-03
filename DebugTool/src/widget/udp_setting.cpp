#include "widget/udp_setting.h"
#include "core/udp_client.h"
#include "core/udp_server.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtDrawer.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

namespace Widget {

UdpServerSetting::UdpServerSetting(QWidget *parent) : FrameSetting(parent) {
  main_layout_ = new QVBoxLayout(this);

  QWidget *linkConfig = new QWidget;
  Ui::TtVerticalLayout *linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  self_ip_ = new Ui::TtLabelLineEdit(tr("本地地址: "), linkConfig);
  // IPv4地址正则表达式验证
  QString ipPattern = "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
                      "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b";
  // IPv6地址正则表达式验证(简化版)
  QString ipv6Pattern = "([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}";
  self_ip_->body()->setValidator(
      new QRegularExpressionValidator(QRegularExpression(ipPattern), this));
  self_port_ = new Ui::TtLabelLineEdit(tr("本地端口: "), linkConfig);

  linkConfigLayout->addWidget(self_ip_);
  linkConfigLayout->addWidget(self_port_);
  Ui::TtDrawer *drawerLinkSetting = new Ui::TtDrawer(
      tr("连接设置"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", linkConfig, false, this);

  QWidget *framingWidget = new QWidget;
  Ui::TtVerticalLayout *framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelLineEdit(tr("时间(ms):"));
  framing_fixed_length_ = new Ui::TtLabelLineEdit(tr("长度:"));

  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);

  Ui::TtDrawer *drawerFraming = new Ui::TtDrawer(
      tr("分帧[收数据包](暂未提供使用)"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", framingWidget, false, this);
  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawerFraming](int index) {
            switch (index) {
            case 0: {
              framing_timeout_->setVisible(false);
              framing_fixed_length_->setVisible(false);
              break;
            }
            case 1: {
              framing_timeout_->setVisible(true);
              framing_fixed_length_->setVisible(false);
              break;
            }
            case 2: {
              framing_timeout_->setVisible(false);
              framing_fixed_length_->setVisible(true);
              break;
            }
            }
            const auto event =
                new QResizeEvent(drawerFraming->size(), drawerFraming->size());
            QCoreApplication::postEvent(drawerFraming, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  retransmission_ = new Ui::TtLabelBtnComboBox(tr("目标: "));
  retransmission_->addItem(tr("无"));
  Ui::TtDrawer *drawerRetransmission = new Ui::TtDrawer(
      tr("转发"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", retransmission_, false, this);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawerLinkSetting, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawerFraming);
  scrollContentLayout->addWidget(drawerRetransmission);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);

  addLineEdit(self_ip_->body());
  addLineEdit(self_port_->body());
  addLineEdit(framing_timeout_->body());
  addLineEdit(framing_fixed_length_->body());

  addComboBox(framing_model_->body());
  addComboBox(retransmission_->body());

  link();
}

UdpServerSetting::~UdpServerSetting() { qDebug() << "delete" << __FUNCTION__; }

Core::UdpServerConfiguration UdpServerSetting::getUdpServerConfiguration() {
  Core::UdpServerConfiguration config(self_ip_->body()->text(),
                                      self_port_->body()->text());
  return config;
}

const QJsonObject &UdpServerSetting::getUdpServerSetting() {
  auto config = getUdpServerConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("SelfHost", QJsonValue(config.self_host));
  linkSetting.insert("SelfPort", QJsonValue(config.self_port));
  udp_server_save_config_.insert("LinkSetting", QJsonValue(linkSetting));
  return udp_server_save_config_;
  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  // framing.insert("Timeout",
  //                QJsonValue(framing_timeout_->body()->currentText()));
  // framing.insert("FixedLength",
  //                QJsonValue(framing_fixed_length_->body()->currentText()));
  framing.insert("Timeout", QJsonValue(framing_timeout_->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->currentText()));
  udp_server_save_config_.insert("Framing", QJsonValue(framing));
  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  udp_server_save_config_.insert("Retransmission", QJsonValue(retransmission));
  return udp_server_save_config_;
}

void UdpServerSetting::setOldSettings(const QJsonObject &config) {
  if (config.isEmpty()) {
    return;
  }
  QJsonObject linkSetting = config.value("LinkSetting").toObject();
  QString selfHost = linkSetting.value("SelfHost").toString();
  QString selfPort = linkSetting.value("SelfPort").toString();

  QJsonObject framing = config.value("Framing").toObject();
  QString model = framing.value("Model").toString();
  QString timeout = framing.value("Timeout").toString();
  QString fixedLength = framing.value("FixedLength").toString();

  QJsonObject retransmission = config.value("Retransmission").toObject();
  QString target = retransmission.value("Target").toString();

  self_ip_->setText(selfHost);
  self_port_->setText(selfPort);

  for (int i = 0; i < framing_model_->body()->count(); ++i) {
    if (framing_model_->body()->itemData(i).toString() == model) {
      framing_model_->body()->setCurrentIndex(i);
      break;
    }
  }

  framing_timeout_->setText(timeout);
  framing_fixed_length_->setText(fixedLength);

  for (int i = 0; i < retransmission_->body()->count(); ++i) {
    if (retransmission_->body()->itemData(i).toString() == target) {
      retransmission_->body()->setCurrentIndex(i);
      break;
    }
  }
}

void UdpServerSetting::setControlState(bool state) {
  self_ip_->setEnabled(state);
  self_port_->setEnabled(state);
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  retransmission_->setEnabled(state);
}

UdpClientSetting::UdpClientSetting(QWidget *parent) : FrameSetting(parent) {

  main_layout_ = new QVBoxLayout(this);

  QWidget *linkConfig = new QWidget;
  Ui::TtVerticalLayout *linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  mode_ = new Ui::TtLabelComboBox(tr("模式: "), linkConfig);
  target_ip_ = new Ui::TtLabelLineEdit(tr("目标地址: "), linkConfig);
  target_port_ = new Ui::TtLabelLineEdit(tr("目标端口: "), linkConfig);
  self_ip_ = new Ui::TtLabelLineEdit(tr("本地地址: "), linkConfig);
  self_port_ = new Ui::TtLabelLineEdit(tr("本地端口: "), linkConfig);
  send_package_max_size_ =
      new Ui::TtLabelLineEdit(tr("发送包最大尺寸:"), linkConfig);
  send_package_interval_ =
      new Ui::TtLabelLineEdit(tr("发送端间隔: "), linkConfig);

  setLinkMode();
  linkConfigLayout->addWidget(mode_);
  linkConfigLayout->addWidget(target_ip_);
  linkConfigLayout->addWidget(target_port_);
  linkConfigLayout->addWidget(self_ip_);
  linkConfigLayout->addWidget(self_port_);
  linkConfigLayout->addWidget(send_package_interval_);
  linkConfigLayout->addWidget(send_package_max_size_);
  Ui::TtDrawer *drawerLinkSetting = new Ui::TtDrawer(
      tr("连接设置"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", linkConfig, false, this);

  connect(mode_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawerLinkSetting](int index) {
            switch (index) {
            case 0:
              target_ip_->setVisible(true);
              target_port_->setVisible(true);
              self_ip_->setVisible(true);
              self_port_->setVisible(true);
              break;
            case 1:
              target_ip_->setVisible(true);
              target_port_->setVisible(true);
              self_ip_->setVisible(false);
              self_port_->setVisible(false);
              break;
            case 2:
              target_ip_->setVisible(true);
              target_port_->setVisible(true);
              self_ip_->setVisible(false);
              self_port_->setVisible(false);
              break;
            }

            const auto event = new QResizeEvent(drawerLinkSetting->size(),
                                                drawerLinkSetting->size());
            QCoreApplication::postEvent(drawerLinkSetting, event);
          });
  mode_->setCurrentItem(0);

  QWidget *framingWidget = new QWidget;
  Ui::TtVerticalLayout *framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelLineEdit(tr("时间(ms):"));
  framing_fixed_length_ = new Ui::TtLabelLineEdit(tr("长度:"));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);
  Ui::TtDrawer *drawerFraming = new Ui::TtDrawer(
      tr("分帧[收数据包](暂未提供使用)"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", framingWidget, false, this);
  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawerFraming](int index) {
            switch (index) {
            case 0: {
              framing_timeout_->setVisible(false);
              framing_fixed_length_->setVisible(false);
              break;
            }
            case 1: {
              framing_timeout_->setVisible(true);
              framing_fixed_length_->setVisible(false);
              break;
            }
            case 2: {
              framing_timeout_->setVisible(false);
              framing_fixed_length_->setVisible(true);
              break;
            }
            }
            const auto event =
                new QResizeEvent(drawerFraming->size(), drawerFraming->size());
            QCoreApplication::postEvent(drawerFraming, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  retransmission_ = new Ui::TtLabelBtnComboBox(tr("目标: "));
  retransmission_->addItem(tr("无"));
  Ui::TtDrawer *drawerRetransmission = new Ui::TtDrawer(
      tr("转发"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", retransmission_, false, this);

  // 心跳界面
  QWidget *heartbeatWidget = new QWidget;
  Ui::TtVerticalLayout *heartbeatWidgetLayout =
      new Ui::TtVerticalLayout(heartbeatWidget);
  heartbeatWidget->adjustSize(); // 确保大小正确
  heartbeat_send_type_ = new Ui::TtLabelComboBox(tr("类型: "));
  heartbeat_send_type_->addItem(tr("无"), TtTextFormat::None);
  heartbeat_send_type_->addItem(tr("文本"), TtTextFormat::TEXT);
  heartbeat_send_type_->addItem(tr("HEX"), TtTextFormat::HEX);
  heartbeat_interval_ = new Ui::TtLabelLineEdit(tr("间隔: "));
  heartbeat_content_ = new Ui::TtLabelLineEdit(tr("内容: "));
  heartbeatWidgetLayout->addWidget(heartbeat_send_type_);
  heartbeatWidgetLayout->addWidget(heartbeat_interval_);
  heartbeatWidgetLayout->addWidget(heartbeat_content_);

  Ui::TtDrawer *drawerHeartBeat = new Ui::TtDrawer(
      tr("心跳"), ":/sys/chevron-double-up.svg",
      ":/sys/chevron-double-down.svg", heartbeatWidget, false, this);

  connect(heartbeat_send_type_, &Ui::TtLabelComboBox::currentIndexChanged, this,
          [this, heartbeatWidget, drawerHeartBeat](int index) {
            switch (index) {
            case 0: {
              heartbeat_interval_->setVisible(false);
              heartbeat_content_->setVisible(false);
              break;
            }
            case 1: {
              heartbeat_interval_->setVisible(true);
              heartbeat_content_->setVisible(true);
              break;
            }
            case 2: {
              heartbeat_interval_->setVisible(true);
              heartbeat_content_->setVisible(true);
              break;
            }
            }
            qDebug() << "switch index" << index;
            emit heartbeatType(static_cast<TtTextFormat::Type>(index));
            // emit heartbeatType(index);
            const auto event = new QResizeEvent(drawerHeartBeat->size(),
                                                drawerHeartBeat->size());
            QCoreApplication::postEvent(drawerHeartBeat, event);
          });
  heartbeat_send_type_->setCurrentItem(0);
  heartbeat_interval_->setVisible(false);
  heartbeat_content_->setVisible(false);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawerLinkSetting, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawerFraming);
  scrollContentLayout->addWidget(drawerHeartBeat);
  scrollContentLayout->addWidget(drawerRetransmission);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);
  main_layout_->addWidget(scroll);

  addLineEdit(target_port_->body());
  addLineEdit(self_port_->body());
  addLineEdit(send_package_max_size_->body());
  addLineEdit(send_package_interval_->body());
  addLineEdit(framing_timeout_->body());
  addLineEdit(framing_fixed_length_->body());
  addLineEdit(heartbeat_interval_->body());
  addLineEdit(heartbeat_content_->body());

  addComboBox(framing_model_->body());
  addComboBox(retransmission_->body());
  addComboBox(heartbeat_send_type_->body());

  link();

  connect(heartbeat_content_, &Ui::TtLabelLineEdit::currentTextChanged, this,
          [this](const QString &text) { emit heartbeatContentChanged(text); });
  connect(heartbeat_interval_, &Ui::TtLabelLineEdit::currentTextToUInt32, this,
          &FrameSetting::heartbeatInterval);

  connect(send_package_interval_, &Ui::TtLabelLineEdit::currentTextToUInt32,
          this, &FrameSetting::sendPackageIntervalChanged);
  connect(send_package_max_size_, &Ui::TtLabelLineEdit::currentTextToUInt32,
          this, &FrameSetting::sendPackageMaxSizeChanged);
}

UdpClientSetting::~UdpClientSetting() { qDebug() << "delete" << __FUNCTION__; }

Core::UdpClientConfiguration UdpClientSetting::getUdpClientConfiguration() {
  Core::UdpClientConfiguration config(
      static_cast<TtUdpMode::Mode>(mode_->body()->currentData().toInt()),
      target_ip_->currentText(), target_port_->currentText(),
      self_ip_->currentText(), self_port_->currentText());
  return config;
}

const QJsonObject &UdpClientSetting::getUdpClientSetting() {
  auto config = getUdpClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("Mode", QJsonValue(config.mode));
  linkSetting.insert("TargetHost", QJsonValue(config.target_ip));
  linkSetting.insert("TargetPort", QJsonValue(config.target_port));
  linkSetting.insert("SelfHost", QJsonValue(config.self_ip));
  linkSetting.insert("SelfPort", QJsonValue(config.self_port));
  linkSetting.insert("SendPacketInterval",
                     QJsonValue(send_package_interval_->currentText()));
  linkSetting.insert("SendPackageMaxSize",
                     QJsonValue(send_package_max_size_->currentText()));
  udp_client_save_config_.insert("LinkSetting", QJsonValue(linkSetting));
  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  // framing.insert("Timeout",
  //                QJsonValue(framing_timeout_->body()->currentText()));
  // framing.insert("FixedLength",
  //                QJsonValue(framing_fixed_length_->body()->currentText()));
  framing.insert("Timeout", QJsonValue(framing_timeout_->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->currentText()));
  udp_client_save_config_.insert("Framing", QJsonValue(framing));
  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  udp_client_save_config_.insert("Retransmission", QJsonValue(retransmission));

  QJsonObject heartbeat;
  heartbeat.insert(
      "Type", QJsonValue(heartbeat_send_type_->body()->currentData().toInt()));
  heartbeat.insert("Interval", QJsonValue(heartbeat_interval_->body()->text()));
  heartbeat.insert("Content", QJsonValue(heartbeat_content_->body()->text()));
  udp_client_save_config_.insert("Heartbeat", QJsonValue(heartbeat));

  return udp_client_save_config_;
}

void UdpClientSetting::setOldSettings(const QJsonObject &config) {
  if (config.isEmpty()) {
    return;
  }
  QJsonObject linkSetting = config.value("LinkSetting").toObject();
  int mode = linkSetting.value("Mode").toInt(-1);
  QString targetHost = linkSetting.value("TargetHost").toString();
  QString targetPort = linkSetting.value("TargetPort").toString();
  QString selfHost = linkSetting.value("SelfHost").toString();
  QString selfPort = linkSetting.value("SelfPort").toString();
  QString packetInterval = linkSetting.value("SendPacketInterval").toString();
  QString packageMaxSize = linkSetting.value("SendPackageMaxSize").toString();

  QJsonObject framing = config.value("Framing").toObject();
  QString model = framing.value("Model").toString();
  QString timeout = framing.value("Timeout").toString();
  QString fixedLength = framing.value("FixedLength").toString();

  QJsonObject retransmission = config.value("Retransmission").toObject();
  QString target = retransmission.value("Target").toString();

  QJsonObject heartBeat = config.value("Heartbeat").toObject();
  int type = heartBeat.value("Type").toInt();
  QString content = heartBeat.value("Content").toString();
  QString interval = heartBeat.value("Interval").toString();

  for (int i = 0; i < mode_->body()->count(); ++i) {
    if (mode_->body()->itemData(i).toInt() == mode) {
      mode_->body()->setCurrentIndex(i);
    }
  }
  target_ip_->setText(targetHost);
  target_port_->setText(targetPort);
  self_ip_->setText(selfHost);
  self_port_->setText(selfPort);
  send_package_interval_->setText(packetInterval);
  send_package_max_size_->setText(packageMaxSize);

  for (int i = 0; i < framing_model_->body()->count(); ++i) {
    if (framing_model_->body()->itemData(i).toString() == model) {
      framing_model_->body()->setCurrentIndex(i);
      break;
    }
  }
  framing_timeout_->setText(timeout);
  framing_fixed_length_->setText(fixedLength);
  for (int i = 0; i < retransmission_->body()->count(); ++i) {
    if (retransmission_->body()->itemData(i).toString() == target) {
      retransmission_->body()->setCurrentIndex(i);
      break;
    }
  }

  for (int i = 0; i < heartbeat_send_type_->body()->count(); ++i) {
    if (heartbeat_send_type_->body()->itemData(i).toInt() == type) {
      heartbeat_send_type_->body()->setCurrentIndex(i);
      break;
    }
  }

  heartbeat_interval_->setText(interval);
  heartbeat_content_->setText(content);
}

void UdpClientSetting::setControlState(bool state) {
  target_ip_->setEnabled(state);
  target_port_->setEnabled(state);
  self_ip_->setEnabled(state);
  self_port_->setEnabled(state);
  send_package_interval_->setEnabled(state);
  send_package_max_size_->setEnabled(state);
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  retransmission_->setEnabled(state);
  heartbeat_content_->setEnabled(state);
  heartbeat_interval_->setEnabled(state);
  heartbeat_send_type_->setEnabled(state);
}

quint32 UdpClientSetting::getRefreshInterval() { return 1000; }

void UdpClientSetting::setLinkMode() {
  mode_->addItem(tr("单播"), TtUdpMode::Unicast);
  mode_->addItem(tr("组播"), TtUdpMode::Multicast);
  mode_->addItem(tr("广播"), TtUdpMode::Broadcast);
}

} // namespace Widget
