#ifndef WINDOW_MODBUS_WINDOW_H
#define WINDOW_MODBUS_WINDOW_H

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
  void sloveDataReceived(const int& addr, const QVector<quint16>& data);

  void timerRefreshValue();
  void getSpecificValue();
  void getHoldingRegisterValue();
  void getCoilValue();

 private:
  void init();
  void connectSignals();
  void updatePlot(double value1) {
    double currentKey =
        QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
    // // 添加数据点
    xData.append(currentKey);
    // yData.append(value1);

    // // 保持合理的数据量
    // if (xData.size() > 100) {
    //   xData.removeFirst();
    //   yData.removeFirst();
    // }

    // // 更新图表数据
    // customPlot->graph(0)->setData(xData, yData);

    // // 动态调整X轴范围
    // if (xData.size() > 1) {
    //   customPlot->xAxis->setRange(currentKey - 60, currentKey);
    // }

    // // 动态调整Y轴范围，确保变化可见
    // if (yData.size() > 0) {
    //   double minY = *std::min_element(yData.begin(), yData.end());
    //   double maxY = *std::max_element(yData.begin(), yData.end());
    //   double yDiff = maxY - minY;

    //   // 如果值变化很小或没有变化，设置一个最小范围
    //   if (yDiff < 5) {
    //     double yCenter = (maxY + minY) / 2;
    //     customPlot->yAxis->setRange(yCenter - 5, yCenter + 5);
    //   } else {
    //     // 添加10%的边距
    //     double margin = yDiff * 0.1;
    //     customPlot->yAxis->setRange(minY - margin, maxY + margin);
    //   }
    // }
    // // 重绘图表
    // customPlot->replot();
    // yData.append(value1);
    // customPlot->addGraphsData(xData, yData);
    // customPlot->replot();
    customPlot->addData(value1);
    customPlot->setAutoScaleY(true);
    customPlot->setShowTooltip(true);
  }

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

  Ui::TtMaskWidget* m_overlay = nullptr;
  // 消息展示框
  Ui::TtChatView* message_view_;
  // 数据
  Ui::TtChatMessageModel* message_model_;

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
