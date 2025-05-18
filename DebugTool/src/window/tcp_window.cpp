#include "window/tcp_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/widget_group.h>

#include "widget/frame_setting.h"
#include <lib/qtmaterialcheckable.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>
#include <ui/controls/TtSerialLexer.h>

#include <QTableView>

#include "core/tcp_client.h"
#include "core/tcp_server.h"
#include "ui/controls/TtTableView.h"
#include "widget/tcp_setting.h"

namespace Window {

TcpWindow::TcpWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    : FrameWindow(parent), role_(role) {

  init();

  if (role_ == TtProtocolType::Client) {
    connect(tcp_client_, &Core::TcpClient::connected, this,
            [this]() { opened_ = true; });
    connect(tcp_client_, &Core::TcpClient::disconnected, this,
            [this]() { opened_ = false; });
    connect(tcp_client_, &Core::TcpClient::dataReceived, this,
            &TcpWindow::dataReceived);
    connect(tcp_client_, &Core::TcpClient::errorOccurred, this,
            [this](const QString &text) {
              // Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error,
              //                         1500, this);
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500,
                                      this);
              on_off_btn_->setChecked(false);
              tcp_client_setting_->setControlState(true);
              opened_ = false;
            });
  } else {
    connect(tcp_server_, &Core::TcpServer::serverStarted, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::serverStopped, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::errorOccurred, this,
            [this](const QString &text) {
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500,
                                      this);
              tcp_server_setting_->setControlState(true);
              on_off_btn_->setChecked(false);
              tcp_client_setting_->setControlState(true);
              opened_ = false;
            });
    connect(tcp_server_, &Core::TcpServer::dataReceived, this,
            &TcpWindow::dataReceived);
  }
  connectSignals();
}

TcpWindow::~TcpWindow() { qDebug() << "Delete TCPWindow"; }

QJsonObject TcpWindow::getConfiguration() const { return config_; }

bool TcpWindow::workState() const { return opened_; }

bool TcpWindow::saveState() { return saved_; }

void TcpWindow::setSaveState(bool state) { saved_ = state; }

void TcpWindow::setSetting(const QJsonObject &config) {
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    tcp_client_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    tcp_server_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());

  int sendType = config_.value("SendType").toInt();
  if (sendType == 1) {
    chose_text_btn_->setChecked(true);
  } else if (sendType == 2) {
    chose_hex_btn_->setChecked(true);
  } else if (sendType == 3) {
    // ascll 格式
  }
  // 上方显示
  int displayType = config_.value("DisplayType").toInt();
  if (displayType == 1) {
    display_text_btn_->setChecked(true);
  } else if (displayType == 2) {
    display_hex_btn_->setChecked(true);
  }
  saved_ = true;
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

void TcpWindow::sendMessageToPort() {
  QString editorText = editor_->text();
  if (editor_->text().isEmpty()) {
    return;
  }
  // radio button 切换发送类型
  // 适用于 分包机制
  sendMessage(editorText, send_type_);
}

void TcpWindow::sendMessageToPort(const QString &data) {
  // 原有的
  // 心跳不需要添加
  // 平均有 10ms 的延时
  // 如果有分包, 需要应用分包, 否则直接发送
  sendMessage(data);
}

void TcpWindow::sendMessageToPort(const QString &data, const int &times) {
  QTimer::singleShot(times, this, [this, data]() {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    if (role_ == TtProtocolType::Client) {
      tcp_client_->sendMessage(data.toUtf8());
    } else {
      tcp_server_->sendMessageToClients(data.toUtf8());
    }
    showMessage(data);
  });
}

void TcpWindow::setHeartbeartContent() {
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    if (heart_beat_type_ == TtTextFormat::TEXT) {
      // qDebug() << "发送心跳的 TEXT 格式内容";
      sendMessage(heartbeat_, TtTextFormat::TEXT);
    } else if (heart_beat_type_ == TtTextFormat::HEX) {
      sendMessage(heartbeat_, TtTextFormat::HEX);
    } else if (heart_beat_type_ == TtTextFormat::None) {
    }
  }
}

void TcpWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitle", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("TcpClientSetting",
                   tcp_client_setting_->getTcpClientSetting());
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("TcpServerSetting",
                   tcp_server_setting_->getTcpServerSetting());
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());
  config_.insert("SendType", QJsonValue(send_type_));
  config_.insert("DisplayType", QJsonValue(display_type_));
  saved_ = true;
  emit requestSaveConfig();
}

QString TcpWindow::getTitle() const { return title_->text(); }

void TcpWindow::updateServerStatus() {
  const bool running = tcp_server_->isRunning();
  auto text =
      (running ? tr("服务运行中 (端口: %1)").arg("111") : tr("服务已停止"));
  qDebug() << text;
}

void TcpWindow::dataReceived(const QByteArray &data) {
  // qDebug() << "Received data:" << data;
  // recv_byte_count += data.size();
  // auto tmp = new Ui::TtChatMessage();
  // tmp->setContent(data);
  // tmp->setOutgoing(false);
  // tmp->setBubbleColor(QColor("#0ea5e9"));
  // QList<Ui::TtChatMessage *> list;
  // list.append(tmp);
  // message_model_->appendMessages(list);
  // recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  // message_view_->scrollToBottom();
  showMessage(data, false);
}

void TcpWindow::init() {
  initUi();
  // 初始化虚基类
  if (role_ == TtProtocolType::Client) {
    tcp_client_ = new Core::TcpClient;
    title_->setText(tr("未命名的 TCP 连接"));
    // setting_ = new Widget::TcpClientSetting(this);
    tcp_client_setting_ = new Widget::TcpClientSetting(this);
    serRightWidget(tcp_client_setting_);
  } else if (role_ == TtProtocolType::Server) {
    tcp_server_ = new Core::TcpServer;
    // setting_ = new Widget::TcpServerSetting(this);
    tcp_server_setting_ = new Widget::TcpServerSetting(this);
    title_->setText(tr("未命名的 TCP 服务模拟端"));
    serRightWidget(tcp_server_setting_);
  }
}

void TcpWindow::connectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &TcpWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this] {
    if (opened_) {
      if (role_ == TtProtocolType::Client) {
        qDebug() << "test";
        tcp_client_->disconnectFromServer();
        tcp_client_setting_->setControlState(true);
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->close();
        tcp_client_setting_->setControlState(true);
      }
    } else {
      // 转换成对应的实际子类
      if (role_ == TtProtocolType::Client) {
        qDebug() << "connectToServer";
        tcp_client_->connectToServer(
            tcp_client_setting_->getTcpClientConfiguration());
        tcp_client_setting_->setControlState(false);
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->startServer(
            tcp_server_setting_->getTcpServerConfiguration());
        tcp_server_setting_->setControlState(false);
      }
    }
  });

  connect(clear_history_, &Ui::TtSvgButton::clicked, this,
          [this]() { message_model_->clearModelData(); });

  connect(send_btn_, &QtMaterialFlatButton::clicked, this,
          qOverload<>(&TcpWindow::sendMessageToPort));

  // connect(serial_setting_, &Widget::SerialSetting::showScriptSetting,
  //         [this]() { lua_code_->show(); });

  if (role_ == TtProtocolType::Client) {
    connect(tcp_client_setting_,
            &Widget::TcpClientSetting::sendPackageMaxSizeChanged, this,
            [this](const uint16_t size) {
              // 不设定的时候异常数字
              if (package_size_ != size) {
                package_size_ = size;
                qDebug() << "packageSize: " << package_size_;
              }
            });
    // 链接不同的信号槽
    connect(tcp_client_setting_, &Widget::FrameSetting::settingChanged, this,
            [this]() {
              qDebug() << "saved Changed false";
              saved_ = false;
            });
  } else if (role_ == TtProtocolType::Server) {
    // 链接不同的信号槽
    connect(tcp_server_setting_, &Widget::FrameSetting::settingChanged, this,
            [this]() {
              qDebug() << "saved Changed false";
              saved_ = false;
            });
  }

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    qDebug() << "send package timer";
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      msg_queue_.clear();
      return;
    }
    // 取出数据
    if (!msg_queue_.isEmpty()) {
      QString package = msg_queue_.dequeue();
      send_byte_count_ += package.size();
      QByteArray dataUtf8;
      //
      bool isHexMode =
          (send_type_ == TtTextFormat::HEX) ||
          (heart_beat_type_ == TtTextFormat::HEX && isEnableHeartbeart());
      if (isHexMode) {
        QString hexStr = package.remove(QRegularExpression("[^0-9A-Fa-f]"));
        for (int i = 0; i < hexStr.length(); i += 2) {
          if (i + 1 >= hexStr.length()) {
            // 在未成对的位置前插入0
            hexStr.insert(i, '0');
          }
        }
        dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
      } else {
        // 保存 text 格式发送
        dataUtf8 = package.toUtf8();
      }
      // 发送并显示
      if (!dataUtf8.isEmpty()) {
        if (role_ == TtProtocolType::Client) {
          tcp_client_->sendMessage(dataUtf8);
        } else if (role_ == TtProtocolType::Server) {
          tcp_server_->sendMessageToClients(dataUtf8);
        }
        // 更新显示
        send_byte_count_ += dataUtf8.size();
        send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
        // 在UI中显示
        showMessage(dataUtf8, true);
      }
    }
  });

  connect(heartbeat_timer_, &QTimer::timeout, this, [this] {
    setHeartbeartContent(); // 适用于发送包间隔
  });

  connect(chose_hex_btn_, &Ui::TtRadioButton::toggled, [this](bool checked) {
    if (checked) {
      send_type_ = TtTextFormat::HEX;
    }
  });
  connect(chose_text_btn_, &Ui::TtRadioButton::toggled, [this](bool checked) {
    if (checked) {
      send_type_ = TtTextFormat::TEXT;
    }
  });

  connect(instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
          qOverload<const QString &>(&TcpWindow::sendMessageToPort));
  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const QVector<QPair<QString, int>> &datas) {
            if (datas.isEmpty()) {
              return;
            }
            foreach (const auto &pair, datas) {
              sendMessageToPort(pair.first, pair.second);
            }
          });
}

void TcpWindow::sendMessage(const QString &data, TtTextFormat::Type type) {
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
    // 根据格式不同存储
    if (type == TtTextFormat::HEX) {
      // 按照 16 进制分包
      int byteSize = package_size_;
      int charSize = byteSize * 2; // 每个字节转换为两个十六进制字符
      for (int i = 0; i < processedText.length(); i += charSize) {
        // 截取固定字符数的十六进制字符串片段
        QString chunk = processedText.mid(i, charSize);
        msg_queue_.enqueue(chunk);
        qDebug() << "分包HEX: " << chunk;
      }
    } else {
      // 没有设置间隔, 可以直接秒发送
      // TEXT模式：直接按字节分包
      // 加入多个到 msg_ 中
      for (int i = 0; i < dataUtf8.size(); i += package_size_) {
        QByteArray chunk = dataUtf8.mid(i, package_size_);
        msg_queue_.enqueue(QString::fromUtf8(chunk));
        qDebug() << "分包TEXT: " << QString::fromUtf8(chunk);
      }
    }
  } else {
    // 没有分包, 直接发送
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    if (role_ == TtProtocolType::Client) {
      tcp_client_->sendMessage(dataUtf8);
    } else if (role_ == TtProtocolType::Server) {
      tcp_server_->sendMessageToClients(dataUtf8);
    }
    // QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
    //                           Q_ARG(QByteArray, dataUtf8));

    showMessage(dataUtf8, true);
  }
}

void TcpWindow::showMessage(const QByteArray &data, bool out) {
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
  if (display_type_ == TtTextFormat::TEXT) {
    // 直接追加文本
    formattedMessage.append(data);
  } else if (display_type_ == TtTextFormat::HEX) {
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

void TcpWindow::showMessage(const QString &data, bool out) {
  // qDebug() << out << data;
  QByteArray dataUtf8 = data.toUtf8();

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
  if (display_type_ == TtTextFormat::TEXT) {
    // TEXT 格式, data 保持, 有问题
    formattedMessage.append(data);
  } else if (display_type_ == TtTextFormat::HEX) {
    // qDebug() << "hear" << dataUtf8.toHex(' ').toUpper().trimmed();
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

bool TcpWindow::isEnableHeartbeart() {
  // 使能了当前的条件
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    return true;
  }
  return false;
}

} // namespace Window
