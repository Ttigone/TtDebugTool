#ifndef WIDGET_TCP_SETTING_H
#define WIDGET_TCP_SETTING_H

#include <QPropertyAnimation>

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
}  // namespace Ui

namespace Core {
struct TcpServerConfiguration;
struct TcpClientConfiguration;
}  // namespace Core

namespace Widget {

class TcpServerSetting : public QWidget {
  Q_OBJECT

 public:
  TcpServerSetting(QWidget* parent = nullptr);
  ~TcpServerSetting();

  Core::TcpServerConfiguration getTcpServerConfiguration();

  const QJsonObject& getTcpServerSetting();

 public slots:
  void setHost();
  void setPort();

 private:
  void setHostAddress();

  QVBoxLayout* main_layout_;

  Ui::TtLabelComboBox* host_;
  Ui::TtLabelLineEdit * port_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;

  Ui::TtLabelBtnComboBox* retransmission_;

  QJsonObject tcp_server_save_config_;
};

class TcpClientSetting : public QWidget {
  Q_OBJECT

 public:
  TcpClientSetting(QWidget* parent = nullptr);
  ~TcpClientSetting();

  Core::TcpClientConfiguration getTcpClientConfiguration();

  const QJsonObject& getTcpClientSetting();

 private:
  void setHostAddress();

  QVBoxLayout* main_layout_;
  Ui::TtLabelComboBox* target_host_;
  Ui::TtLabelLineEdit* target_port_;
  Ui::TtLabelLineEdit* self_host_;
  Ui::TtLabelLineEdit* self_port_;

  Ui::TtLabelLineEdit* send_packet_interval_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;

  Ui::TtLabelBtnComboBox* retransmission_;

  Ui::TtLabelComboBox* heartbeat_send_type_;
  Ui::TtLabelComboBox* heartbeat_interval_;
  Ui::TtLabelComboBox* heartbeat_content_;

  QJsonObject tcp_client_save_config_;
};

}  // namespace Widget

#endif  // WINDOW_SERIAL_SETTING_H
