#ifndef WINDOW_TCP_WINDOW_H
#define WINDOW_TCP_WINDOW_H

#include <Qsci/qsciscintilla.h>

#include "Def.h"
// #include "frame_window.h"
#include "window/frame_window.h"

QT_BEGIN_NAMESPACE
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
class TcpServerSetting;
class TcpClientSetting;
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
  void switchToEditMode();
  void switchToDisplayMode();
  void updateServerStatus();
  void onDataReceived(const QByteArray &data);

private:
  void init();
  void connectSignals();

  Ui::TtVerticalLayout *main_layout_;

  Ui::TtNormalLabel *title_;
  Ui::TtSvgButton *modify_title_btn_;
  Ui::TtSvgButton *save_btn_;
  Ui::TtSvgButton *on_off_btn_;

  Ui::TtChatView *message_view_;
  Ui::TtChatMessageModel *message_model_;

  Ui::TtTableWidget *instruction_table_;

  Core::TcpClient *tcp_client_{nullptr};
  Core::TcpServer *tcp_server_{nullptr};

  Widget::TcpServerSetting *tcp_server_setting_{nullptr};
  Widget::TcpClientSetting *tcp_client_setting_{nullptr};

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  QsciScintilla *editor;

  QWidget *original_widget_{nullptr};
  QWidget *edit_widget_{nullptr};
  Ui::TtLineEdit *title_edit_{nullptr};
  QStackedWidget *stack_{nullptr};

  bool tcp_opened_{false};

  TtProtocolType::ProtocolRole role_;
  QJsonObject config_;
};

} // namespace Window

#endif // TCP_WINDOW_H
