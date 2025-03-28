#ifndef WINDOW_MODBUS_WINDOW_H
#define WINDOW_MODBUS_WINDOW_H

#include <QModbusDataUnit>
#include <QTimer>
#include <QWidget>

#include <ui/controls/TtModbusPlot.h>
#include "Def.h"
#include "qcustomplot/qcustomplot.h"

#include <ui/controls/TtQCustomPlot.h>

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

namespace Ui {

class TtModbusTableWidget;
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class TtVerticalLayout;
class TtLineEdit;
class TtChatView;
class TtChatMessageModel;
class TtMaskWidget;
}  // namespace Ui

namespace Widget {
class ModbusClientSetting;
}  // namespace Widget

namespace Core {
class ModbusMaster;
}  // namespace Core

namespace Window {

class ModbusWindow : public QWidget {
  Q_OBJECT
 public:
  enum class RegisterType {
    Coil,
    HoldingRegister,
    InputRegisters,
  };

  explicit ModbusWindow(TtProtocolType::ProtocolRole role,
                        QWidget* parent = nullptr);
  ~ModbusWindow();

  QString getTitle() const;
  QJsonObject getConfiguration() const;

 signals:
  void requestSaveConfig();

 private slots:
  void switchToEditMode();
  void switchToDisplayMode();
  // void sloveDataReceived(const QVector<quint16>& data);
  // void sloveDataReceived(const int& addr, const QVector<quint16>& data);
  void sloveDataReceived(const QModbusDataUnit& dataUnit);

  void timerRefreshValue();
  void getSpecificValue();
  void getHoldingRegisterValue();
  void getCoilValue();
  void getDiscreteInputsValue();
  void getInputRegistersValue();

 private:
  void init();
  void connectSignals();
  void updatePlot(TtModbusRegisterType::Type type, const int& addr,
                  const double& value1);

  QWidget* createCoilWidget();
  QWidget* createDiscreteInputsWidget();
  QWidget* createHoldingRegisterWidget();
  QWidget* createInputRegisterWidget();

  ModbusPlot* customPlot;
  QVector<double> xData, yData;
  double lastPointKey;

  QTabWidget* function_selection_;

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtNormalLabel* title_;           // 名称
  Ui::TtSvgButton* modify_title_btn_;  // 修改连接名称
  Ui::TtSvgButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;        // 开启 or 关闭
  Ui::TtSvgButton* refresh_btn_;

  Widget::ModbusClientSetting* modbus_client_setting_;

  QWidget* original_widget_ = nullptr;
  QWidget* edit_widget_ = nullptr;
  Ui::TtLineEdit* title_edit_ = nullptr;
  QStackedWidget* stack_ = nullptr;

  Ui::TtSvgButton* subscriptionBtn;

  TtProtocolType::ProtocolRole role_;

  Core::ModbusMaster* modbus_master_;

  Ui::TtModbusTableWidget* coil_table_;
  Ui::TtModbusTableWidget* discrete_inputs_table_;
  Ui::TtModbusTableWidget* holding_registers_table_;
  Ui::TtModbusTableWidget* input_registers_table_;

  QJsonObject config_;
  QTimer refresh_timer_;
};

}  // namespace Window

#endif  // MODBUS_WINDOW_H
