#include "widget/tcp_setting.h"

#include <ui/layout/vertical_layout.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/fields/customize_fields.h>
#include <ui/window/combobox.h>

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
}

TcpServerSetting::~TcpServerSetting() {}

Core::TcpServerConfiguration TcpServerSetting::getTcpServerConfiguration() {
  // 使用构造函数初始化
  Core::TcpServerConfiguration cfg(host_->body()->currentText(),
                                   port_->body()->text().toLong());
  // Core::TcpServerConfiguration cfg;
  // Core::TcpServerConfiguration cfg(host_->body()->itemData());
  // static_cast<QSerialPort::FlowControl>(
  //     host->body()
  //         ->itemData(select_fluid_control_->body()->currentIndex())
  //         .toInt()));
  return cfg;
}

void TcpServerSetting::setHost() {}

void TcpServerSetting::setPort() {}

TcpClientSetting::TcpClientSetting(QWidget* parent) : QWidget(parent) {}
TcpClientSetting::~TcpClientSetting() {}

}  // namespace Widget
