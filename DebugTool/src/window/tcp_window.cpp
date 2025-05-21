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
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500,
                                      this);
              opened_ = false;
              on_off_btn_->setChecked(false);
              setControlState(!opened_);
              send_package_timer_->stop();
              heartbeat_timer_->stop();
              msg_queue_.clear();
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
    tcp_client_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    tcp_server_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  }

  QJsonObject instructionTableData =
      config.value("InstructionTable").toObject();
  instruction_table_->setupTable(instructionTableData);

  int sendType = config.value("SendType").toInt();
  send_type_ = static_cast<TtTextFormat::Type>(sendType);
  if (sendType == 1) {
    chose_text_btn_->setChecked(true);
  } else if (sendType == 2) {
    chose_hex_btn_->setChecked(true);
  } else if (sendType == 3) {
    // ascll 格式
  }
  // 上方显示
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

void TcpWindow::sendInstructionTableContent(const QString &text,
                                            TtTextFormat::Type type,
                                            uint32_t time) {
  QByteArray dataUtf8;
  if (type == TtTextFormat::TEXT) {
    dataUtf8 = text.toUtf8();
  } else if (type == TtTextFormat::HEX) {
    // QString hexStr = text.remove(QRegularExpression("[^0-9A-Fa-f]"));
    QString hexStr = QString(text);
    hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));

    if (hexStr.isEmpty()) {
      qDebug() << "存在无效的十六进制字符";
      return;
    }
    if (hexStr.length() % 2 != 0) {
      for (int i = 0; i < hexStr.length(); i += 2) {
        if (i + 1 >= hexStr.length()) {
          hexStr.insert(i, '0');
        }
      }
    }
    dataUtf8 = QByteArray::fromHex(hexStr.toUtf8());
  }
  if (!dataUtf8.isEmpty()) {
    QTimer::singleShot(time, Qt::PreciseTimer, this,
                       [this, dataUtf8] { sendPackagedData(dataUtf8, true); });
  }
}

void TcpWindow::sendInstructionTableContent(const Data::MsgInfo &msg) {
  // 进入了
  qDebug() << "send Mess";
  sendInstructionTableContent(msg.text, msg.type, msg.time);
}

void TcpWindow::saveSetting() {
  config_.insert("WindowTitle", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    config_.insert("TcpClientSetting",
                   tcp_client_setting_->getTcpClientSetting());
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    config_.insert("TcpServerSetting",
                   tcp_server_setting_->getTcpServerSetting());
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());
  config_.insert("SendType", QJsonValue(int(send_type_)));
  config_.insert("DisplayType", QJsonValue(int(display_type_)));

  setSaveStatus(true);
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
  showMessage(data, false);
}

void TcpWindow::init() {
  initUi();
  if (role_ == TtProtocolType::Client) {
    tcp_client_ = new Core::TcpClient;
    title_->setText(tr("未命名的 TCP 连接"));
    tcp_client_setting_ = new Widget::TcpClientSetting(this);
    serRightWidget(tcp_client_setting_);
  } else if (role_ == TtProtocolType::Server) {
    tcp_server_ = new Core::TcpServer;
    tcp_server_setting_ = new Widget::TcpServerSetting(this);
    title_->setText(tr("未命名的 TCP 服务模拟端"));
    serRightWidget(tcp_server_setting_);
  }
}

void TcpWindow::connectSignals() {
  initSignalsConnection();

  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &TcpWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this] {
    if (opened_) {
      qDebug() << "closed";
      if (role_ == TtProtocolType::Client) {
        tcp_client_->disconnectFromServer();
        tcp_client_setting_->setControlState(true);
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->close();
        tcp_client_setting_->setControlState(true);
      }
      opened_ = false;
      on_off_btn_->setChecked(false);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
      msg_queue_.clear();

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
      opened_ = true;
      on_off_btn_->setChecked(true);

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
          qOverload<>(&TcpWindow::sendMessageToPort));

  // connect(serial_setting_, &Widget::SerialSetting::showScriptSetting,
  //         [this]() { lua_code_->show(); });

  if (role_ == TtProtocolType::Client) {
    connect(tcp_client_setting_,
            &Widget::FrameSetting::sendPackageMaxSizeChanged, this,
            [this](const uint16_t size) {
              if (package_size_ != size) {
                package_size_ = size;
                qDebug() << "packageSize: " << package_size_;
              }
            });
    connect(tcp_client_setting_, &Widget::FrameSetting::settingChanged, this,
            [this]() {
              qDebug() << "changed savestate";
              setSaveStatus(false);
            });

    connect(tcp_client_setting_,
            &Widget::FrameSetting::sendPackageIntervalChanged, this,
            [this](const uint32_t &interval) {
              qDebug() << "interval: " << interval;
              send_package_timer_->setInterval(interval);
            });

    connect(tcp_client_setting_, &Widget::FrameSetting::heartbeatType, this,
            [this](TtTextFormat::Type type) {
              qDebug() << "heart_bear_type_" << heart_beat_type_;
              heart_beat_type_ = type;
            });

    connect(tcp_client_setting_, &Widget::FrameSetting::heartbeatContentChanged,
            this, [this](const QString &content) {
              qDebug() << content;
              if (heartbeat_ != content) {
                heartbeat_ = content;
                heartbeat_utf8_ = heartbeat_.toUtf8().toHex(' ');
              }
            });

    connect(tcp_client_setting_, &Widget::FrameSetting::heartbeatInterval, this,
            [this](const uint32_t times) {
              qDebug() << times;
              if (heartbeat_interval_ != times) {
                heartbeat_interval_ = times;
                qDebug() << heartbeat_interval_;
                heartbeat_timer_->setInterval(times);
              }
            });

  } else if (role_ == TtProtocolType::Server) {
    // 链接不同的信号槽
    connect(tcp_server_setting_, &Widget::TcpServerSetting::settingChanged,
            this, [this]() {
              // qDebug() << "saved Changed false";
              // saved_ = false;
              setSaveStatus(false);
            });
  }

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    // 有问题
    // qDebug() << "send package timer";
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("端口未链接"),
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
              &TcpWindow::sendInstructionTableContent));
  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const std::vector<Data::MsgInfo> &msgs) {
            // 群发进入了
            qDebug() << "群发消息";
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

void TcpWindow::setControlState(bool state) {
  if (state) {
    // 启用控件
    if (role_ == TtProtocolType::Client) {
      tcp_client_setting_->setControlState(true);
    } else if (role_ == TtProtocolType::Server) {
      tcp_server_setting_->setControlState(true);
    }
    instruction_table_->setEnabled(true);
    send_btn_->setEnabled(false);
  } else {
    // 禁用控件
    // 启用控件
    if (role_ == TtProtocolType::Client) {
      tcp_client_setting_->setControlState(false);
    } else if (role_ == TtProtocolType::Server) {
      tcp_server_setting_->setControlState(false);
    }
    // instruction_table_->setEnabled(false);
    instruction_table_->setEnabled(false);
    send_btn_->setEnabled(true);
  }
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
      for (int i = 0; i < dataUtf8.size(); i += package_size_) {
        QByteArray chunk = dataUtf8.mid(i, package_size_);
        msg_queue_.enqueue(QString::fromUtf8(chunk));
        qDebug() << "分包TEXT: " << QString::fromUtf8(chunk);
      }
    }
  } else {
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
    showMessage(dataUtf8, true);
  }
}

bool TcpWindow::isEnableHeartbeart() {
  // 使能了当前的条件
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    return true;
  }
  return false;
}

void TcpWindow::sendPackagedData(const QByteArray &data, bool isHeartbeat) {
  if (!opened_) {
    if (!isHeartbeat) {
      // 非心跳数据
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
    }
    return;
  }

  if (package_size_ > 0) {
    for (int i = 0; i < data.size(); i += package_size_) {
      QByteArray chunk = data.mid(i, package_size_);
      if (role_ == TtProtocolType::Client) {
        tcp_client_->sendMessage(chunk);
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->sendMessageToClients(chunk);
      }
      send_byte_count_ += chunk.size();
      send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
      // 显示消息
      showMessage(chunk, true);
    }
  } else {
    if (role_ == TtProtocolType::Client) {
      tcp_client_->sendMessage(data);
    } else if (role_ == TtProtocolType::Server) {
      tcp_server_->sendMessageToClients(data);
    }
    send_byte_count_ += data.size();
    send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
    // 显示消息
    showMessage(data, true);
  }
}

} // namespace Window
