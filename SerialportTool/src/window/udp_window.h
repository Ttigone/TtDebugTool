#ifndef UDP_WINDOW_H
#define UDP_WINDOW_H

#include <Qsci/qsciscintilla.h>

#include "Def.h"

QT_BEGIN_NAMESPACE
class QWidget;
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
class UdpServerSetting;
class UdpClientSetting;
}  // namespace Widget

namespace Core {
class UdpServer;
class UdpClient;
}  // namespace Core

namespace Window {

class UdpWindow : public QWidget {
  Q_OBJECT
 public:
  explicit UdpWindow(TtProtocolType::ProtocolRole role,
                     QWidget* parent = nullptr);

  QString getTitle();

 signals:
  void requestSaveConfig();

 private slots:
  void switchToEditMode();
  void switchToDisplayMode();
  void updateServerStatus();
  void onDataReceived(const QByteArray& data);

 private:
  void init();
  void connectSignals();

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtNormalLabel* title_;  // 名称
  // Ui::TtImageButton* modify_title_btn_;  // 修改连接名称
  Ui::TtSvgButton* modify_title_btn_;  // 修改连接名称
  // Ui::TtImageButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* save_btn_;    // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;  // 开启 or 关闭

  // 消息展示框
  Ui::TtChatView* message_view_;
  // 数据
  Ui::TtChatMessageModel* message_model_;

  Core::UdpClient* udp_client_{nullptr};
  Core::UdpServer* udp_server_{nullptr};

  Widget::UdpServerSetting* udp_server_setting_{nullptr};
  Widget::UdpClientSetting* udp_client_setting_{nullptr};

  Ui::TtNormalLabel* send_byte;
  Ui::TtNormalLabel* recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  QsciScintilla* editor;

  QWidget* original_widget_{nullptr};
  QWidget* edit_widget_{nullptr};
  Ui::TtLineEdit* title_edit_{nullptr};
  QStackedWidget* stack_{nullptr};

  bool opened_{false};

  TtProtocolType::ProtocolRole role_;
};

} // namespace Window


#endif  // UDP_WINDOW_H
