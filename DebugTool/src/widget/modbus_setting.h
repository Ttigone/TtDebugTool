#ifndef WIDGET_MODBUS_SETTING_H
#define WIDGET_MODBUS_SETTING_H

#include "Def.h"
#include <QStyledItemDelegate>
#include <QWidget>

namespace Ui {
class TtLabelComboBox;
class TtLabelBtnComboBox;
class TtLabelLineEdit;
class TtCheckBox;
class TtSwitchButton;
class TtVerticalLayout;
class TtLineEdit;
} // namespace Ui

namespace Core {
struct ModbusMasterConfiguration;
} // namespace Core

namespace Widget {

class ModbusClientSetting : public QWidget {
  Q_OBJECT
public:
  explicit ModbusClientSetting(QWidget *parent = nullptr);
  ~ModbusClientSetting();

  Core::ModbusMasterConfiguration getModbusClientConfiguration();

  void setOldSettings(const QJsonObject &config);

  void setModbusClientSetting(const QJsonObject &obj);
  const QJsonObject &getModbusClientSetting();

  TtModbusProcotol::Type getModbusLinkType() const;
  int getModbusDeviceId() const;

signals:
  void drawerStateChanged(bool state);

signals:
  void settingChanged();
  void autoRefreshStateChanged(bool enable);
  void refreshIntervalChanged(uint32_t interval);
  void graphNumsChanged(quint16 nums);

public slots:
  void setLinkType();
  void setSerialPortsName();
  void setSerialPortsBaudRate();
  void setSerialPortsDataBit();
  void setSerialPortsParityBit();
  void setSerialPortsStopBit();

  void setControlState(bool state);
  quint32 getRefreshInterval();

private:
  void init();
  Ui::TtVerticalLayout *main_layout_;

  Ui::TtLabelComboBox *link_type_;

  Ui::TtLabelBtnComboBox *path_;
  Ui::TtLabelComboBox *baud_;
  Ui::TtLabelComboBox *data_bit_;
  Ui::TtLabelComboBox *parity_bit_;
  Ui::TtLabelComboBox *stop_bit_;

  Ui::TtLabelLineEdit *host_;
  Ui::TtLabelLineEdit *port_;

  Ui::TtLabelLineEdit *device_id_;
  Ui::TtLabelLineEdit *timeout_;
  Ui::TtSwitchButton *auto_refresh_;
  Ui::TtLabelLineEdit *refresh_interval_;

  Ui::TtLabelLineEdit *graph_capacity_;

  QJsonObject modbus_client_save_config_;
};

} // namespace Widget

#endif // MODBUS_SETTING_H
