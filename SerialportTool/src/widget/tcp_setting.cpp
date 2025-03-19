#include "widget/tcp_setting.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>
#include <QNetworkInterface>

#include "core/tcp_client.h"
#include "core/tcp_server.h"

namespace Widget {
TcpServerSetting::TcpServerSetting(QWidget* parent) : QWidget(parent) {

  main_layout_ = new QVBoxLayout(this);

  QWidget* linkConfig = new QWidget;
  Ui::TtVerticalLayout* linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  host_ = new Ui::TtLabelComboBox(tr("主机: "), linkConfig);
  port_ = new Ui::TtLabelLineEdit(tr("端口: "), linkConfig);
  linkConfigLayout->addWidget(host_);
  linkConfigLayout->addWidget(port_);
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("TCP 连接"), linkConfig);

  QWidget* framingWidget = new QWidget;
  Ui::TtVerticalLayout* framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelComboBox(tr("时间: "));
  framing_fixed_length_ = new Ui::TtLabelComboBox(tr("长度: "));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);
  Ui::Drawer* drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);
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
  Ui::Drawer* drawer3 = new Ui::Drawer(tr("转发"), retransmission_);

  QScrollArea* scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget* scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout* scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2);
  scrollContentLayout->addWidget(drawer3);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);

  setHostAddress();
}

TcpServerSetting::~TcpServerSetting() {}

Core::TcpServerConfiguration TcpServerSetting::getTcpServerConfiguration() {
  // 使用构造函数初始化
  Core::TcpServerConfiguration cfg(host_->body()->currentText(),
                                   port_->body()->text().toLong());

  return cfg;
}

const QJsonObject& TcpServerSetting::getTcpServerSetting() {
  auto config = getTcpServerConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("SelfHost", QJsonValue(config.host));
  linkSetting.insert("SelfPort", QJsonValue(config.port));
  tcp_server_save_config_.insert("LinkSetting", QJsonValue(linkSetting));

  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  framing.insert("Timeout",
                 QJsonValue(framing_timeout_->body()->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->body()->currentText()));
  tcp_server_save_config_.insert("Framing", QJsonValue(framing));

  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  tcp_server_save_config_.insert("Retransmission", QJsonValue(retransmission));

  return tcp_server_save_config_;
}

void TcpServerSetting::setHost() {}

void TcpServerSetting::setPort() {}

void TcpServerSetting::setHostAddress() {
  QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
  for (const QHostAddress& address : ipAddresses) {
    host_->addItem(address.toString());
  }
  host_->body()->model()->sort(0);
}

TcpClientSetting::TcpClientSetting(QWidget* parent) : QWidget(parent) {

  main_layout_ = new QVBoxLayout(this);

  QWidget* linkConfig = new QWidget;
  Ui::TtVerticalLayout* linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  target_host_ = new Ui::TtLabelComboBox(tr("地址: "), linkConfig);
  target_port_ = new Ui::TtLabelLineEdit(tr("端口: "), linkConfig);
  self_host_ = new Ui::TtLabelLineEdit(tr("本地地址: "), linkConfig);
  self_port_ = new Ui::TtLabelLineEdit(tr("本地端口: "), linkConfig);

  send_packet_interval_ =
      new Ui::TtLabelLineEdit(tr("发送端间隔: "), linkConfig);
  linkConfigLayout->addWidget(target_host_);
  linkConfigLayout->addWidget(target_port_);
  linkConfigLayout->addWidget(self_host_);
  linkConfigLayout->addWidget(self_port_);
  linkConfigLayout->addWidget(send_packet_interval_);
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), linkConfig);

  QWidget* framingWidget = new QWidget;
  Ui::TtVerticalLayout* framingWidgetLayout =
      new Ui::TtVerticalLayout(framingWidget);
  framing_model_ = new Ui::TtLabelComboBox(tr("模式: "));
  framing_model_->addItem(tr("无"));
  framing_model_->addItem(tr("超时时间"));
  framing_model_->addItem(tr("固定长度"));
  framing_timeout_ = new Ui::TtLabelComboBox(tr("时间: "));
  framing_fixed_length_ = new Ui::TtLabelComboBox(tr("长度: "));
  framingWidgetLayout->addWidget(framing_model_);
  framingWidgetLayout->addWidget(framing_timeout_);
  framingWidgetLayout->addWidget(framing_fixed_length_);
  Ui::Drawer* drawer2 = new Ui::Drawer(tr("分帧"), framingWidget);
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
  Ui::Drawer* drawer3 = new Ui::Drawer(tr("转发"), retransmission_);

  QScrollArea* scroll = new QScrollArea(this);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setWidgetResizable(true);
  QWidget* scrollContent = new QWidget(scroll);

  Ui::TtVerticalLayout* scrollContentLayout =
      new Ui::TtVerticalLayout(scrollContent);
  scrollContentLayout->addWidget(drawer1, 0, Qt::AlignTop);
  scrollContentLayout->addWidget(drawer2);
  scrollContentLayout->addWidget(drawer3);
  scrollContentLayout->addStretch();

  scroll->setWidget(scrollContent);

  main_layout_->addWidget(scroll);

  setHostAddress();
}
TcpClientSetting::~TcpClientSetting() {}

Core::TcpClientConfiguration TcpClientSetting::getTcpClientConfiguration() {
  Core::TcpClientConfiguration cfg(target_host_->body()->currentText(),
                                   target_port_->body()->text(),
                                   self_host_->body()->text(),
                                   self_port_->body()->text());
  return cfg;
}

const QJsonObject& TcpClientSetting::getTcpClientSetting() {
  auto config = getTcpClientConfiguration();
  QJsonObject linkSetting;
  linkSetting.insert("TargetHost", QJsonValue(config.target_host));
  linkSetting.insert("TargetPort", QJsonValue(config.target_port));
  linkSetting.insert("SelfHost", QJsonValue(config.self_host));
  linkSetting.insert("SelfPort", QJsonValue(config.self_port));
  tcp_client_save_config_.insert("LinkSetting", QJsonValue(linkSetting));

  QJsonObject framing;
  framing.insert("Model", QJsonValue(framing_model_->body()->currentText()));
  framing.insert("Timeout",
                 QJsonValue(framing_timeout_->body()->currentText()));
  framing.insert("FixedLength",
                 QJsonValue(framing_fixed_length_->body()->currentText()));
  tcp_client_save_config_.insert("Framing", QJsonValue(framing));

  QJsonObject retransmission;
  retransmission.insert("Target", QJsonValue(retransmission_->currentText()));
  tcp_client_save_config_.insert("Retransmission", QJsonValue(retransmission));

  return tcp_client_save_config_;
}

void TcpClientSetting::setHostAddress() {
  QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
  for (const QHostAddress& address : ipAddresses) {
    target_host_->addItem(address.toString());
  }
  target_host_->body()->model()->sort(0);
}

}  // namespace Widget
