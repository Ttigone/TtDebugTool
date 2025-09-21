#include "window/serial_window.h"

#include <core/lua_kernel.h>
#include <lib/qtmaterialcheckable.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>
#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/controls/TtModbusPlot.h>
#include <ui/controls/TtQCustomPlot.h>
#include <ui/controls/TtTerminalLexer.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/widget_group.h>

#include <QTime>

#include "Def.h"
#include "storage/setting_manager.h"
#include "ui/controls/TtChannelButton.h"
#include "ui/controls/TtChannelButtonEditorDialog.h"
#include "ui/controls/TtLuaInputBox.h"
#include "ui/controls/TtSerialPortPlot.h"
#include "ui/controls/TtTableView.h"
#include "widget/serial_setting.h"
#include "window/frame_window.h"

namespace Window {

SerialWindow::SerialWindow(QWidget *parent)
    : FrameWindow(parent),
      worker_thread_(new QThread(this)),
      serial_port_(new Core::SerialPortWorker) {
  qDebug() << "SerialWindow::Init() 开始";
  Init();
  qDebug() << "测试1";
  ConnectSignals();
  qDebug() << "测试2";

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
  qDebug() << "SerialWindow 初始化完成: " << this;
}

// SerialWindow::~SerialWindow() {
//   // message_model_->clearModelData();
//   terminal_->clear();
//   if (worker_thread_) {
//     worker_thread_->quit();
//     // 3. 等待线程完成(设置超时，避免无限等待)
//     if (!worker_thread_->wait(200)) {
//       qWarning()
//           << "Worker thread did not exit gracefully, forcing termination";
//       worker_thread_->terminate();
//       worker_thread_->wait();
//     }
//   }
// }
SerialWindow::~SerialWindow() {
  qDebug() << "SerialWindow 开始析构: " << this;

  // 1. 断开所有信号连接
  disconnect();

  // 2. 停止所有定时器
  if (send_package_timer_) {
    send_package_timer_->stop();
    send_package_timer_->disconnect();
  }
  if (heartbeat_timer_) {
    heartbeat_timer_->stop();
    heartbeat_timer_->disconnect();
  }

  // 3. 优先清理表格控件
  if (instruction_table_) {
    instruction_table_->disconnect();
    // 让表格进入析构状态，避免控件回收
    instruction_table_->deleteLater();
    instruction_table_ = nullptr;
  }
  if (auto_replay_table_) {
    auto_replay_table_->disconnect();
    auto_replay_table_->deleteLater();
    auto_replay_table_ = nullptr;
  }

  // 4. 清理其他UI组件
  if (terminal_) {
    terminal_->clear();
    terminal_->disconnect();
  }

  // 5. 清理串口绘图
  if (serial_plot_) {
    serial_plot_->disconnect();
  }

  // 6. 关闭工作线程
  if (worker_thread_ && worker_thread_->isRunning()) {
    qDebug() << "正在关闭工作线程...";

    if (serial_port_) {
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::BlockingQueuedConnection);
    }

    worker_thread_->quit();
    if (!worker_thread_->wait(3000)) {
      qWarning() << "工作线程未正常退出，强制终止";
      worker_thread_->terminate();
      worker_thread_->wait(1000);
    }
    qDebug() << "工作线程已关闭";
  }

  // 7. 清理数据结构
  rules_.clear();
  channel_info_.clear();
  lua_script_codes_.clear();
  receive_buffer_.clear();
  msg_queue_.clear();

  qDebug() << "SerialWindow 析构完成: " << this;
}

QString SerialWindow::getTitle() { return title_->text(); }

QJsonObject SerialWindow::getConfiguration() const { return config_; }

bool SerialWindow::saveWaveFormData(const QString &fileName) {
  if (!serial_plot_) {
    Ui::TtMessageBar::warning(TtMessageBarType::Top, tr("保存失败"),
                              tr("没有可用的波形数据"), 1500, this);
    return false;
  }
  QString actualFileName;
  QString saveDir = Storage::SettingsManager::instance()
                        .getSetting("SaveDirectory")
                        .toString();
  if (saveDir.isEmpty()) {
    // 默认使用程序安装目录下的 Data 文件夹
    saveDir = QCoreApplication::applicationDirPath() + "/Data";
    // 创建路径目录
    QDir().mkpath(saveDir);
  }
  if (fileName.isEmpty()) {
    // 如果没有提供文件名，弹出保存对话框让用户选择
    QString defaultName =
        getTitle().isEmpty() ? tr("未命名波形数据") : getTitle();
    defaultName.replace(QRegularExpression("[\\\\/:*?\"<>|]"),
                        "_");  // 移除文件名中的非法字符

    actualFileName = QFileDialog::getSaveFileName(
        this, tr("保存波形数据"), QDir::homePath() + "/" + defaultName + ".csv",
        tr("CSV文件 (*.csv);;所有文件 (*.*)"));

    if (actualFileName.isEmpty()) {
      // 用户取消操作
      return false;
    }
  } else {
    if (QFileInfo(fileName).isRelative() && !fileName.contains("/") &&
        !fileName.contains("\\")) {
      // 相对路径，使用配置目录
      actualFileName = saveDir + "/" + fileName;
      // 确保有.csv扩展名
      if (!actualFileName.endsWith(".csv", Qt::CaseInsensitive)) {
        actualFileName += ".csv";
      }
    } else {
      // 绝对路径或包含路径分隔符，使用原路径
      actualFileName = fileName;
      if (!actualFileName.endsWith(".csv", Qt::CaseInsensitive)) {
        actualFileName += ".csv";
      }
    }
  }

  // 调用绘图组件的保存函数
  bool success = serial_plot_->saveWaveFormData(actualFileName);

  // 不需要了
  if (success) {
    // Ui::TtMessageBar::success(
    //     TtMessageBarType::Top, tr("保存成功"),
    //     tr("波形数据已保存到: %1").arg(QFileInfo(actualFileName).fileName()),
    //     1500, this);
  } else {
    // // BUG 这里又出现一次
    // Ui::TtMessageBar::error(TtMessageBarType::Top, tr("保存失败"),
    //                         tr("无法保存波形数据，请检查文件权限或磁盘空间"),
    //                         1500, this);
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
  config_.insert("InstructionTable", instruction_table_->GetTableRecord());

  // 还有 btn plot 的配置
  // config_.insert("PlotSetting", QJsonValue(channel_info_));
  // 为 channel_info_ 创建一个自定义 JSON 对象

  QJsonObject channelInfoJson;
  for (auto it = channel_info_.constBegin(); it != channel_info_.constEnd();
       ++it) {
    // 遍历每一个 channel 信息
    QJsonObject channelData;
    // 保存通道号
    channelData["channel"] = it.value().channel_num_;  // 保存通道号

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
  instruction_table_->SetupTable(instructionTableData);

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
  return FrameWindow::eventFilter(watched, event);
}

void SerialWindow::setHeartbeartContent() {
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    QByteArray dataUtf8;
    if (heart_beat_type_ == TtTextFormat::TEXT) {
      dataUtf8 = heartbeat_.toUtf8();
    } else if (heart_beat_type_ == TtTextFormat::HEX) {
      // QString hexStr = heartbeat_.remove(QRegularExpression("[^0-9A-Fa-f]"));
      QString hexStr = heartbeat_.remove(hexFilterRegex);
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
    // hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
    hexStr.remove(hexFilterRegex);

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

void SerialWindow::SaveSerialLog() {
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

void SerialWindow::SetControlState(bool state) {
  if (state) {
    // 启用控件
    serial_setting_->setControlState(true);
    instruction_table_->SetEnabled(true);
    send_btn_->setEnabled(false);
  } else {
    // 禁用控件
    serial_setting_->setControlState(false);
    instruction_table_->SetEnabled(false);
    send_btn_->setEnabled(true);
  }
}

void SerialWindow::addChannel(const QByteArray &blob, const QColor &color,
                              QString &uuid) {
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

  // // 无校验
  // ParserRule rule = {false,
  //                    channel_nums_,                        // 通道
  //                    QByteArray::fromHex(header.toUtf8()), // 头部
  //                    headerLength.toInt(),                 // 帧头长度
  //                    typeOffset.toInt(),                   // 类型字段偏移
  //                    lengthOffset.toInt(),                 // 长度字段偏移
  //                    0,                                    // 帧尾长度
  //                    false,                                // 无校验
  //                    {},
  //                    [this, channel](uint8_t type, const QByteArray &payload,
  //                                    const QString &luaCode) {
  //                      // qDebug() << "payload: " << payload;

  //                      // 长度为 x 个 hex 字节
  //                      // 解析的 payload 长度, 每个字节提供给 lua 使用
  //                      quint8 paramNums = payload.size();

  //                      // 原始数据保存在 payload 中
  //                      // 根据 payload.size() 决定个数长度
  //                      // // 十六进制
  //                      // // 小端序解析 (低字节在前)
  //                      size_t len = payload.size();
  //                      quint16 value = 0;

  //                      // 为什么 "\x00\x01" 解析的值是 256 呢???
  //                      if (len >= 2) {
  //                        value = (static_cast<quint8>(payload[1]) << 8) |
  //                                static_cast<quint8>(payload[0]);
  //                      } else if (len == 1) {
  //                        value = static_cast<quint8>(payload[0]);
  //                      }
  //                      // qDebug() << "解析的整数值:" << value;

  //                      //  // 或者按大端序解析 (高字节在前)
  //                      //  quint16 valueBeEndian = 0;
  //                      //  if (payload.size() >= 2) {
  //                      //    valueBeEndian =
  //                      //        (static_cast<quint8>(payload[0]) << 8) |
  //                      //        static_cast<quint8>(payload[1]);
  //                      //  } else if (payload.size() == 1) {
  //                      //    valueBeEndian = static_cast<quint8>(payload[0]);
  //                      //  }
  //                      //  qDebug() << "大端序解析的整数值:" <<
  //                      valueBeEndian;

  //                      Core::LuaKernel kernel;
  //                      //  qDebug() << luaCode;
  //                      if (!luaCode.isEmpty()) {
  //                        QVariantList params;
  //                        //  params << payload;
  //                        params << value;
  //                        double ret;
  //                        // 执行 lua 代码
  //                        // 参数列表被 Lua 识别成了表

  //                        if (kernel.doLuaCode(luaCode, params, ret)) {
  //                          // 执行成功
  //                          serial_plot_->addData(channel, ret);
  //                        } else {
  //                          serial_plot_->addData(channel, value);
  //                        }
  //                        // qDebug() << "ret: " << ret;
  //                      } else {
  //                        serial_plot_->addData(channel, value);
  //                      }
  //                      // // 或大端序解析 (高字节在前)
  //                      // quint16 valueBeEndian =
  //                      //     (static_cast<quint8>(data[0]) << 8) |
  //                      //     static_cast<quint8>(data[1]);

  //                      // qDebug() << "解析的整数值:" << value;
  //                    }};

  ParserRule rule = {false,
                     channel_nums_,                         // 通道
                     QByteArray::fromHex(header.toUtf8()),  // 头部
                     headerLength.toInt(),                  // 帧头长度
                     typeOffset.toInt(),    // 类型字段偏移
                     lengthOffset.toInt(),  // 长度字段偏移
                     0,                     // 帧尾长度
                     false,                 // 无校验
                     {},
                     [this, channel](uint8_t type, const QByteArray &payload,
                                     const QString &luaCode) {
                       // // BUG 解析确定, 判断是 那一种协议
                       // switch (protocol_) {
                       //   case TtProtocolSetting::RawData: {
                       //     // RawData 协议 - 直接解析原始数据
                       //     // 假设数据是以空格分隔的浮点数字符串
                       //     QString dataString = QString::fromUtf8(payload);
                       //     QStringList values = dataString.split(' ',
                       //     Qt::SkipEmptyParts);

                       //     for (int i = 0; i < values.size() && i <
                       //     MAX_CHANNELS; i++) {
                       //       bool ok;
                       //       double value = values[i].toDouble(&ok);
                       //       if (ok) {
                       //         serial_plot_->addData(channel + i, value);
                       //       }
                       //     }
                       //     return;  // 已处理，跳过后续代码
                       //     break;
                       //   }
                       //   case TtProtocolSetting::JustFloat: {
                       //     // JustFloat 协议 - 解析连续的 32 位浮点数
                       //     // 每个浮点数占 4 字节，结尾为 0x00, 0x00, 0x80,
                       //     0x7F
                       //     // 检查数据中是否包含完整的浮点数(至少4字节)
                       //     int floatCount = payload.size() / 4;

                       //     for (int i = 0; i < floatCount; i++) {
                       //       // 提取4字节并转换为浮点数
                       //       QByteArray floatBytes = payload.mid(i * 4, 4);

                       //       // 检查结束标记 (0x00, 0x00, 0x80, 0x7F)
                       //       if (i == floatCount - 1 &&
                       //           floatBytes == QByteArray("\x00\x00\x80\x7F",
                       //           4)) {
                       //         break;  // 跳过结束标记
                       //       }

                       //       // 使用 union 安全地转换字节到 float
                       //       union {
                       //         char bytes[4];
                       //         float value;
                       //       } converter;

                       //       // 复制字节 (考虑字节序)
                       //       for (int j = 0; j < 4; j++) {
                       //         converter.bytes[j] = floatBytes[j];
                       //       }

                       //       // 添加到图表
                       //       if (i < MAX_CHANNELS) {
                       //         serial_plot_->addData(channel + i,
                       //         converter.value);
                       //       }
                       //     }
                       //     return;  // 已处理，跳过后续代码
                       //     break;
                       //   }
                       //   case TtProtocolSetting::FireWater: {
                       //     // FireWater 协议 - 格式: [包头 + 通道 + 数据 +
                       //     校验和]
                       //     // 包头: 0x55, 0xAA
                       //     // 通道: 单字节通道号
                       //     // 数据: 通常为4字节浮点数
                       //     // 校验和: 1字节校验和

                       //     // 至少需要8字节 (2字节包头 + 1字节通道 +
                       //     4字节数据 + 1字节校验和) if (payload.size() < 8) {
                       //       qDebug() << "FireWater数据包过短:" <<
                       //       payload.toHex(); return;
                       //     }

                       //     // 验证包头 (0x55, 0xAA)
                       //     if ((uchar)payload[0] != 0x55 || (uchar)payload[1]
                       //     != 0xAA) {
                       //       qDebug() << "FireWater包头无效:" <<
                       //       payload.toHex(); return;
                       //     }

                       //     // 提取通道号
                       //     int channelNumber = (uchar)payload[2];

                       //     // 提取数据 (4字节浮点数)
                       //     QByteArray dataBytes = payload.mid(3, 4);

                       //     // 计算校验和 (对所有字节求和取低8位)
                       //     uchar calculatedChecksum = 0;
                       //     for (int i = 0; i < payload.size() - 1; i++) {
                       //       calculatedChecksum += (uchar)payload[i];
                       //     }

                       //     uchar receivedChecksum =
                       //     (uchar)payload[payload.size() - 1];

                       //     // 校验
                       //     if (calculatedChecksum != receivedChecksum) {
                       //       qDebug() << "FireWater校验和错误:"
                       //                << QString("计算值: 0x%1, 接收值:
                       //                0x%2")
                       //                       .arg(calculatedChecksum, 2, 16,
                       //                       QChar('0'))
                       //                       .arg(receivedChecksum, 2, 16,
                       //                       QChar('0'));
                       //       return;
                       //     }

                       //     // 转换数据为浮点数
                       //     union {
                       //       char bytes[4];
                       //       float value;
                       //     } converter;

                       //     // 复制字节 (考虑字节序)
                       //     for (int j = 0; j < 4; j++) {
                       //       converter.bytes[j] = dataBytes[j];
                       //     }

                       //     // 添加到图表
                       //     serial_plot_->addData(channelNumber,
                       //     converter.value); return;  // 已处理，跳过后续代码
                       //     break;
                       //   }
                       // }
                       // qDebug() << "payload: " << payload;
                       // 长度为 x 个 hex 字节
                       // 解析的 payload 长度, 每个字节提供给 lua 使用
                       quint8 paramNums = payload.size();

                       // 原始数据保存在 payload 中
                       // 根据 payload.size() 决定个数长度
                       // // 十六进制
                       // // 小端序解析 (低字节在前)
                       size_t len = payload.size();
                       quint16 value = 0;

                       // 为什么 "\x00\x01" 解析的值是 256 呢???
                       if (len >= 2) {
                         value = (static_cast<quint8>(payload[1]) << 8) |
                                 static_cast<quint8>(payload[0]);
                       } else if (len == 1) {
                         value = static_cast<quint8>(payload[0]);
                       }
                       // qDebug() << "解析的整数值:" << value;

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
                         // qDebug() << "ret: " << ret;
                       } else {
                         serial_plot_->addData(channel, value);
                       }
                       // // 或大端序解析 (高字节在前)
                       // quint16 valueBeEndian =
                       //     (static_cast<quint8>(data[0]) << 8) |
                       //     static_cast<quint8>(data[1]);

                       // qDebug() << "解析的整数值:" << value;
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
}  // namespace Window

void SerialWindow::HandleDialogData(const QString &uuid, quint16 channel,
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

  // qDebug() << "修改后的通道设置: uuid=" << uuid << ", channel=" << channel
  //          << ", title=" << title;

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

void SerialWindow::SendMessage(const QString &data, TtTextFormat::Type type) {
  QByteArray dataUtf8;
  // 预处理的数据
  QString processedText = data;
  // 判断发送的类型
  if (type == TtTextFormat::HEX) {
    QString hexStr = data;
    // 移除所有非十六进制字符
    hexStr = hexStr.remove(hexFilterRegex);
    if (hexStr.length() % 2 != 0) {
      for (int i = 0; i < hexStr.length(); i += 2) {
        if (i + 1 >= hexStr.length()) {
          hexStr.insert(i, '0');
          // qDebug() << "在位置" << i << "插入0，结果:" << hexStr;
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
      int charSize = byteSize * 2;  // 每个字节转换为两个十六进制字符
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

// void SerialWindow::parseBuffer() {
//   while (!receive_buffer_.isEmpty()) {
//     // 遍历所有解析规则，寻找最早出现的有效帧头
//     int bestPos = -1;
//     ParserRule const* bestRule = nullptr;  // 解析规则
//     // 遍历当前存在的规则
//     for (auto& rule : rules_) {
//       if (rule.enable) {
//         // 能够查找头部
//         qDebug() << "header: " << rule.header;
//         // qDebug() << "rece: " << receive_buffer_;
//         // 索引某个规则的头部, 谁先索引得到, 那就先解析
//         int pos = receive_buffer_.indexOf(rule.header);
//         if (pos >= 0 && (bestPos == -1 || pos < bestPos)) {
//           // 保存了当前索引头部字节的 地址
//           bestPos = pos;
//           bestRule = &rule;
//         }
//       }
//     }
//     // 没有匹配的帧头, 丢弃所有数据
//     if (!bestRule) {
//       receive_buffer_.clear();
//       return;
//     }
//     // 丢弃帧头前的无用字节
//     if (bestPos > 0) {
//       receive_buffer_.remove(0, bestPos);
//     }
//     // 长度没问题
//     // 长度是 6, 也没有问题
//     qDebug() << "size: " << receive_buffer_.size();
//     // qDebug() << bestRule->header_len + bestRule->len_offset +
//     //                 bestRule->len_bytes;
//     // 剩余最小长度 >= 头部长度
//     // if (receive_buffer_.size() <
//     //     bestRule->header_len + bestRule->type_offset +
//     bestRule->len_offset +
//     //     bestRule->len_bytes) {
//     //   return;
//     // }
//     // 1 + 1 + 2 +
//     // 缺少数据字段和校验字段, 等待
//     // 帧头长度与类型偏移总是一致的

//     if (receive_buffer_.size() <
//         bestRule->header_len + bestRule->type_offset + bestRule->len_offset)
//         {
//       return;
//     }

//     // 有问题 应当是 长度为 2
//     // 提取长度字段 (ex.多字节长度)
//     // len_offset 是 2
//     quint32 payloadLen = 0;

//     // BUG, 获取的是 BB, 长度转换后是非常大的数据, 有问题

//     // 获取长度字段
//     // payloadLen = receive_buffer_[bestRule->len_offset];

//     // 无符号强制转变
//     payloadLen = static_cast<quint8>(receive_buffer_[bestRule->len_offset]);

//     // 长度字段偏移是 1,

//     // 长度为 3， 负载为 2
//     qDebug() << "len_offset: " << bestRule->len_offset << "payloadLen"
//              << payloadLen;

//     // 2 1 1 的配置
//     // 帧头给了 2
//     // 类型字段与长度字段相同, 都是 BB, 转换成十进制

//     const int maxAllowedPayloadLen = 1024;  // 最大允许的负载长度
//     if (payloadLen > maxAllowedPayloadLen) {
//       qDebug() << "检测到异常长度字段值:" << payloadLen
//                << "，可能是无效帧或协议错误";
//       // 丢弃当前帧头，继续查找下一个帧
//       receive_buffer_.remove(0, 1);
//       continue;
//     }

//     const int fullSize = bestRule->header_len + 1 +
//                          (bestRule->len_offset - bestRule->header_len) +
//                          payloadLen + bestRule->tail_len +
//                          (bestRule->has_checksum ? 1 : 0);

//     if (fullSize <= 0 ||
//         fullSize > receive_buffer_.size() + maxAllowedPayloadLen) {
//       qWarning() << "计算的帧长度无效:" << fullSize << "，丢弃当前帧头";
//       receive_buffer_.remove(0, 1);
//       continue;
//     }
//     // 输出的是 负数
//     // qDebug() << "size: " << fullSize;
//     // if (fullSize < 0) {
//     //   // 但是没有进入后续的情况了, 需要删除前面的数据吗???
//     //   qWarning("截取的帧长度为负数, 提供了长度字段过大");
//     //   return;
//     // }
//     // if (receive_buffer_.size() < fullSize) {
//     //   return;
//     // }

//     // 切出一整帧
//     QByteArray frame = receive_buffer_.left(fullSize);

//     // 这个后面就出现了问题
//     // 可选校验
//     if (bestRule->has_checksum && !bestRule->validate(frame)) {
//       if (!bestRule->validate(frame)) {
//         qDebug() << "校验失败";
//         // 校验失败，丢弃这个 header 字节，重试
//         receive_buffer_.remove(0, 1);
//         continue;
//       }
//     }

//     // 解析帧有问题

//     qDebug() << "frame: " << frame;

//     // 调用各自的处理函数
//     quint8 type = static_cast<quint8>(frame.at(bestRule->type_offset));

//     // 数据字节
//     /**
//      * @brief payload
//      * 1 字节头部长度
//      * 1 字节类型长度
//      * 1 字节长度字段
//      * 剩余是 数据字段 + cs
//      */
//     // 截取出现问题
//     QByteArray payload = frame.mid(
//         bestRule->header_len + 1 + bestRule->len_offset -
//         bestRule->header_len, payloadLen);
//     qDebug() << "rule 的 channel" << bestRule->channel;
//     bestRule->processFrame(type, payload,
//     lua_script_codes_[bestRule->channel]);

//     // 调用自己的处理的这帧
//     receive_buffer_.remove(0, fullSize);
//   }
// }

void SerialWindow::parseBuffer() {
  // 检查是否使用VOFA协议
  if (protocol_ == TtProtocolSetting::RawData ||
      protocol_ == TtProtocolSetting::JustFloat ||
      protocol_ == TtProtocolSetting::FireWater) {
    parseVofaProtocol();
    return;
  }
  while (!receive_buffer_.isEmpty()) {
    // 遍历所有解析规则，寻找最早出现的有效帧头
    int bestPos = -1;
    ParserRule const *bestRule = nullptr;  // 解析规则
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

    const int maxAllowedPayloadLen = 1024;  // 最大允许的负载长度
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
    if (!isHeartbeat) {
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

void SerialWindow::parseVofaProtocol() {
  switch (protocol_) {
    case TtProtocolSetting::RawData:
      parseRawDataProtocol();
      break;
    case TtProtocolSetting::JustFloat:
      parseJustFloatProtocol();
      break;
    case TtProtocolSetting::FireWater:
      parseFireWaterProtocol();
      break;
  }
}

void SerialWindow::parseRawDataProtocol() {
  if (receive_buffer_.isEmpty()) return;

  // 直接显示原始数据
  showMessage(receive_buffer_, false);

  // 尝试将数据解析为空格分隔的数值（用于绘图）
  QString dataString = QString::fromUtf8(receive_buffer_);
  QStringList values = dataString.split(' ', Qt::SkipEmptyParts);

  for (int i = 0; i < values.size() && i < MAX_CHANNELS; i++) {
    bool ok;
    double value = values[i].toDouble(&ok);
    if (ok) {
      serial_plot_->addData(i, value);
    }
  }

  // 清空缓冲区
  receive_buffer_.clear();
}

void SerialWindow::parseJustFloatProtocol() {
  // JustFloat协议需要4字节一组解析浮点数
  // 确保有足够的数据
  while (receive_buffer_.size() >= 4) {
    // 检查是否为结束标记(0x00, 0x00, 0x80, 0x7F)
    if (receive_buffer_.size() >= 4 &&
        receive_buffer_.startsWith(QByteArray("\x00\x00\x80\x7F", 4))) {
      receive_buffer_.remove(0, 4);
      continue;
    }

    // 转换小端浮点数
    union {
      char bytes[4];
      float value;
    } converter;

    // 复制字节
    for (int j = 0; j < 4; j++) {
      converter.bytes[j] = receive_buffer_[j];
    }

    // 添加数据点到对应通道
    static int channelIndex = 0;
    serial_plot_->addData(channelIndex, converter.value);

    // 循环通道索引
    channelIndex = (channelIndex + 1) % MAX_CHANNELS;

    // 移除已处理的字节
    receive_buffer_.remove(0, 4);
  }
}

void SerialWindow::parseFireWaterProtocol() {
  // FireWater协议: 查找"$"开头和"\r\n"结尾的数据包
  int startPos = receive_buffer_.indexOf('$');
  if (startPos < 0) {
    return;  // 没有找到开始标记
  }

  int endPos = receive_buffer_.indexOf("\r\n", startPos);
  if (endPos < 0) {
    // 没有找到结束标记，等待更多数据
    if (receive_buffer_.size() > 10000) {
      // 防止缓冲区无限增长
      receive_buffer_.remove(0, startPos);
    }
    return;
  }

  // 提取数据包并分割CSV
  QByteArray packet = receive_buffer_.mid(startPos + 1, endPos - startPos - 1);
  QString dataStr = QString::fromUtf8(packet);
  QStringList values = dataStr.split(',', Qt::SkipEmptyParts);

  // 将数据添加到对应通道
  for (int i = 0; i < values.size() && i < MAX_CHANNELS; i++) {
    bool ok;
    double value = values[i].toDouble(&ok);
    if (ok) {
      serial_plot_->addData(i, value);
    }
  }

  // 显示接收到的消息
  showMessage(receive_buffer_.mid(startPos, endPos - startPos + 2), false);

  // 移除已处理的数据
  receive_buffer_.remove(0, endPos + 2);
}

// QByteArray
// SerialWindow::generateRandomTestData(TtProtocolSetting::Protocol protocol) {
//   QByteArray testData;
//   static QRandomGenerator random = QRandomGenerator::securelySeeded();

//   switch (protocol) {
//   case TtProtocolSetting::RawData: {
//     // 生成5个随机浮点数，以空格分隔
//     QString dataStr;
//     for (int i = 0; i < 5; i++) {
//       float value = random.bounded(-100.0f, 100.0f);
//       dataStr += QString::number(value, 'f', 2);
//       if (i < 4)
//         dataStr += " ";
//     }
//     testData = dataStr.toUtf8();
//     break;
//   }

//   case TtProtocolSetting::JustFloat: {
//     // 生成5个二进制浮点数
//     for (int i = 0; i < 5; i++) {
//       float value = random.bounded(-100.0f, 100.0f);
//       testData.append(reinterpret_cast<const char *>(&value), sizeof(float));
//     }
//     break;
//   }

//   case TtProtocolSetting::FireWater: {
//     // 生成CSV格式数据
//     QString dataStr = "$";
//     for (int i = 0; i < 5; i++) {
//       float value = random.bounded(-100.0f, 100.0f);
//       dataStr += QString::number(value, 'f', 2);
//       if (i < 4)
//         dataStr += ",";
//     }
//     dataStr += "\r\n";
//     testData = dataStr.toUtf8();
//     break;
//   }
//   }

//   return testData;
// }
QByteArray SerialWindow::GenerateRandomTestData(
    TtProtocolSetting::Protocol protocol) {
  QByteArray testData;
  static QRandomGenerator random = QRandomGenerator::securelySeeded();

  switch (protocol) {
    case TtProtocolSetting::RawData: {
      // 生成5个随机浮点数，以空格分隔
      QString dataStr;
      for (int i = 0; i < 5; i++) {
        // 使用generateDouble()替代bounded()，并手动调整范围
        float value =
            static_cast<float>(random.generateDouble() * 200.0 - 100.0);
        dataStr += QString::number(value, 'f', 2);
        if (i < 4) dataStr += " ";
      }
      testData = dataStr.toUtf8();
      break;
    }

    case TtProtocolSetting::JustFloat: {
      // 生成5个二进制浮点数
      for (int i = 0; i < 5; i++) {
        // 使用generateDouble()替代bounded()
        float value =
            static_cast<float>(random.generateDouble() * 200.0 - 100.0);
        testData.append(reinterpret_cast<const char *>(&value), sizeof(float));
      }
      break;
    }

    case TtProtocolSetting::FireWater: {
      // 生成CSV格式数据
      QString dataStr = "$";
      for (int i = 0; i < 5; i++) {
        // 使用generateDouble()替代bounded()
        float value =
            static_cast<float>(random.generateDouble() * 200.0 - 100.0);
        dataStr += QString::number(value, 'f', 2);
        if (i < 4) dataStr += ",";
      }
      dataStr += "\r\n";
      testData = dataStr.toUtf8();
      break;
    }
  }

  return testData;
}

void SerialWindow::StartRandomDataTest(TtProtocolSetting::Protocol protocol) {
  // protocol_ = protocol;
  //// 创建定时器，定期发送随机数据
  // QTimer *testTimer = new QTimer(this);
  // connect(testTimer, &QTimer::timeout, this, [this, protocol]() {
  //   QByteArray testData = generateRandomTestData(protocol);
  //   // 模拟接收数据
  //   dataReceived(testData);
  // });

  //// 每100ms发送一次数据
  // testTimer->start(50);
  //  constexpr double freq = 1.0; // 2Hz
  constexpr double freq = 1.0;  // 2Hz
  // const double samplePeriod =
  //     static_cast<double>(SAMPLE_GENERATION_PERIOD) / 1000.0;
  // constexpr double twoPi = 2.0 * M_PI;

  // for (int i = 0; i < dataPoints.size(); i++) {
  // double phase = (i * M_PI) / dataPoints.size();
  // double value = qSin(phase + twoPi * freq * sampleNumber * samplePeriod);
  // // qDebug() << sampleNumber << value;
  // QVector<point_t> data;
  // point_t point = {
  //     .x = static_cast<double>(sampleNumber),
  //     .y = value,
  // };

  // data << point;
  // 点数组
  // dataPoints.at(i)->appendPoints(data);
  // }
}

void SerialWindow::showErrorMessage(const QString &text) {
  Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500, this);
  on_off_btn_->setChecked(false);
  opened_ = false;
  SetControlState(!opened_);
  send_package_timer_->stop();
  heartbeat_timer_->stop();
  msg_queue_.clear();
  emit workStateChanged(false);
}

void SerialWindow::dataReceived(const QByteArray &data) {
  // qDebug() << "data: " << data;

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
  QString editorText = editor_->text();
  if (editor_->text().isEmpty()) {
    return;
  }
  // 十六进制出现问题, 如果心跳是 HEX 模式, 使
  // radio button 切换发送类型
  // 适用于 分包机制
  SendMessage(editorText, send_type_);
}

void SerialWindow::sendMessageToPort(const QString &data) { SendMessage(data); }

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

void SerialWindow::Init() {
  InitUi();
  // main_layout_ = new Ui::TtVerticalLayout(this);

  // title_ = new Ui::TtNormalLabel(this);

  // modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  // modify_title_btn_->setSvgSize(18, 18);

  // original_widget_ = new QWidget(this);
  // Ui::TtHorizontalLayout *originalWidgetLayout =
  //     new Ui::TtHorizontalLayout(original_widget_);
  // originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  // originalWidgetLayout->addWidget(title_, 0, Qt::AlignLeft);
  // originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  // originalWidgetLayout->addWidget(modify_title_btn_);
  // originalWidgetLayout->addStretch();

  // edit_widget_ = new QWidget(this);
  // title_edit_ = new Ui::TtLineEdit(this);

  // Ui::TtHorizontalLayout *edit_layout =
  //     new Ui::TtHorizontalLayout(edit_widget_);
  // edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  // edit_layout->addWidget(title_edit_);
  // edit_layout->addStretch();

  // stack_ = new QStackedWidget(this);
  // stack_->setMaximumHeight(40);
  // stack_->addWidget(original_widget_);
  // stack_->addWidget(edit_widget_);

  // connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
  //         &SerialWindow::switchToEditMode);

  // Ui::TtHorizontalLayout *stackLayout = new Ui::TtHorizontalLayout;
  // stackLayout->addWidget(stack_);

  // Ui::TtHorizontalLayout *topLayout = new Ui::TtHorizontalLayout;

  // auto handleSave = [this]() {
  //   if (!title_edit_->text().isEmpty()) {
  //     switchToDisplayMode();
  //   } else {
  //     title_edit_->setPlaceholderText(tr("名称不能为空！"));
  //   }
  // };

  // connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  // Ui::TtHorizontalLayout *operationButtonLayout = new Ui::TtHorizontalLayout;

  // save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  // save_btn_->setSvgSize(18, 18);
  // // 默认显示蓝色
  // save_btn_->setColors(QColor("#2196F3"), QColor("#2196F3"));

  // on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  // on_off_btn_->setColors(Qt::black, Qt::red);
  // on_off_btn_->setSvgSize(18, 18);

  // operationButtonLayout->addWidget(save_btn_);
  // operationButtonLayout->addWidget(on_off_btn_, 0, Qt::AlignRight);
  // operationButtonLayout->addSpacerItem(new QSpacerItem(10, 10));

  // topLayout->addLayout(stackLayout);
  // topLayout->addLayout(operationButtonLayout);

  // main_layout_->addLayout(topLayout);

  // main_splitter_ = new QSplitter;
  // main_splitter_->setOrientation(Qt::Horizontal);

  // QWidget *chose_function = new QWidget;
  // Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  // chose_function_layout->setSpacing(5);
  // chose_function->setLayout(chose_function_layout);

  // // 存在 2 层
  // page_btn_widget_ = new QWidget(chose_function);
  // page_btn_layout_ = new Ui::TtHorizontalLayout(page_btn_widget_);
  // Ui::TtSvgButton *terminalButton =
  //     new Ui::TtSvgButton(":/sys/terminal.svg", page_btn_widget_);
  // terminalButton->setSvgSize(18, 18);
  // terminalButton->setColors(Qt::black, Qt::blue);

  // // Ui::TtSvgButton *chatButton =
  // //     new Ui::TtSvgButton(":/sys/chat.svg", page_btn_widget_);
  // // chatButton->setSvgSize(18, 18);
  // // chatButton->setColors(Qt::black, Qt::blue);
  // // rightBtn->setEnableHoldToCheck(true);

  // page_btn_layout_->addWidget(terminalButton);
  // // page_btn_layout_->addWidget(chatButton);

  // // 左侧切换逻辑
  // page_btn_logical_ = new Ui::TtWidgetGroup(this);
  // // showStyle->setHoldingChecked(true);
  // page_btn_logical_->setHoldingChecked(true);
  // page_btn_logical_->addWidget(terminalButton);
  // // page_btn_logical_->addWidget(chatButton);
  // page_btn_logical_->setExclusive(true);
  // page_btn_logical_->setCheckedIndex(0);
  // chose_function_layout->addWidget(page_btn_widget_);
  // chose_function_layout->addStretch();

  // clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  // clear_history_->setSvgSize(18, 18);

  // // 切换显示 text/hex
  // Ui::TtWidgetGroup *displayLogic = new Ui::TtWidgetGroup(this);
  // displayLogic->setHoldingChecked(true);
  // display_text_btn_ = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  // display_text_btn_->setCheckedColor(QColor(0, 102, 180));
  // display_text_btn_->setLightDefaultColor(Qt::white);  // 临时白色文字
  // display_hex_btn_ = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
  // display_hex_btn_->setCheckedColor(QColor(0, 102, 180));
  // display_text_btn_->setLightTextColor(Qt::white);  // 临时白色文字

  // displayLogic->addWidget(display_text_btn_);
  // displayLogic->addWidget(display_hex_btn_);

  // displayLogic->setCheckedIndex(0);
  // displayLogic->setExclusive(true);  // 开启了互斥

  // chose_function_layout->addWidget(display_text_btn_);
  // chose_function_layout->addWidget(display_hex_btn_);
  // display_text_btn_->setChecked(true);

  // connect(displayLogic, &Ui::TtWidgetGroup::widgetClicked, this,
  //         [this](int idx) {
  //           setDisplayType(idx == 1 ? TtTextFormat::HEX :
  //           TtTextFormat::TEXT);
  //         });
  // // 清除历史按钮
  // chose_function_layout->addWidget(clear_history_);

  // QSplitter *VSplitter = new QSplitter;
  // VSplitter->setOrientation(Qt::Vertical);
  // VSplitter->setContentsMargins(QMargins());

  // // 上方选择功能以及信息框
  // QWidget *contentWidget = new QWidget(this);
  // Ui::TtVerticalLayout *contentWidgetLayout =
  //     new Ui::TtVerticalLayout(contentWidget);

  // // 不同类型展示数据栈窗口
  // message_stacked_view_ = new QStackedWidget(contentWidget);

  // terminal_ = new QPlainTextEdit(this);
  // terminal_->setReadOnly(true);
  // terminal_->setFrameStyle(QFrame::NoFrame);
  // lexer_ =
  // std::make_unique<Ui::TtTerminalHighlighter>(terminal_->document());

  // message_stacked_view_->addWidget(terminal_);

  // // message_view_ = new Ui::TtChatView(message_stacked_view_);
  // // message_view_->setResizeMode(QListView::Adjust);
  // // message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  // // message_view_->setMouseTracking(true);
  // // message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  // // message_stacked_view_->addWidget(message_view_);

  // contentWidgetLayout->addWidget(chose_function);
  // contentWidgetLayout->addWidget(message_stacked_view_);

  // // 显示的图标
  // connect(page_btn_logical_, &Ui::TtWidgetGroup::widgetClicked, this,
  //         [this](const int &idx) {
  //           message_stacked_view_->setCurrentIndex(idx);
  //           if (idx != 0) {
  //             // 图表
  //             display_hex_btn_->setVisible(false);
  //             display_text_btn_->setVisible(false);
  //           } else {
  //             display_hex_btn_->setVisible(true);
  //             display_text_btn_->setVisible(true);
  //           }
  //         });

  // // message_model_ = new Ui::TtChatMessageModel;
  // // message_view_->setModel(message_model_);
  // // message_view_->scrollToBottom();
  // QWidget *bottomAll = new QWidget(this);
  // Ui::TtVerticalLayout *bottomAllLayout = new
  // Ui::TtVerticalLayout(bottomAll); bottomAll->setLayout(bottomAllLayout);

  // // 图表格设置
  // QWidget *graphSetting = new QWidget(bottomAll);
  // Ui::TtHorizontalLayout *graphSettingLayout =
  //     new Ui::TtHorizontalLayout(graphSetting);
  // QLabel *setDetaT = new QLabel("设置数据点间隔: ", graphSetting);
  // QSpinBox *setDetaTSpin = new QSpinBox(graphSetting);
  // setDetaTSpin->setSuffix("ms");
  // graphSettingLayout->addWidget(setDetaT);
  // graphSettingLayout->addWidget(setDetaTSpin);

  // //   connect(useFixedDeltaCheckBox, &QCheckBox::toggled, this, [this,
  // //   deltaSpinBox](bool checked) {
  // //     serial_plot_->setTimeDeltaMode(checked);
  // //     deltaSpinBox->setEnabled(checked);
  // // });
  // connect(setDetaTSpin, &QSpinBox::valueChanged, this,
  //         [this](int value) { serial_plot_->setFixedTimeDelta(value); });

  // Ui::TtLabelLineEdit *maxPoint =
  //     new Ui::TtLabelLineEdit("最大数据点数: ", graphSetting);
  // maxPoint->setText("50000");
  // QLabel *maxPointLabel = new QLabel("/ch", graphSetting);
  // graphSettingLayout->addWidget(maxPoint);
  // graphSettingLayout->addWidget(maxPointLabel);

  // connect(maxPoint->body(), &Ui::TtLineEdit::textChanged, this,
  //         [this](const QString &text) {
  //           bool ok = false;
  //           int maxPoint = text.toInt(&ok);
  //           if (ok) {
  //             qDebug() << "this set maxPoint";
  //             // BUG 缺少
  //             // serial_plot_->setMaxPoint(maxPoint);
  //           } else {
  //             Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
  //                                     tr("最大数据点数输入错误"), 1500,
  //                                     this);
  //           }
  //         });

  // Ui::TtTextButton *autoScaleBtn =
  //     new Ui::TtTextButton(QColor(Qt::blue), "自动缩放", graphSetting);
  // // connect(autoScaleBtn, &Ui::TtTextButton::clicked, serial_plot_,
  // //         &Ui::TtSerialPortPlot::autoScale);
  // connect(autoScaleBtn, &Ui::TtTextButton::clicked, this, [this] {
  //   // qDebug() << "自动缩放";
  //   serial_plot_->autoScale();
  // });
  // autoScaleBtn->setToolTip(tr("自动缩放"));
  // graphSettingLayout->addStretch();
  // graphSettingLayout->addWidget(autoScaleBtn);

  // bottomAllLayout->addWidget(graphSetting);

  // // 下方自定义指令
  // tabs_widget_ = new QWidget(this);
  // Ui::TtHorizontalLayout *tacLayout = new
  // Ui::TtHorizontalLayout(tabs_widget_); tabs_widget_->setLayout(tacLayout);

  // tabs_ = new QtMaterialTabs(tabs_widget_);
  // tabs_->addTab(tr("手动"));
  // tabs_->addTab(tr("片段"));
  // tabs_->addTab(tr("自动应答"));
  // // tabs_->setFixedHeight(30);
  // // tabs_->setMinimumWidth(80);
  // tabs_->setBackgroundColor(QColor(192, 120, 196));

  // tacLayout->addWidget(tabs_);
  // // tacLayout->addStretch();

  // // 显示发送字节和接收字节数
  // send_byte_ = new Ui::TtElidedLabel(tr("发送字节数: 0 B"), tabs_widget_);
  // send_byte_->setFixedHeight(30);
  // recv_byte_ = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_widget_);
  // recv_byte_->setFixedHeight(30);

  // tacLayout->addWidget(send_byte_);
  // tacLayout->addWidget(recv_byte_);

  // display_widget_ = new QStackedWidget(this);

  // QWidget *messageEdit = new QWidget(display_widget_);
  // QVBoxLayout *messageEditLayout = new QVBoxLayout;
  // messageEdit->setLayout(messageEditLayout);
  // messageEditLayout->setContentsMargins(3, 0, 3, 0);
  // messageEditLayout->setSpacing(0);

  // editor_ = new QsciScintilla(messageEdit);
  // editor_->setWrapMode(QsciScintilla::WrapWord);
  // editor_->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
  //                             QsciScintilla::WrapFlagInMargin, 0);
  // editor_->setCaretWidth(10);
  // editor_->setMarginType(1, QsciScintilla::NumberMargin);
  // editor_->setFrameStyle(QFrame::NoFrame);

  // messageEditLayout->addWidget(editor_);

  // QWidget *bottomBtnWidget = new QWidget(messageEdit);
  // bottomBtnWidget->setMinimumHeight(40);
  // Ui::TtHorizontalLayout *bottomBtnWidgetLayout =
  //     new Ui::TtHorizontalLayout(bottomBtnWidget);

  // chose_text_btn_ = new Ui::TtRadioButton("TEXT", bottomBtnWidget);
  // chose_hex_btn_ = new Ui::TtRadioButton("HEX", bottomBtnWidget);
  // chose_text_btn_->setChecked(true);

  // send_btn_ = new QtMaterialFlatButton(bottomBtnWidget);
  // send_btn_->setIcon(QIcon(":/sys/send.svg"));
  // bottomBtnWidgetLayout->addWidget(chose_text_btn_);
  // bottomBtnWidgetLayout->addWidget(chose_hex_btn_);
  // bottomBtnWidgetLayout->addStretch();
  // bottomBtnWidgetLayout->addWidget(send_btn_);

  // messageEditLayout->addWidget(bottomBtnWidget);

  // instruction_table_ = new Ui::TtTableWidget(display_widget_);
  // auto_replay_table_ = new Ui::TtTableWidget(display_widget_);

  // display_widget_->addWidget(messageEdit);
  // display_widget_->addWidget(instruction_table_);
  // display_widget_->addWidget(auto_replay_table_);

  // display_widget_->setCurrentIndex(0);

  // // 显示, 并输入 lua 脚本
  // // lua_code_ = new Ui::TtLuaInputBox(this);
  // // lua_actuator_ = new Core::LuaKernel;

  // bottomAllLayout->addWidget(tabs_widget_);
  // bottomAllLayout->addWidget(display_widget_);

  // VSplitter->addWidget(contentWidget);
  // VSplitter->addWidget(bottomAll);
  // VSplitter->setStretchFactor(0, 3);
  // VSplitter->setStretchFactor(1, 2);
  // VSplitter->setCollapsible(0, false);
  // VSplitter->setSizes(QList<int>() << 450 << 200);

  // main_splitter_->addWidget(VSplitter);

  // // 设置右侧展示窗口, 放一个置位窗口, 因为后面要设置属性
  // QWidget *rightWidget = new QWidget(this);
  // main_splitter_->addWidget(rightWidget);

  // main_splitter_->setSizes(QList<int>() << 500 << 220);
  // main_splitter_->setCollapsible(0, false);
  // main_splitter_->setCollapsible(1, true);

  // main_splitter_->setStretchFactor(0, 3);
  // main_splitter_->setStretchFactor(1, 2);

  // main_layout_->addWidget(main_splitter_);

  // send_package_timer_ = new QTimer(this);
  // send_package_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  // send_package_timer_->setInterval(0);
  // heartbeat_timer_ = new QTimer(this);
  // heartbeat_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  // heartbeat_timer_->setInterval(0);
  // // initSignalsConnection();

  // title_->setText(tr("未命名的串口链接"));

  serial_setting_ = new Widget::SerialSetting;
  serRightWidget(serial_setting_);

  qDebug() << "测试6";
  qDebug() << "测试5";
  QSplitter *graphSpiltter = new QSplitter;
  serial_plot_ = new Ui::TtSerialPortPlot;
  graphSpiltter->addWidget(serial_plot_);

  serialDataList = new QListWidget(graphSpiltter);
  serialDataList->setContextMenuPolicy(
      Qt::CustomContextMenu);  // 启用自定义右键菜单

  connect(serialDataList, &QListWidget::customContextMenuRequested, this,
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
                // 编辑
                // saved_ = false;
                QString uuid;
                setSaveStatus(false);
                addChannel(editorDialog.metaInfo(),
                           editorDialog.checkBlockColor(), uuid);
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
                    serialDataList->takeItem(serialDataList->row(item));  //
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
                    // qDebug() << "编辑时的(关乎于 button 的特定值) channel
                    // uuid: "
                    // << btn->getUuid()
                    // << channel_info_.value(btn->getUuid()).channel_num_;

                    HandleDialogData(
                        btn->getUuid(),
                        channel_info_.value(btn->getUuid()).channel_num_,
                        editorDialog.metaInfo(),
                        editorDialog.checkBlockColor());
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

  // messageStackedView->addWidget(graphSpiltter);
  qDebug() << "测试3";

  Ui::TtSvgButton *graphBtn = new Ui::TtSvgButton(":/sys/graph-up.svg", this);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  qDebug() << "测试8";
  addDisplayWidget(graphBtn, graphSpiltter);

  qDebug() << "测试8";
  SetControlState(true);

  qDebug() << "测试8";
}

void SerialWindow::SetSerialSetting() {
  serial_setting_->setSerialPortsName();
  serial_setting_->setSerialPortsBaudRate();
  serial_setting_->setSerialPortsDataBit();
  serial_setting_->setSerialPortsParityBit();
  serial_setting_->setSerialPortsStopBit();
  serial_setting_->setSerialPortsFluidControl();
  serial_setting_->displayDefaultSetting();
}

void SerialWindow::ConnectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, this,
          &SerialWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this]() {
    // // 检查是否处于打开状态
    // serial_port 已经移动到了 工作线程中
    // 将openSerialPort的调用通过Qt的信号槽机制排队到worker_thread_中执行，而不是直接在主线程调用
    if (opened_) {
      qDebug() << "close";
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::QueuedConnection);
      opened_ = false;
      on_off_btn_->setChecked(false);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
      // 清空待发送数组
      msg_queue_.clear();
      emit workStateChanged(false);
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
      send_package_timer_->start();

      if (heartbeat_timer_->interval() != 0) {
        heartbeat_timer_->start();
      } else {
        heartbeat_timer_->stop();
      }
      // 处于工作信号
      emit workStateChanged(true);
      StartRandomDataTest(TtProtocolSetting::FireWater);
    }
    SetControlState(!opened_);
  });

  connect(send_btn_, &QtMaterialFlatButton::clicked, this,
          qOverload<>(&SerialWindow::sendMessageToPort));
  connect(serial_setting_, &Widget::SerialSetting::showScriptSetting, this,
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
  connect(serial_setting_, &Widget::SerialSetting::protocolTypeChanged, this,
          [this](int type) {
            // 在确定之后就改变不了
            protocol_ = static_cast<TtProtocolSetting::Protocol>(type);
            qDebug() << "changed" << protocol_;
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
        // QString hexStr = package.remove(QRegularExpression("[^0-9A-Fa-f]"));
        QString hexStr = package.remove(hexFilterRegex);
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
    setHeartbeartContent();  // 适用于发送包间隔
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

  connect(instruction_table_, &Ui::TtTableWidget::SendRowMsg, this,
          qOverload<const QString &, TtTextFormat::Type, uint32_t>(
              &SerialWindow::sendInstructionTableContent));
  connect(instruction_table_, &Ui::TtTableWidget::SendRowsMsg, this,
          [this](const std::vector<Data::MsgInfo> &msgs) {
            // 群发消息,没有延时的能够立马发送
            if (msgs.size() == 0) {
              return;
            }
            foreach (const auto &msg, msgs) {
              sendInstructionTableContent(msg);
            }
          });
}

}  // namespace Window
