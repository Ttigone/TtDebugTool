#include "widget/serial_operation.h"

namespace Widget {

SerialOperation::SerialOperation(QWidget *parent)
    : QWidget(parent)
{
  init();
}

SerialOperation::~SerialOperation()
{

}

void SerialOperation::needDisabled()
{
  open_serial_->setDisabled(true);
}

void SerialOperation::init()
{
  open_serial_ = new QPushButton(tr("打开串口"), this);
  close_serial_ = new QPushButton(tr("关闭串口"), this);
  clear_receive_ = new QPushButton(tr("清空接收"), this);
  save_receive_ = new QPushButton(tr("保存接收"), this);
  send_datas_ = new QPushButton(tr("发送数据"), this);
  send_return_ = new QPushButton(tr("发送回车"), this);
  show_receive_time_ = new QCheckBox(tr("接收时间"), this);
  show_hex_ = new QCheckBox(tr("HEX显示"), this);
  word_wrap_ = new QCheckBox(tr("自动换行"), this);



  QGridLayout *layout = new QGridLayout;
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());
  this->setLayout(layout);

  layout->addWidget(open_serial_, 0, 0, 1, 1, Qt::AlignLeft);
  layout->addWidget(close_serial_, 1, 0, 1, 1, Qt::AlignLeft);
  layout->addWidget(clear_receive_, 0, 1, 1, 1, Qt::AlignLeft);
  layout->addWidget(save_receive_, 1, 1, 1, 1, Qt::AlignLeft);
  layout->addWidget(send_datas_, 0, 4, 1, 1, Qt::AlignLeft);
  layout->addWidget(send_return_, 1, 4, 1, 1, Qt::AlignLeft);
  layout->addWidget(show_receive_time_, 0, 2, 2, 1, Qt::AlignLeft);
  layout->addWidget(show_hex_, 0, 3, 1, 1, Qt::AlignLeft);
  layout->addWidget(word_wrap_, 1, 3, 1, 1, Qt::AlignLeft);

  connect(open_serial_, &QPushButton::clicked, [this]() {
    qDebug() << QThread::currentThread();
    emit openSerialPort();
  });
  connect(send_datas_, &QPushButton::clicked, [this]() {
    // 发出信号
    emit sendData();
  });
}

} // namespace Widget
