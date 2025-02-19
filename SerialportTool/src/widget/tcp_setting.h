#ifndef WIDGET_TCP_SETTING_H
#define WIDGET_TCP_SETTING_H

namespace Ui {
class TtLabelComboBox;
class TtLabelLineEdit;
}  // namespace Ui

namespace Widget {

class TcpServerSetting : public QWidget {
  Q_OBJECT

 public:
  TcpServerSetting(QWidget* parent = nullptr);
  ~TcpServerSetting();

 private:
  Ui::TtLabelComboBox* host_;
  Ui::TtLabelLineEdit * port_;
};

class TcpClientSetting : public QWidget {
  Q_OBJECT

 public:
  TcpClientSetting(QWidget* parent = nullptr);
  ~TcpClientSetting();
};

}  // namespace Widget

#endif  // WINDOW_SERIAL_SETTING_H
