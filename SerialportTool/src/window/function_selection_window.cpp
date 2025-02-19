#include "function_selection_window.h"

#include <ui/widgets/buttons.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/labels.h>

namespace Window {

FunctionSelectionWindow::FunctionSelectionWindow(QWidget* parent)
    : QWidget{parent} {
  init();
}

FunctionSelectionWindow::~FunctionSelectionWindow()
{

}

void FunctionSelectionWindow::init()
{
  Ui::TtVerticalLayout *mainLayout = new Ui::TtVerticalLayout();
  setLayout(mainLayout);

  label_ = new Ui::TtNormalLabel(tr("新建连接"));
  label_->setMaximumSize(100, 80);

  QWidget *cont = new QWidget();
  // 功能窗口布局
  QGridLayout *function_layout_ = new QGridLayout();

  cont->setLayout(function_layout_);

  Ui::RichTextButton *serial_ = new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("串口"), tr("连接到串口设备以进行数据发送和接收"));

  Ui::RichTextButton* tcp_ =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("TCP 客户端"),
                             tr("新建 TCP 客户端以连接到远程服务器"));

  Ui::RichTextButton* udp_ =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("UDP"),
                             tr("新建 UDP 发送端以发送数据到远程目标"));

  Ui::RichTextButton* blueteeth_ = new Ui::RichTextButton(
      QImage(":/sys/bluetooth-contact.svg"), tr("BT 蓝牙主机"),
      tr("新建经典蓝牙主机以扫描并连接到远程设备"));

  Ui::RichTextButton* modbus_ =
      new Ui::RichTextButton(QImage(":/sys/modbus.svg"), tr("Modbus 主机"),
                             tr("新建 Modubus 主机以连接到从机设备"));

  Ui::RichTextButton *mqtt_ = new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("MQTT 客户端"), tr("新建 MQTT 客户端用于连接到远程服务器"));

  function_layout_->addWidget(serial_, 0, 0);
  function_layout_->addWidget(tcp_, 0, 1);
  function_layout_->addWidget(udp_, 0, 2);
  function_layout_->addWidget(blueteeth_, 1, 0);
  function_layout_->addWidget(modbus_, 1, 1);
  function_layout_->addWidget(mqtt_, 1, 2);

  mainLayout->addStretch();
  mainLayout->addWidget(label_, 0, Qt::AlignVCenter);
  mainLayout->addSpacerItem(new QSpacerItem(0, 20));
  mainLayout->addWidget(cont, 0, Qt::AlignVCenter);
  mainLayout->addStretch();

  connect(serial_, &Ui::RichTextButton::clicked, [this]() {
    // qDebug() << "yyyy";
    // emit toSerial();
    emit switchRequested(0);
  });

  connect(tcp_, &Ui::RichTextButton::clicked, [this]() {
    // emit toTcp();
    emit switchRequested(1);
  });

  connect(udp_, &Ui::RichTextButton::clicked, [this]() {
    // emit toUdp();
    emit switchRequested(2);
  });

  connect(blueteeth_, &Ui::RichTextButton::clicked, [this]() {
    // emit toBlueTeeth();
    emit switchRequested(3);
  });

  connect(modbus_, &Ui::RichTextButton::clicked, [this]() {
    // emit toModubus();
    emit switchRequested(4);
  });

  connect(mqtt_, &Ui::RichTextButton::clicked, [this]() {
    // emit toMQTT();
    emit switchRequested(5);
  });
}

}
