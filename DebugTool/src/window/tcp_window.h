#ifndef WINDOW_TCP_WINDOW_H
#define WINDOW_TCP_WINDOW_H

// #include <Qsci/qsciscintilla.h>
#include <Qscintilla/include/Qsci/qsciscintilla.h>

#include "Def.h"
#include "window/frame_window.h"

#include <QPlainTextEdit>
#include <QQueue>
#include <QtMaterialFlatButton.h>

#include "data/communication_metadata.h"
#include <ui/controls/TtLuaInputBox.h>

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

class QSplitter;
namespace Ui {

class TtRadioButton;
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
class TcpServerSetting;
class TcpClientSetting;
class FrameSetting;
} // namespace Widget

namespace Core {
class TcpServer;
class TcpClient;
} // namespace Core

namespace Window {

class TcpWindow : public FrameWindow {
  Q_OBJECT
public:
  explicit TcpWindow(TtProtocolType::ProtocolRole role,
                     QWidget *parent = nullptr);
  ~TcpWindow();

  QString getTitle() const;
  QJsonObject getConfiguration() const;
  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;
  Q_INVOKABLE void saveSetting() override;
  Q_INVOKABLE void setSetting(const QJsonObject &config) override;

signals:
  void requestSaveConfig();
  void requestSendMessage(const QByteArray &data);

private slots:
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

  void updateServerStatus();
  ///
  /// @brief dataReceived
  /// @param data
  /// 数据接收
  void dataReceived(const QByteArray &data);

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

private:
  void init();
  void connectSignals();

  ///
  /// @brief setControlState
  /// @param state
  /// 设置主界面控件状态
  void setControlState(bool state);

  void sendMessage(const QString &data,
                   TtTextFormat::Type type = TtTextFormat::TEXT);

  // // 文本发送 hex 格式 并显示
  // // 数据接收显示 hex
  // void showMessage(const QByteArray &data, bool out = true); // 为 hex
  // 进制提供
  // // 分开
  // void showMessage(const QString &data, bool out = true);

  bool isEnableHeartbeart();

  ///
  /// @brief sendPackagedData
  /// @param data
  /// @param isHeartbeat
  /// 处理分包发送
  void sendPackagedData(const QByteArray &data, bool isHeartbeat = false);

  Core::TcpClient *tcp_client_{nullptr};
  Core::TcpServer *tcp_server_{nullptr};

  // 只需要保存一个
  Widget::TcpServerSetting *tcp_server_setting_{nullptr};
  Widget::TcpClientSetting *tcp_client_setting_{nullptr};
  // Widget::FrameSetting *setting_{nullptr};

  TtProtocolType::ProtocolRole role_;
  Ui::TtLuaInputBox *lua_code_;
};

} // namespace Window

#endif // TCP_WINDOW_H
