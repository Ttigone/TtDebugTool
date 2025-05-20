#ifndef WINDOW_UDP_WINDOW_H
#define WINDOW_UDP_WINDOW_H

// #include <Qsci/qsciscintilla.h>
#include <Qscintilla/include/Qsci/qsciscintilla.h>

#include "Def.h"
#include "data/communication_metadata.h"
#include "window/frame_window.h"

QT_BEGIN_NAMESPACE
class QWidget;
class QStackedWidget;
QT_END_NAMESPACE

namespace Ui {

class TtTableWidget;
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;
class TtLineEdit;

class TtChatView;
class TtChatMessageModel;
} // namespace Ui

namespace Widget {
class UdpServerSetting;
class UdpClientSetting;
} // namespace Widget

namespace Core {
class UdpServer;
class UdpClient;
} // namespace Core

namespace Window {

class UdpWindow : public FrameWindow {
  Q_OBJECT
public:
  explicit UdpWindow(TtProtocolType::ProtocolRole role,
                     QWidget *parent = nullptr);
  ~UdpWindow();

  QString getTitle() const;
  QJsonObject getConfiguration() const;

  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;

  Q_INVOKABLE void saveSetting() override;
  Q_INVOKABLE void setSetting(const QJsonObject &config) override;

signals:
  void requestSaveConfig();

private slots:
  void updateServerStatus();
  void dataReceived(const QByteArray &data);

  ///
  /// @brief sendMessageToPort
  /// editor 编辑器发送编辑数据
  void sendMessageToPort();
  ///
  /// @brief sendMessageToPort
  /// @param data
  /// 立刻发送 data
  /// 用于发送心跳内容, 发送指令表格的内容 sendRowMsg 信号
  void sendMessageToPort(const QString &data);

  ///
  /// @brief sendMessageToPort
  /// @param data
  /// @param times
  /// 在 times 时间后发送 data
  void sendMessageToPort(const QString &data, const int &times);

private:
  void init();
  void connectSignals();

  void sendMessage(const QString &data,
                   TtTextFormat::Type type = TtTextFormat::TEXT);

  ///
  /// @brief setControlState
  /// @param state
  /// 设置主界面控件状态
  void setControlState(bool state);

  ///
  /// @brief setHeartbeartContent
  /// 发送心跳内容
  void setHeartbeartContent();

  ///
  /// @brief sendInstructionTableContent
  /// @param text 消息本体
  /// @param type   类型
  /// @param times  间隔时间
  /// 发送表格的内容
  void sendInstructionTableContent(const QString &text, TtTextFormat::Type type,
                                   uint32_t times);
  ///
  /// @brief sendInstructionTableContent
  /// @param msg
  /// 构造 MsgInfo
  void sendInstructionTableContent(const Data::MsgInfo &msg);

  ///
  /// @brief sendPackagedData
  /// @param data
  /// @param isHeartbeat
  /// 处理分包发送
  void sendPackagedData(const QByteArray &data, bool isHeartbeat = false);

  bool isEnableHeartbeart();

  Widget::UdpServerSetting *udp_server_setting_{nullptr};
  Widget::UdpClientSetting *udp_client_setting_{nullptr};

  TtProtocolType::ProtocolRole role_;
  Core::UdpClient *udp_client_{nullptr};
  Core::UdpServer *udp_server_{nullptr};
};

} // namespace Window

#endif // WINDOW_UDP_WINDOW_H
