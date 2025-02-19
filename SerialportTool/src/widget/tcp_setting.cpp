#include "widget/tcp_setting.h"

#include <ui/widgets/fields/customize_fields.h>
#include <ui/window/combobox.h>

namespace Widget {
TcpServerSetting::TcpServerSetting(QWidget* parent)
    : QWidget(parent),
      host_(new Ui::TtLabelComboBox(tr("主机: "), this)),
      port_(new Ui::TtLabelLineEdit(tr("端口: "), this)) {

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());
  this->setLayout(layout);

  layout->addWidget(host_);
  layout->addWidget(port_);

  //connect(select_serial_port_, &Ui::TtLabelBtnComboBox::clicked,
  //        [this]() { setSerialPortsName(); });
}
TcpServerSetting::~TcpServerSetting() {}
TcpClientSetting::TcpClientSetting(QWidget* parent) : QWidget(parent) {}
TcpClientSetting::~TcpClientSetting() {}
}  // namespace Widget
