#ifndef WIDGET_TCP_SETTING_H
#define WIDGET_TCP_SETTING_H

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
}  // namespace Ui

namespace Core {
struct TcpServerConfiguration;
}  // namespace Core

namespace Widget {

class TcpServerSetting : public QWidget {
  Q_OBJECT

 public:
  TcpServerSetting(QWidget* parent = nullptr);
  ~TcpServerSetting();

  Core::TcpServerConfiguration getTcpServerConfiguration();

 public slots:
  void setHost();
  void setPort();

 private:
  QVBoxLayout* main_layout_;
  Ui::TtLabelComboBox* host_;
  Ui::TtLabelLineEdit * port_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;

  // Ui::TtLabelComboBox *retransmission_;
  Ui::TtLabelBtnComboBox* retransmission_;
};

class TcpClientSetting : public QWidget {
  Q_OBJECT

 public:
  TcpClientSetting(QWidget* parent = nullptr);
  ~TcpClientSetting();
};

}  // namespace Widget

#endif  // WINDOW_SERIAL_SETTING_H
