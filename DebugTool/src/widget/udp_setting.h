#ifndef WIDGET_UDP_SETTING_H
#define WIDGET_UDP_SETTING_H

#include <QWidget>

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
class TtLabelLineEdit;
class TtCheckBox;
class TtVerticalLayout;
class TtRadioButton;
} // namespace Ui

namespace Core {
struct UdpServerConfiguration;
struct UdpClientConfiguration;
} // namespace Core

namespace Widget {

class UdpServerSetting : public QWidget {
  Q_OBJECT
public:
  explicit UdpServerSetting(QWidget *parent = nullptr);
  ~UdpServerSetting();

  Core::UdpServerConfiguration getUdpServerConfiguration();
  const QJsonObject &getUdpServerSetting();

  void setOldSettings(const QJsonObject &config);
  const QJsonObject &getSerialSetting();

signals:

private:
  QVBoxLayout *main_layout_;

  Ui::TtLabelLineEdit *self_ip_;
  Ui::TtLabelLineEdit *self_port_;

  Ui::TtLabelComboBox *framing_model_;

  Ui::TtLabelLineEdit *framing_timeout_;
  Ui::TtLabelLineEdit *framing_fixed_length_;

  Ui::TtLabelBtnComboBox *retransmission_;

  QJsonObject udp_server_save_config_;
};

class UdpClientSetting : public QWidget {
  Q_OBJECT
public:
  explicit UdpClientSetting(QWidget *parent = nullptr);
  ~UdpClientSetting();

  Core::UdpClientConfiguration getUdpClientConfiguration();

  const QJsonObject &getUdpClientSetting();

  void setOldSettings(const QJsonObject &config);

signals:

private:
  void setLinkMode();

  QVBoxLayout *main_layout_;

  Ui::TtLabelComboBox *mode_; // 模式
  Ui::TtLabelLineEdit *target_ip_;
  Ui::TtLabelLineEdit *target_port_;
  Ui::TtLabelLineEdit *self_ip_;
  Ui::TtLabelLineEdit *self_port_;

  Ui::TtLabelLineEdit *send_packet_interval_;

  Ui::TtLabelComboBox *framing_model_;
  Ui::TtLabelLineEdit *framing_timeout_;
  Ui::TtLabelLineEdit *framing_fixed_length_;

  Ui::TtLabelBtnComboBox *retransmission_;

  QJsonObject udp_client_save_config_;
};

} // namespace Widget

#endif // WIDGET_UDP_SETTING_H
