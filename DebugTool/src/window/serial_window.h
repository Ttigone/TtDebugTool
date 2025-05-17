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

#include "Def.h"

class SerialPlot;

QT_BEGIN_NAMESPACE
class QSplitter;
class QListWidget;
QT_END_NAMESPACE

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
}

namespace Window {

class SerialWindow : public FrameWindow {
  Q_OBJECT
public:
  enum class MsgType { TEXT = 0x01, HEX = 0x02 };

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
  bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
  ///
  /// @brief sendMessageToPort
  /// editor 编辑器发送编辑数据
  void sendMessageToPort();
  ///
  /// @brief sendMessageToPort
  /// @param data
  /// 立刻发送 data
  /// 用于发送心跳内容, 发送指令表格的内容 sendRowMsg 信号
  void sendMessageToPort(const QString &data);

  ///
  /// @brief sendMessageToPort
  /// @param data
  /// @param times
  /// 在 times 时间后发送 data
  void sendMessageToPort(const QString &data, const int &times);
  void showErrorMessage(const QString &text);
  void dataReceived(const QByteArray &data);
  void switchToEditMode();
  void switchToDisplayMode();
  // void setDisplayType(MsgType type);
  void setDisplayType(TtTextFormat::Type type);
  void setHeartbeartContent();

private:
  void init();
  void connectSignals();         // 信号槽链接
  void setSerialSetting();       // 设置通讯配置
  void saveSerialLog();          // 保存串口日志
  void refreshTerminalDisplay(); // 显示方式刷新
  void addChannel(const QByteArray &blob, const QColor &color,
                  const QString &uuid = "");

  void handleDialogData(const QString &label, quint16 channel,
                        const QByteArray &blob, const QColor &colork);

  // 文本发送 hex 格式 并显示
  // 数据接收显示 hex
  void showMessage(const QByteArray &data, bool out = true); // 为 hex 进制提供
  // 分开
  void showMessage(const QString &data, bool out = true);

  // 发送 editor 的文本
  // void sendMessage(const QString &data, MsgType type = MsgType::TEXT);
  void sendMessage(const QString &data,
                   TtTextFormat::Type type = TtTextFormat::TEXT);
  void parseBuffer();                                        // 解析数据
  void processFrame(quint8 type, const QByteArray &payload); // 解析帧
  bool isEnableHeartbeart();

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

  QSplitter *main_splitter_;

  Widget::SerialSetting *serial_setting_;

  Ui::TtNormalLabel *send_byte_;
  Ui::TtNormalLabel *recv_byte_;
  quint64 send_byte_count_ = 0;
  quint64 recv_byte_count_ = 0;

  QsciScintilla *editor;

  Ui::TtRadioButton *chose_text_;
  Ui::TtRadioButton *chose_hex_;
  TtTextFormat::Type send_type_ = TtTextFormat::TEXT;

  QWidget *original_widget_ = nullptr;
  QWidget *edit_widget_ = nullptr;
  Ui::TtLineEdit *title_edit_ = nullptr;
  QStackedWidget *stack_ = nullptr;
  QtMaterialFlatButton *sendBtn;
  // TtFlatButton *sendBtn;
  Ui::TtTableWidget *instruction_table_;
  // MsgType display_type_ = MsgType::TEXT;
  TtTextFormat::Type display_type_ = TtTextFormat::TEXT;

  uint16_t package_size_ = 0;
  // 存储 QByteArray
  QQueue<QString> msg_queue_;
  QTimer *send_package_timer_;

  QTimer *heartbeat_timer_;
  QString heartbeat_;
  QByteArray heartbeat_utf8_;
  uint32_t heartbeat_interval_;

  Ui::TtLuaInputBox *lua_code_;

  Ui::TtSerialPortPlot *serial_plot_;
  QListWidget *serialDataList;

  struct ParserRule {
    bool enable;       // 是否使能
    quint16 channel;   // 通道
    QByteArray header; // 帧头字节序列
    int header_len;    // header.size()
    int type_offset;   // 从 buffer[offset] 读类型
    int len_offset;    // 从 buffer[offset] 读长度
    int tail_len;      // 帧尾长度（若无尾则 = 0）
    bool has_checksum;
    // 怎么存储对应的回调函数内容呢 ???
    std::function<bool(const QByteArray &)> validate; // 可选：校验函数
    std::function<void(quint8, const QByteArray &, const QString &)>
        processFrame;
  };

  // 修改相关配置
  QMap<QString, ParserRule> rules_;

  struct ChannelSetting {
    quint16 channel_num_;
    QString title;
    QByteArray data;
    QColor color;
  };

  // 存储了 plot 按钮的配置, first 是 uuid
  // uuid 通道
  quint16 channel_nums_ = 0;
  // 缺少颜色配置
  // QMap<QString, QPair<quint16, QByteArray>> channel_info_;
  QMap<QString, ChannelSetting> channel_info_;

  // 每个通道, 保存对应的 lua 解析代码
  QMap<quint16, QString> lua_script_codes_;

  QByteArray receive_buffer_; // 接收缓冲区

  QJsonObject config_;

  TtTextFormat::Type heart_beat_type_;
  // MsgType heart_beat_type_;
};

} // namespace Window

#endif // WINDOW_SERIAL_WINDOW_H
