#ifndef WIDGET_SERIAL_SETTING_H
#define WIDGET_SERIAL_SETTING_H

#include "core/serial_port.h"

// #include
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

 public Q_SLOTS:
  void setSerialPortsName();
  void setSerialPortsBaudRate();
  void setSerialPortsDataBit();
  void setSerialPortsParityBit();
  void setSerialPortsStopBit();
  void setSerialPortsFluidControl();

 private:
  void init();
  void refreshSerialCOMx();
  QString matchingSerialCOMx(const QString& name);

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtLabelBtnComboBox* select_serial_port_;
  Ui::TtLabelComboBox* select_baud_rate_;
  Ui::TtLabelComboBox *select_data_bit_;
  Ui::TtLabelComboBox *select_parity_bit_;
  Ui::TtLabelComboBox *select_stop_bit_;
  Ui::TtLabelComboBox *select_fluid_control_;

  Ui::TtLabelComboBox* framing_model_;
  Ui::TtLabelComboBox* framing_timeout_;
  Ui::TtLabelComboBox* framing_fixed_length_;


  Ui::TtLabelComboBox* line_break_;

  Ui::TtLabelComboBox* heartbeat_send_type_;
  Ui::TtLabelComboBox* heartbeat_interval_;
  Ui::TtLabelComboBox* heartbeat_content_;

  bool has_old_settings;

  QList<int64> list_baud_rate_;
  QList<uint8> list_data_bits_;
  QMap<QString, uint8> map_parity_;
  QMap<QString, uint8> map_stop_bits_;
  QMap<QString, uint8> map_flow_control_;

  QJsonObject serial_save_config_;
};

} // namespace Window


#endif // WINDOW_SERIAL_SETTING_H
