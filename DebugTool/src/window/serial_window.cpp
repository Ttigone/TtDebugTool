#include "window/serial_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/controls/TtModbusPlot.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/widget_group.h>

#include <core/lua_kernel.h>
#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>
#include <ui/controls/TtQCustomPlot.h>
#include <ui/controls/TtSerialLexer.h>

#include "ui/controls/TtLuaInputBox.h"
#include "ui/controls/TtTableView.h"
#include "widget/serial_setting.h"

#include "ui/controls/TtChannelButton.h"
#include "ui/controls/TtChannelButtonEditorDialog.h"
#include "ui/controls/TtSerialPortPlot.h"

namespace Window {

SerialWindow::SerialWindow(QWidget* parent)
    : FrameWindow(parent),
      worker_thread_(new QThread(this)),
      serial_port_(new Core::SerialPortWorker),
      serial_setting_(new Widget::SerialSetting) {

  init();
  connectSignals();

  // 放在线程中执行
  serial_port_->moveToThread(worker_thread_);

  // 成功 析构函数在工作线程中执行, 否则 serial_port_ 无法执行析构
  connect(worker_thread_, &QThread::finished, serial_port_,
          &QObject::deleteLater);
  connect(worker_thread_, &QThread::finished, worker_thread_,
          &QObject::deleteLater);

  connect(serial_port_, &Core::SerialPortWorker::dataReceived, this,
          &SerialWindow::dataReceived);

  connect(serial_port_, &Core::SerialPortWorker::errorOccurred, this,
          &SerialWindow::showErrorMessage);


  worker_thread_->start();
}

SerialWindow::~SerialWindow() {
  // // qDebug() << "delete SerialWindow";
  // worker_thread_->quit();
  // // worker_thread_->exit();
  // worker_thread_->wait();
  // // 手动触发 SerialPortWorker 的析构（在工作线程中）
  // // 修改为这样调试
  // qDebug() << "准备调用deleteLater";
  // bool success = QMetaObject::invokeMethod(serial_port_, "deleteLater",
  //                                          Qt::QueuedConnection);
  // qDebug() << "调用结果:" << success;
  // // 确保事件得到处理
  // // if (serial_port_) {
  // //   qDebug() << "delete SerialWindow";
  // //   serial_port_->deleteLater();
  // //   // QMetaObject::invokeMethod(serial_port_.get(), "deleteLater",
  // //   // QMetaObject::invokeMethod(serial_port_, "deleteLater",
  // //   //                           Qt::QueuedConnection);
  // // }
  // delete worker_thread_;  // 删除线程对象
  // QMetaObject::invokeMethod(serial_port_, "deleteLater", Qt::QueuedConnection);

  if (worker_thread_) {
    worker_thread_->quit();
    // 3. 等待线程完成(设置超时，避免无限等待)
    if (!worker_thread_->wait(200)) {
      qWarning()
          << "Worker thread did not exit gracefully, forcing termination";
      worker_thread_->terminate();  // 强制终止(不推荐，但作为最后手段)
      worker_thread_->wait();       // 等待强制终止完成
    }
    // 无效
    // delete serial_port_;
    // QMetaObject::invokeMethod(serial_port_, "deleteLater",
    //                           Qt::QueuedConnection);
  }
}

QString SerialWindow::getTitle() {
  return title_->text();
}

QJsonObject SerialWindow::getConfiguration() const {
  return cfg_.obj;
}

void SerialWindow::saveWaveFormData() {
  // save to csv data
  if (serial_plot_) {
    qDebug() << "csv";
    serial_plot_->saveWaveFormData();
  }
}

bool SerialWindow::IsWorking() const {
  // 如果打开的是异常串口, 同时又保存了, 有问题
  return serial_port_opened;
}

bool SerialWindow::IsSaved() {
  // serial_setting_->getSerialSetting();

  // if (cfg_.obj.value("WindowTitle").toString() != title_->text()) {
  // }
  return !unsaved_;
}

void SerialWindow::switchToEditMode() {
  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  // 预先获取之前的标题
  title_edit_->setText(title_->text());
  // 显示 edit 模式
  stack_->setCurrentWidget(edit_widget_);
  // 获取焦点
  title_edit_->setFocus();  // 自动聚焦输入框
}

void SerialWindow::switchToDisplayMode() {
  //QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(original_widget_);
  //original_widget_->setGraphicsEffect(effect);
  //QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  //anim->setDuration(300);
  //anim->setStartValue(0);
  //anim->setEndValue(1);
  //anim->start(QAbstractAnimation::DeleteWhenStopped);
  // 切换显示模式
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

void SerialWindow::setDisplayType(MsgType type) {
  display_type_ = type;
  refreshTerminalDisplay();
}

void SerialWindow::setHeartbeartContent() {
  // 发送包间隔不适用于心跳, 但是发送包尺寸适用于心跳内容
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    sendMessageToPort(heartbeat_);
  }
}

void SerialWindow::saveLog() {
  // saveBtn->connect(saveBtn, &QPushButton::clicked, [this]() {
  //   QString fileName = QFileDialog::getSaveFileName(this, tr("保存日志"),
  //                                                   QDir::homePath() + "/serial_log.txt",
  //                                                   tr("文本文件 (*.txt)"));
  //   if (!fileName.isEmpty()) {
  //     QFile file(fileName);
  //     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
  //       QTextStream stream(&file);
  //       stream << terminal_->text();
  //       file.close();
  //     }
  //   }
  // });
}

void SerialWindow::refreshTerminalDisplay() {
  terminal_->clear();
  terminal_->setUpdatesEnabled(false);

  // 遍历模型生成内容
  for (int i = 0; i < message_model_->rowCount(); ++i) {
    QModelIndex idx = message_model_->index(i);

    Ui::TtChatMessage* msg = qobject_cast<Ui::TtChatMessage*>(
        idx.data(Ui::TtChatMessageModel::MessageObjectRole).value<QObject*>());

    // 构建带换行的完整内容
    // QString header = msg->timestamp().toString("[yyyy-MM-dd hh:mm:ss.zzz] ") +
    //                  (msg->isOutgoing() ? " TX " : " RX ");

    QString header = msg->isOutgoing() ? "[Tx]" : "[Rx]";
    header.append(' ');
    header.append(msg->timestamp().toString("[yyyy-MM-dd hh:mm:ss.zzz]"));
    header.append(' ');

    //  text 发送, 点击 text 时，文本消失 content 为 0
    QString content;
    if (display_type_ == MsgType::HEX) {
      content = msg->contentAsHex().trimmed();
    } else if (display_type_ == MsgType::TEXT) {
      content = msg->contentAsText();
    }
    // QString content =
    //     display_type_
    //         ? msg->contentAsHex().trimmed()  // 移除hex内容末尾可能的多余空格
    //         : msg->contentAsText();

    // 使用appendPlainText保持换行格式
    // header.append(content).append('\n');
    header.append(content);
    qDebug() << header;

    terminal_->appendPlainText(header);

    // terminal_->appendPlainText(header);
    // terminal_->appendPlainText(content + "\n");
  }

  terminal_->setUpdatesEnabled(true);
}

void SerialWindow::addChannelInfo(const QString& uuid, const QColor& color,
                                  const QByteArray& blob) {
  channel_info_[uuid] = qMakePair(channel_nums_, blob);
  quint16 channel = channel_nums_;
  // title 可以重复
  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  // QString title, header, headerLength, type, typeOffset, length, lengthOffset,
  //     tail;
  // in >> title >> header >> headerLength >> type >> typeOffset >> length >>
  //     lengthOffset >> tail;

  QString title, header, headerLength, typeOffset, lengthOffset, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >> tail;

  QString luaCode;
  in >> luaCode;

  if (!luaCode.isEmpty()) {
    // 存储 lua 代码
    lua_script_codes_[channel_nums_] = luaCode;
  }

  // 无校验
  ParserRule rule = {
      false,
      channel_nums_,                         // 通道
      QByteArray::fromHex(header.toUtf8()),  // 头部
      headerLength.toInt(),                  // 帧头长度
      typeOffset.toInt(),                    // 类型字段偏移
      lengthOffset.toInt(),                  // 长度字段偏移
      0,                                     // 帧尾长度
      false,                                 // 无校验
      {},
      [this, channel](uint8_t type, const QByteArray& payload,
                      const QString& luaCode) {
        qDebug() << "payload: " << payload;

        // 长度为 x 个 hex 字节
        // 解析的 payload 长度, 每个字节提供给 lua 使用
        quint8 paramNums = payload.size();

        QVariantList params;
        params << payload;
        double ret;

        qDebug() << luaCode;

        Core::LuaKernel kernel;
        kernel.doLuaCode(luaCode, params, ret);

        // // 十六进制
        // // 小端序解析 (低字节在前)
        quint16 value = (static_cast<quint8>(payload[1]) << 8) |
                        static_cast<quint8>(payload[0]);

        serial_plot_->addData(channel, value);

        // // 或大端序解析 (高字节在前)
        // quint16 valueBeEndian =
        //     (static_cast<quint8>(data[0]) << 8) |
        //     static_cast<quint8>(data[1]);

        qDebug() << "解析的整数值:" << value;
      }};

  serial_plot_->addGraphs(channel, color);
  // 修改则覆盖原有的值
  rules_.insert(uuid, rule);
  channel_nums_++;
}

void SerialWindow::handleDialogData(const QString& uuid, quint16 channel,
                                    const QByteArray& blob) {
  // title 可以重复
  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  // QString title, header, headerLength, type, typeOffset, length, lengthOffset,
  //     tail;
  // in >> title >> header >> headerLength >> type >> typeOffset >> length >>
  //     lengthOffset >> tail;

  QString title, header, headerLength, typeOffset, lengthOffset, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >> tail;

  QString luaCode;
  in >> luaCode;

  if (!luaCode.isEmpty()) {
    qDebug() << "test";
    lua_script_codes_[channel] = luaCode;
  }

  // 每个 uuid 的对应配置数组
  channel_info_[uuid] = qMakePair(channel, blob);

  // qDebug() << title << header << headerLength << typeOffset << lengthOffset
  //          << length << tail;

  auto it = rules_.find(uuid);
  if (it != rules_.end()) {
    // 修改配置
    ParserRule& rule = it.value();
    rule.header = QByteArray::fromHex(header.toUtf8());

    if (rule.header.isEmpty()) {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "",
                                    tr("输入的不是合法的十六进制字符串!"), 1500,
                                    this);
      return;
    }

    rule.header_len = headerLength.toInt();
    // rule.type_bytes = type.toInt();
    rule.type_offset = typeOffset.toInt();
    // rule.len_bytes = length.toInt();
    rule.len_offset = lengthOffset.toInt();
  } else {
    Ui::TtMessageBar::information(TtMessageBarType::Top, "",
                                  tr("不存在当前配置, 请删除配置选项"), 1500,
                                  this);
    return;
  }
}

void SerialWindow::parseBuffer() {
  //   // 解析, 每个通道都有不同的 uuid 值
  //   for (auto& channel : channel_info_) {
  //     // 每个 channel 都保存者一个 receive_buffer_ 的副本
  //     // channel.second;
  //     QByteArrayView view(channel.second.header.toUtf8());
  //   }

  //   static constexpr char HDR0 = char(0xAA);
  //   static constexpr char HDR1 = char(0x55);
  //   // qDebug() << "raw hex:" << receive_buffer_.toHex().toUpper();

  //   // 根据具体协议处理帧数据
  //   // qDebug() << "收到完整帧:" << frame.toHex();

  //   // 校验位是否需要 ?
  //   // 最小帧长度
  //   // const int MIN_FRAME_SIZE = 2 /*hdr*/ + 1 /*type*/ + 1 /*len*/ + 1 /*cs*/;
  //   const int MIN_FRAME_SIZE = 2 /*hdr*/ + 1 /*type*/ + 1 /*len*/;

  //   // while (true) {
  //   while (receive_buffer_.size() >= MIN_FRAME_SIZE) {

  //     if (receive_buffer_.size() < MIN_FRAME_SIZE) {
  //       return;
  //     }
  //     // 检索帧头
  //     int pos = receive_buffer_.indexOf(QByteArrayView(&HDR0), 0);

  //     // 找到帧头
  //     while (pos >= 0 && (pos + 1 >= receive_buffer_.size() ||
  //                         receive_buffer_[pos + 1] != HDR1)) {
  //       pos = receive_buffer_.indexOf(QByteArrayView(&HDR0, 1), pos + 1);
  //     }
  //     // 没有帧头
  //     if (pos < 0) {
  //       receive_buffer_.clear();
  //       return;
  //     }
  //     // 丢弃帧头前的无效字节
  //     if (pos > 0) {
  //       // [0] == header0 [1] = header2
  //       receive_buffer_.remove(0, pos);
  //     }
  //     // 剩余长度是否小于最小长度, 满足一个帧的长度
  //     if (receive_buffer_.size() < MIN_FRAME_SIZE) {
  //       // 等下一次 readAll
  //       return;
  //     }

  //     quint8 type = quint8(receive_buffer_[2]);
  //     quint8 len = quint8(receive_buffer_[3]);

  //     // hdr0+hdr1+type+len+payload+cs
  //     // int fullSize = 2 + 1 + 1 + len + 1;  // 附带校验
  //     int fullSize = 2 + 1 + 1 + len;  // 无校验

  //     if (receive_buffer_.size() < fullSize) {
  //       // 完整帧没齐, 等下一次 readAll
  //       return;
  //     }

  //     // 提取帧
  //     QByteArray frame = receive_buffer_.mid(0, fullSize);

  //     // // 校验：简单求和取低8位
  //     // quint8 cs = 0;
  //     // for (int i = 2; i < 2 + 1 + 1 + len; ++i)  // 从 type 开始到 payload 末尾
  //     //   cs += quint8(receive_buffer_[i]);
  //     // if (cs == quint8(receive_buffer_[2 + 1 + 1 + len])) {
  //     //   // 校验通过
  //     //   processFrame(type, receive_buffer_.mid(4, len));
  //     // } else {
  //     //   qWarning() << "Frame checksum error!";
  //     // }

  //     // 处理帧
  //     processFrame(type, receive_buffer_.mid(4, len));

  //     // 移出已经处理的一帧字节, 保留后面的
  //     receive_buffer_.remove(0, fullSize);
  //   }
  // }

  // void SerialWindow::processFrame(quint8 type, const QByteArray& payload) {
  //   // qDebug() << payload;
  //   // 能够解析 0102
  //   // 按 type 分发、解析 payload
  //   switch (type) {
  //     case 0x01:
  //       // 解析命令 A
  //       qDebug() << "type 01: " << payload;
  //       break;
  //     case 0x02:
  //       // 解析命令 B
  //       break;
  //     default:
  //       qWarning() << "Unknown frame type" << type;
  //   }
  //   // 处理完后, 帧应该如何做 ?
  //   // 对应的图像通道展示数据

  while (!receive_buffer_.isEmpty()) {
    // 遍历所有解析规则，寻找最早出现的有效帧头
    int bestPos = -1;
    ParserRule const* bestRule = nullptr;  // 解析规则
    // 遍历当前存在的规则
    for (auto& rule : rules_) {
      if (rule.enable) {
        // 能够查找头部
        qDebug() << "header: " << rule.header;
        // qDebug() << "rece: " << receive_buffer_;
        // 索引某个规则的头部, 谁先索引得到, 那就先解析
        int pos = receive_buffer_.indexOf(rule.header);
        if (pos >= 0 && (bestPos == -1 || pos < bestPos)) {
          // 保存了当前索引头部字节的 地址
          bestPos = pos;
          bestRule = &rule;
        }
      }
    }
    // 没有匹配的帧头, 丢弃所有数据
    if (!bestRule) {
      receive_buffer_.clear();
      return;
    }
    // 丢弃帧头前的无用字节
    if (bestPos > 0) {
      receive_buffer_.remove(0, bestPos);
    }
    qDebug() << "size: " << receive_buffer_.size();
    // qDebug() << bestRule->header_len + bestRule->len_offset +
    //                 bestRule->len_bytes;
    // 剩余最小长度 >= 头部长度
    // if (receive_buffer_.size() <
    //     bestRule->header_len + bestRule->type_offset + bestRule->len_offset + bestRule->len_bytes) {
    //   return;
    // }
    // 1 + 1 + 2 +
    // 缺少数据字段和校验字段, 等待
    // 帧头长度与类型偏移总是一致的

    if (receive_buffer_.size() <
        bestRule->header_len + bestRule->type_offset + bestRule->len_offset) {
      return;
    }

    // 有问题 应当是 长度为 2
    // 提取长度字段 (ex.多字节长度)
    // len_offset 是 2
    quint32 payloadLen = 0;

    // 获取长度字段
    payloadLen = receive_buffer_[bestRule->len_offset];

    // 长度为 3， 负载为 2
    qDebug() << bestRule->len_offset << payloadLen;

    // 2 + 1 + 1 + 2 + 0 + 0
    const int fullSize = bestRule->header_len + 1 +
                         (bestRule->len_offset - bestRule->header_len) +
                         payloadLen + bestRule->tail_len +
                         (bestRule->has_checksum ? 1 : 0);

    // 输出6
    qDebug() << "size: " << fullSize;

    if (receive_buffer_.size() < fullSize) {
      return;
    }

    // 切出一整帧
    QByteArray frame = receive_buffer_.left(fullSize);

    // 可选校验
    if (bestRule->has_checksum && !bestRule->validate(frame)) {
      if (!bestRule->validate(frame)) {
        // 校验失败，丢弃这个 header 字节，重试
        receive_buffer_.remove(0, 1);
        continue;
      }
    }

    qDebug() << "frame: " << frame;

    // 调用各自的处理函数
    quint8 type = static_cast<quint8>(frame.at(bestRule->type_offset));


    // 数据字节
    /**
     * @brief payload 
     * 1 字节头部长度 
     * 1 字节类型长度
     * 1 字节长度字段
     * 剩余是 数据字段 + cs
     */
    QByteArray payload = frame.mid(
        bestRule->header_len + 1 + bestRule->len_offset - bestRule->header_len,
        payloadLen);

    // 调用自己的帧处理函数
    bestRule->processFrame(type, payload, lua_script_codes_[bestRule->channel]);

    // 调用自己的处理的这帧
    receive_buffer_.remove(0, fullSize);
  }
}

void SerialWindow::showErrorMessage(const QString& text) {
  Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500, this);
  on_off_btn_->setChecked(false);
  serial_port_opened = false;
  serial_setting_->setControlState(true);
  send_package_timer_->stop();
  heartbeat_timer_->stop();
}

void SerialWindow::dataReceived(const QByteArray& data) {
  // 接收数据
  // qDebug() << "data: " << data;
  receive_buffer_.append(data);
  // qDebug() << "rece_buf"
  // 解析数据帧
  parseBuffer();

  // 上面只对 receive_buffer_ 处理

  // serial_plot_->saveWaveFormData();

  // 导出 csv 格式. 在 menu 中实现

  QString content;
  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");

  recv_byte_count += data.size();
  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setRawData(data);
  tmp->setOutgoing(false);
  tmp->setBubbleColor(QColor("#0ea5e9"));
  tmp->setTimestamp(now);
  QList<Ui::TtChatMessage*> list;
  list.append(tmp);
  message_model_->appendMessages(list);

  content += "[Rx]";
  content += ' ';
  content.append(timestamp);
  content += ' ';
  // terminal_->appendPlainText(timestamp + "[Rx]");  // 单独追加箭头行

  if (display_type_ == MsgType::HEX) {
    // terminal_->appendPlainText(data.toHex(' ').toUpper());
    content.append(data.toHex(' ').toUpper());
  } else if (display_type_ == MsgType::TEXT) {
    // terminal_->appendPlainText(data);  // 数据内容作为独立块
    content.append(data);
  }
  qDebug() << "Rx: " << content;

  terminal_->appendPlainText(content);

  recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  message_view_->scrollToBottom();
}

QByteArray SerialWindow::saveState() const {
  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  // 保存需要的数据
  stream << title_->text();
  return state;
}

bool SerialWindow::restoreState(const QByteArray& state) {
  QDataStream stream(state);
  // 恢复数据
  stream >> title;
  // stream >> someData_;
  return true;
}

void SerialWindow::sendMessageToPort() {
  // 一开始是 0
  // 发送包间隔可以没有, 但是心跳必须得有间隔
  // 点击发送按钮, 消息获取从 editor
  // 判断当前的发送类型
  // TEXT 发送, 接收正确
  // HEX 发送, 接收正确, 但是显示错误, TEXT 应该显示 error, 切换到  HEX 则显示正确
  // HEX 显示上有问题

  QString content;
  QString text = editor->text();
  QByteArray data;

  if (send_type_ == MsgType::HEX) {
    QString hexStr = text;
    hexStr = hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (hexStr.length() % 2 != 0) {
      qDebug() << "HEX 字符串长度不是偶数，可能出错";
      return;
    }
    data = QByteArray::fromHex(hexStr.toUtf8());
  } else if (send_type_ == MsgType::TEXT) {
    data = text.toUtf8();
  }

  // 发送包拆分
  if (package_size_ > 0) {
    for (int i = 0; i < text.size(); i += package_size_) {
      msg_queue_.enqueue(text.mid(i, package_size_));
    }
  } else {
    // 如果发送包大小为 0
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    send_byte_count += text.size();
    // 获取当前时间并格式化消息
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    // terminal_->appendPlainText(timestamp + " << ");  // 单独追加箭头行
    content += "[Tx]";
    content += ' ';
    content.append(timestamp);
    content += ' ';

    if (display_type_ == MsgType::TEXT) {
      // terminal_->appendPlainText(QString::fromUtf8(data));
      content.append(QString::fromUtf8(data));
    } else if (display_type_ == MsgType::HEX) {
      // terminal_->appendPlainText(data.toHex(' ').toUpper().trimmed());
      content.append(data.toHex(' ').toUpper().trimmed());
    }
    qDebug() << "TX: " << content;
    terminal_->appendPlainText(content);

    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(text);  // 显示的文本
    // tmp->setRawData(text.toUtf8());
    tmp->setRawData(data);  // 原始文本
    tmp->setTimestamp(now);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage*> list;
    list.append(tmp);
    message_model_->appendMessages(list);

    // 串口发送
    // QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
    //                           Q_ARG(QString, data));

    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));

    send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    message_view_->scrollToBottom();
  }
}

void SerialWindow::sendMessageToPort(const QString& data) {
  // 平均有 10ms 的延时
  if (package_size_ > 0) {
    for (int i = 0; i < data.size(); i += package_size_) {
      msg_queue_.enqueue(data.mid(i, package_size_));
    }
  }

  // if (!serial_port_opened) {
  //   Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
  //                           1500, this);
  //   return;
  // }
  // send_byte_count += data.size();
  // // 获取当前时间并格式化消息
  // QDateTime now = QDateTime::currentDateTime();
  // QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  // QString formattedMessage = timestamp + " << " + "\n" + data + "\n";
  // // 添加到终端
  // terminal_->appendPlainText(formattedMessage);

  // auto tmp = new Ui::TtChatMessage();
  // tmp->setContent(data);
  // tmp->setRawData(data.toUtf8());
  // tmp->setTimestamp(now);
  // tmp->setOutgoing(true);
  // tmp->setBubbleColor(QColor("#DCF8C6"));
  // QList<Ui::TtChatMessage*> list;
  // list.append(tmp);
  // message_model_->appendMessages(list);

  // // 串口发送
  // QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
  //                           Q_ARG(QString, data));
  // send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
  // message_view_->scrollToBottom();
}

void SerialWindow::sendMessageToPort(const QString& data, const int& times) {
  QTimer::singleShot(times, this, [this, data]() {
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    send_byte_count += data.size();
    // 获取当前时间并格式化消息
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    QString formattedMessage = timestamp + " << " + "\n" + data + "\n";
    // 添加到终端
    terminal_->appendPlainText(formattedMessage);

    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(data);
    tmp->setRawData(data.toUtf8());
    tmp->setTimestamp(now);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage*> list;
    list.append(tmp);
    message_model_->appendMessages(list);
    // 串口发送
    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QString, data));
    send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    message_view_->scrollToBottom();
  });
}

void SerialWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  title_ = new Ui::TtNormalLabel(tr("未命名串口连接"));
  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout* tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout* edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &SerialWindow::switchToEditMode);

  Ui::TtHorizontalLayout* tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout* tmpAll = new Ui::TtHorizontalLayout;

  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout* tmpl2 = new Ui::TtHorizontalLayout;
  // 保存按钮
  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);

  // 删除按钮, 是需要保存在 leftbar 才会添加的

  // 开关按钮
  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setColors(Qt::black, Qt::red);
  on_off_btn_->setSvgSize(18, 18);

  tmpl2->addWidget(save_btn_);
  tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  tmpAll->addLayout(tmpP1);
  tmpAll->addLayout(tmpl2);

  main_layout_->addLayout(tmpAll);

  // 左右分隔器
  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget* chose_function = new QWidget;
  Ui::TtHorizontalLayout* chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  // 切换
  QWidget* twoBtnForGroup = new QWidget(chose_function);
  QHBoxLayout* layouttest = new QHBoxLayout(twoBtnForGroup);
  Ui::TtSvgButton* terminalButton =
      new Ui::TtSvgButton(":/sys/terminal.svg", twoBtnForGroup);
  terminalButton->setSvgSize(18, 18);
  terminalButton->setColors(Qt::black, Qt::blue);
  // leftBtn->setEnableHoldToCheck(true);
  Ui::TtSvgButton* chatButton =
      new Ui::TtSvgButton(":/sys/chat.svg", twoBtnForGroup);
  chatButton->setSvgSize(18, 18);
  chatButton->setColors(Qt::black, Qt::blue);
  // rightBtn->setEnableHoldToCheck(true);

  Ui::TtSvgButton* graphBtn =
      new Ui::TtSvgButton(":/sys/graph-up.svg", twoBtnForGroup);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);

  layouttest->addWidget(terminalButton);
  layouttest->addWidget(chatButton);
  layouttest->addWidget(graphBtn);
  // 互斥
  Ui::TtWidgetGroup* test_ = new Ui::TtWidgetGroup(this);
  test_->setHoldingChecked(true);
  test_->addWidget(terminalButton);
  test_->addWidget(chatButton);
  test_->addWidget(graphBtn);
  test_->setExclusive(true);
  test_->setCheckedIndex(0);
  chose_function_layout->addWidget(twoBtnForGroup);
  chose_function_layout->addStretch();

  clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history_->setSvgSize(18, 18);

  //// 选择 text/hex
  Ui::TtWidgetGroup* styleGroup = new Ui::TtWidgetGroup(this);
  styleGroup->setHoldingChecked(true);
  // 点击切换后
  // 时间戳消失, 渲染有问题
  Ui::TtTextButton* textBtn = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  // textBtn->setLightDefaultColor(QColor(0, 102, 180));
  // textBtn->setLightHoverColor(QColor(0, 112, 198));
  textBtn->setCheckedColor(QColor(0, 102, 180));
  Ui::TtTextButton* hexBtn = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
  // hexBtn->setLightDefaultColor(QColor(0, 102, 180));
  // hexBtn->setLightHoverColor(QColor(0, 112, 198));
  hexBtn->setCheckedColor(QColor(0, 102, 180));

  styleGroup->addWidget(textBtn);
  styleGroup->addWidget(hexBtn);
  styleGroup->setCheckedIndex(0);
  styleGroup->setExclusive(true);
  chose_function_layout->addWidget(textBtn);
  chose_function_layout->addWidget(hexBtn);
  textBtn->setChecked(true);
  connect(styleGroup, &Ui::TtWidgetGroup::widgetClicked,
          [this](const int& index) {
            setDisplayType(index == 1 ? MsgType::HEX : MsgType::TEXT);
          });

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history_);

  QSplitter* VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  VSplitter->setSizes(QList<int>() << 500 << 200);

  // 上方选择功能以及信息框
  QWidget* contentWidget = new QWidget;
  Ui::TtVerticalLayout* contentWidgetLayout =
      new Ui::TtVerticalLayout(contentWidget);

  QStackedWidget* messageStackedView = new QStackedWidget(contentWidget);

  terminal_ = new QPlainTextEdit(this);
  terminal_->setReadOnly(true);
  terminal_->setFrameStyle(QFrame::NoFrame);
  SerialHighlighter* lexer = new SerialHighlighter(terminal_->document());

  messageStackedView->addWidget(terminal_);

  message_view_ = new Ui::TtChatView(messageStackedView);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  messageStackedView->addWidget(message_view_);

  QSplitter* graphSpiltter = new QSplitter;

  serial_plot_ = new Ui::TtSerialPortPlot;
  graphSpiltter->addWidget(serial_plot_);

  QListWidget* serialDataList = new QListWidget(graphSpiltter);
  serialDataList->setContextMenuPolicy(
      Qt::CustomContextMenu);  // 启用自定义右键菜单
  connect(serialDataList, &QListWidget::customContextMenuRequested, this,
          [=](const QPoint& pos) {
            // 获取当前右键点击的项
            QListWidgetItem* item = serialDataList->itemAt(pos);

            // 创建菜单
            QMenu menu;
            QAction* addAction = menu.addAction(tr("添加"));

            QAction* editAction = nullptr;
            QAction* deleteAction = nullptr;
            QAction* renameAction = nullptr;

            if (item) {
              editAction = menu.addAction(tr("编辑"));
              deleteAction = menu.addAction(tr("删除"));
              renameAction = menu.addAction(tr("重命名"));
            }

            // 处理菜单点击
            QAction* selectedAction =
                menu.exec(serialDataList->viewport()->mapToGlobal(pos));
            if (!selectedAction) {
              // 点击 menu 的其他区域
              return;
            }
            if (selectedAction == addAction) {
              TtChannelButtonEditorDialog editorDialog(this);
              if (editorDialog.exec() == QDialog::Accepted) {
                QListWidgetItem* item = new QListWidgetItem(serialDataList);
                item->setSizeHint(QSize(78, 30));
                TtChannelButton* channelBtn = new TtChannelButton(
                    QColor(100, 100, 140), editorDialog.title(), this);
                channelBtn->setCheckBlockColor(editorDialog.checkBlockColor());
                channelBtn->setFocusPolicy(Qt::NoFocus);
                QUuid uuid = QUuid::createUuid();
                channelBtn->setUuid(uuid.toString());
                serialDataList->setItemWidget(item, channelBtn);
                // 添加解析规则
                addChannelInfo(uuid.toString(),
                               channelBtn->getCheckBlockColor(),
                               editorDialog.metaInfo());
                // 绑定选中符号
                connect(channelBtn, &TtChannelButton::toggled, this,
                        [this, channelBtn](bool check) {
                          // 设定是否使能
                          auto it = rules_.find(channelBtn->getUuid());
                          if (it != rules_.end()) {
                            it->enable = check;
                          }
                        });
              }
            } else if (item) {
              if (selectedAction == renameAction) {
                if (auto* btn = qobject_cast<TtChannelButton*>(
                        serialDataList->itemWidget(item))) {
                  btn->modifyText();
                }
              } else if (selectedAction == deleteAction) {
                auto* deleteItem = serialDataList->takeItem(
                    serialDataList->row(item));  // 删除项
                if (deleteItem) {
                  // 获取 button
                  if (auto* btn = qobject_cast<TtChannelButton*>(
                          serialDataList->itemWidget(deleteItem))) {
                    quint16 channel = channel_info_.value(btn->getUuid()).first;
                    serial_plot_->removeGraphs(channel);
                    lua_script_codes_.remove(channel);
                    // 移出规则
                    channel_info_.remove(btn->getUuid());
                    rules_.remove(btn->getUuid());
                  }
                  qDebug() << "delete";
                  // 是移出了, 但是 graph 没有
                  delete deleteItem;
                }

              } else if (selectedAction == editAction) {
                if (auto* btn = qobject_cast<TtChannelButton*>(
                        serialDataList->itemWidget(item))) {
                  TtChannelButtonEditorDialog editorDialog(btn, this);
                  editorDialog.setMetaInfo(
                      channel_info_.value(btn->getUuid()).second);

                  if (editorDialog.exec() == QDialog::Accepted) {
                    handleDialogData(btn->getUuid(),
                                     channel_info_.value(btn->getUuid()).first,
                                     editorDialog.metaInfo());
                  }
                }
              }
            }

          });

  serialDataList->setContentsMargins(QMargins());
  serialDataList->setSpacing(0);
  serialDataList->setStyleSheet(
      // 设置列表整体背景色
      "QListWidget {"
      "   background-color: white;"
      "   outline: none;"  // 移除焦点虚线框（关键）
      "}"
      // 项选中状态样式
      "QListWidget::item:selected {"
      "   background: transparent;"  // 强制透明背景
      "}"
      // 项悬停状态样式
      "QListWidget::item:hover {"
      "   background: transparent;"  // 防止悬停变色
      "}"
      // 项按下状态样式
      "QListWidget::item:pressed {"
      "   background: transparent;"  // 防止按压变色
      "}");

  // 启用该模式, 会导致有虚线框
  graphSpiltter->addWidget(serialDataList);
  graphSpiltter->setSizes(QList<int>{500, 100});
  graphSpiltter->setCollapsible(0, false);
  graphSpiltter->setCollapsible(1, false);
  graphSpiltter->setStretchFactor(0, 1);
  graphSpiltter->setStretchFactor(1, 0);

  messageStackedView->addWidget(graphSpiltter);

  contentWidgetLayout->addWidget(chose_function);
  contentWidgetLayout->addWidget(messageStackedView);

  connect(test_, &Ui::TtWidgetGroup::widgetClicked, this,
          [this, messageStackedView, textBtn, hexBtn](const int& idx) {
            messageStackedView->setCurrentIndex(idx);
            if (idx != 0) {
              // 图表
              textBtn->setVisible(false);
              hexBtn->setVisible(false);
            } else {
              textBtn->setVisible(true);
              hexBtn->setVisible(true);
            }
          });

  base::DetectRunningTime runtime;

  message_model_ = new Ui::TtChatMessageModel;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget* bottomAll = new QWidget;
  Ui::TtVerticalLayout* bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget* tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout* tacLayout = new Ui::TtHorizontalLayout();
  tabs_and_count->setLayout(tacLayout);

  auto m_tabs = new QtMaterialTabs(tabs_and_count);
  m_tabs->addTab(tr("手动"));
  m_tabs->addTab(tr("片段"));
  // m_tabs->setFixedHeight(30);
  m_tabs->setBackgroundColor(QColor(192, 120, 196));
  // m_tabs->setMinimumWidth(80);

  tacLayout->addWidget(m_tabs);
  tacLayout->addStretch();

  // 显示发送字节和接收字节数
  send_byte = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  send_byte->setFixedHeight(30);
  recv_byte = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  recv_byte->setFixedHeight(30);

  tacLayout->addWidget(send_byte);
  tacLayout->addWidget(recv_byte);

  QWidget* la_w = new QWidget(this);
  QStackedLayout* layout = new QStackedLayout(la_w);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QWidget* messageEdit = new QWidget(la_w);
  QVBoxLayout* messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 0, 3, 0);
  messageEditLayout->setSpacing(0);

  editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);
  editor->setFrameStyle(QFrame::NoFrame);

  messageEditLayout->addWidget(editor);

  QWidget* bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  Ui::TtHorizontalLayout* bottomBtnWidgetLayout =
      new Ui::TtHorizontalLayout(bottomBtnWidget);

  // 发送的格式
  chose_text_ = new Ui::TtRadioButton("TEXT", bottomBtnWidget);
  chose_hex_ = new Ui::TtRadioButton("HEX", bottomBtnWidget);
  chose_text_->setChecked(true);

  sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  sendBtn->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(chose_text_);
  bottomBtnWidgetLayout->addWidget(chose_hex_);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(sendBtn);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(la_w);

  layout->addWidget(messageEdit);
  layout->addWidget(instruction_table_);


  connect(instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
          qOverload<const QString&>(&SerialWindow::sendMessageToPort));

  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const QVector<QPair<QString, int>>& datas) {
            if (datas.isEmpty()) {
              return;
            }
            foreach (const auto& pair, datas) {
              sendMessageToPort(pair.first, pair.second);
            }
          });

  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, [this, layout](int index) {
    layout->setCurrentIndex(index);
  });

  // 显示, 并输入 lua 脚本
  // lua_code_ = new Ui::TtLuaInputBox(this);
  // lua_actuator_ = new Core::LuaKernel;

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(contentWidget);
  VSplitter->addWidget(bottomAll);

  // 左右分区
  mainSplitter->addWidget(VSplitter);
  mainSplitter->addWidget(serial_setting_);
  mainSplitter->setSizes(QList<int>() << 500 << 200);

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);

  // qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();

  send_package_timer_ = new QTimer(this);

  heartbeat_timer_ = new QTimer(this);
}

void SerialWindow::setSerialSetting() {
  serial_setting_->setSerialPortsName();
  serial_setting_->setSerialPortsBaudRate();
  serial_setting_->setSerialPortsDataBit();
  serial_setting_->setSerialPortsParityBit();
  serial_setting_->setSerialPortsStopBit();
  serial_setting_->setSerialPortsFluidControl();

  serial_setting_->displayDefaultSetting();
}

void SerialWindow::connectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    cfg_.obj.insert("WindowTitle", title_->text());
    cfg_.obj.insert("SerialSetting", serial_setting_->getSerialSetting());
    cfg_.obj.insert("InstructionTable", instruction_table_->getTableRecord());
    emit requestSaveConfig();
    Ui::TtMessageBar::success(TtMessageBarType::Top, "", tr("保存成功"), 1500,
                              this);
    unsaved_ = false;
    // 配置文件保存到文件中
    // 当前的 tabWidget 匹配对应的 QJsonObject
  });

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // // 检查是否处于打开状态
    // serial_port 已经移动到了 工作线程中
    // 将openSerialPort的调用通过Qt的信号槽机制排队到worker_thread_中执行，而不是直接在主线程调用
    if (serial_port_opened) {
      // 关闭串口时也需跨线程调用
      // QMetaObject::invokeMethod(serial_port_.get(), "closeSerialPort",
      //                           Qt::QueuedConnection);
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::QueuedConnection);
      // serial_port_->closeSerialPort();
      serial_port_opened = false;
      serial_setting_->setControlState(true);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
    } else {
      // 获取配置后通过 invokeMethod 调用
      Core::SerialPortConfiguration cfg =
          serial_setting_->getSerialPortConfiguration();
      QMetaObject::invokeMethod(serial_port_, "openSerialPort",
                                Qt::QueuedConnection,
                                Q_ARG(Core::SerialPortConfiguration, cfg));
      serial_port_opened = true;
      serial_setting_->setControlState(false);

      // 还得查看包的发送间隔, 为 0
      send_package_timer_->start();

      // 如果为 0, 停止
      heartbeat_timer_->start();
    }
  });

  connect(clear_history_, &Ui::TtSvgButton::clicked, [this]() {
    message_model_->clearModelData();
    terminal_->clear();
  });

  connect(sendBtn, &QtMaterialFlatButton::clicked, this,
          qOverload<>(&SerialWindow::sendMessageToPort));
  connect(serial_setting_, &Widget::SerialSetting::showScriptSetting,
          [this]() { lua_code_->show(); });

  connect(serial_setting_, &Widget::SerialSetting::sendPackageMaxSizeChanged,
          this, [this](const uint16_t size) {
            // 不设定的时候异常数字
            if (package_size_ != size) {
              package_size_ = size;
              qDebug() << "packageSize: " << package_size_;
            }
          });

  connect(serial_setting_, &Widget::SerialSetting::sendPackageIntervalChanged,
          this, [this](const uint32_t& interval) {
            qDebug() << "interval: " << interval;
            send_package_timer_->setInterval(interval);
          });

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      msg_queue_.clear();
      return;
    }

    if (!msg_queue_.isEmpty()) {
      auto package = msg_queue_.dequeue();
      send_byte_count += package.size();

      // 获取当前时间并格式化消息
      QDateTime now = QDateTime::currentDateTime();
      QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
      terminal_->appendPlainText(timestamp + " << ");  // 单独追加箭头行
      if (display_type_ == MsgType::HEX) {
        terminal_->appendPlainText(package.toUtf8().toHex(' ').toUpper());
      } else if (display_type_ == MsgType::TEXT) {
        terminal_->appendPlainText(package);  // 数据内容作为独立块
      }
      terminal_->appendPlainText("");  // 保留空行分隔

      auto tmp = new Ui::TtChatMessage();
      tmp->setContent(package);
      tmp->setRawData(package.toUtf8());
      tmp->setTimestamp(now);
      tmp->setOutgoing(true);
      tmp->setBubbleColor(QColor("#DCF8C6"));
      QList<Ui::TtChatMessage*> list;
      list.append(tmp);
      message_model_->appendMessages(list);

      // 串口发送
      QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                                Q_ARG(QString, package));

      send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
      message_view_->scrollToBottom();
    }
  });

  connect(heartbeat_timer_, &QTimer::timeout, this,
          // 适用于发送包间隔
          [this] { setHeartbeartContent(); });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatContentChanged,
          this, [this](const QString& content) {
            qDebug() << content;
            // 心跳的内容改变
            if (heartbeat_ != content) {
              heartbeat_ = content;
            }
          });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatInterval, this,
          [this](const uint32_t times) {
            qDebug() << times;
            // 如果为 0, 不生效
            if (heartbeat_interval_ != times) {
              heartbeat_interval_ = times;
              heartbeat_timer_->setInterval(times);
            }
          });
  connect(chose_hex_, &Ui::TtRadioButton::toggled, [this](bool checked) {
    if (checked) {
      send_type_ = MsgType::HEX;
    }
  });
  connect(chose_text_, &Ui::TtRadioButton::toggled, [this](bool checked) {
    if (checked) {
      send_type_ = MsgType::TEXT;
    }
  });
}

}  // namespace Window
