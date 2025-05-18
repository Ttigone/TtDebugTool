#include "widget/tcp_setting.h"

#include <QNetworkInterface>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>

#include "core/tcp_client.h"
#include "core/tcp_server.h"

namespace Widget {

TcpServerSetting::TcpServerSetting(QWidget *parent) : FrameSetting(parent) {

  main_layout_ = new QVBoxLayout(this);

  QWidget *linkConfig = new QWidget;
  Ui::TtVerticalLayout *linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  host_ = new Ui::TtLabelComboBox(tr("主机: "), linkConfig);
  port_ = new Ui::TtLabelLineEdit(tr("端口: "), linkConfig);
  linkConfigLayout->addWidget(host_);
  linkConfigLayout->addWidget(port_);
  Ui::Drawer *drawer1 = new Ui::Drawer(tr("TCP 连接"), linkConfig);

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
  connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged, this,
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

  addLineEdit(port_->body());
  addLineEdit(framing_timeout_->body());
  addLineEdit(framing_fixed_length_->body());

  addComboBox(host_->body());
  addComboBox(framing_model_->body());
  addComboBox(retransmission_->body());

  link();

  setHostAddress();
}

TcpServerSetting::~TcpServerSetting() {}

Core::TcpServerConfiguration TcpServerSetting::getTcpServerConfiguration() {
  // 使用构造函数初始化
  Core::TcpServerConfiguration cfg(host_->body()->currentText(),
                                   port_->body()->text().toLong());

  return cfg;
}

const QJsonObject &TcpServerSetting::getTcpServerSetting() {
  auto config = getTcpServerConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("SelfHost", QJsonValue(config.host));
  linkSetting.insert("SelfPort", QJsonValue(config.port));
  tcp_server_save_config_.insert("LinkSetting", QJsonValue(linkSetting));

  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  framing.insert("Timeout", QJsonValue(framing_timeout_->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->currentText()));
  tcp_server_save_config_.insert("Framing", QJsonValue(framing));

  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  tcp_server_save_config_.insert("Retransmission", QJsonValue(retransmission));

  return tcp_server_save_config_;
}

void TcpServerSetting::setOldSettings(const QJsonObject &config) {
  if (config.isEmpty()) {
    return;
  }
  QJsonObject linkSetting = config.value("LinkSetting").toObject();
  QString selfHost = linkSetting.value("SelfHost").toString();
  QString selfPort = linkSetting.value("SelfPort").toString();
  QString packageInterval = linkSetting.value("SendPackageInterval").toString();
  QString packageMaxSize = linkSetting.value("SendPackageMaxSize").toString();

  QJsonObject framing = config.value("Framing").toObject();
  QString model = framing.value("Model").toString();
  QString timeout = framing.value("Timeout").toString();
  QString fixedLength = framing.value("FixedLength").toString();

  QJsonObject retransmission = config.value("Retransmission").toObject();
  QString target = retransmission.value("Target").toString();

  // 宏配置

  // 设置串口名
  for (int i = 0; i < host_->body()->count(); ++i) {
    if (host_->body()->itemData(i).toString() == selfHost) {
      host_->body()->setCurrentIndex(i);
      break;
    }
  }
  port_->setText(selfPort);

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

const QJsonObject &TcpServerSetting::getSerialSetting() {
  return tcp_server_save_config_;
}

void TcpServerSetting::setHost() {}

void TcpServerSetting::setPort() {}

void TcpServerSetting::setControlState(bool state) {
  host_->setEnabled(state);
  port_->setEnabled(state);
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  retransmission_->setEnabled(state);
}

void TcpServerSetting::setHostAddress() {
  QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
  for (const QHostAddress &address : ipAddresses) {
    host_->addItem(address.toString());
  }
  host_->body()->model()->sort(0);
  host_->body()->setCurrentIndex(0);
}

TcpClientSetting::TcpClientSetting(QWidget *parent) : FrameSetting(parent) {
  main_layout_ = new QVBoxLayout(this);

  QWidget *linkConfig = new QWidget;
  Ui::TtVerticalLayout *linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  target_host_ = new Ui::TtLabelComboBox(tr("地址:"), linkConfig);
  target_port_ = new Ui::TtLabelLineEdit(tr("端口:"), linkConfig);
  self_host_ = new Ui::TtLabelLineEdit(tr("本地地址:"), linkConfig);
  self_port_ = new Ui::TtLabelLineEdit(tr("本地端口:"), linkConfig);

  send_package_max_size_ =
      new Ui::TtLabelLineEdit(tr("发送包最大尺寸:"), linkConfig);
  send_package_interval_ =
      new Ui::TtLabelLineEdit(tr("发送端间隔(ms):"), linkConfig);
  linkConfigLayout->addWidget(target_host_);
  linkConfigLayout->addWidget(target_port_);
  linkConfigLayout->addWidget(self_host_);
  linkConfigLayout->addWidget(self_port_);
  linkConfigLayout->addWidget(send_package_interval_);
  linkConfigLayout->addWidget(send_package_max_size_);
  Ui::Drawer *drawer1 = new Ui::Drawer(tr("连接设置"), linkConfig);

  QWidget *framingWidget = new QWidget;
  Ui::TtVerticalLayout *framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelLineEdit(tr("时间:"));
  framing_fixed_length_ = new Ui::TtLabelLineEdit(tr("长度:"));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);

  Ui::Drawer *drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);
  QObject::connect(framing_model_, &Ui::TtLabelComboBox::currentIndexChanged,
                   this, [this, drawer2](int index) {
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

  // Ui::TtDrawer *drawerHeartBeat = new Ui::TtDrawer(
  //     tr("心跳"), ":/sys/chevron-double-up.svg",
  //     ":/sys/chevron-double-down.svg", heartbeatWidget, false, this);
  Ui::Drawer *drawerHeartBeat = new Ui::Drawer(tr("心跳"), heartbeatWidget);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget *scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout *scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2);
  scrollContentLayout->addWidget(drawer3);
  scrollContentLayout->addWidget(drawerHeartBeat);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);

  addLineEdit(target_port_->body());
  addLineEdit(self_host_->body());
  addLineEdit(self_port_->body());
  addLineEdit(send_package_max_size_->body());
  addLineEdit(send_package_interval_->body());
  addLineEdit(framing_timeout_->body());
  addLineEdit(framing_fixed_length_->body());
  addLineEdit(heartbeat_interval_->body());
  addLineEdit(heartbeat_content_->body());

  addComboBox(target_host_->body());
  addComboBox(framing_model_->body());
  addComboBox(retransmission_->body());
  addComboBox(heartbeat_send_type_->body());

  // yes
  link();

  setHostAddress();
}
TcpClientSetting::~TcpClientSetting() {}

Core::TcpClientConfiguration TcpClientSetting::getTcpClientConfiguration() {
  Core::TcpClientConfiguration cfg(
      target_host_->body()->currentText(), target_port_->body()->text(),
      self_host_->body()->text(), self_port_->body()->text());
  return cfg;
}

const QJsonObject &TcpClientSetting::getTcpClientSetting() {
  auto config = getTcpClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("TargetHost", QJsonValue(config.target_host));
  linkSetting.insert("TargetPort", QJsonValue(config.target_port));
  linkSetting.insert("SelfHost", QJsonValue(config.self_host));
  linkSetting.insert("SelfPort", QJsonValue(config.self_port));
  linkSetting.insert("SendPackageInterval",
                     QJsonValue(send_package_interval_->currentText()));
  linkSetting.insert("SendPackageMaxSize",
                     QJsonValue(send_package_max_size_->currentText()));
  tcp_client_save_config_.insert("LinkSetting", QJsonValue(linkSetting));

  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  framing.insert("Timeout", QJsonValue(framing_timeout_->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->currentText()));
  tcp_client_save_config_.insert("Framing", QJsonValue(framing));

  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  tcp_client_save_config_.insert("Retransmission", QJsonValue(retransmission));

  return tcp_client_save_config_;
}

void TcpClientSetting::setOldSettings(const QJsonObject &config) {
  if (config.isEmpty()) {
    return;
  }
  QJsonObject linkSetting = config.value("LinkSetting").toObject();
  QString targetHost = linkSetting.value("TargetHost").toString();
  QString targetPort = linkSetting.value("TargetPort").toString();
  QString selfHost = linkSetting.value("SelfHost").toString();
  QString selfPort = linkSetting.value("SelfPort").toString();
  QString packageInterval = linkSetting.value("PackageInterval").toString();
  QString packageMaxSize = linkSetting.value("SendPackageMaxSize").toString();

  QJsonObject framing = config.value("Framing").toObject();
  QString model = framing.value("Model").toString();
  QString timeout = framing.value("Timeout").toString();
  QString fixedLength = framing.value("FixedLength").toString();

  QJsonObject retransmission = config.value("Retransmission").toObject();
  QString target = retransmission.value("Target").toString();

  for (int i = 0; i < target_host_->body()->count(); ++i) {
    if (target_host_->body()->itemData(i).toString() == targetHost) {
      qDebug() << "find";
      target_host_->body()->setCurrentIndex(i);
      break;
    }
  }

  target_port_->setText(targetPort);
  self_host_->setText(selfHost);
  self_port_->setText(selfPort);

  send_package_interval_->setText(packageInterval);
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
}

void TcpClientSetting::setControlState(bool state) {
  framing_model_->setEnabled(state);
  framing_timeout_->setEnabled(state);
  framing_fixed_length_->setEnabled(state);
  send_package_interval_->setEnabled(state);
  send_package_max_size_->setEnabled(state);
  heartbeat_send_type_->setEnabled(state);
  heartbeat_interval_->setEnabled(state);
  heartbeat_content_->setEnabled(state);
}

void TcpClientSetting::setHostAddress() {
  QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
  for (const QHostAddress &address : ipAddresses) {
    // 相同的
    target_host_->addItem(address.toString(), address.toString());
  }
  target_host_->body()->model()->sort(0);
  target_host_->body()->setCurrentIndex(0);
}

} // namespace Widget
