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

class SerialSetting : public QWidget {
  Q_OBJECT
 public:
  SerialSetting(QWidget *parent = nullptr);
  ~SerialSetting();

  void setOldSettings();
  Core::SerialPortConfiguration getSerialPortConfiguration();
  Core::SerialPortConfiguration defaultSerialPortConfiguration();

  void displayDefaultSetting();
  const QJsonObject& getSerialSetting();

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
  QString matchingSerialCOMx(const QString& name);

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtLabelBtnComboBox* serial_port_;
  Ui::TtLabelComboBox* baud_rate_;
  Ui::TtLabelComboBox* data_bit_;
  Ui::TtLabelComboBox* parity_bit_;
  Ui::TtLabelComboBox* stop_bit_;
  Ui::TtLabelComboBox* flow_control_;

  // QComboBox* serial_port_;
  // QComboBox* baud_rate_;
  // QComboBox* data_bit_;
  // QComboBox* parity_bit_;
  // QComboBox* stop_bit_;
  // QComboBox* flow_control_;

  Ui::TtLabelLineEdit* send_package_interval_;
  Ui::TtLabelLineEdit* send_package_max_size_;

  Ui::TtLabelLineEdit* script_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;


  Ui::TtLabelComboBox* line_break_;

  Ui::TtLabelComboBox* heartbeat_send_type_;
  Ui::TtLabelLineEdit* heartbeat_interval_;
  Ui::TtLabelLineEdit* heartbeat_content_;

  bool has_old_settings;

  QJsonObject serial_save_config_;
};

} // namespace Window


#endif // WINDOW_SERIAL_SETTING_H
