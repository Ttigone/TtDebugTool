#include "function_selection_window.h"

#include <QScrollArea>

#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>

#include "Def.h"

namespace Window {

FunctionSelectionWindow::FunctionSelectionWindow(QWidget *parent)
    : QWidget{parent} {
  // : Ui::CustomTabPage{parent} {
  init();
}

FunctionSelectionWindow::~FunctionSelectionWindow() {
  // qDebug() << "delete " << __FUNCTION__;
}

void FunctionSelectionWindow::init() {
  Ui::TtVerticalLayout *mainLayout = new Ui::TtVerticalLayout(this);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameShape(QFrame::NoFrame);
  Ui::TtVerticalLayout *scrollLayout = new Ui::TtVerticalLayout(scroll);

  mainLayout->addWidget(scroll);

  QWidget *cont = new QWidget();
  label_ = new Ui::TtElidedLabel(tr("新建连接"), scroll);
  label_->resize(68, 68);

  // 功能窗口布局
  QGridLayout *function_layout_ = new QGridLayout(cont);

  Ui::RichTextButton *serial_ =
      new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("串口"),
                             tr("连接到串口设备以进行数据发送和接收"));

  Ui::RichTextButton *tcp_ =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("TCP 客户端"),
                             tr("新建 TCP 客户端以连接到远程服务器"));

  Ui::RichTextButton *udp_ =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("UDP"),
                             tr("新建 UDP 发送端以发送数据到远程目标"));

  Ui::RichTextButton *mqtt_ =
      new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("MQTT 客户端"),
                             tr("新建 MQTT 客户端用于连接到远程服务器"));

  Ui::RichTextButton *modbus_ =
      new Ui::RichTextButton(QImage(":/sys/modbus.svg"), tr("Modbus 主机"),
                             tr("新建 Modubus 主机以连接到从机设备"));

  Ui::RichTextButton *blueteeth_ = new Ui::RichTextButton(
      QImage(":/sys/bluetooth-contact.svg"), tr("BT 蓝牙主机"),
      tr("新建经典蓝牙主机以扫描并连接到远程设备"));

  function_layout_->addWidget(serial_, 0, 0);
  function_layout_->addWidget(tcp_, 0, 1);
  function_layout_->addWidget(udp_, 0, 2);
  function_layout_->addWidget(mqtt_, 1, 0);
  function_layout_->addWidget(modbus_, 1, 1);
  function_layout_->addWidget(blueteeth_, 1, 2);

  scrollLayout->addStretch();
  scrollLayout->addWidget(label_, 0, Qt::AlignVCenter);
  scrollLayout->addSpacerItem(new QSpacerItem(0, 20));
  scrollLayout->addWidget(cont, 0, Qt::AlignVCenter);
  scrollLayout->addStretch();

  connect(serial_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::Serial); });
  connect(tcp_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::TcpClient); });
  connect(udp_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::UdpClient); });
  connect(mqtt_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::MqttClient); });
  connect(modbus_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::ModbusClient); });
  connect(blueteeth_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::BlueTeeth); });
}

SimulateFunctionSelectionWindow::SimulateFunctionSelectionWindow(
    QWidget *parent)
    : QWidget{parent} {
  init();
}

SimulateFunctionSelectionWindow::~SimulateFunctionSelectionWindow() {
  qDebug() << "Delete" << __FUNCTION__;
}

void SimulateFunctionSelectionWindow::init() {
  Ui::TtVerticalLayout *mainLayout = new Ui::TtVerticalLayout(this);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameShape(QFrame::NoFrame);
  Ui::TtVerticalLayout *scrollLayout = new Ui::TtVerticalLayout(scroll);

  mainLayout->addWidget(scroll);

  QWidget *cont = new QWidget();
  label_ = new Ui::TtElidedLabel(tr("新建连接"), scroll);
  label_->resize(68, 68);

  // 功能窗口布局
  QGridLayout *function_layout_ = new QGridLayout(cont);

  Ui::RichTextButton *tcp_ = new Ui::RichTextButton(
      QImage(":/sys/network.svg"), tr("TCP 服务端模拟"),
      tr("通过配置匹配规则以自动响应来自 TCP 连接的请求"));

  Ui::RichTextButton *udp_ = new Ui::RichTextButton(
      QImage(":/sys/network.svg"), tr("UDP 服务端模拟"),
      tr("通过配置匹配规则以自动响应来自 UDP 连接的请求"));

  Ui::RichTextButton *modbus_ =
      new Ui::RichTextButton(QImage(":/sys/modbus.svg"), tr("Modbus 模拟"),
                             tr("模拟线圈/离散输入/保持寄存器/输入寄存器操作"));

  Ui::RichTextButton *mqtt_ = new Ui::RichTextButton(
      QImage(":/sys/tmp.png"), tr("MQTT Broker 模拟"),
      tr("管理主题、客户端，并通过配置匹配规则以使消息自动回复"));

  function_layout_->addWidget(tcp_, 0, 1);
  function_layout_->addWidget(udp_, 0, 2);
  function_layout_->addWidget(modbus_, 1, 1);
  function_layout_->addWidget(mqtt_, 1, 2);

  scrollLayout->addStretch();
  scrollLayout->addWidget(label_, 0, Qt::AlignVCenter);
  scrollLayout->addSpacerItem(new QSpacerItem(0, 20));
  scrollLayout->addWidget(cont, 0, Qt::AlignVCenter);
  scrollLayout->addStretch();

  connect(tcp_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::TcpServer); });

  connect(udp_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::UdpServer); });

  connect(mqtt_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::MqttBroker); });

  connect(modbus_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::ModbusServer); });
}
} // namespace Window

AllFunctionSelectionWindow::AllFunctionSelectionWindow(QWidget *parent) {
  init();
}

AllFunctionSelectionWindow::~AllFunctionSelectionWindow() {}

void AllFunctionSelectionWindow::init() {
  Ui::TtVerticalLayout *mainLayout = new Ui::TtVerticalLayout(this);

  QScrollArea *scroll = new QScrollArea(this);
  scroll->setFrameShape(QFrame::NoFrame);
  Ui::TtVerticalLayout *scrollLayout = new Ui::TtVerticalLayout(scroll);

  mainLayout->addWidget(scroll);

  QWidget *cont = new QWidget();
  label_ = new Ui::TtElidedLabel(tr("新建连接"), scroll);
  label_->resize(68, 68);

  function_layout_ = new QGridLayout(cont);

  Ui::RichTextButton *serial_ =
      new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("串口"),
                             tr("连接到串口设备以进行数据发送和接收"));

  Ui::RichTextButton *tcpClient =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("TCP 客户端"),
                             tr("新建 TCP 客户端以连接到远程服务器"));

  Ui::RichTextButton *udpClient =
      new Ui::RichTextButton(QImage(":/sys/network.svg"), tr("UDP"),
                             tr("新建 UDP 发送端以发送数据到远程目标"));

  Ui::RichTextButton *mqttClient =
      new Ui::RichTextButton(QImage(":/sys/tmp.png"), tr("MQTT 客户端"),
                             tr("新建 MQTT 客户端用于连接到远程服务器"));

  Ui::RichTextButton *modbusClient =
      new Ui::RichTextButton(QImage(":/sys/modbus.svg"), tr("Modbus 主机"),
                             tr("新建 Modubus 主机以连接到从机设备"));

  Ui::RichTextButton *blueteeth_ = new Ui::RichTextButton(
      QImage(":/sys/bluetooth-contact.svg"), tr("BT 蓝牙主机"),
      tr("新建经典蓝牙主机以扫描并连接到远程设备"));

  function_layout_->addWidget(serial_, 0, 0);
  function_layout_->addWidget(tcpClient, 0, 1);
  function_layout_->addWidget(udpClient, 0, 2);
  function_layout_->addWidget(mqttClient, 1, 0);
  function_layout_->addWidget(modbusClient, 1, 1);
  function_layout_->addWidget(blueteeth_, 1, 2);

  Ui::RichTextButton *tcpServer = new Ui::RichTextButton(
      QImage(":/sys/network.svg"), tr("TCP 服务端模拟"),
      tr("通过配置匹配规则以自动响应来自 TCP 连接的请求"));

  Ui::RichTextButton *udpServer = new Ui::RichTextButton(
      QImage(":/sys/network.svg"), tr("UDP 服务端模拟"),
      tr("通过配置匹配规则以自动响应来自 UDP 连接的请求"));

  Ui::RichTextButton *modbusServer =
      new Ui::RichTextButton(QImage(":/sys/modbus.svg"), tr("Modbus 模拟"),
                             tr("模拟线圈/离散输入/保持寄存器/输入寄存器操作"));

  Ui::RichTextButton *mqttServer = new Ui::RichTextButton(
      QImage(":/sys/tmp.png"), tr("MQTT Broker 模拟"),
      tr("管理主题、客户端，并通过配置匹配规则以使消息自动回复"));

  function_layout_->addWidget(tcpServer, 2, 1);
  function_layout_->addWidget(udpServer, 2, 2);
  function_layout_->addWidget(modbusServer, 3, 1);
  function_layout_->addWidget(mqttServer, 3, 2);

  scrollLayout->addStretch();
  scrollLayout->addWidget(label_, 0, Qt::AlignVCenter);
  scrollLayout->addSpacerItem(new QSpacerItem(0, 20));
  scrollLayout->addWidget(cont, 0, Qt::AlignVCenter);
  scrollLayout->addStretch();

  connect(serial_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::Serial); });
  connect(tcpClient, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::TcpClient); });
  connect(udpClient, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::UdpClient); });
  connect(mqttClient, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::MqttClient); });
  connect(modbusClient, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::ModbusClient); });
  connect(blueteeth_, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::BlueTeeth); });

  connect(tcpServer, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::TcpServer); });

  connect(udpServer, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::UdpServer); });

  connect(mqttServer, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::MqttBroker); });

  connect(modbusServer, &Ui::RichTextButton::clicked,
          [this]() { emit switchRequested(TtProtocolRole::ModbusServer); });
}
