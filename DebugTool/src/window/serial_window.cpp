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

#include "ui/controls/TtLuaInputBox.h"
#include "ui/controls/TtTableView.h"
#include "widget/serial_setting.h"

#include "ui/controls/TtChannelButton.h"
#include "ui/controls/TtChannelButtonEditorDialog.h"
#include "ui/controls/TtSerialPortPlot.h"

#include "storage/setting_manager.h"

#include "Def.h"
#include "window/frame_window.h"

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
  message_model_->clearModelData();
  terminal_->clear();

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
  qDebug() << "delete serialwindow exit";
}

QString SerialWindow::getTitle() { return title_->text(); }

QJsonObject SerialWindow::getConfiguration() const { return config_; }

bool SerialWindow::saveWaveFormData(const QString &fileName) {
  // if (serial_plot_) {
  //   qDebug() << "csv";
  //   serial_plot_->saveWaveFormData();
  //   return true;
  // }
  if (!serial_plot_) {
    Ui::TtMessageBar::warning(TtMessageBarType::Top, tr("保存失败"),
                              tr("没有可用的波形数据"), 1500, this);
    return false;
  }
  QString actualFileName;

  if (fileName.isEmpty()) {
    // 如果没有提供文件名，弹出保存对话框让用户选择
    QString defaultName =
        getTitle().isEmpty() ? tr("未命名波形数据") : getTitle();
    defaultName.replace(QRegularExpression("[\\\\/:*?\"<>|]"),
                        "_"); // 移除文件名中的非法字符

    actualFileName = QFileDialog::getSaveFileName(
        this, tr("保存波形数据"), QDir::homePath() + "/" + defaultName + ".csv",
        tr("CSV文件 (*.csv);;所有文件 (*.*)"));

    if (actualFileName.isEmpty()) {
      // 用户取消操作
      return false;
    }
  } else {
    // 使用提供的文件名，确保有正确的扩展名
    actualFileName = fileName;
    if (!actualFileName.contains("/") && !actualFileName.contains("\\")) {
      // 如果只有文件名没有路径，添加默认路径
      actualFileName = QDir::homePath() + "/" + actualFileName;
    }

    // 确保有.csv扩展名
    if (!actualFileName.endsWith(".csv", Qt::CaseInsensitive)) {
      actualFileName += ".csv";
    }
  }

  // 调用绘图组件的保存函数
  bool success = serial_plot_->saveWaveFormData(actualFileName);

  if (success) {
    Ui::TtMessageBar::success(
        TtMessageBarType::Top, tr("保存成功"),
        tr("波形数据已保存到: %1").arg(QFileInfo(actualFileName).fileName()),
        1500, this);
  } else {
    Ui::TtMessageBar::error(TtMessageBarType::Top, tr("保存失败"),
                            tr("无法保存波形数据，请检查文件权限或磁盘空间"),
                            1500, this);
  }
  return success;
  // return false;
}

bool SerialWindow::workState() const {
  // 如果打开的是异常串口, 同时又保存了, 有问题
  return opened_;
}

bool SerialWindow::saveState() { return saved_; }

void SerialWindow::setSaveState(bool state) {
  // 设置保存
  // saved_ = state;
  setSaveStatus(state);
}

void SerialWindow::saveSetting() {
  // qDebug() << "Save Times";
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
    // 保存通道号
    channelData["channel"] = it.value().channel_num_; // 保存通道号

    // 将配置信息 QByteArray 转换为 Base64 字符串
    // QString base64Data = QString::fromLatin1(it.value().second.toBase64());
    QString base64Data = QString::fromLatin1(it.value().data.toBase64());
    channelData["data"] = base64Data;

    channelData["color"] = it.value().color.name();

    channelInfoJson[it.key()] = channelData;
  }
  config_.insert("ChannelInfo", channelInfoJson);

  QJsonObject luaScripts;
  for (auto it = lua_script_codes_.constBegin();
       it != lua_script_codes_.constEnd(); ++it) {
    luaScripts[QString::number(it.key())] = it.value();
  }
  config_.insert("LuaScripts", luaScripts);

  config_.insert("SendType", QJsonValue(int(send_type_)));
  config_.insert("DisplayType", QJsonValue(int(display_type_)));

  // 点击按钮触发一次信号
  // saved_ = true;
  setSaveStatus(true);
  // 提供给 MainWindow 使用
  emit requestSaveConfig();
}

void SerialWindow::setSetting(const QJsonObject &config) {
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  serial_setting_->setOldSettings(
      config.value("SerialSetting").toObject(QJsonObject()));

  QJsonObject instructionTableData =
      config.value("InstructionTable").toObject();
  instruction_table_->setupTable(instructionTableData);

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

  int sendType = config.value("SendType").toInt();
  send_type_ = static_cast<TtTextFormat::Type>(sendType);
  if (sendType == 1) {
    chose_text_btn_->setChecked(true);
  } else if (sendType == 2) {
    chose_hex_btn_->setChecked(true);
  } else if (sendType == 3) {
  }
  int displayType = config.value("DisplayType").toInt();
  display_type_ = static_cast<TtTextFormat::Type>(displayType);
  if (displayType == 1) {
    display_text_btn_->setChecked(true);
    display_hex_btn_->setChecked(false);
  } else if (displayType == 2) {
    display_text_btn_->setChecked(false);
    display_hex_btn_->setChecked(true);
  }
  setSaveStatus(true);
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

void SerialWindow::setSaveStatus(bool state) {
  FrameWindow::setSaveStatus(state);
}

bool SerialWindow::eventFilter(QObject *watched, QEvent *event) {
  return SerialWindow::eventFilter(watched, event);
}

void SerialWindow::setHeartbeartContent() {
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    QByteArray dataUtf8;
    if (heart_beat_type_ == TtTextFormat::TEXT) {
      dataUtf8 = heartbeat_.toUtf8();
    } else if (heart_beat_type_ == TtTextFormat::HEX) {
      QString hexStr = heartbeat_.remove(QRegularExpression("[^0-9A-Fa-f]"));

      if (hexStr.isEmpty()) {
        qDebug() << "存在无效的十六进制字符";
        return;
      }

      // 确保字节对齐
      if (hexStr.length() % 2 != 0) {
        for (int i = 0; i < hexStr.length(); i += 2) {
          if (i + 1 >= hexStr.length()) {
            hexStr.insert(i, '0');
          }
        }
      }
      // 将十六进制字符串转换为字节数组
      dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
    }

    if (!dataUtf8.isEmpty()) {
      // 作为心跳数据发送
      sendPackagedData(dataUtf8, true);
    }
  }
}

void SerialWindow::sendInstructionTableContent(const QString &text,
                                               TtTextFormat::Type type,
                                               uint32_t time) {
  QByteArray dataUtf8;
  if (type == TtTextFormat::TEXT) {
    dataUtf8 = text.toUtf8();
  } else if (type == TtTextFormat::HEX) {
    QString hexStr = QString(text);
    hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));

    if (hexStr.isEmpty()) {
      qDebug() << "存在无效的十六进制字符";
      return;
    }

    // 确保字节对齐
    if (hexStr.length() % 2 != 0) {
      for (int i = 0; i < hexStr.length(); i += 2) {
        if (i + 1 >= hexStr.length()) {
          hexStr.insert(i, '0');
        }
      }
    }
    // 将十六进制字符串转换为字节数组
    dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
  }
  if (!dataUtf8.isEmpty()) {
    // 定时发送效果, 单次触发
    QTimer::singleShot(time, Qt::PreciseTimer, this,
                       [this, dataUtf8] { sendPackagedData(dataUtf8, true); });
  }
}

void SerialWindow::sendInstructionTableContent(const Data::MsgInfo &msg) {
  sendInstructionTableContent(msg.text, msg.type, msg.time);
}

void SerialWindow::saveSerialLog() {
  // 保存串口日志
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

void SerialWindow::setControlState(bool state) {
  if (state) {
    // 启用控件
    serial_setting_->setControlState(true);
    instruction_table_->setEnabled(true);
    send_btn_->setEnabled(false);
  } else {
    // 禁用控件
    serial_setting_->setControlState(false);
    instruction_table_->setEnabled(false);
    send_btn_->setEnabled(true);
  }
}

void SerialWindow::addChannel(const QByteArray &blob, const QColor &color,
                              const QString &uuid) {

  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  QString title, header, headerLength, typeOffset, lengthOffset, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >> tail;

  QListWidgetItem *item = new QListWidgetItem(serialDataList);
  item->setSizeHint(QSize(78, 30));

  // qDebug() << color;

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

  qDebug() << "添加配置";

  quint16 channel = channel_nums_;
  ChannelSetting setting{channel_nums_, title, blob, color};
  channel_info_[uuid] = setting;
  qDebug() << "uuid: channel info: " << uuid
           << channel_info_.value(uuid).channel_num_;

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

                       // 原始数据保存在 payload 中
                       // 根据 payload.size() 决定个数长度
                       // // 十六进制
                       // // 小端序解析 (低字节在前)
                       size_t len = payload.size();
                       quint16 value = 0;
                       //  for (int i = 0; i < len; ++i) {
                       //    value += static_cast<quint8>(payload[i]);
                       //  }

                       // 为什么 "\x00\x01" 解析的值是 256 呢???
                       if (len >= 2) {
                         value = (static_cast<quint8>(payload[1]) << 8) |
                                 static_cast<quint8>(payload[0]);
                       } else if (len == 1) {
                         value = static_cast<quint8>(payload[0]);
                       }
                       qDebug() << "解析的整数值:" << value;

                       //  // 或者按大端序解析 (高字节在前)
                       //  quint16 valueBeEndian = 0;
                       //  if (payload.size() >= 2) {
                       //    valueBeEndian =
                       //        (static_cast<quint8>(payload[0]) << 8) |
                       //        static_cast<quint8>(payload[1]);
                       //  } else if (payload.size() == 1) {
                       //    valueBeEndian = static_cast<quint8>(payload[0]);
                       //  }
                       //  qDebug() << "大端序解析的整数值:" << valueBeEndian;

                       Core::LuaKernel kernel;
                       //  qDebug() << luaCode;
                       if (!luaCode.isEmpty()) {
                         QVariantList params;
                         //  params << payload;
                         params << value;
                         double ret;
                         // 执行 lua 代码
                         // 参数列表被 Lua 识别成了表

                         if (kernel.doLuaCode(luaCode, params, ret)) {
                           // 执行成功
                           serial_plot_->addData(channel, ret);
                         } else {
                           serial_plot_->addData(channel, value);
                         }
                         qDebug() << "ret: " << ret;
                       } else {
                         serial_plot_->addData(channel, value);
                       }
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

  // 最后才会递增 1
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
  QDataStream in(blob);
  in.setVersion(QDataStream::Qt_6_4);

  QString title, header, headerLength, typeOffset, lengthOffset, tail;
  in >> title >> header >> headerLength >> typeOffset >> lengthOffset >> tail;

  QString luaCode;
  in >> luaCode;

  if (!luaCode.isEmpty()) {
    lua_script_codes_[channel] = luaCode;
  }

  ChannelSetting setting{channel, title, blob, color};
  channel_info_[uuid] = setting;

  qDebug() << "修改后的通道设置: uuid=" << uuid << ", channel=" << channel
           << ", title=" << title;

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

// void SerialWindow::showMessage(const QByteArray &data, bool out) {
//   QDateTime now = QDateTime::currentDateTime();
//   QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
//   QString formattedMessage;

//   if (out) {
//     formattedMessage += "[Tx]";
//   } else {
//     formattedMessage += "[Rx]";
//   }

//   formattedMessage += ' ';
//   formattedMessage.append(timestamp);
//   formattedMessage += ' ';

//   if (display_type_ == TtTextFormat::TEXT) {
//     formattedMessage.append(data);
//   } else if (display_type_ == TtTextFormat::HEX) {
//     formattedMessage.append(data.toHex(' ').toUpper().trimmed());
//   }
//   terminal_->appendPlainText(formattedMessage);

//   auto *msg = new Ui::TtChatMessage();
//   msg->setContent(data);
//   msg->setRawData(data);
//   msg->setTimestamp(now);
//   if (out) {
//     msg->setOutgoing(true);
//     send_byte_count_ += data.size();
//     send_byte_->setText(QString(tr("发送字节数:%1
//     B")).arg(send_byte_count_));
//   } else {
//     msg->setOutgoing(false);
//     recv_byte_count_ += data.size();
//     recv_byte_->setText(QString(tr("接收字节数:%1
//     B")).arg(recv_byte_count_));
//   }
//   msg->setBubbleColor(QColor("#DCF8C6"));
//   QList<Ui::TtChatMessage *> list;
//   list.append(msg);
//   message_model_->appendMessages(list);
//   message_view_->scrollToBottom();
// }

// void SerialWindow::showMessage(const QString &data, bool out) {
//   QByteArray dataUtf8 = data.toUtf8();
//   QDateTime now = QDateTime::currentDateTime();
//   QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
//   QString formattedMessage;
//   if (out) {
//     formattedMessage += "[Tx]";
//   } else {
//     formattedMessage += "[Rx]";
//   }
//   formattedMessage += ' ';
//   formattedMessage.append(timestamp);
//   formattedMessage += ' ';

//   if (display_type_ == TtTextFormat::TEXT) {
//     // TEXT 格式, data 保持, 有问题
//     formattedMessage.append(data);
//   } else if (display_type_ == TtTextFormat::HEX) {
//     qDebug() << "hear" << dataUtf8.toHex(' ').toUpper().trimmed();
//     formattedMessage.append(dataUtf8.toHex(' ').toUpper().trimmed());
//   }
//   terminal_->appendPlainText(formattedMessage);

//   auto *msg = new Ui::TtChatMessage();
//   msg->setContent(data);
//   msg->setRawData(data.toUtf8());
//   msg->setTimestamp(now);
//   if (out) {
//     msg->setOutgoing(true);
//     send_byte_count_ += data.size();
//     send_byte_->setText(QString(tr("发送字节数:%1
//     B")).arg(send_byte_count_));
//   } else {
//     msg->setOutgoing(false);
//     recv_byte_count_ += data.size();
//     recv_byte_->setText(QString(tr("接收字节数:%1
//     B")).arg(recv_byte_count_));
//   }
//   msg->setBubbleColor(QColor("#DCF8C6"));
//   QList<Ui::TtChatMessage *> list;
//   list.append(msg);
//   message_model_->appendMessages(list);
//   message_view_->scrollToBottom();
// }

void SerialWindow::sendMessage(const QString &data, TtTextFormat::Type type) {
  QByteArray dataUtf8;
  // 预处理的数据
  QString processedText = data;
  // 判断发送的类型
  if (type == TtTextFormat::HEX) {
    QString hexStr = data;
    // 移除所有非十六进制字符
    hexStr = hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (hexStr.length() % 2 != 0) {
      for (int i = 0; i < hexStr.length(); i += 2) {
        if (i + 1 >= hexStr.length()) {
          hexStr.insert(i, '0');
          qDebug() << "在位置" << i << "插入0，结果:" << hexStr;
        }
      }
    }
    // 存储处理后的文本用于分包
    processedText = hexStr;
    // 转换为字节数组
    dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());

  } else if (type == TtTextFormat::TEXT) {
    dataUtf8 = data.toUtf8();
  }
  if (package_size_ > 0) {
    if (type == TtTextFormat::HEX) {
      // 按照 16 进制分包
      int byteSize = package_size_;
      int charSize = byteSize * 2; // 每个字节转换为两个十六进制字符
      for (int i = 0; i < processedText.length(); i += charSize) {
        // 截取固定字符数的十六进制字符串片段
        QString chunk = processedText.mid(i, charSize);
        msg_queue_.enqueue(chunk);
        // qDebug() << "分包HEX: " << chunk;
      }
    } else {
      for (int i = 0; i < dataUtf8.size(); i += package_size_) {
        QByteArray chunk = dataUtf8.mid(i, package_size_);
        msg_queue_.enqueue(QString::fromUtf8(chunk));
        // qDebug() << "msg_queue size: " << msg_queue_.size();
        // qDebug() << "分包TEXT: " << QString::fromUtf8(chunk);
      }
    }
  } else {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, dataUtf8));

    showMessage(dataUtf8, true);
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
    // 长度是 6, 也没有问题
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

    // BUG, 获取的是 BB, 长度转换后是非常大的数据, 有问题

    // 获取长度字段
    // payloadLen = receive_buffer_[bestRule->len_offset];

    // 无符号强制转变
    payloadLen = static_cast<quint8>(receive_buffer_[bestRule->len_offset]);

    // 长度字段偏移是 1,

    // 长度为 3， 负载为 2
    qDebug() << "len_offset: " << bestRule->len_offset << "payloadLen"
             << payloadLen;

    // 2 1 1 的配置
    // 帧头给了 2
    // 类型字段与长度字段相同, 都是 BB, 转换成十进制

    const int maxAllowedPayloadLen = 1024; // 最大允许的负载长度
    if (payloadLen > maxAllowedPayloadLen) {
      qDebug() << "检测到异常长度字段值:" << payloadLen
               << "，可能是无效帧或协议错误";
      // 丢弃当前帧头，继续查找下一个帧
      receive_buffer_.remove(0, 1);
      continue;
    }

    const int fullSize = bestRule->header_len + 1 +
                         (bestRule->len_offset - bestRule->header_len) +
                         payloadLen + bestRule->tail_len +
                         (bestRule->has_checksum ? 1 : 0);

    if (fullSize <= 0 ||
        fullSize > receive_buffer_.size() + maxAllowedPayloadLen) {
      qWarning() << "计算的帧长度无效:" << fullSize << "，丢弃当前帧头";
      receive_buffer_.remove(0, 1);
      continue;
    }
    // 输出的是 负数
    // qDebug() << "size: " << fullSize;
    // if (fullSize < 0) {
    //   // 但是没有进入后续的情况了, 需要删除前面的数据吗???
    //   qWarning("截取的帧长度为负数, 提供了长度字段过大");
    //   return;
    // }
    // if (receive_buffer_.size() < fullSize) {
    //   return;
    // }

    // 切出一整帧
    QByteArray frame = receive_buffer_.left(fullSize);

    // 这个后面就出现了问题
    // 可选校验
    if (bestRule->has_checksum && !bestRule->validate(frame)) {
      if (!bestRule->validate(frame)) {
        qDebug() << "校验失败";
        // 校验失败，丢弃这个 header 字节，重试
        receive_buffer_.remove(0, 1);
        continue;
      }
    }

    // 解析帧有问题

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
    // 截取出现问题
    QByteArray payload = frame.mid(
        bestRule->header_len + 1 + bestRule->len_offset - bestRule->header_len,
        payloadLen);
    qDebug() << "rule 的 channel" << bestRule->channel;
    bestRule->processFrame(type, payload, lua_script_codes_[bestRule->channel]);

    // 调用自己的处理的这帧
    receive_buffer_.remove(0, fullSize);
  }
}

bool SerialWindow::isEnableHeartbeart() {
  // 使能了当前的条件
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    return true;
  }
  return false;
}

void SerialWindow::sendPackagedData(const QByteArray &data, bool isHeartbeat) {
  if (!opened_) {
    // 不会进入
    // 只有心跳 ???
    if (!isHeartbeat) {
      // 非心跳数据
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
    }
    return;
  }

  if (package_size_ > 0) {
    // 分包发送
    for (int i = 0; i < data.size(); i += package_size_) {
      QByteArray chunk = data.mid(i, package_size_);
      QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                                Q_ARG(QByteArray, chunk));
      // 更新统计
      send_byte_count_ += chunk.size();
      send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
      // 显示消息
      showMessage(chunk, true);
    }
  } else {
    // 不分包，直接发送
    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
    // 更新统计
    send_byte_count_ += data.size();
    send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
    // 显示消息
    showMessage(data, true);
  }
}

void SerialWindow::showErrorMessage(const QString &text) {
  Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500, this);
  on_off_btn_->setChecked(false);
  opened_ = false;
  // serial_setting_->setControlState(true);
  setControlState(!opened_);
  send_package_timer_->stop();
  heartbeat_timer_->stop();
  msg_queue_.clear();
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

  showMessage(data, false);

  // 上面只对 receive_buffer_ 处理

  // serial_plot_->saveWaveFormData();

  // 导出 csv 格式. 在 menu 中实现
}

void SerialWindow::sendMessageToPort() {
  if (!opened_) {
    Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                            1500, this);
    return;
  }
  // 使能状态
  // 从 editor_ 获取内容
  QString editorText = editor_->text();
  if (editor_->text().isEmpty()) {
    // qDebug() << "empty";
    return;
  }
  // 十六进制出现问题, 如果心跳是 HEX 模式, 使
  // radio button 切换发送类型
  // 适用于 分包机制
  sendMessage(editorText, send_type_);
}

void SerialWindow::sendMessageToPort(const QString &data) { sendMessage(data); }

void SerialWindow::sendMessageToPort(const QString &data, const int &times) {
  // 定时发送
  // 无分包发送
  QTimer::singleShot(times, this, [this, data]() {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                              Q_ARG(QString, data));
    showMessage(data);
  });
}

void SerialWindow::init() {
  initUi();
  title_->setText(tr("未命名的串口链接"));

  serial_setting_ = new Widget::SerialSetting;
  serRightWidget(serial_setting_);

  QSplitter *graphSpiltter = new QSplitter;
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
            // saved_ = false;
            setSaveStatus(false);
            addChannel(editorDialog.metaInfo(), editorDialog.checkBlockColor());
          }
        } else if (item) {
          if (selectedAction == renameAction) {
            if (auto *btn = qobject_cast<TtChannelButton *>(
                    serialDataList->itemWidget(item))) {
              // saved_ = false;
              setSaveStatus(false);
              btn->modifyText();
            }
          } else if (selectedAction == deleteAction) {
            auto *btn = qobject_cast<TtChannelButton *>(
                serialDataList->itemWidget(item));
            if (btn) {
              // 删除
              // saved_ = false;
              setSaveStatus(false);
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
                serialDataList->takeItem(serialDataList->row(item)); //
            // 删除项
            // qDebug() << "delete";
            delete deleteItem;

          } else if (selectedAction == editAction) {
            if (auto *btn = qobject_cast<TtChannelButton *>(
                    serialDataList->itemWidget(item))) {
              // saved_ = false;
              setSaveStatus(false);
              TtChannelButtonEditorDialog editorDialog(btn, this);
              // 设置编辑的数据
              editorDialog.setMetaInfo(
                  channel_info_.value(btn->getUuid()).data);
              if (editorDialog.exec() == QDialog::Accepted) {
                qDebug() << editorDialog.checkBlockColor();
                // qDebug() << "编辑时的(关乎于 button 的特定值) channel uuid: "
                // << btn->getUuid()
                // << channel_info_.value(btn->getUuid()).channel_num_;

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

  // messageStackedView->addWidget(graphSpiltter);

  Ui::TtSvgButton *graphBtn = new Ui::TtSvgButton(":/sys/graph-up.svg", this);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  addDisplayWidget(graphBtn, graphSpiltter);
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
  initSignalsConnection();

  connect(save_btn_, &Ui::TtSvgButton::clicked, this,
          &SerialWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // // 检查是否处于打开状态
    // serial_port 已经移动到了 工作线程中
    // 将openSerialPort的调用通过Qt的信号槽机制排队到worker_thread_中执行，而不是直接在主线程调用
    if (opened_) {
      qDebug() << "close";
      // 关闭串口时也需跨线程调用
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::QueuedConnection);
      // serial_port_->closeSerialPort();
      opened_ = false;
      on_off_btn_->setChecked(false);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
      // 清空待发送数组
      msg_queue_.clear();
    } else {
      // 获取配置后通过 invokeMethod 调用
      Core::SerialPortConfiguration cfg =
          serial_setting_->getSerialPortConfiguration();
      QMetaObject::invokeMethod(serial_port_, "openSerialPort",
                                Qt::QueuedConnection,
                                Q_ARG(Core::SerialPortConfiguration, cfg));
      qDebug() << "serialopen";
      opened_ = true;
      on_off_btn_->setChecked(true);
      // 发送包可以不需要间隔
      send_package_timer_->start();

      // 心跳需要间隔
      if (heartbeat_timer_->interval() != 0) {
        heartbeat_timer_->start();
      } else {
        heartbeat_timer_->stop();
      }
    }
    setControlState(!opened_);
  });

  connect(send_btn_, &QtMaterialFlatButton::clicked, this,
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
          [this]() { setSaveStatus(false); });
  connect(serial_setting_, &Widget::SerialSetting::drawerStateChanged, this,
          [this](bool state) {
            // drawer 打开与关闭
            // QList<int> sizes = main_splitter_->sizes();
            // qDebug() << sizes[1];
          });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatType, this,
          [this](TtTextFormat::Type type) {
            qDebug() << "heart_bear_type_" << heart_beat_type_;
            heart_beat_type_ = type;
          });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatContentChanged,
          this, [this](const QString &content) {
            qDebug() << content;
            if (heartbeat_ != content) {
              heartbeat_ = content;
              heartbeat_utf8_ = heartbeat_.toUtf8().toHex(' ');
            }
          });

  connect(serial_setting_, &Widget::SerialSetting::heartbeatInterval, this,
          [this](const uint32_t times) {
            qDebug() << times;
            if (heartbeat_interval_ != times) {
              heartbeat_interval_ = times;
              qDebug() << heartbeat_interval_;
              heartbeat_timer_->setInterval(times);
            }
          });

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      msg_queue_.clear();
      return;
    }
    // 取出数据
    if (!msg_queue_.isEmpty()) {
      // 不为空, 正确
      // 显示 0A
      qDebug() << "msg_queue_ : " << msg_queue_;
      QString package = msg_queue_.dequeue();
      send_byte_count_ += package.size();
      QByteArray dataUtf8;
      // 错误, 如果还有当前的根据表格发送的内容
      // 如果当前包
      bool isHexMode =
          (send_type_ == TtTextFormat::HEX) ||
          (heart_beat_type_ == TtTextFormat::HEX && isEnableHeartbeart());
      if (isHexMode) {
        // qDebug() << "this HEX MODE";
        QString hexStr = package.remove(QRegularExpression("[^0-9A-Fa-f]"));
        for (int i = 0; i < hexStr.length(); i += 2) {
          if (i + 1 >= hexStr.length()) {
            // 在未成对的位置前插入0
            hexStr.insert(i, '0');
          }
        }
        dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
      } else {
        qDebug() << "this TEXT MODE";
        dataUtf8 = package.toUtf8();
      }
      qDebug() << "get dataUtf8" << dataUtf8;
      // 发送并显示
      if (!dataUtf8.isEmpty()) {
        QMetaObject::invokeMethod(serial_port_, "sendData",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, dataUtf8));

        send_byte_count_ += dataUtf8.size();
        send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
        // 在UI中显示
        showMessage(dataUtf8, true);
      }
    }
  });

  connect(heartbeat_timer_, &QTimer::timeout, this, [this] {
    // 心跳定时器定时发送
    // qDebug() << "心跳中断";
    setHeartbeartContent(); // 适用于发送包间隔
  });

  connect(chose_hex_btn_, &Ui::TtRadioButton::toggled, this,
          [this](bool checked) {
            if (checked) {
              // 一直点击
              if (send_type_ != TtTextFormat::HEX) {
                send_type_ = TtTextFormat::HEX;
                setSaveStatus(false);
              }
            }
          });
  connect(chose_text_btn_, &Ui::TtRadioButton::toggled, this,
          [this](bool checked) {
            if (checked) {
              if (send_type_ != TtTextFormat::TEXT) {
                send_type_ = TtTextFormat::TEXT;
                setSaveStatus(false);
              }
            }
          });

  connect(instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
          qOverload<const QString &, TtTextFormat::Type, uint32_t>(
              &SerialWindow::sendInstructionTableContent));
  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const std::vector<Data::MsgInfo> &msgs) {
            // 群发消息
            // 没有延时的能够立马发送
            if (msgs.size() == 0) {
              return;
            }
            foreach (const auto &msg, msgs) {
              sendInstructionTableContent(msg);
            }
          });
}

} // namespace Window
