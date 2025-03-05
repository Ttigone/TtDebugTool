#ifndef TCP_WINDOW_H
#define TCP_WINDOW_H

#include <Qsci/qsciscintilla.h>

#include "Def.h"

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

namespace Ui {
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;
class TtLineEdit;

class TtChatView;
class TtChatMessageModel;
}  // namespace Ui

namespace Widget {
class TcpServerSetting;
class TcpClientSetting;
}  // namespace Widget

namespace Core {
class TcpServer;
class TcpClient;
}  // namespace Core

namespace Window {

class TcpWindow : public QWidget {
  Q_OBJECT
 public:
  explicit TcpWindow(TtProtocolType::ProtocolRole role,
                     QWidget* parent = nullptr);

  QString getTitle();

 signals:
  void requestSaveConfig();
  void requestSendMessage(const QByteArray& data);

 private slots:
  void switchToEditMode();
  void switchToDisplayMode();
  // 更新服务端状态
  void updateServerStatus();
  void onDataReceived(const QByteArray& data);

 private:
  void init();
  void connectSignals();

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtNormalLabel* title_;             // 名称
  // Ui::TtImageButton* modify_title_btn_;  // 修改连接名称
  Ui::TtSvgButton* modify_title_btn_;    // 修改连接名称
  // Ui::TtImageButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* save_btn_;    // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;  // 开启 or 关闭

  // 消息展示框
  Ui::TtChatView* message_view_;
  // 数据
  Ui::TtChatMessageModel* message_model_;

  Core::TcpClient* tcp_client_{nullptr};
  Core::TcpServer* tcp_server_{nullptr};

  Widget::TcpServerSetting* tcp_server_setting_{nullptr};
  Widget::TcpClientSetting* tcp_client_setting_{nullptr};

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  // 使用开源编辑组件 QScintilla
  QsciScintilla* editor;

  QWidget* original_widget_{nullptr};
  QWidget* edit_widget_{nullptr};
  Ui::TtLineEdit* title_edit_{nullptr};
  QStackedWidget* stack_{nullptr};

  bool tcp_opened_{false};

  TtProtocolType::ProtocolRole role_;
};

} // namespace Window


#endif  // TCP_WINDOW_H
