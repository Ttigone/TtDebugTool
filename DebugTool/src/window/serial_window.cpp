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

#include "storage/setting_manager.h"

#include "Def.h"

#include <QTime>

namespace Window {

SerialWindow::SerialWindow(QWidget *parent)
    : FrameWindow(parent), worker_thread_(new QThread(this)),
      serial_port_(new Core::SerialPortWorker) {

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

  // 初始化完成
  qDebug() << "SerialWindow 初始化完成: " << this;
}

SerialWindow::~SerialWindow() {
  qDebug() << "delete SerialWindow";
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
  // QMetaObject::invokeMethod(serial_port_, "deleteLater",
  // Qt::QueuedConnection);

  if (worker_thread_) {
    worker_thread_->quit();
    // 3. 等待线程完成(设置超时，避免无限等待)
    if (!worker_thread_->wait(200)) {
      qWarning()
          << "Worker thread did not exit gracefully, forcing termination";
      worker_thread_->terminate(); // 强制终止(不推荐，但作为最后手段)
      worker_thread_->wait();      // 等待强制终止完成
    }
    // 无效
    // delete serial_port_;
    // QMetaObject::invokeMethod(serial_port_, "deleteLater",
    //                           Qt::QueuedConnection);
  }
}

QString SerialWindow::getTitle() { return title_->text(); }

QJsonObject SerialWindow::getConfiguration() const {
  // qDebug() << "get obj";
  // return cfg_.obj;
  return config_;
}

void SerialWindow::saveWaveFormData() {
  // save to csv data
  if (serial_plot_) {
    qDebug() << "csv";
    serial_plot_->saveWaveFormData();
  }
}

bool SerialWindow::workState() const {
  // 如果打开的是异常串口, 同时又保存了, 有问题
  return serial_port_opened;
}

bool SerialWindow::saveState() {
  // 需要检测配置的区分, 还是改动一处, 标志位设置为 false
  return saved_;
}

void SerialWindow::setSaveState(bool state) { saved_ = state; }

void SerialWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitle", title_->text());
  config_.insert("SerialSetting", serial_setting_->getSerialSetting());
  config_.insert("InstructionTable", instruction_table_->getTableRecord());

  // 还有 btn plot 的配置
  // config_.insert("PlotSetting", QJsonValue(channel_info_));
  // 为 channel_info_ 创建一个自定义 JSON 对象

  QJsonObject channelInfoJson;
  for (auto it = channel_info_.constBegin(); it != channel_info_.constEnd();
       ++it) {
    // 遍历每一个 channel 信息
    QJsonObject channelData;
    // 是否会重复通道值
    // channelData["channel"] = static_cast<int>(it.value().first); //
    // 保存通道号
    channelData["channel"] = it.value().channel_num_; // 保存通道号

    // 将配置信息 QByteArray 转换为 Base64 字符串
    // QString base64Data = QString::fromLatin1(it.value().second.toBase64());
    QString base64Data = QString::fromLatin1(it.value().data.toBase64());
    channelData["data"] = base64Data;

    // 名字的方式存储
    channelData["color"] = it.value().color.name();

    // 使用 UUID 作为键
    channelInfoJson[it.key()] = channelData;
  }
  // channelInfo 下面有多个 uuid 为 key 的 jsonobject
  config_.insert("ChannelInfo", channelInfoJson);

  // rules 由 data 解析得来
  // // 添加 rules_ 相关配置（如果需要）
  // // 规则不是要了, 直接
  // QJsonObject rulesConfig;
  // for (auto it = rules_.constBegin(); it != rules_.constEnd(); ++it) {
  //   // 遍历存在的 rule
  //   QJsonObject ruleData;
  //   ruleData["enabled"] = it.value().enable;
  //   ruleData["channel"] = static_cast<int>(it.value().channel);
  //   ruleData["headerHex"] = QString::fromLatin1(it.value().header.toHex());
  //   ruleData["headerLen"] = it.value().header_len;
  //   ruleData["typeOffset"] = it.value().type_offset;
  //   ruleData["lenOffset"] = it.value().len_offset;

  //   rulesConfig[it.key()] = ruleData;
  // }
  // config_.insert("ChannelRules", rulesConfig);

  // 保存 lua 脚本代码
  QJsonObject luaScripts;
  for (auto it = lua_script_codes_.constBegin();
       it != lua_script_codes_.constEnd(); ++it) {
    luaScripts[QString::number(it.key())] = it.value();
  }
  config_.insert("LuaScripts", luaScripts);

  saved_ = true;
  // 支持保存到了 config_ 中,
  emit requestSaveConfig();
}

void SerialWindow::setSetting(const QJsonObject &config) {
  // 设置后
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  serial_setting_->setOldSettings(
      config.value("SerialSetting").toObject(QJsonObject()));

  // 读取 channel_info_
  if (config.contains("ChannelInfo")) {
    qDebug() << "channel info";
    QJsonObject channelInfoJson = config.value("ChannelInfo").toObject();
    for (auto it = channelInfoJson.constBegin();
         it != channelInfoJson.constEnd(); ++it) {
      QString uuid = it.key();
      QJsonObject data = it.value().toObject();

      quint16 channel = data["channel"].toInt();

      QByteArray binaryData =
          QByteArray::fromBase64(data["data"].toString().toLatin1());

      QColor color;
      if (data.contains("color")) {
        qDebug() << "contain";
        color = QColor(data["color"].toString("#2cccff"));
      }
      // qDebug() << "配置信息 color" << color;
      addChannel(binaryData, color, uuid);
    }
  }

  // 读取 Lua 脚本
  if (config.contains("LuaScripts")) {
    QJsonObject scripts = config.value("LuaScripts").toObject();
    for (auto it = scripts.constBegin(); it != scripts.constEnd(); ++it) {
      quint16 channel = it.key().toUInt();
      lua_script_codes_[channel] = it.value().toString();
    }
  }

  saved_ = true; // 初始时 saved_ 为真
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

bool SerialWindow::eventFilter(QObject *watched, QEvent *event) {
  return SerialWindow::eventFilter(watched, event);
}

void SerialWindow::switchToEditMode() {
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  // 预先获取之前的标题
  title_edit_->setText(title_->text());
  // 显示 edit 模式
  stack_->setCurrentWidget(edit_widget_);
  // 获取焦点
  title_edit_->setFocus(); // 自动聚焦输入框
}

void SerialWindow::switchToDisplayMode() {
  // QGraphicsOpacityEffect* effect = new
  // QGraphicsOpacityEffect(original_widget_);
  // original_widget_->setGraphicsEffect(effect);
  // QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  // anim->setDuration(300);
  // anim->setStartValue(0);
  // anim->setEndValue(1);
  // anim->start(QAbstractAnimation::DeleteWhenStopped);
  //  切换显示模式
  if (title_->text() != title_edit_->text()) {
    title_->setText(title_edit_->text());
    saved_ = false;
  }
  stack_->setCurrentWidget(original_widget_);
}

void SerialWindow::setDisplayType(MsgType type) {
  if (display_type_ != type) {
    display_type_ = type;
    refreshTerminalDisplay();
    saved_ = false;
  }
}

void SerialWindow::setHeartbeartContent() {
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    sendMessageToPort(heartbeat_);
  }
}

void SerialWindow::saveSerialLog() {
  // saveBtn->connect(saveBtn, &QPushButton::clicked, [this]() {
  //   QString fileName = QFileDialog::getSaveFileName(this, tr("保存日志"),
  //                                                   QDir::homePath() +
  //                                                   "/serial_log.txt",
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

    Ui::TtChatMessage *msg = qobject_cast<Ui::TtChatMessage *>(
        idx.data(Ui::TtChatMessageModel::MessageObjectRole).value<QObject *>());

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
    header.append(content);
    qDebug() << header;
    terminal_->appendPlainText(header);
  }

  terminal_->setUpdatesEnabled(true);
}

void SerialWindow::addChannel(const QByteArray &blob, const QColor &color,
                              const QString &uuid) {

  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  // QString title, header, headerLength, type, typeOffset, length,
  // lengthOffset,
  //     tail;
  // in >> title >> header >> headerLength >> type >> typeOffset >> length >>
  //     lengthOffset >> tail;

  QString title, header, headerLength, typeOffset, lengthOffset, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >> tail;

  QListWidgetItem *item = new QListWidgetItem(serialDataList);
  item->setSizeHint(QSize(78, 30));

  // color: 1. 生成的; 2. 历史的
  // title 1. 历史添加, blob 中解析
  // 获取的 color 有问题
  qDebug() << color;

  // 创建 button
  TtChannelButton *channelBtn =
      new TtChannelButton(QColor(100, 100, 140), title, this);
  channelBtn->setCheckBlockColor(color);

  // 都出来的颜色直接无效
  qDebug() << channelBtn->getCheckBlockColor();

  channelBtn->setFocusPolicy(Qt::NoFocus);
  if (uuid.isEmpty()) {
    qDebug() << "创建新的 uuid";
    uuid = QUuid::createUuid().toString();
  }
  channelBtn->setUuid(uuid);
  serialDataList->setItemWidget(item, channelBtn);

  // 颜色的添加

  // 添加到配置中

  // 添加解析规则
  // 添加到了 rules_ 中
  // 外部也可以提供 uuid

  qDebug() << "添加配置";
  // channel_info_[uuid] = qMakePair(channel_nums_, blob);
  // channel_info_[uuid]
  // channel_nums_, blob);

  quint16 channel = channel_nums_;
  // title 可以重复

  // 保存配置信息
  ChannelSetting setting{channel_nums_, title, blob, color};
  channel_info_[uuid] = setting;

  QString luaCode;
  in >> luaCode;

  if (!luaCode.isEmpty()) {
    // 存储 lua 代码
    lua_script_codes_[channel_nums_] = luaCode;
  }

  // 以上是解析代码

  // 无校验
  ParserRule rule = {false,
                     channel_nums_,                        // 通道
                     QByteArray::fromHex(header.toUtf8()), // 头部
                     headerLength.toInt(),                 // 帧头长度
                     typeOffset.toInt(),                   // 类型字段偏移
                     lengthOffset.toInt(),                 // 长度字段偏移
                     0,                                    // 帧尾长度
                     false,                                // 无校验
                     {},
                     [this, channel](uint8_t type, const QByteArray &payload,
                                     const QString &luaCode) {
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

                       qDebug() << payload.size();

                       // 原始数据保存在 payload 中
                       // 根据 payload.size() 决定个数长度
                       // // 十六进制
                       // // 小端序解析 (低字节在前)
                       size_t len = payload.size();
                       quint16 value = 0;
                       for (int i = 0; i < len; ++i) {
                         value += static_cast<quint8>(payload[i]);
                       }
                       // quint16 value = (static_cast<quint8>(payload[1]) << 8)
                       // |
                       //                 static_cast<quint8>(payload[0]);

                       // 只在一个窗口添加
                       // qDebug() << "add Data Window" << this;
                       // 一旦分开窗口, 就会出现问题
                       serial_plot_->addData(channel, value);

                       // // 或大端序解析 (高字节在前)
                       // quint16 valueBeEndian =
                       //     (static_cast<quint8>(data[0]) << 8) |
                       //     static_cast<quint8>(data[1]);

                       qDebug() << "解析的整数值:" << value;
                     }};

  // 应用颜色
  serial_plot_->addGraphs(channel, color);
  // 修改则覆盖原有的值
  rules_.insert(uuid, rule);
  channel_nums_++;

  // 绑定选中符号
  connect(channelBtn, &TtChannelButton::toggled, this,
          [this, channelBtn](bool check) {
            // 失效勾选, 删除图像, 并删除原有消息
            // 设定是否使能
            auto it = rules_.find(channelBtn->getUuid());
            if (it != rules_.end()) {
              it->enable = check;
            }
          });
} // namespace Window

void SerialWindow::handleDialogData(const QString &uuid, quint16 channel,
                                    const QByteArray &blob,
                                    const QColor &color) {
  // title 可以重复
  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  // QString title, header, headerLength, type, typeOffset, length,
  // lengthOffset,
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

  // // 每个 uuid 的对应配置数组
  // channel_info_[uuid] = qMakePair(channel, blob);
  // color 从哪里来 ?
  // 修改后的
  qDebug() << "edit color" << color;
  ChannelSetting setting{channel_nums_, title, blob, color};
  channel_info_[uuid] = setting;

  // qDebug() << title << header << headerLength << typeOffset << lengthOffset
  //          << length << tail;

  auto it = rules_.find(uuid);
  if (it != rules_.end()) {
    // 修改配置
    ParserRule &rule = it.value();
    rule.header = QByteArray::fromHex(header.toUtf8());

    if (rule.header.isEmpty()) {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "",
                                    tr("输入的不是合法的十六进制字符串!"), 1500,
                                    this);
      return;
    }

    rule.header_len = headerLength.toInt();
    rule.type_offset = typeOffset.toInt();
    rule.len_offset = lengthOffset.toInt();
  } else {
    Ui::TtMessageBar::information(TtMessageBarType::Top, "",
                                  tr("不存在当前配置, 请删除配置选项"), 1500,
                                  this);
    return;
  }
}

void SerialWindow::showMessage(const QByteArray &data, bool out) {

  if (out) {
    send_byte_count_ += data.size();
  } else {
    recv_byte_count_ += data.size();
  }

  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  QString formattedMessage;

  if (out) {
    formattedMessage += "[Tx]";
  } else {
    formattedMessage += "[Rx]";
  }

  formattedMessage += ' ';
  formattedMessage.append(timestamp);
  formattedMessage += ' ';

  // 出现问题
  if (display_type_ == MsgType::TEXT) {
    // 直接追加文本
    formattedMessage.append(data);
  } else if (display_type_ == MsgType::HEX) {
    // 转换成 utf8 -> hex
    formattedMessage.append(data.toHex(' ').toUpper().trimmed());
  }
  terminal_->appendPlainText(formattedMessage);

  auto *msg = new Ui::TtChatMessage();
  msg->setContent(data);
  msg->setRawData(data);
  msg->setTimestamp(now);
  if (out) {
    msg->setOutgoing(true);
  } else {
    msg->setOutgoing(false);
  }
  msg->setBubbleColor(QColor("#DCF8C6"));
  QList<Ui::TtChatMessage *> list;
  list.append(msg);
  message_model_->appendMessages(list);
  message_view_->scrollToBottom();
}

void SerialWindow::showMessage(const QString &data, bool out) {

  // false 返回 ?
  // 文本 1234
  qDebug() << out << data;
  QByteArray dataUtf8 = data.toUtf8();

  // qDebug() << out << dataUtf8;

  if (out) {
    send_byte_count_ += data.size();
  } else {
    recv_byte_count_ += data.size();
  }

  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  QString formattedMessage;
  if (out) {
    formattedMessage += "[Tx]";
  } else {
    formattedMessage += "[Rx]";
  }
  formattedMessage += ' ';
  formattedMessage.append(timestamp);
  formattedMessage += ' ';

  // 出现问题
  if (display_type_ == MsgType::TEXT) {
    // TEXT 格式, data 保持, 有问题
    formattedMessage.append(data);
  } else if (display_type_ == MsgType::HEX) {
    // 这里
    // 发送端
    qDebug() << "hear" << dataUtf8.toHex(' ').toUpper().trimmed();
    formattedMessage.append(dataUtf8.toHex(' ').toUpper().trimmed());
  }
  terminal_->appendPlainText(formattedMessage);

  auto *msg = new Ui::TtChatMessage();
  msg->setContent(data);
  msg->setRawData(data.toUtf8());
  msg->setTimestamp(now);
  if (out) {
    msg->setOutgoing(true);
  } else {
    msg->setOutgoing(false);
  }
  msg->setBubbleColor(QColor("#DCF8C6"));
  QList<Ui::TtChatMessage *> list;
  list.append(msg);
  message_model_->appendMessages(list);
  if (out) {
    send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
  } else {
  }
  message_view_->scrollToBottom();
}

void SerialWindow::sendMessage(const QString &data, MsgType type) {

  QByteArray dataUtf8;

  // 判断发送的类型
  if (type == MsgType::HEX) {
    QString hexStr = data;
    hexStr = hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
    // hex
    if (hexStr.length() % 2 != 0) {
      // qDebug() << "HEX 字符串长度不是偶数，可能出错";
      Ui::TtMessageBar::warning(TtMessageBarType::Top, tr(""),
                                tr("HEX 字符串长度不是偶数，可能出错"), 1000,
                                this);
      return;
    }
    dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
    // qDebug() << "dataUtf8" << data; // 原始数据
  } else {
    dataUtf8 = data.toUtf8();
  }
  // 将dataUtf8 发送出去

  if (package_size_ > 0) {
    // 交给 sendPackageTimer 处理
    for (int i = 0; i < data.size(); i += package_size_) {
      // 循环塞入, 但是这边塞入很快, 那边是 计时读取
      qDebug() << "enqueue package";
      // 心跳 timeout 会从 msg_queuq_ 中取出内容
      // 存储的是 QString ?
      // 确实入队了
      msg_queue_.enqueue(data.mid(i, package_size_));
    }
  } else {
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    qDebug() << "no package";
    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, dataUtf8));

    // 显示上问题
    // showMessage(data);
    showMessage(dataUtf8);
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
  //   // const int MIN_FRAME_SIZE = 2 /*hdr*/ + 1 /*type*/ + 1 /*len*/ + 1
  //   /*cs*/; const int MIN_FRAME_SIZE = 2 /*hdr*/ + 1 /*type*/ + 1 /*len*/;

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
  //     // for (int i = 2; i < 2 + 1 + 1 + len; ++i)  // 从 type 开始到 payload
  //     末尾
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
    ParserRule const *bestRule = nullptr; // 解析规则
    // 遍历当前存在的规则
    for (auto &rule : rules_) {
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
    // 长度没问题
    qDebug() << "size: " << receive_buffer_.size();
    // qDebug() << bestRule->header_len + bestRule->len_offset +
    //                 bestRule->len_bytes;
    // 剩余最小长度 >= 头部长度
    // if (receive_buffer_.size() <
    //     bestRule->header_len + bestRule->type_offset + bestRule->len_offset +
    //     bestRule->len_bytes) {
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

    // 长度是 1
    // 获取长度字段
    payloadLen = receive_buffer_[bestRule->len_offset];

    // 长度为 3， 负载为 2
    qDebug() << bestRule->len_offset << payloadLen;

    // 1 + 1 + 1 + 0 + 0 = 3 + 1

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

void SerialWindow::showErrorMessage(const QString &text) {
  Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500, this);
  on_off_btn_->setChecked(false);
  serial_port_opened = false;
  serial_setting_->setControlState(true);
  send_package_timer_->stop();
  heartbeat_timer_->stop();
}

void SerialWindow::dataReceived(const QByteArray &data) {
  qDebug() << "data: " << data;

  // 添加是正确的
  receive_buffer_.append(data);
  // qDebug() << "rece_buf"
  // 解析数据帧
  parseBuffer();

  // // 转换失败有问题
  // auto str = QString::fromUtf8(data);
  // auto hexstr = QByteArray::fromHex(str.toUtf8());
  // qDebug() << "hex: " << hexstr;

  showMessage(data);

  // 上面只对 receive_buffer_ 处理

  // serial_plot_->saveWaveFormData();

  // 导出 csv 格式. 在 menu 中实现

  // 此处有 bug, 传入的是 16 进制的格式时
  // showMessage(data);
  // 转换 utf8, 有转换 hex
  // showMessage(QString::fromUtf8(data), false);

  // QString content;

  // QDateTime now = QDateTime::currentDateTime();
  // QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  // recv_byte_count += data.size();
  // auto tmp = new Ui::TtChatMessage();
  // tmp->setContent(data);
  // tmp->setRawData(data);
  // tmp->setOutgoing(false);
  // tmp->setBubbleColor(QColor("#0ea5e9"));
  // tmp->setTimestamp(now);
  // QList<Ui::TtChatMessage *> list;
  // list.append(tmp);
  // message_model_->appendMessages(list);

  // content += "[Rx]";
  // content += ' ';
  // content.append(timestamp);
  // content += ' ';
  // // terminal_->appendPlainText(timestamp + "[Rx]");  // 单独追加箭头行

  // if (display_type_ == MsgType::HEX) {
  //   // terminal_->appendPlainText(data.toHex(' ').toUpper());
  //   content.append(data.toHex(' ').toUpper());
  // } else if (display_type_ == MsgType::TEXT) {
  //   // terminal_->appendPlainText(data);  // 数据内容作为独立块
  //   content.append(data);
  // }
  // qDebug() << "Rx: " << content;

  // terminal_->appendPlainText(content);

  // recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  // message_view_->scrollToBottom();
}

void SerialWindow::sendMessageToPort() {
  // 一开始是 0
  // 发送包间隔可以没有, 但是心跳必须得有间隔
  // 点击发送按钮, 消息获取从 editor
  // 判断当前的发送类型
  // TEXT 发送, 接收正确
  // HEX 发送, 接收正确, 但是显示错误, TEXT 应该显示 error, 切换到  HEX
  // 则显示正确 HEX 显示上有问题

  // 从 editor 获取内容
  QString editorText = editor->text();
  if (editor->text().isEmpty()) {
    qDebug() << "empty";
    return;
  }
  // radio button 切换发送类型
  // 适用于 分包机制
  sendMessage(editorText, send_type_);
}

void SerialWindow::sendMessageToPort(const QString &data) {
  // 原有的
  // 心跳不需要添加
  // 平均有 10ms 的延时
  // 如果有分包, 需要应用分包, 否则直接发送
  // if (package_size_ > 0) {
  //   // 通过 packetTimer 处理
  //   for (int i = 0; i < data.size(); i += package_size_) {
  //     msg_queue_.enqueue(data.mid(i, package_size_));
  //   }
  // } else {
  //   // 心跳结束, 立马发送
  //   sendMessageToPort(data, 0);
  // }
  sendMessage(data);
}

void SerialWindow::sendMessageToPort(const QString &data, const int &times) {
  // 定时发送
  QTimer::singleShot(times, this, [this, data]() {
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }

    // send_byte_count += data.size();

    // QDateTime now = QDateTime::currentDateTime();
    // QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    // QString formattedMessage;
    // formattedMessage += "[Tx]";
    // formattedMessage += ' ';
    // formattedMessage.append(timestamp);
    // formattedMessage += ' ';

    // if (display_type_ == MsgType::TEXT) {
    //   // formattedMessage.append(QString::fromUtf8(data));
    //   formattedMessage.append(data);
    // } else if (display_type_ == MsgType::HEX) {
    //   formattedMessage.append(data.toUtf8().toHex(' ').toUpper().trimmed());
    // }
    // terminal_->appendPlainText(formattedMessage);

    // auto msg = new Ui::TtChatMessage();
    // msg->setContent(data);
    // msg->setRawData(data.toUtf8());
    // msg->setTimestamp(now);
    // msg->setOutgoing(true);
    // msg->setBubbleColor(QColor("#DCF8C6"));
    // QList<Ui::TtChatMessage *> list;
    // list.append(msg);
    // message_model_->appendMessages(list);
    // // 串口发送
    // QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
    //                           Q_ARG(QString, data));
    // send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    // message_view_->scrollToBottom();

    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QString, data));
    showMessage(data);
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
  Ui::TtHorizontalLayout *tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout *edit_layout =
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

  Ui::TtHorizontalLayout *tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout *tmpAll = new Ui::TtHorizontalLayout;

  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout *tmpl2 = new Ui::TtHorizontalLayout;
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
  main_splitter_ = new QSplitter;
  main_splitter_->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget *chose_function = new QWidget;
  Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  // 切换
  QWidget *twoBtnForGroup = new QWidget(chose_function);
  QHBoxLayout *layouttest = new QHBoxLayout(twoBtnForGroup);
  Ui::TtSvgButton *terminalButton =
      new Ui::TtSvgButton(":/sys/terminal.svg", twoBtnForGroup);
  terminalButton->setSvgSize(18, 18);
  terminalButton->setColors(Qt::black, Qt::blue);

  // qDebug() << "terminal: " << terminalButton;
  // leftBtn->setEnableHoldToCheck(true);
  Ui::TtSvgButton *chatButton =
      new Ui::TtSvgButton(":/sys/chat.svg", twoBtnForGroup);
  chatButton->setSvgSize(18, 18);
  chatButton->setColors(Qt::black, Qt::blue);
  // qDebug() << "chat: " << chatButton;
  // rightBtn->setEnableHoldToCheck(true);

  Ui::TtSvgButton *graphBtn =
      new Ui::TtSvgButton(":/sys/graph-up.svg", twoBtnForGroup);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  // qDebug() << "graph: " << graphBtn;

  layouttest->addWidget(terminalButton);
  layouttest->addWidget(chatButton);
  layouttest->addWidget(graphBtn);

  // QAction *test = new QAction;
  // 图标颜色显示有问题
  // 互斥
  Ui::TtWidgetGroup *showStyle = new Ui::TtWidgetGroup(this);
  showStyle->setHoldingChecked(true);
  showStyle->addWidget(terminalButton);
  showStyle->addWidget(chatButton);
  showStyle->addWidget(graphBtn);
  showStyle->setExclusive(true);
  showStyle->setCheckedIndex(0);
  chose_function_layout->addWidget(twoBtnForGroup);
  chose_function_layout->addStretch();

  // 点击多次有 bug
  clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history_->setSvgSize(18, 18);

  //// 选择 text/hex
  Ui::TtWidgetGroup *styleGroup = new Ui::TtWidgetGroup(this);
  styleGroup->setHoldingChecked(true);
  // 点击切换后
  // 时间戳消失, 渲染有问题
  Ui::TtTextButton *textBtn = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  // textBtn->setLightDefaultColor(QColor(0, 102, 180));
  // textBtn->setLightHoverColor(QColor(0, 112, 198));
  textBtn->setCheckedColor(QColor(0, 102, 180));
  Ui::TtTextButton *hexBtn = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
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
          [this](const int &index) {
            setDisplayType(index == 1 ? MsgType::HEX : MsgType::TEXT);
          });

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history_);

  QSplitter *VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  VSplitter->setSizes(QList<int>() << 500 << 200);

  // 上方选择功能以及信息框
  QWidget *contentWidget = new QWidget;
  Ui::TtVerticalLayout *contentWidgetLayout =
      new Ui::TtVerticalLayout(contentWidget);

  QStackedWidget *messageStackedView = new QStackedWidget(contentWidget);

  terminal_ = new QPlainTextEdit(this);
  terminal_->setReadOnly(true);
  terminal_->setFrameStyle(QFrame::NoFrame);
  SerialHighlighter *lexer = new SerialHighlighter(terminal_->document());

  messageStackedView->addWidget(terminal_);

  message_view_ = new Ui::TtChatView(messageStackedView);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  messageStackedView->addWidget(message_view_);

  QSplitter *graphSpiltter = new QSplitter;

  // 为什么不同的窗口, 会实现同一份数据
  serial_plot_ = new Ui::TtSerialPortPlot;
  graphSpiltter->addWidget(serial_plot_);

  serialDataList = new QListWidget(graphSpiltter);
  serialDataList->setContextMenuPolicy(
      Qt::CustomContextMenu); // 启用自定义右键菜单
  connect(
      serialDataList, &QListWidget::customContextMenuRequested, this,
      [=](const QPoint &pos) {
        // 获取当前右键点击的项
        QListWidgetItem *item = serialDataList->itemAt(pos);

        // 创建菜单
        QMenu menu;
        QAction *addAction = menu.addAction(tr("添加"));

        QAction *editAction = nullptr;
        QAction *deleteAction = nullptr;
        QAction *renameAction = nullptr;

        if (item) {
          editAction = menu.addAction(tr("编辑"));
          deleteAction = menu.addAction(tr("删除"));
          renameAction = menu.addAction(tr("重命名"));
        }

        // 处理菜单点击
        QAction *selectedAction =
            menu.exec(serialDataList->viewport()->mapToGlobal(pos));
        if (!selectedAction) {
          // 点击 menu 的其他区域
          return;
        }
        if (selectedAction == addAction) {
          // 此处可以更改颜色
          TtChannelButtonEditorDialog editorDialog(this);
          if (editorDialog.exec() == QDialog::Accepted) {
            saved_ = false;
            // 颜色生成
            // 创建 button, 添加 channel 信息
            // 随机生成的颜色保存在 editorDialog 中
            // addChannel(channelBtn->getCheckBlockColor(),
            // editorDialog.metaInfo());
            addChannel(editorDialog.metaInfo(), editorDialog.checkBlockColor());
          }
        } else if (item) {
          if (selectedAction == renameAction) {
            if (auto *btn = qobject_cast<TtChannelButton *>(
                    serialDataList->itemWidget(item))) {
              saved_ = false;
              btn->modifyText();
            }
          } else if (selectedAction == deleteAction) {
            auto *btn = qobject_cast<TtChannelButton *>(
                serialDataList->itemWidget(item));
            if (btn) {
              // 删除
              saved_ = false;
              // quint16 channel = channel_info_.value(btn->getUuid()).first;
              quint16 channel =
                  channel_info_.value(btn->getUuid()).channel_num_;
              qDebug() << channel;
              serial_plot_->removeGraphs(channel);
              lua_script_codes_.remove(channel);
              // 移出规则
              channel_info_.remove(btn->getUuid());
              rules_.remove(btn->getUuid());
            }
            auto *deleteItem =
                serialDataList->takeItem(serialDataList->row(item)); // 删除项

            qDebug() << "delete";
            delete deleteItem;

          } else if (selectedAction == editAction) {
            if (auto *btn = qobject_cast<TtChannelButton *>(
                    serialDataList->itemWidget(item))) {
              saved_ = false;
              // 数据保存在 info 中
              // 根据存在的 btn
              // 编辑状态
              TtChannelButtonEditorDialog editorDialog(btn, this);
              editorDialog.setMetaInfo(
                  channel_info_.value(btn->getUuid()).data);
              if (editorDialog.exec() == QDialog::Accepted) {
                // 编辑成功
                qDebug() << editorDialog.checkBlockColor();
                handleDialogData(
                    btn->getUuid(),
                    channel_info_.value(btn->getUuid()).channel_num_,
                    editorDialog.metaInfo(), editorDialog.checkBlockColor());
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
      "   outline: none;" // 移除焦点虚线框（关键）
      "}"
      // 项选中状态样式
      "QListWidget::item:selected {"
      "   background: transparent;" // 强制透明背景
      "}"
      // 项悬停状态样式
      "QListWidget::item:hover {"
      "   background: transparent;" // 防止悬停变色
      "}"
      // 项按下状态样式
      "QListWidget::item:pressed {"
      "   background: transparent;" // 防止按压变色
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

  connect(showStyle, &Ui::TtWidgetGroup::widgetClicked, this,
          [this, messageStackedView, textBtn, hexBtn](const int &idx) {
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

  QWidget *bottomAll = new QWidget;
  Ui::TtVerticalLayout *bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget *tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout *tacLayout = new Ui::TtHorizontalLayout();
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
  send_byte_ = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  send_byte_->setFixedHeight(30);
  recv_byte_ = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  recv_byte_->setFixedHeight(30);

  tacLayout->addWidget(send_byte_);
  tacLayout->addWidget(recv_byte_);

  QWidget *la_w = new QWidget(this);
  QStackedLayout *layout = new QStackedLayout(la_w);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QWidget *messageEdit = new QWidget(la_w);
  QVBoxLayout *messageEditLayout = new QVBoxLayout;
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

  QWidget *bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  Ui::TtHorizontalLayout *bottomBtnWidgetLayout =
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
          qOverload<const QString &>(&SerialWindow::sendMessageToPort));

  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const QVector<QPair<QString, int>> &datas) {
            if (datas.isEmpty()) {
              return;
            }
            foreach (const auto &pair, datas) {
              sendMessageToPort(pair.first, pair.second);
            }
          });
  connect(instruction_table_, &Ui::TtTableWidget::rowsChanged, this,
          [this]() { saved_ = false; });

  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged,
          [this, layout](int index) { layout->setCurrentIndex(index); });

  // 显示, 并输入 lua 脚本
  // lua_code_ = new Ui::TtLuaInputBox(this);
  // lua_actuator_ = new Core::LuaKernel;

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(contentWidget);
  VSplitter->addWidget(bottomAll);

  main_splitter_->addWidget(VSplitter);
  serial_setting_ = new Widget::SerialSetting;
  main_splitter_->addWidget(serial_setting_);
  main_splitter_->setSizes(QList<int>() << 500 << 220);
  main_splitter_->setCollapsible(0, false);
  main_splitter_->setCollapsible(1, true);

  main_splitter_->setStretchFactor(0, 3);
  main_splitter_->setStretchFactor(1, 2);

  // 主界面是左右分隔
  main_layout_->addWidget(main_splitter_);

  send_package_timer_ = new QTimer(this);
  send_package_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  send_package_timer_->setInterval(0);
  heartbeat_timer_ = new QTimer(this);
  heartbeat_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  heartbeat_timer_->setInterval(0);
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
  connect(save_btn_, &Ui::TtSvgButton::clicked, this,
          &SerialWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // // 检查是否处于打开状态
    // serial_port 已经移动到了 工作线程中
    // 将openSerialPort的调用通过Qt的信号槽机制排队到worker_thread_中执行，而不是直接在主线程调用
    if (serial_port_opened) {
      qDebug() << "close";
      // 关闭串口时也需跨线程调用
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::QueuedConnection);
      // serial_port_->closeSerialPort();
      serial_port_opened = false;
      serial_setting_->setControlState(true);
      on_off_btn_->setChecked(false);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
    } else {
      // 获取配置后通过 invokeMethod 调用
      Core::SerialPortConfiguration cfg =
          serial_setting_->getSerialPortConfiguration();
      QMetaObject::invokeMethod(serial_port_, "openSerialPort",
                                Qt::QueuedConnection,
                                Q_ARG(Core::SerialPortConfiguration, cfg));
      qDebug() << "serialopen";
      serial_port_opened = true;
      serial_setting_->setControlState(false);
      on_off_btn_->setChecked(true);

      if (send_package_timer_->interval() != 0) {
        send_package_timer_->start();
      } else {
        send_package_timer_->stop();
      }

      if (heartbeat_timer_->interval() != 0) {
        heartbeat_timer_->start();
      } else {
        heartbeat_timer_->stop();
      }
    }
  });

  connect(clear_history_, &Ui::TtSvgButton::clicked, [this]() {
    // 多次点击出现问题
    message_model_->clearModelData();
    terminal_->clear();
    // or
    // QCoreApplication::processEvents();
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
          this, [this](const uint32_t &interval) {
            qDebug() << "interval: " << interval;
            send_package_timer_->setInterval(interval);
          });
  connect(serial_setting_, &Widget::SerialSetting::settingChanged, this,
          [this]() { saved_ = false; });
  connect(serial_setting_, &Widget::SerialSetting::drawerStateChanged, this,
          [this](bool state) {
            // drawer 打开与关闭
            // QList<int> sizes = main_splitter_->sizes();
            // qDebug() << sizes[1];
          });

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    // 此处发送显示 渲染有问题
    if (!serial_port_opened) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      msg_queue_.clear();
      return;
    }
    // 时长偏差有点大, 发送显示 bug
    if (!msg_queue_.isEmpty()) {
      // QString 类型
      QString package = msg_queue_.dequeue();
      send_byte_count_ += package.size();

      // 判断发送的类型
      QByteArray dataUtf8;
      if (send_type_ == MsgType::HEX) {
        QString hexStr = package;
        hexStr = hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
        // hex
        if (hexStr.length() % 2 != 0) {
          // qDebug() << "HEX 字符串长度不是偶数，可能出错";
          // TtMessageBarType::Warning(TtMessageBarType::Top, tr(""),
          //                           tr("HEX 字符串长度不是偶数，可能出错"),
          //                           this);
          Ui::TtMessageBar::warning(TtMessageBarType::Top, tr(""),
                                    tr("HEX 字符串长度不是偶数，可能出错"),
                                    1000, this);
          return;
        }
        dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
      } else {
        // 保持格式发送
        dataUtf8 = package.toUtf8();
      }
      // 十六进制的格式
      qDebug() << "send: " << dataUtf8;
      // // 显示
      // QString content;
      // // 获取当前时间并格式化消息
      // QDateTime now = QDateTime::currentDateTime();
      // QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
      // content += "[Tx]";
      // content += ' ';
      // content.append(timestamp);
      // content += ' ';
      // if (display_type_ == MsgType::TEXT) {
      //   // terminal_->appendPlainText(package.toUtf8().toHex(' ').toUpper());
      //   // content.append(QString::fromUtf8(package));
      //   content.append(package);
      // } else if (display_type_ == MsgType::HEX) {
      //   // terminal_->appendPlainText(package); // 数据内容作为独立块
      //   content.append(package.toUtf8().toHex(' ').toUpper().trimmed());
      // }
      // // terminal_->appendPlainText(""); // 保留空行分隔
      // terminal_->appendPlainText(content);

      // auto msg = new Ui::TtChatMessage();
      // msg->setContent(package);
      // msg->setRawData(package.toUtf8());
      // msg->setTimestamp(now);
      // msg->setOutgoing(true);
      // msg->setBubbleColor(QColor("#DCF8C6"));
      // QList<Ui::TtChatMessage *> list;
      // list.append(msg);
      // message_model_->appendMessages(list);

      // send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
      // message_view_->scrollToBottom();

      // 串口发送
      // QMetaObject::invokeMethod(serial_port_, "sendData",
      // Qt::QueuedConnection,
      //                           Q_ARG(QString, package));

      // 发送有问题
      // 这里变为了 QString 类型
      // bug
      // QMetaObject::invokeMethod(serial_port_, "sendData",
      // Qt::QueuedConnection,
      //                           Q_ARG(QString, dataUtf8));
      QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                                Q_ARG(QByteArray, dataUtf8));

      // 展示在 termimal 中
      showMessage(dataUtf8);
    }
  });

  connect(heartbeat_timer_, &QTimer::timeout, this, [this] {
    setHeartbeartContent(); // 适用于发送包间隔
  });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatContentChanged,
          this, [this](const QString &content) {
            qDebug() << content;
            // 心跳的内容改变
            if (heartbeat_ != content) {
              heartbeat_ = content;
              qDebug() << "GET contenr: " << heartbeat_;
            }
          });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatInterval, this,
          [this](const uint32_t times) {
            qDebug() << times;
            // 如果为 0, 不生效
            if (heartbeat_interval_ != times) {
              heartbeat_interval_ = times;
              qDebug() << heartbeat_interval_;
              heartbeat_timer_->setInterval(times);
              qDebug() << "GET interval" << heartbeat_timer_->interval();
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

} // namespace Window
