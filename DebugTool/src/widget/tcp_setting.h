#ifndef WIDGET_TCP_SETTING_H
#define WIDGET_TCP_SETTING_H

#include <QPropertyAnimation>

#include "widget/frame_setting.h"

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
} // namespace Ui

namespace Core {
struct TcpServerConfiguration;
struct TcpClientConfiguration;
} // namespace Core

namespace Widget {

class TcpServerSetting : public FrameSetting {
  Q_OBJECT

public:
  TcpServerSetting(QWidget *parent = nullptr);
  ~TcpServerSetting();

  // 返回值不同
  Core::TcpServerConfiguration getTcpServerConfiguration();
  const QJsonObject &getTcpServerSetting();

  void setOldSettings(const QJsonObject &config);
  const QJsonObject &getSerialSetting();

public slots:
  void setHost();
  void setPort();
  void setControlState(bool state);

private:
  void setHostAddress();

  QVBoxLayout *main_layout_;

  Ui::TtLabelComboBox *host_;
  Ui::TtLabelLineEdit *port_;

  Ui::TtLabelComboBox *framing_model_;
  Ui::TtLabelLineEdit *framing_timeout_;
  Ui::TtLabelLineEdit *framing_fixed_length_;

  Ui::TtLabelBtnComboBox *retransmission_;

  QJsonObject tcp_server_save_config_;
};

class TcpClientSetting : public FrameSetting {
  Q_OBJECT
public:
  TcpClientSetting(QWidget *parent = nullptr);
  ~TcpClientSetting();

  Core::TcpClientConfiguration getTcpClientConfiguration();
  const QJsonObject &getTcpClientSetting();

  void setOldSettings(const QJsonObject &config);
  const QJsonObject &getSerialSetting();

signals:

public slots:
  void setControlState(bool state);

private:
  // void init();
  void setHostAddress();

  QVBoxLayout *main_layout_;
  Ui::TtLabelComboBox *target_host_;
  Ui::TtLabelLineEdit *target_port_;
  Ui::TtLabelLineEdit *self_host_;
  Ui::TtLabelLineEdit *self_port_;

  Ui::TtLabelLineEdit *send_package_max_size_;
  Ui::TtLabelLineEdit *send_package_interval_;

  Ui::TtLabelComboBox *framing_model_;
  Ui::TtLabelLineEdit *framing_timeout_;
  Ui::TtLabelLineEdit *framing_fixed_length_;

  Ui::TtLabelBtnComboBox *retransmission_;

  Ui::TtLabelComboBox *heartbeat_send_type_;
  Ui::TtLabelLineEdit *heartbeat_interval_;
  Ui::TtLabelLineEdit *heartbeat_content_;

  QJsonObject tcp_client_save_config_;
};

} // namespace Widget

#endif // WINDOW_SERIAL_SETTING_H
