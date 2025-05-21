#include "udp_window.h"

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

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>

#include "core/udp_client.h"
#include "core/udp_server.h"
#include "ui/controls/TtTableView.h"
#include "widget/udp_setting.h"

namespace Window {

UdpWindow::UdpWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    : FrameWindow(parent), role_(role) {
  init();

  if (role_ == TtProtocolType::Client) {
    // connect(udp_client_, &Core::UdpClient::data)

  } else if (role_ == TtProtocolType::Server) {
    connect(udp_server_, &Core::UdpServer::datagramReceived,
            [this](const QString &peerInfo, const quint16 &peerPort,
                   const QByteArray &message) { dataReceived(message); });
    QObject::connect(udp_server_, &Core::UdpServer::errorOccurred,
                     [this](const QString &error) {
                       Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                                               error, 1500, this);
                     });
  }
  connectSignals();
}

UdpWindow::~UdpWindow() { qDebug() << "Delete UdpWindow"; }

QJsonObject UdpWindow::getConfiguration() const { return config_; }

bool UdpWindow::workState() const { return opened_; }

bool UdpWindow::saveState() { return saved_; }

void UdpWindow::setSaveState(bool state) { saved_ = state; }

void UdpWindow::saveSetting() {
  // 外部调用 saveSetting
  config_.insert("WindowTitle", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    config_.insert("UdpClientSetting",
                   udp_client_setting_->getUdpClientSetting());
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    config_.insert("UdpServerSetting",
                   udp_server_setting_->getUdpServerSetting());
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());
  setSaveStatus(true);
  emit requestSaveConfig();
}

void UdpWindow::setSetting(const QJsonObject &config) {
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  if (role_ == TtProtocolType::Client) {
    udp_client_setting_->setOldSettings(
        config.value("UdpClientSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    udp_server_setting_->setOldSettings(
        config.value("UdpServerSetting").toObject(QJsonObject()));
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

  // 设置完配置之后, 会最终调用一次, 但是直接的调用, 是否会直接影响未保存的状态
  setSaveStatus(true);
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

QString UdpWindow::getTitle() const { return title_->text(); }

void UdpWindow::updateServerStatus() {
  // const bool running = tcp_server_->isRunning();
  // auto text =
  //     (running ? tr("服务运行中 (端口: %1)").arg("111") : tr("服务已停止"));
  // qDebug() << text;
}

void UdpWindow::dataReceived(const QByteArray &data) {
  // recv_byte_count += data.size();
  // recv_byte_count_ += data.size();
  // auto tmp = new Ui::TtChatMessage();
  // tmp->setContent(data);
  // tmp->setOutgoing(false);
  // tmp->setBubbleColor(QColor("#0ea5e9"));
  // QList<Ui::TtChatMessage *> list;
  // list.append(tmp);
  // message_model_->appendMessages(list);
  // recv_byte_->setText(QString("接收字节数: %1 B").arg(recv_byte_count_));
  // message_view_->scrollToBottom();
  qDebug() << "data: " << data;
  showMessage(data, false);
}

void UdpWindow::sendMessageToPort() {
  QString editorText = editor_->text();
  if (editor_->text().isEmpty()) {
    return;
  }
  // radio button 切换发送类型
  // 适用于 分包机制
  sendMessage(editorText, send_type_);
}

void UdpWindow::sendMessageToPort(const QString &data) {
  qDebug() << "udp send" << data;
  // 原有的
  // 心跳不需要添加
  // 平均有 10ms 的延时
  // 如果有分包, 需要应用分包, 否则直接发送
  sendMessage(data);
}

void UdpWindow::sendMessageToPort(const QString &data, int time) {
  QTimer::singleShot(time, Qt::TimerType::PreciseTimer, this, [this, data] {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                              1500, this);
      return;
    }
    if (role_ == TtProtocolType::Client) {
      udp_client_->sendMessage(data.toUtf8());
    } else {
      // udp_server_->sendMessageToClients(data.toUtf8());
      udp_server_->sendMessage(data.toUtf8());
      ;
    }
    showMessage(data);
  });
}

void UdpWindow::init() {
  initUi();
  if (role_ == TtProtocolType::Client) {
    udp_client_ = new Core::UdpClient;
    title_->setText(tr("未命名的 UDP 连接"));
    udp_client_setting_ = new Widget::UdpClientSetting(this);
    serRightWidget(udp_client_setting_);
  } else if (role_ == TtProtocolType::Server) {
    udp_server_ = new Core::UdpServer;
    title_->setText(tr("未命名的 UDP 模拟服务"));
    udp_server_setting_ = new Widget::UdpServerSetting(this);
    serRightWidget(udp_server_setting_);
  }
}

void UdpWindow::connectSignals() {
  initSignalsConnection();

  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &UdpWindow::saveSetting);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this]() {
    if (opened_) {
      if (role_ == TtProtocolType::Client) {
        udp_client_->close();
      } else if (role_ == TtProtocolType::Server) {
        udp_server_->close();
      }
      opened_ = false;
      on_off_btn_->setChecked(false);
      send_package_timer_->stop();
      heartbeat_timer_->stop();
      msg_queue_.clear();
    } else {
      if (role_ == TtProtocolType::Client) {
        // 客户端
        udp_client_->connectToOther(
            udp_client_setting_->getUdpClientConfiguration());
      } else if (role_ == TtProtocolType::Server) {
        // 服务端
        if (udp_server_->listen(
                udp_server_setting_->getUdpServerConfiguration())) {
          opened_ = true;
        } else {
          opened_ = false;
        }
      }
      opened_ = true;

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
          qOverload<>(&UdpWindow::sendMessageToPort));

  if (role_ == TtProtocolType::Client) {
    connect(udp_client_setting_,
            &Widget::UdpClientSetting::sendPackageMaxSizeChanged, this,
            [this](const uint16_t size) {
              if (package_size_ != size) {
                package_size_ = size;
                qDebug() << "packageSize: " << package_size_;
              }
            });
    connect(udp_client_setting_, &Widget::FrameSetting::settingChanged, this,
            [this]() {
              qDebug() << "changed savestate";
              setSaveStatus(false);
            });

    connect(udp_client_setting_,
            &Widget::FrameSetting::sendPackageIntervalChanged, this,
            [this](const uint32_t &interval) {
              qDebug() << "interval: " << interval;
              send_package_timer_->setInterval(interval);
            });

  } else if (role_ == TtProtocolType::Server) {
    connect(udp_server_setting_, &Widget::FrameSetting::settingChanged, this,
            [this]() {
              // qDebug() << "saved Changed false";
              // saved_ = false;
              setSaveStatus(false);
            });
  }

  connect(send_package_timer_, &QTimer::timeout, this, [this] {
    if (!opened_) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("端口未链接"),
                              1500, this);
      msg_queue_.clear();
      return;
    }
    if (!msg_queue_.isEmpty()) {
      QString package = msg_queue_.dequeue();
      send_byte_count_ += package.size();
      QByteArray dataUtf8;
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
        dataUtf8 = package.toUtf8();
      }
      if (!dataUtf8.isEmpty()) {
        if (role_ == TtProtocolType::Client) {
          udp_client_->sendMessage(dataUtf8);
        } else if (role_ == TtProtocolType::Server) {
          // tcp_server_->sendMessageToClients(dataUtf8);
        }

        send_byte_count_ += dataUtf8.size();
        send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
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
              &UdpWindow::sendInstructionTableContent));
  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const std::vector<Data::MsgInfo> &msgs) {
            if (msgs.size() == 0) {
              return;
            }
            foreach (const auto &msg, msgs) {
              sendInstructionTableContent(msg);
            }
          });
}

void UdpWindow::sendMessage(const QString &data, TtTextFormat::Type type) {
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
    processedText = hexStr;
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
      udp_client_->sendMessage(dataUtf8);
    } else if (role_ == TtProtocolType::Server) {
      udp_server_->sendMessage(dataUtf8);
    }

    showMessage(dataUtf8, true);
  }
}

void UdpWindow::setControlState(bool state) {
  if (state) {
    if (role_ == TtProtocolType::Client) {
      udp_client_setting_->setControlState(true);
    } else if (role_ == TtProtocolType::Server) {
      udp_server_setting_->setControlState(true);
    }
    instruction_table_->setEnabled(true);
    send_btn_->setEnabled(false);
  } else {
    if (role_ == TtProtocolType::Client) {
      udp_client_setting_->setControlState(false);
    } else if (role_ == TtProtocolType::Server) {
      udp_server_setting_->setControlState(false);
    }
    instruction_table_->setEnabled(false);
    send_btn_->setEnabled(true);
  }
}

void UdpWindow::setHeartbeartContent() {
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

void UdpWindow::sendInstructionTableContent(const QString &text,
                                            TtTextFormat::Type type,
                                            uint32_t time) {
  QByteArray dataUtf8;
  if (type == TtTextFormat::TEXT) {
    dataUtf8 = text.toUtf8();
  } else if (type == TtTextFormat::HEX) {
    QString hexStr = QString(text);
    hexStr.remove(QRegularExpression("[^0-9A-Fa-f]"));
    // QString hexStr = text.remove(QRegularExpression("[^0-9A-Fa-f]"));

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

void UdpWindow::sendInstructionTableContent(const Data::MsgInfo &msg) {
  sendInstructionTableContent(msg.text, msg.type, msg.time);
}

void UdpWindow::sendPackagedData(const QByteArray &data, bool isHeartbeat) {
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

      // QMetaObject::invokeMethod(serial_port_, "sendData",
      // Qt::QueuedConnection,
      //                           Q_ARG(QByteArray, chunk));
      // 更新统计
      send_byte_count_ += chunk.size();
      send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
      // 显示消息
      showMessage(chunk, true);
    }
  } else {
    // 不分包，直接发送
    // QMetaObject::invokeMethod(serial_port_, "sendData",
    // Qt::QueuedConnection,
    //                           Q_ARG(QByteArray, data));
    // 更新统计
    send_byte_count_ += data.size();
    send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
    // 显示消息
    showMessage(data, true);
  }
}

bool UdpWindow::isEnableHeartbeart() {
  // 使能了当前的条件
  if (!heartbeat_.isEmpty() && heartbeat_interval_ != 0) {
    return true;
  }
  return false;
}

} // namespace Window
