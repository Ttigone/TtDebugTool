#ifndef WIDGET_SERIAL_SETTING_H
#define WIDGET_SERIAL_SETTING_H

#include "core/serial_port.h"

namespace Ui {
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

  ///
  /// @brief setOldSettings
  /// 配置为以前旧的配置项
  void setOldSettings();

  ///
  /// @brief getSerialPortConfiguration 获取当前配置
  /// @return
  ///
  Core::SerialPortConfiguration getSerialPortConfiguration();

  ///
  /// @brief defaultSerialPortConfiguration 默认配置, 在初始化的时候使用
  /// @return
  ///
  Core::SerialPortConfiguration defaultSerialPortConfiguration();

  void displayDefaultSetting();

  const QJsonObject& getSerialSetting();

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
  void refreshSerialCOMx();
  QString matchingSerialCOMx(const QString& name);

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtLabelBtnComboBox* serial_port_;
  Ui::TtLabelComboBox* baud_rate_;
  Ui::TtLabelComboBox* data_bit_;
  Ui::TtLabelComboBox* parity_bit_;
  Ui::TtLabelComboBox* stop_bit_;
  Ui::TtLabelComboBox* flow_control_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;


  Ui::TtLabelComboBox* line_break_;

  Ui::TtLabelComboBox* heartbeat_send_type_;
  Ui::TtLabelComboBox* heartbeat_interval_;
  Ui::TtLabelComboBox* heartbeat_content_;

  bool has_old_settings;

  QJsonObject serial_save_config_;
};

} // namespace Window


#endif // WINDOW_SERIAL_SETTING_H
