#include "core/serial_port.h"

namespace Core {

SerialPortWorker::SerialPortWorker(QObject* parent)
    : QObject(parent), serial_(new QSerialPort(this)) {
  // : QObject(parent) {
  // serial_ = new QSerialPort(this);
  qRegisterMetaType<SerialPortConfiguration>(
      "SerialPortConfiguration");  // 注册类型

  init();
  // 发送错误通过errorOccurred信号报告
  connect(serial_, &QSerialPort::errorOccurred,
          [this](QSerialPort::SerialPortError error) {
            if (error != QSerialPort::NoError) {
              emit errorOccurred(serial_->errorString());
            }
          });
}

SerialPortWorker::~SerialPortWorker() {
  qDebug() << "开始析构 SerialPortWorker" << QThread::currentThread();

  if (serial_ && serial_->isOpen()) {
    qDebug() << "关闭串口";
    serial_->close();
  }

  qDebug() << "删除serial_对象";
  delete serial_;

  qDebug() << "SerialPortWorker析构完成";
  // qDebug() << "delete" << QThread::currentThread();
  // if (serial_ && serial_->isOpen()) {
  //   serial_->close();
  // }
  // delete serial_;
}

bool SerialPortWorker::isOpened() {
  if (serial_) {
    return serial_->isOpen();
  }
  return false;
}

// void SerialPortWorker::initSerialPort() {
//   if (!serial_) {
//     qDebug() << "QSerialPort created in thread:" << QThread::currentThread();
//     // 在工作线程中创建，父对象为 this（属于工作线程）
//     serial_ = new QSerialPort(this);
//     qDebug() << "serial_: " << serial_;
//     connect(serial_, &QSerialPort::errorOccurred,
//             [this](QSerialPort::SerialPortError error) {
//               if (error != QSerialPort::NoError) {
//                 emit errorOccurred(serial_->errorString());
//               }
//             });
//   } else {
//     qDebug() << "QSerialPort created in thread:" << QThread::currentThread();
//   }
// }

void SerialPortWorker::sendData(const QString& send_string) {
  // 发送是非阻塞的
  QByteArray data = send_string.toUtf8().append("\r\n");
  {
    QMutexLocker locker(&send_mutex_);
    send_queue_.enqueue(data);
  }
  // 线程队列
  QMetaObject::invokeMethod(this, "handleWriteRequest", Qt::QueuedConnection);

  // if (serial_ && serial_->isOpen()) {
  //   QByteArray bts = send_string.toUtf8();
  //   bts.append("\r\n");
  //   serial_->write(bts);
  //   if (!serial_->waitForBytesWritten(1000)) {
  //     emit errorOccurred(serial_->errorString());
  //   } else {
  //     emit errorOccurred(tr("串口未打开"));
  //   }
  // }

  // 向串口发送数据
  // 可以通过一个 QPlainText 显示当前发送数据
  // send_string."\r\n";
  //QString out = send_string;
  //out.append("\r\n"); // 要在 QByteArray 后面加换行符
  //qDebug() << "write test: " << send_string.toLocal8Bit();
  //qDebug() << "write test: " << send_string.toUtf8();
  //qDebug() << "write test: " << send_string.toLatin1();
  //qDebug() << "write test: " << send_string;

  //QByteArray bts = send_string.toLocal8Bit();   //转换为8位字符数据数组
  // QByteArray bts = send_string.toUtf8();   // 支持中文
  // bts.append("\r\n"); // 要在 QByteArray 后面加换行符
  // serial_->write(bts);
}

void SerialPortWorker::readData() {
  // 事件驱动接收
  if (serial_ && serial_->isOpen()) {
    QByteArray data = serial_->readAll();
    if (!data.isEmpty()) {
      emit dataReceived(data);
    }
  }
  // qint64 bytesAvailable = serial_->bytesAvailable();
  // qDebug() << "Bytes available to read:" << bytesAvailable;

  // if (bytesAvailable > 0) {
  //   // 读取全部字节
  //   QByteArray data = serial_->readAll();
  //   //QString str(data);  // 采用的是默认编码
  //   QString str = QString::fromUtf8(data);  // 采用的是默认编码
  //   qDebug() << "recv: " << str;
  //   emit recvData(data);
  //   // if ()
  // } else {
  //   qDebug() << "No data available to read";
  // }
  // // showNowSendOrRecvSize(data.length(), false);
}

void SerialPortWorker::handleWriteRequest() {
  QMutexLocker locker(&send_mutex_);
  while (!send_queue_.isEmpty()) {
    QByteArray data = send_queue_.dequeue();
    serial_->write(data);
    // 移除 waitForBytesWritten，依赖异步错误信号
  }
}

void SerialPortWorker::openSerialPort(SerialPortConfiguration cfg) {
  // qDebug() << "openSerialPort called in thread:" << QThread::currentThread();

  if (!serial_) {
    qWarning() << "QSerialPort is not initialized!";
    return;
  }

  serial_->setPortName(cfg.com.left(cfg.com.indexOf('-')));
  //qDebug() << "TEST:" << cfg.com.left(cfg.com.indexOf('-'));
  serial_->setBaudRate(cfg.baud_rate);
  serial_->setDataBits(cfg.data_bits);
  serial_->setParity(cfg.parity);
  serial_->setStopBits(cfg.stop_bits);
  serial_->setFlowControl(cfg.flow_control);

  if (serial_->open(QIODevice::ReadWrite)) {
    // qDebug() << "test";
    connect(serial_, &QSerialPort::readyRead, this,
            &SerialPortWorker::readData);
    emit serialPortStatusChanged(true);  // 发射串口已开启的信号
  }
}

void SerialPortWorker::closeSerialPort() {
  if (serial_ && serial_->isOpen()) {
    serial_->close();
    emit serialPortStatusChanged(false);  // 发射串口已开启的信号
  }
}

void SerialPortWorker::init() {
  // foreach(int64 baud, QSerialPortInfo::standardBaudRates()) {
  //   list_baud_rate_.append(baud);
  // }

  // list_data_bits_ << QSerialPort::Data5 << QSerialPort::Data6 << QSerialPort::Data7 << QSerialPort::Data8;

  // map_parity_[QObject::tr("无校验")] = QSerialPort::NoParity;
  // map_parity_[QObject::tr("偶校验")] = QSerialPort::EvenParity;
  // map_parity_[QObject::tr("奇校验")] = QSerialPort::OddParity;
  // map_parity_[QObject::tr("0 校验")] = QSerialPort::SpaceParity;
  // map_parity_[QObject::tr("1 校验")] = QSerialPort::MarkParity;

  // map_stop_bits_[QObject::tr("1 位")] = QSerialPort::OneStop;
  // map_stop_bits_[QObject::tr("1.5 位")] = QSerialPort::OneAndHalfStop;
  // map_stop_bits_[QObject::tr("2 位")] = QSerialPort::TwoStop;

  // map_flow_control_[QObject::tr("None")] = QSerialPort::NoFlowControl;
  // map_flow_control_[QObject::tr("RTS/CTS")] = QSerialPort::HardwareControl;
  // map_flow_control_[QObject::tr("Xon/Xoff")] = QSerialPort::SoftwareControl;
}

}  // namespace Core
