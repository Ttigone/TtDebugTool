#ifndef WINDOW_MODBUS_WINDOW_H
#define WINDOW_MODBUS_WINDOW_H

#include <QModbusDataUnit>
#include <QProgressDialog>
#include <QTimer>
#include <QWidget>

#include "Def.h"
#include "window/frame_window.h"

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
class TtLabelLineEdit;

class TtModbusPlot;
} // namespace Ui

namespace Widget {
class ModbusClientSetting;
} // namespace Widget

namespace Core {
class ModbusMaster;
class ModbusWorker;
} // namespace Core

namespace Window {

// 在适当的头文件中
const QMap<TtModbusRegisterType::Type, QString> TYPE_NAMES = {
    {TtModbusRegisterType::Coils, "Coil"},
    {TtModbusRegisterType::DiscreteInputs, "DiscreteInputs"},
    {TtModbusRegisterType::HoldingRegisters, "HoldingRegisters"},
    {TtModbusRegisterType::InputRegisters, "InputRegisters"}};

class ModbusWindow : public FrameWindow {
  Q_OBJECT
public:
  enum class RegisterType {
    Coil,
    HoldingRegister,
    InputRegisters,
  };

  explicit ModbusWindow(TtProtocolType::ProtocolRole role,
                        QWidget *parent = nullptr);
  ~ModbusWindow();

  QString getTitle() const;
  QJsonObject getConfiguration() const;

  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;

  void saveSetting() override;
  void setSetting(const QJsonObject &config) override;

signals:
  void requestSaveConfig();

private slots:
  void switchToEditMode() override;
  void switchToDisplayMode() override;
  // void sloveDataReceived(const QVector<quint16>& data);
  // void sloveDataReceived(const int& addr, const QVector<quint16>& data);
  void sloveDataReceived(const QModbusDataUnit &dataUnit);

  void timerRefreshValue();
  void getSpecificValue();
  void getHoldingRegisterValue();
  void getCoilValue();
  void getDiscreteInputsValue();
  void getInputRegistersValue();

  // 在 ModbusWindow 类中添加以下方法
  void onDisconnectStarted();

  void updateDisconnectProgress(int percent);

  void onDisconnectFinished();

private:
  void init();
  void connectSignals();
  ///
  /// @brief setControlState
  /// @param state
  /// 设置主界面控件状态
  void setControlState(bool state);

  void updatePlot(TtModbusRegisterType::Type type, const int &addr,
                  const double &value1);

  // 合并以下四个函数
  QWidget *createRegisterWidget(TtModbusRegisterType::Type type,
                                const QString &title);

  void setFunctionTableState(bool state);

  QWidget *createCoilWidget();
  QWidget *createDiscreteInputsWidget();
  QWidget *createHoldingRegisterWidget();
  QWidget *createInputRegisterWidget();

  QScopedPointer<Ui::TtModbusPlot> modbus_plot_;

  QVector<double> xData, yData;
  double lastPointKey;

  QTabWidget *function_selection_;

  Ui::TtVerticalLayout *main_layout_;

  Ui::TtNormalLabel *title_{nullptr};          // 名称
  Ui::TtSvgButton *modify_title_btn_{nullptr}; // 修改连接名称
  Ui::TtSvgButton *save_btn_{nullptr};         // 保存连接记录
  Ui::TtSvgButton *on_off_btn_{nullptr};       // 开启 or 关闭
  Ui::TtSvgButton *refresh_btn_{nullptr};

  Widget::ModbusClientSetting *modbus_client_setting_{nullptr};

  QWidget *original_widget_{nullptr};
  QWidget *edit_widget_{nullptr};
  Ui::TtLineEdit *title_edit_{nullptr};
  QStackedWidget *stack_{nullptr};

  Ui::TtSvgButton *subscriptionBtn{nullptr};

  TtProtocolType::ProtocolRole role_;

  // Core::ModbusMaster *modbus_master_;
  QScopedPointer<Core::ModbusMaster> modbus_master_;

  Ui::TtModbusTableWidget *coil_table_{nullptr};
  Ui::TtModbusTableWidget *discrete_inputs_table_{nullptr};
  Ui::TtModbusTableWidget *holding_registers_table_{nullptr};
  Ui::TtModbusTableWidget *input_registers_table_{nullptr};

  // 一共只有 4 张表格
  QHash<TtModbusRegisterType::Type, Ui::TtModbusTableWidget *> function_table_;
  QList<Ui::TtLabelLineEdit *> origin_address_;
  QList<Ui::TtLabelLineEdit *> quantity_;
  QList<QPushButton *> plus_btn_;

  QJsonObject config_;
  QTimer refresh_timer_;

  bool opened_{false};

private:
  QThread worker_thread_;
  Core::ModbusWorker *modbus_worker_ = nullptr;
  QProgressDialog *disconnect_progress_ = nullptr;
};

} // namespace Window

#endif // MODBUS_WINDOW_H
