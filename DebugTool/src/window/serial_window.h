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

// #include <realtimeplot/plotarea.h>
// #include <realtimeplot/pointstream.h>

#include "data/communication_metadata.h"
#include "window/frame_window.h"

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
  explicit SerialWindow(QWidget *parent = nullptr);
  ~SerialWindow();

  QString getTitle();
  QJsonObject getConfiguration() const;
  bool saveWaveFormData(const QString &fileName = QString());

  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;

  Q_INVOKABLE void saveSetting() override;
  Q_INVOKABLE void setSetting(const QJsonObject &config) override;

  void setSaveStatus(bool state);

signals:
  ///
  /// @brief requestSaveConfig
  /// 信号: 请求保存配置
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

  ///
  /// @brief showErrorMessage
  /// @param text
  /// 展现错误信息
  void showErrorMessage(const QString &text);

  ///
  /// @brief dataReceived
  /// @param data
  /// 串口接收函数
  void dataReceived(const QByteArray &data);

  ///
  /// @brief setHeartbeartContent
  /// 发送心跳内容
  void setHeartbeartContent();

  ///
  /// @brief sendInstructionTableContent
  /// @param text 消息本体
  /// @param type   类型
  /// @param times  间隔时间
  /// 发送表格的内容
  void sendInstructionTableContent(const QString &text, TtTextFormat::Type type,
                                   uint32_t times);
  ///
  /// @brief sendInstructionTableContent
  /// @param msg
  /// 构造 MsgInfo
  void sendInstructionTableContent(const Data::MsgInfo &msg);

private:
  void init();
  void connectSignals();   // 信号槽链接
  void setSerialSetting(); // 设置通讯配置
  void saveSerialLog();
  ///
  /// @brief setControlState
  /// @param state
  /// 设置主界面控件状态
  void setControlState(bool state);

  ///
  /// @brief addChannel
  /// @param blob
  /// @param color
  /// @param uuid
  /// 添加新的通道信息
  void addChannel(const QByteArray &blob, const QColor &color, QString &uuid);

  ///
  /// @brief handleDialogData
  /// @param label
  /// @param channel
  /// @param blob
  /// @param colork
  /// 处理编辑通道按钮信息
  void handleDialogData(const QString &label, quint16 channel,
                        const QByteArray &blob, const QColor &colork);

  // // 文本发送 hex 格式 并显示
  // // 数据接收显示 hex
  // void showMessage(const QByteArray &data, bool out = true); // 为 hex
  // 进制提供
  // // 分开
  // void showMessage(const QString &data, bool out = true);

  ///
  /// @brief sendMessage
  /// @param data
  /// @param type
  /// 发送串口消息
  void sendMessage(const QString &data,
                   TtTextFormat::Type type = TtTextFormat::TEXT);

  ///
  /// @brief parseBuffer
  /// 解析缓存区数组
  void parseBuffer(); // 解析数据

  ///
  /// @brief processFrame
  /// @param type
  /// @param payload
  /// 解析帧数据
  void processFrame(quint8 type, const QByteArray &payload); // 解析帧

  ///
  /// @brief isEnableHeartbeart
  /// @return
  /// 是否使能心跳
  bool isEnableHeartbeart();

  ///
  /// @brief sendPackagedData
  /// @param data
  /// @param isHeartbeat
  /// 处理分包发送
  void sendPackagedData(const QByteArray &data, bool isHeartbeat = false);

  void parseVofaProtocol();
  void parseRawDataProtocol();
  void parseJustFloatProtocol();
  void parseFireWaterProtocol();

  // 生成随机测试数据的辅助函数
  QByteArray generateRandomTestData(TtProtocolSetting::Protocol protocol);

  void startRandomDataTest(TtProtocolSetting::Protocol protocol);

  QThread *worker_thread_ = nullptr;
  Core::SerialPortWorker *serial_port_;
  Widget::SerialSetting *serial_setting_{nullptr};
  Ui::TtLuaInputBox *lua_code_{nullptr};
  Ui::TtSerialPortPlot *serial_plot_{nullptr};
  QListWidget *serialDataList{nullptr};

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

  // static const QRegularExpression hexFilterRegex;

  TtProtocolSetting::Protocol protocol_;
  static constexpr int MAX_CHANNELS = 8; // 或者根据您的需求设置其他值
  quint64 sampleNumber = 0;
  // QList<QSharedPointer<PointStream<point_t>>> dataPoints;
};

} // namespace Window

#endif // WINDOW_SERIAL_WINDOW_H
