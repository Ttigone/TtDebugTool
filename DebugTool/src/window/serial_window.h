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

#include "qtmaterialflatbutton.h"
#include <Qsci/qsciscintilla.h>

#include "frame_window.h"
#include "ui/widgets/window_switcher.h"

class SerialPlot;

namespace Ui {

class TtRadioButton;
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
class TtSerialPortPlot;
} // namespace Ui

namespace Widget {
class SerialSetting;
} // namespace Widget

namespace Core {
class SerialPortWorker;
};

namespace Window {

// class SerialWindow : public QWidget, public Ui::TabManager::ISerializable {
class SerialWindow : public FrameWindow, public Ui::TabWindow::ISerializable {
  Q_OBJECT
public:
  enum class MsgType { TEXT = 0x01, HEX = 0x02 };
  // enum class

  explicit SerialWindow(QWidget *parent = nullptr);
  ~SerialWindow();

  QString getTitle();
  QJsonObject getConfiguration() const;
  void saveWaveFormData();

  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;

  Q_INVOKABLE void saveSetting() override;
  Q_INVOKABLE void setSetting(const QJsonObject &config) override;

signals:
  void requestSaveConfig();

protected:
  // 实现序列化接口
  QByteArray saveState() const override;
  bool restoreState(const QByteArray &state) override;

private slots:
  void sendMessageToPort();
  void sendMessageToPort(const QString &data);
  void sendMessageToPort(const QString &data, const int &times);
  void showErrorMessage(const QString &text);
  void dataReceived(const QByteArray &data);
  void switchToEditMode();
  void switchToDisplayMode();
  void setDisplayType(MsgType type);
  void setHeartbeartContent();

private:
  void init();
  void connectSignals();         // 信号槽链接
  void setSerialSetting();       // 设置通讯配置
  void saveSerialLog();          // 保存串口日志
  void refreshTerminalDisplay(); // 显示方式刷新
  void addChannelInfo(const QString &label, const QColor &olor,
                      const QByteArray &blob);
  void handleDialogData(const QString &label, quint16 channel,
                        const QByteArray &blob);

  void parseBuffer();                                        // 解析数据
  void processFrame(quint8 type, const QByteArray &payload); // 解析帧

  // bool saved_ = false;

  Ui::TtVerticalLayout *main_layout_;

  Ui::TtNormalLabel *title_;
  Ui::TtSvgButton *modify_title_btn_;
  Ui::TtSvgButton *save_btn_;
  Ui::TtSvgButton *on_off_btn_;

  Ui::TtSvgButton *clear_history_;
  Ui::TtChatView *message_view_;
  Ui::TtChatMessageModel *message_model_;

  QPlainTextEdit *terminal_;

  bool serial_port_opened = false;
  QThread *worker_thread_ = nullptr;
  Core::SerialPortWorker *serial_port_;

  Widget::SerialSetting *serial_setting_;

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  QsciScintilla *editor;

  Ui::TtRadioButton *chose_text_;
  Ui::TtRadioButton *chose_hex_;
  MsgType send_type_ = MsgType::TEXT;

  QWidget *original_widget_ = nullptr;
  QWidget *edit_widget_ = nullptr;
  Ui::TtLineEdit *title_edit_ = nullptr;
  QStackedWidget *stack_ = nullptr;
  QtMaterialFlatButton *sendBtn;
  Ui::TtTableWidget *instruction_table_;
  MsgType display_type_ = MsgType::TEXT;

  uint16_t package_size_ = 0;
  QQueue<QString> msg_queue_;
  QTimer *send_package_timer_;

  QTimer *heartbeat_timer_;
  QString heartbeat_;
  uint32_t heartbeat_interval_;

  Ui::TtLuaInputBox *lua_code_;

  Ui::TtSerialPortPlot *serial_plot_;

  struct ParserRule {
    bool enable;       // 是否使能
    quint16 channel;   // 通道
    QByteArray header; // 帧头字节序列
    int header_len;    // header.size()
    int type_offset;   // 从 buffer[offset] 读类型
    int len_offset;    // 从 buffer[offset] 读长度
    int tail_len;      // 帧尾长度（若无尾则 = 0）
    bool has_checksum;
    std::function<bool(const QByteArray &)> validate; // 可选：校验函数
    std::function<void(quint8, const QByteArray &, const QString &)>
        processFrame;
  };

  // 修改相关配置
  QMap<QString, ParserRule> rules_;

  // uuid 通道
  quint16 channel_nums_ = 0;
  QMap<QString, QPair<quint16, QByteArray>> channel_info_;

  // 每个通道, 保存对应的 lua 解析代码
  QMap<quint16, QString> lua_script_codes_;

  QByteArray receive_buffer_; // 接收缓冲区

  QJsonObject config_;
};

} // namespace Window

#endif // WINDOW_SERIAL_WINDOW_H
