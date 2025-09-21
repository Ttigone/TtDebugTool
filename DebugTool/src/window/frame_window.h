#ifndef WINDOW_FRAME_WINDOW_H
#define WINDOW_FRAME_WINDOW_H

#include <QtMaterialFlatButton.h>

#include <QQueue>
#include <QWidget>

#include "Def.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QStackedWidget;
QT_END_NAMESPACE

class QsciScintilla;
class QtMaterialTabs;
namespace Ui {
class TtElidedLabel;
class TtWidgetGroup;
class TtTextButton;
class TtRadioButton;
class TtTableWidget;
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;
class TtHorizontalLayout;
class TtLineEdit;
class TtTerminalHighlighter;

class TtChatView;
class TtChatMessageModel;

}  // namespace Ui

class QSplitter;
namespace Window {

class FrameWindow : public QWidget {
  Q_OBJECT
  Q_PROPERTY(
      bool saved READ saveStatus WRITE setSaveStatus NOTIFY savedChanged);

 public:
  explicit FrameWindow(QWidget *parent = nullptr);
  virtual ~FrameWindow();

  virtual QString title() const;
  virtual bool workState() const = 0;
  virtual bool saveState() = 0;
  virtual void setSaveState(bool state) = 0;
  Q_INVOKABLE virtual void saveSetting() = 0;
  Q_INVOKABLE virtual void setSetting(const QJsonObject &config) = 0;

  ///
  /// @brief serRightWidget
  /// @param widget
  /// 设置右侧显示设置窗口
  void serRightWidget(QWidget *widget);

  ///
  /// @brief addDisplayWidget
  /// @param btn
  /// @param widget
  /// 添加展示页面
  void addDisplayWidget(Ui::TtSvgButton *btn, QWidget *widget);

  ///
  /// @brief InitUi
  /// 初始化 Ui 框架
  virtual void InitUi();

  ///
  /// @brief InitSignalsConnection
  /// 初始化信号槽链接
  void InitSignalsConnection();

  ///
  /// @brief isValidHexString
  /// @param hexString
  /// @param errorMsg
  /// @return
  /// 检查字符串是否包含有效的十六进制字符
  bool isValidHexString(const QString &hexString, QString *errorMsg = nullptr);

  ///
  /// @brief getValidHexString
  /// @param input
  /// @param errorMsg
  /// @return
  /// 获取有效的十六进制字符串（移除无效字符并处理奇数长度）
  QString getValidHexString(const QString &input, QString *errorMsg = nullptr);

  bool saveStatus() { return saved_; }
  ///
  /// @brief setSaveStatus
  /// @param state
  /// 调用内部原有的通用槽函数实现
  void setSaveStatus(bool state);

 signals:
  ///
  /// @brief savedChanged
  /// @param saved
  /// 保存状态改变
  void savedChanged(bool saved);
  ///
  /// @brief workStateChanged
  /// @param saved
  /// 运行状态改变
  void workStateChanged(bool saved);

 protected:
  virtual void switchToEditMode();
  virtual void switchToDisplayMode();
  ///
  /// @brief setDisplayType
  /// @param type
  /// 切换显示 TEXT/HEX
  virtual void setDisplayType(TtTextFormat::Type type);
  ///
  /// @brief refreshTerminalDisplay
  /// 刷新 TEXT/HEX 界面
  void refreshTerminalDisplay();

  // 文本发送 hex 格式 并显示
  // 数据接收显示 hex
  virtual void showMessage(const QByteArray &data,
                           bool out = true);  // 为 hex 进制提供
  // 分开
  virtual void showMessage(const QString &data, bool out = true);

  // 初始状态下处于非保存状态
  bool saved_{false};   // 保存状态标志
  bool opened_{false};  // 打开状态标志

  Ui::TtVerticalLayout *main_layout_{nullptr};

  Ui::TtNormalLabel *title_{nullptr};           // 显示标题栏
  Ui::TtSvgButton *modify_title_btn_{nullptr};  // 编辑标题栏按钮
  Ui::TtSvgButton *save_btn_{nullptr};          // 保存按钮
  Ui::TtSvgButton *on_off_btn_{nullptr};        // 打开关闭功能按钮

  Ui::TtTextButton *display_text_btn_{nullptr};
  Ui::TtTextButton *display_hex_btn_{nullptr};

  QWidget *original_widget_{nullptr};
  QWidget *edit_widget_{nullptr};
  Ui::TtLineEdit *title_edit_{nullptr};
  QStackedWidget *stack_{nullptr};

  // 对应 message_stacked_view_ 的切换按钮界面
  QWidget *page_btn_widget_;
  Ui::TtHorizontalLayout *page_btn_layout_;
  Ui::TtWidgetGroup *page_btn_logical_;
  // 不同类型展示数据栈窗口
  QStackedWidget *message_stacked_view_;

  Ui::TtSvgButton *clear_history_{nullptr};  // 清楚历史记录按钮
  // Ui::TtChatView *message_view_{nullptr};          // 聊天显示
  // Ui::TtChatMessageModel *message_model_{nullptr}; // 聊先显示数据模型

  QSplitter *main_splitter_{nullptr};
  QPlainTextEdit *terminal_{nullptr};                 // 终端显示
  std::unique_ptr<Ui::TtTerminalHighlighter> lexer_;  // 终端字符高亮显示
  // Ui::TtNormalLabel *send_byte_{nullptr}; // 显示发送字节数
  // Ui::TtNormalLabel *recv_byte_{nullptr}; // 显示接收字节数
  Ui::TtElidedLabel *send_byte_{nullptr};  // 显示发送字节数
  Ui::TtNormalLabel *recv_byte_{nullptr};  // 显示接收字节数
  quint64 send_byte_count_ = 0;            // 存储发送字节数
  quint64 recv_byte_count_ = 0;            // 接收发送字节数

  QtMaterialTabs *tabs_{nullptr};
  QStackedWidget *display_widget_{nullptr};
  QWidget *tabs_widget_{nullptr};

  QsciScintilla *editor_{nullptr};              // 编辑框
  Ui::TtRadioButton *chose_text_btn_{nullptr};  // 选择 TEXT 格式发送
  Ui::TtRadioButton *chose_hex_btn_{nullptr};   // 选择 HEX 格式发送

  QtMaterialFlatButton *send_btn_;                 // 发送按钮
  QPointer<Ui::TtTableWidget> instruction_table_;  // 片段指令表格
  QPointer<Ui::TtTableWidget> auto_replay_table_;  // 自动应答表格

  TtTextFormat::Type send_type_ = TtTextFormat::TEXT;  // 底部选择发送类型
  TtTextFormat::Type display_type_ = TtTextFormat::TEXT;  // 顶部显示接收类型

  uint16_t package_size_ = 0;   // 包大小
  QQueue<QString> msg_queue_;   // 数据队列
  QTimer *send_package_timer_;  // 发送包定时器

  QTimer *heartbeat_timer_;             // 心跳计时器
  QString heartbeat_;                   // 心跳内容
  QByteArray heartbeat_utf8_;           // utf8 格式心跳内容
  uint32_t heartbeat_interval_;         // 心跳间隔
  TtTextFormat::Type heart_beat_type_;  // 心跳格式

  QJsonObject config_;  // 窗口配置
  static const QRegularExpression hexFilterRegex;

 private:
};

}  // namespace Window

#endif  // FRAME_WINDOW_H
