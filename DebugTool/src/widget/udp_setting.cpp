#include "widget/udp_setting.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

#include "core/udp_client.h"
#include "core/udp_server.h"

namespace Widget {

// UdpServerSetting::UdpServerSetting(QWidget *parent) : QWidget(parent) {
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
  Ui::Drawer *drawer1 = new Ui::Drawer(tr("UDP 连接"), linkConfig);

  QWidget *framingWidget = new QWidget;
  Ui::TtVerticalLayout *framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  // framing_timeout_ = new Ui::TtLabelComboBox(tr("时间: "));
  // framing_fixed_length_ = new Ui::TtLabelComboBox(tr("长度: "));
  framing_timeout_ = new Ui::TtLabelLineEdit(tr("时间(ms):"));
  framing_fixed_length_ = new Ui::TtLabelLineEdit(tr("长度:"));

  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);
  Ui::Drawer *drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);
  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawer2](int index) {
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
                new QResizeEvent(drawer2->size(), drawer2->size());
            QCoreApplication::postEvent(drawer2, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  retransmission_ = new Ui::TtLabelBtnComboBox(tr("目标: "));
  retransmission_->addItem(tr("无"));
  Ui::Drawer *drawer3 = new Ui::Drawer(tr("转发"), retransmission_);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2);
  scrollContentLayout->addWidget(drawer3);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);
}

UdpServerSetting::~UdpServerSetting() {}

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
  send_packet_interval_ =
      new Ui::TtLabelLineEdit(tr("发送端间隔: "), linkConfig);

  setLinkMode();
  linkConfigLayout->addWidget(mode_);
  linkConfigLayout->addWidget(target_ip_);
  linkConfigLayout->addWidget(target_port_);
  linkConfigLayout->addWidget(self_ip_);
  linkConfigLayout->addWidget(self_port_);
  linkConfigLayout->addWidget(send_packet_interval_);
  Ui::Drawer *drawer1 = new Ui::Drawer(tr("连接设置"), linkConfig);
  connect(mode_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawer1](int index) {
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

            const auto event =
                new QResizeEvent(drawer1->size(), drawer1->size());
            QCoreApplication::postEvent(drawer1, event);
          });
  mode_->setCurrentItem(0);

  QWidget *framingWidget = new QWidget;
  Ui::TtVerticalLayout *framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  // framing_timeout_ = new Ui::TtLabelComboBox(tr("时间: "));
  // framing_fixed_length_ = new Ui::TtLabelComboBox(tr("长度: "));
  framing_timeout_ = new Ui::TtLabelLineEdit(tr("时间(ms):"));
  framing_fixed_length_ = new Ui::TtLabelLineEdit(tr("长度:"));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);
  Ui::Drawer *drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);
  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
          [this, drawer2](int index) {
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
                new QResizeEvent(drawer2->size(), drawer2->size());
            QCoreApplication::postEvent(drawer2, event);
          });
  framing_model_->setCurrentItem(0);
  framing_timeout_->setVisible(false);
  framing_fixed_length_->setVisible(false);

  retransmission_ = new Ui::TtLabelBtnComboBox(tr("目标: "));
  retransmission_->addItem(tr("无"));
  Ui::Drawer *drawer3 = new Ui::Drawer(tr("转发"), retransmission_);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2);
  scrollContentLayout->addWidget(drawer3);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);
}

UdpClientSetting::~UdpClientSetting() {}

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
                     QJsonValue(send_packet_interval_->currentText()));
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
  QString sendPacketInterval =
      linkSetting.value("SendPacketInterval").toString();

  QJsonObject framing = config.value("Framing").toObject();
  QString model = framing.value("Model").toString();
  QString timeout = framing.value("Timeout").toString();
  QString fixedLength = framing.value("FixedLength").toString();

  QJsonObject retransmission = config.value("Retransmission").toObject();
  QString target = retransmission.value("Target").toString();

  for (int i = 0; i < mode_->body()->count(); ++i) {
    if (mode_->body()->itemData(i).toInt() == mode) {
      mode_->body()->setCurrentIndex(i);
    }
  }
  target_ip_->setText(targetHost);
  target_port_->setText(targetPort);
  self_ip_->setText(selfHost);
  self_port_->setText(selfPort);
  send_packet_interval_->setText(sendPacketInterval);

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

void UdpClientSetting::setControlState(bool state) {
  target_ip_->setEnabled(state);
  target_port_->setEnabled(state);
  self_ip_->setEnabled(state);
  self_port_->setEnabled(state);
  send_packet_interval_->setEnabled(state);
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  retransmission_->setEnabled(state);
}

quint32 UdpClientSetting::getRefreshInterval() { return 1000; }

void UdpClientSetting::setLinkMode() {
  mode_->addItem(tr("单播"), TtUdpMode::Unicast);
  mode_->addItem(tr("组播"), TtUdpMode::Multicast);
  mode_->addItem(tr("广播"), TtUdpMode::Broadcast);
}

} // namespace Widget
