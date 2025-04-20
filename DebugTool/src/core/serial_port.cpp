#include "core/serial_port.h"

namespace Core {

SerialPortWorker::SerialPortWorker(QObject* parent)
    // : QObject(parent), serial_(new QSerialPort(this)) {
    : QObject(parent),
      serial_(new QSerialPort),
      helper_(new SerialHelper(parent)) {
  serial_->setParent(this);
  serial_->setReadBufferSize(recv_max_len_);
  qRegisterMetaType<SerialPortConfiguration>(
      "SerialPortConfiguration");  // 注册类型

  // 延迟初始化定时器
  QTimer::singleShot(0, this, [this]() {
    receive_timer_ = new QTimer(this);  // 在工作线程创建定时器
    // receive_timer_->setInterval(50);    // 50ms 结束一次数据接收
    // receive_timer_->setSingleShot(true);

    // connect(receive_timer_, &QTimer::timeout, this, [this]() {
    //   if (!receive_buffer_.isEmpty()) {
    //     // 50ms 发送一次数据
    //     qDebug() << "rece: " << receive_buffer_;
    //     emit dataReceived(receive_buffer_);
    //     receive_buffer_.clear();
    //   }
    // });
    connect(receive_timer_, &QTimer::timeout, this,
            &SerialPortWorker::handleTimeout);
  });

  // qDebug() << QThread::currentThread();

  // receive_timer_.setInterval(50);
  // receive_timer_.setSingleShot(true);
  // connect(&receive_timer_, &QTimer::timeout, this, [this]() {
  //   if (!receive_buffer_.isEmpty()) {
  //     qDebug() << receive_buffer_;
  //     emit dataReceived(receive_buffer_);
  //     receive_buffer_.clear();
  //   }
  // });

  connect(serial_, &QSerialPort::errorOccurred,
          [this](QSerialPort::SerialPortError error) {
            if (error != QSerialPort::NoError) {
              emit errorOccurred(serial_->errorString());
            }
          });
}

SerialPortWorker::~SerialPortWorker() {
  // qDebug() << "开始析构 SerialPortWorker" << QThread::currentThread();
  if (receive_timer_ && receive_timer_->isActive()) {
    receive_timer_->stop();
  }

  if (serial_ && serial_->isOpen()) {
    // qDebug() << "关闭串口";
    serial_->close();
  }

  // qDebug() << "删除serial_对象";
  delete serial_;

  // qDebug() << "SerialPortWorker析构完成";
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

void SerialPortWorker::sendData(const QString& send_string) {
  // 发送是非阻塞的
  // 以文本格式发送
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

void SerialPortWorker::sendData(const QByteArray& data) {
  {
    QMutexLocker locker(&send_mutex_);
    send_queue_.enqueue(data);
  }
  // 线程队列
  QMetaObject::invokeMethod(this, "handleWriteRequest", Qt::QueuedConnection);
}

void SerialPortWorker::readData() {
  // 重新开启接收定时器 4ms
  // receive_timer_->start(4);

  // 数据追加到缓存区
  // 直接读取全部数据
  // receive_buffer_.append(serial_->readAll());
  // parseBuffer();

  // 发送数据, 内部不做串口数据解析, 交给外部处理
  QByteArray data = serial_->readAll();
  emit dataReceived(data);

  // 这个有问题
  // 读取数据, 串口中有数据, 立马读取
  // recv_len_ = serial_->read(receive_buffer_.data(), recv_max_len_);

  // qDebug() << "buf: " << receive_buffer_ << "len: " << recv_len_;

  // // 遍历
  // int i = 0;
  // while (i < recv_len_) {
  //   switch (helper_->state()) {
  //     case SerialHelper::FrameState::STATE_READY: {
  //       // 头部判断
  //       // 指定的头部, 外部提供
  //       if (receive_buffer_.at(i) == 0x55) {
  //         helper_->setState(SerialHelper::FrameState::STATE_HEAD);
  //       }
  //       break;
  //     }
  //     case SerialHelper::FrameState::STATE_HEAD: {
  //       // 数据类型判断
  //       // 头部, 类型外部提供
  //       if (receive_buffer_.at(i) == 0x02) {  //
  //         helper_->setDataType(SerialHelper::FrameType::Type1);
  //         helper_->setState(SerialHelper::FrameState::STATE_TYPE);
  //       } else {
  //         helper_->setState(SerialHelper::FrameState::STATE_READY);
  //       }
  //       break;
  //     }
  //     case SerialHelper::FrameState::STATE_TYPE: {
  //       // 数据帧长度
  //       if (receive_buffer_.at(i) <= 6) {
  //         helper_->setFrameLength(receive_buffer_.at(i));  // 记录数据长度
  //         helper_->setCurrentRecvLength(0);
  //         helper_->clearCache();
  //         helper_->setState(SerialHelper::FrameState::STATE_NUM);

  //       } else {
  //         helper_->setState(SerialHelper::FrameState::STATE_READY);
  //       }
  //       break;
  //     }
  //     case SerialHelper::FrameState::STATE_NUM: {
  //       // 接收到了数据长度
  //       helper_->setData(helper_->currentBuffLength(), receive_buffer_.at(i));
  //       helper_->currentRecvLengthPlus();
  //       if (helper_->currentBuffLength() >= helper_->frameLength()) {
  //         // 接收的数据大于等于记录的数据长度
  //         // 表示数据接收完毕
  //         helper_->setState(SerialHelper::FrameState::STATE_DATA);
  //       }
  //       break;
  //     }
  //     case SerialHelper::FrameState::STATE_DATA: {
  //       // 接收到数据, 当前的 receive_buffer_.at(i) 是校验和
  //       // 打印当前收到的数据
  //       printf("recv data: ", helper_->data());
  //       break;
  //     }
  //     case SerialHelper::FrameState::STATE_TAIL: {
  //       break;
  //     }
  //     default: {
  //       helper_->setState(SerialHelper::FrameState::STATE_READY);
  //       break;
  //     }
  //   }
  //   ++i;
  // }
}
// // 未处理的数据
// emit dataReceived(receive_buffer_);
// // qDebug() << receive_buffer_;

// // 对方是十六进制的发送
// // 帧头帧尾
// // 假设帧头是0xAA，帧尾是0x55
// int headerIndex = receive_buffer_.indexOf(static_cast<char>(0xAA));
// if (headerIndex != -1) {
//   // 检索到帧头
//   int tailIndex =
//       receive_buffer_.indexOf(static_cast<char>(0x55), headerIndex + 1);
//   if (tailIndex != -1) {
//     // 取出完整数据帧（包含帧头与帧尾）
//     QByteArray frame =
//         receive_buffer_.mid(headerIndex, tailIndex - headerIndex + 1);
//     // 处理这部分完整帧数据
//     processFrame(frame);
//     // 移除已经处理过的部分
//     receive_buffer_.remove(0, tailIndex + 1);
//   }
// }

// // 事件驱动接收
// if (serial_ && serial_->isOpen()) {
//   QByteArray data = serial_->readAll();
//   if (!data.isEmpty()) {
//     receive_buffer_.append(data);
//     receive_timer_->start();
//     // emit dataReceived(data);
//   }
// }

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
// }

void SerialPortWorker::handleWriteRequest() {
  QMutexLocker locker(&send_mutex_);
  while (!send_queue_.isEmpty()) {
    QByteArray data = send_queue_.dequeue();
    // 文本写
    serial_->write(data);
  }
}

void SerialPortWorker::handleTimeout() {
  // 定时器超时：可以在这里决定是否清理buffer或者对不完整的数据帧进行处理
  // if (!receive_buffer_.isEmpty()) {
  // qDebug() << "接收数据超时，处理部分数据或清空缓存";
  // 对未完整的帧进行处理，如丢弃或部分解析，视协议而定
  // receive_buffer_.clear();
  // }
  // qDebug() << "超时，当前缓存内容:" << receive_buffer_.toHex();

  // 索引
  // int headerIndex = receive_buffer_.indexOf(static_cast<char>(0xAA));
  // // 帧尾
  // int tailIndex =
  //     receive_buffer_.indexOf(static_cast<char>(0x55), headerIndex + 1);

  // if (headerIndex != -1 && tailIndex != -1) {
  //   // 还有可能是一帧数据，处理它
  //   QByteArray frame =
  //       receive_buffer_.mid(headerIndex, tailIndex - headerIndex + 1);
  //   processFrame(frame);
  //   receive_buffer_.remove(0, tailIndex + 1);
  // } else {
  //   // 没有发现完整帧，可以选择部分保留
  //   if (headerIndex != -1) {
  //     // 说明头存在但尾巴还没来，可以保留从header开始的部分
  //     receive_buffer_ = receive_buffer_.mid(headerIndex);
  //   } else {
  //     // 没有帧头，数据无效，清空
  //     receive_buffer_.clear();
  //   }
  // }

  // 一次计时结束
  // int bufferlens = 0;
  // QString str = receive_buffer_;
  // receive_timer_->stop();

  // qDebug() << receive_buffer_;
}

void SerialPortWorker::parseBuffer() {
  qDebug() << "raw hex:" << receive_buffer_.toHex().toUpper();
  // 根据具体协议处理帧数据
  // qDebug() << "收到完整帧:" << frame.toHex();

  // 最小帧长度
  const int MIN_FRAME_SIZE = 2 /*hdr*/ + 1 /*type*/ + 1 /*len*/ + 1 /*cs*/;

  // while (true) {
  while (receive_buffer_.size() >= MIN_FRAME_SIZE) {

    if (receive_buffer_.size() < MIN_FRAME_SIZE) {
      return;
    }
    // 检索帧头
    int pos = receive_buffer_.indexOf(QByteArrayView(&HDR0), 0);

    // 找到帧头
    while (pos >= 0 && (pos + 1 >= receive_buffer_.size() ||
                        receive_buffer_[pos + 1] != HDR1)) {
      pos = receive_buffer_.indexOf(QByteArrayView(&HDR0, 1), pos + 1);
    }
    // 没有帧头
    if (pos < 0) {
      receive_buffer_.clear();
      return;
    }
    // 丢弃帧头前的无效字节
    if (pos > 0) {
      // [0] == header0 [1] = header2
      receive_buffer_.remove(0, pos);
    }
    // 剩余长度是否小于最小长度, 满足一个帧的长度
    if (receive_buffer_.size() < MIN_FRAME_SIZE) {
      // 等下一次 readAll
      return;
    }

    quint8 type = quint8(receive_buffer_[2]);
    quint8 len = quint8(receive_buffer_[3]);

    // hdr0+hdr1+type+len+payload+cs
    int fullSize = 2 + 1 + 1 + len + 1;  // 附带校验

    if (receive_buffer_.size() < fullSize) {
      // 完整帧没齐, 等下一次 readAll
      return;
    }

    // 提取帧
    QByteArray frame = receive_buffer_.mid(0, fullSize);

    // // 校验：简单求和取低8位
    // quint8 cs = 0;
    // for (int i = 2; i < 2 + 1 + 1 + len; ++i)  // 从 type 开始到 payload 末尾
    //   cs += quint8(receive_buffer_[i]);
    // if (cs == quint8(receive_buffer_[2 + 1 + 1 + len])) {
    //   // 校验通过
    //   processFrame(type, receive_buffer_.mid(4, len));
    // } else {
    //   qWarning() << "Frame checksum error!";
    // }

    // 处理帧
    processFrame(type, receive_buffer_.mid(4, len));

    // 移出已经处理的一帧字节, 保留后面的
    receive_buffer_.remove(0, fullSize);
  }
}

void SerialPortWorker::processFrame(quint8 type, const QByteArray& payload) {
  // qDebug() << payload;
  // 能够解析 0102
  // 按 type 分发、解析 payload
  switch (type) {
    case 0x01:
      // 解析命令 A
      qDebug() << "type 01: " << payload;
      break;
    case 0x02:
      // 解析命令 B
      break;
    default:
      qWarning() << "Unknown frame type" << type;
  }
  // 处理完后, 帧应该如何做 ?
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

// SerialPortWorker Core::Q_DECLARE_METATYPE(SerialPortConfiguration) {}

}  // namespace Core
