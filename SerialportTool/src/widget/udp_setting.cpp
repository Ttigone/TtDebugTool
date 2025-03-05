#include "widget/udp_setting.h"

#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/fields/customize_fields.h>
#include <ui/window/combobox.h>

#include "core/udp_client.h"
#include "core/udp_server.h"

namespace Widget {

UdpServerSetting::UdpServerSetting(QWidget* parent) : QWidget(parent) {
  main_layout_ = new QVBoxLayout(this);

  QWidget* linkConfig = new QWidget;
  Ui::TtVerticalLayout* linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
  self_ip_ = new Ui::TtLabelLineEdit(tr("本地地址: "), linkConfig);
  self_port_ = new Ui::TtLabelLineEdit(tr("本地端口: "), linkConfig);

  linkConfigLayout->addWidget(self_ip_);
  linkConfigLayout->addWidget(self_port_);
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("UDP 连接"), linkConfig);

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
}

UdpServerSetting::~UdpServerSetting() {}

Core::UdpServerConfiguration UdpServerSetting::getUdpServerConfiguration() {
  Core::UdpServerConfiguration config(self_ip_->body()->text(),
                                      self_port_->body()->text());
  return config;
}

UdpClientSetting::UdpClientSetting(QWidget* parent) : QWidget(parent) {

  main_layout_ = new QVBoxLayout(this);

  QWidget* linkConfig = new QWidget;
  Ui::TtVerticalLayout* linkConfigLayout = new Ui::TtVerticalLayout(linkConfig);
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
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), linkConfig);
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
}

UdpClientSetting::~UdpClientSetting() {}

Core::UdpClientConfiguration UdpClientSetting::getUdpClientConfiguration() {
  Core::UdpClientConfiguration config(
      static_cast<TtUdpMode::Mode>(mode_->body()->currentData().toInt()),
      target_ip_->currentText(), target_port_->currentText(),
      self_ip_->currentText(), self_port_->currentText());
  return config;
}

void UdpClientSetting::setLinkMode() {
  mode_->addItem(tr("单播"), TtUdpMode::Unicast);
  mode_->addItem(tr("组播"), TtUdpMode::Multicast);
  mode_->addItem(tr("广播"), TtUdpMode::Broadcast);
}

}  // namespace Widget
