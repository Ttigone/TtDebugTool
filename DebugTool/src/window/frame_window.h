#ifndef WINDOW_FRAME_WINDOW_H
#define WINDOW_FRAME_WINDOW_H

#include <QWidget>

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

class QSplitter;
namespace Window {

class FrameWindow : public QWidget {
  Q_OBJECT
public:
  explicit FrameWindow(QWidget *parent = nullptr);
  virtual ~FrameWindow();

  virtual QString title() const;
  virtual bool workState() const = 0;
  virtual bool saveState() = 0;
  virtual void setSaveState(bool state) = 0;
  Q_INVOKABLE virtual void saveSetting() = 0;
  Q_INVOKABLE virtual void setSetting(const QJsonObject &config) = 0;

  // virtual void initUi();

protected:
  // 初始状态下处于非保存状态
  bool saved_{false};

  // QSplitter *main_splitter_;

  // Ui::TtVerticalLayout *main_layout_;

  // Ui::TtNormalLabel *title_;
  // Ui::TtSvgButton *modify_title_btn_;
  // Ui::TtSvgButton *save_btn_;
  // Ui::TtSvgButton *on_off_btn_;

  // Ui::TtChatView *message_view_;
  // Ui::TtChatMessageModel *message_model_{nullptr};

  // Ui::TtTableWidget *instruction_table_{nullptr};

  // Core::TcpClient *tcp_client_{nullptr};
  // Core::TcpServer *tcp_server_{nullptr};

  // Widget::TcpServerSetting *tcp_server_setting_{nullptr};
  // Widget::TcpClientSetting *tcp_client_setting_{nullptr};

  // Ui::TtNormalLabel *send_byte{nullptr};
  // Ui::TtNormalLabel *recv_byte{nullptr};
  // quint64 send_byte_count = 0;
  // quint64 recv_byte_count = 0;
};

} // namespace Window

#endif // FRAME_WINDOW_H
