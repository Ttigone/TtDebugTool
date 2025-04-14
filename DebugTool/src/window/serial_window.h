#ifndef WINDOW_SERIAL_WINDOW_H
#define WINDOW_SERIAL_WINDOW_H

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QParallelAnimationGroup>
#include <QPlainTextEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QQueue>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBlock>
#include <QToolBox>
#include <QWidget>

#include <Qsci/qsciscintilla.h>

#include "qtmaterialflatbutton.h"
#include "ui/widgets/window_switcher.h"

class SerialPlot;

namespace Ui {

class TtMaskWidget;
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;
class TtLineEdit;

class TtChatView;
class TtChatMessageModel;
class TtTableWidget;

class TtLuaInputBox;
}  // namespace Ui

namespace Widget {
class SerialSetting;
}  // namespace Widget

namespace Core {
class SerialPortWorker;
class LuaKernel;
};

namespace Window {


struct SerialSaveConfig {
  QJsonObject obj;
};

class SerialWindow : public QWidget, public Ui::TabManager::ISerializable {
  Q_OBJECT
 public:
  explicit SerialWindow(QWidget* parent = nullptr);
  ~SerialWindow();

  QString getTitle();
  QJsonObject getConfiguration() const;

  void saveWaveFormData();

 signals:
  void requestSaveConfig();

 protected:
  // 实现序列化接口
  QByteArray saveState() const override;
  bool restoreState(const QByteArray& state) override;

 private slots:
  void sendMessageToPort();
  void sendMessageToPort(const QString& data);
  void sendMessageToPort(const QString& data, const int& times);
  void showErrorMessage(const QString& text);
  void dataReceived(const QByteArray& data);
  void switchToEditMode();
  void switchToDisplayMode();
  void setDisplayHex(bool hexMode);
  void setHeartbeartContent();

 private:
  void init();
  void setSerialSetting();
  void connectSignals();
  void saveLog();
  void refreshTerminalDisplay();

  Ui::TtVerticalLayout* main_layout_;

  QString title;
  Ui::TtNormalLabel* title_;
  Ui::TtSvgButton* modify_title_btn_;
  Ui::TtSvgButton* save_btn_;
  Ui::TtSvgButton* on_off_btn_;

  Ui::TtSvgButton* clear_history_;
  Ui::TtChatView* message_view_;
  Ui::TtChatMessageModel* message_model_;

  QPlainTextEdit* terminal_;

  bool serial_port_opened = false;
  QThread* worker_thread_ = nullptr;
  Core::SerialPortWorker* serial_port_;

  Widget::SerialSetting* serial_setting_;

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  QsciScintilla* editor;

  QWidget* original_widget_ = nullptr;
  QWidget* edit_widget_ = nullptr;
  Ui::TtLineEdit* title_edit_ = nullptr;
  QStackedWidget* stack_ = nullptr;
  QtMaterialFlatButton* sendBtn;
  Ui::TtTableWidget* instruction_table_;
  bool display_hex_;
  SerialSaveConfig cfg_;

  uint16_t package_size_;
  QQueue<QString> msg_queue_;
  QTimer* send_package_timer_;

  QTimer* heartbeat_timer_;
  QString heartbeat_;
  uint32_t heartbeat_interval_;

  Ui::TtLuaInputBox* lua_code_;
  Core::LuaKernel* lua_actuator_;

  // Ui::TtQCustomPlot* serial_plot_;
  SerialPlot* serial_plot_;
};

}  // namespace Window

#endif  // WINDOW_SERIAL_WINDOW_H
