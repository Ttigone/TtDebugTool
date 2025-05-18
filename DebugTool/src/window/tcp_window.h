#ifndef WINDOW_TCP_WINDOW_H
#define WINDOW_TCP_WINDOW_H

#include <Qsci/qsciscintilla.h>

#include "Def.h"
#include "window/frame_window.h"

#include <QPlainTextEdit>
#include <QQueue>
#include <QtMaterialFlatButton.h>

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

  ///
  /// @brief setHeartbeartContent
  /// 发送心跳内容
  void setHeartbeartContent();
  void updateServerStatus();
  ///
  /// @brief dataReceived
  /// @param data
  /// 数据接收
  void dataReceived(const QByteArray &data);

private:
  void init();
  void connectSignals();
  void sendMessage(const QString &data,
                   TtTextFormat::Type type = TtTextFormat::TEXT);
  // 文本发送 hex 格式 并显示
  // 数据接收显示 hex
  void showMessage(const QByteArray &data, bool out = true); // 为 hex 进制提供
  // 分开
  void showMessage(const QString &data, bool out = true);
  bool isEnableHeartbeart();

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
