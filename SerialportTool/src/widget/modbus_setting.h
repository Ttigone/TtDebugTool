#ifndef WIDGET_MODBUS_SETTING_H
#define WIDGET_MODBUS_SETTING_H

#include <QWidget>

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
class TtCheckBox;
class TtSwitchButton;
class TtVerticalLayout;
class TtLineEdit;
}  // namespace Ui

namespace Core {
struct ModbusMasterConfiguration;
}  // namespace Core

namespace Widget {

class ModbusClientSetting : public QWidget {
  Q_OBJECT
 public:
  explicit ModbusClientSetting(QWidget* parent = nullptr);
  ~ModbusClientSetting();

  Core::ModbusMasterConfiguration getModbusClientConfiguration();
  const QJsonObject& getModbusClientSetting();

 public slots:
  void setLinkType();
  void setSerialPortsName();
  void setSerialPortsBaudRate();
  void setSerialPortsDataBit();
  void setSerialPortsParityBit();
  void setSerialPortsStopBit();

  void setControlState(bool state);

 private:
  void init();
  Ui::TtVerticalLayout* main_layout_;

  Ui::TtLabelComboBox* link_type_;

  Ui::TtLabelBtnComboBox* path_;
  Ui::TtLabelComboBox* baud_;
  Ui::TtLabelComboBox* data_bit_;
  Ui::TtLabelComboBox* parity_bit_;
  Ui::TtLabelComboBox* stop_bit_;

  Ui::TtLabelLineEdit* host_;
  Ui::TtLabelLineEdit* port_;

  Ui::TtLabelLineEdit* device_id_;
  Ui::TtLabelLineEdit* timeout_;
  Ui::TtSwitchButton* auto_refresh_;
  Ui::TtLabelLineEdit* refresh_interval_;

  Ui::TtLabelLineEdit* topic_;
  Ui::TtLabelLineEdit* load_;
  Ui::TtLabelComboBox* qos_;
  Ui::TtCheckBox* retain_;

  QMap<QString, uint8> map_parity_;
  QMap<QString, uint8> map_stop_bits_;
  QMap<QString, uint8> map_flow_control_;

  QJsonObject modbus_client_save_config_;
};

}  // namespace Widget

#endif  // MODBUS_SETTING_H
