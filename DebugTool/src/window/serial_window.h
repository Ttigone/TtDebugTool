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

class TtColorButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
 public:
  explicit TtColorButton(QWidget* parent = nullptr);
  explicit TtColorButton(const QColor& color, const QString& text,
                         QWidget* parent = nullptr);
  ~TtColorButton();

  void setColors(const QColor& color);
  void setText(const QString& text);
  void setHoverBackgroundColor(const QColor& color);
  void setCheckBlockColor(const QColor& color);

  bool isChecked() const;
  void setChecked(bool checked);
  void setEnableHoldToCheck(bool enable);
  void setEnable(bool enabled);

 public slots:
  void modifyText();

 signals:
  void clicked();
  void toggled(bool checked);
  void textChanged(const QString& newText);  // 新增文本修改信号

 protected:
  void paintEvent(QPaintEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  QSize sizeHint() const override;

 private:
  void clearupEditor();
  bool is_pressed_;
  QSize color_size_;
  bool is_checked_;
  bool enable_hold_to_check_;

  QString text_;
  QColor current_color_;
  QColor normal_color_;
  QColor check_block_color_;
  bool is_hovered_;

  QLineEdit* rename_editor_ = nullptr;  // 重命名编辑器
  QString original_text_;               // 保存原始文本
};

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
