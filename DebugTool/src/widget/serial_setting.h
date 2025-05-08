#ifndef WIDGET_SERIAL_SETTING_H
#define WIDGET_SERIAL_SETTING_H

#include "core/serial_port.h"

namespace Ui {

class TtLabelLineEdit;
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtVerticalLayout;
} // namespace Ui

namespace Core {
struct SerialPortConfiguration;
} // namespace Core

namespace Widget {

// 添加一个打开 drawer 的信号, 检测 当前的 widget 宽度, 外部使用改变
class SerialSetting : public QWidget {
  Q_OBJECT
public:
  SerialSetting(QWidget *parent = nullptr);
  ~SerialSetting();

  Core::SerialPortConfiguration getSerialPortConfiguration();
  Core::SerialPortConfiguration defaultSerialPortConfiguration();

  void displayDefaultSetting();

  void setOldSettings(const QJsonObject &config);
  const QJsonObject &getSerialSetting();

signals:
  void drawerStateChanged(bool state);

signals:
  void settingChanged();
  void showScriptSetting();
  void sendPackageMaxSizeChanged(uint16_t size);
  void sendPackageIntervalChanged(uint16_t interval);
  void heartbeatInterval(uint32_t interval);
  void heartbeatContentChanged(QString content);

public slots:
  void setSerialPortsName();
  void setSerialPortsBaudRate();
  void setSerialPortsDataBit();
  void setSerialPortsParityBit();
  void setSerialPortsStopBit();
  void setSerialPortsFluidControl();

  void setControlState(bool state);
  quint32 getRefreshInterval();

private:
  void init();
  void connnectSignals();
  void refreshSerialCOMx();
  QString matchingSerialCOMx(const QString &name);

  Ui::TtVerticalLayout *main_layout_;

  Ui::TtLabelBtnComboBox *serial_port_;
  Ui::TtLabelComboBox *baud_rate_;
  Ui::TtLabelComboBox *data_bit_;
  Ui::TtLabelComboBox *parity_bit_;
  Ui::TtLabelComboBox *stop_bit_;
  Ui::TtLabelComboBox *flow_control_;

  Ui::TtLabelLineEdit *send_package_interval_;
  Ui::TtLabelLineEdit *send_package_max_size_;

  Ui::TtLabelLineEdit *script_;

  Ui::TtLabelComboBox *framing_model_;
  Ui::TtLabelComboBox *framing_timeout_;
  Ui::TtLabelComboBox *framing_fixed_length_;

  Ui::TtLabelComboBox *line_break_;

  Ui::TtLabelComboBox *heartbeat_send_type_;
  Ui::TtLabelLineEdit *heartbeat_interval_;
  Ui::TtLabelLineEdit *heartbeat_content_;

  bool has_old_settings;

  QJsonObject config_;
};

} // namespace Widget

#endif // WINDOW_SERIAL_SETTING_H
