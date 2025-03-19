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
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBlock>
#include <QToolBox>
#include <QWidget>

#include <Qsci/qsciscintilla.h>

#include "ui/widgets/window_switcher.h"

namespace Ui {
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
}  // namespace Ui

namespace Widget {
class SerialSetting;
}  // namespace Widget

namespace Core {
class SerialPortWorker;
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

 signals:
  void requestSaveConfig();

 protected:
  // 实现序列化接口
  QByteArray saveState() const override;
  bool restoreState(const QByteArray& state) override;

 private slots:
  void showErrorMessage(const QString& text);
  void onDataReceived(const QByteArray& data);

  void switchToEditMode();
  void switchToDisplayMode();

 private:
  void init();
  void setSerialSetting();
  void connectSignals();

  Ui::TtVerticalLayout* main_layout_;

  QString title;
  Ui::TtNormalLabel* title_;             // 名称
  // Ui::TtImageButton* modify_title_btn_;  // 修改连接名称
  Ui::TtSvgButton* modify_title_btn_;  // 修改连接名称
  // Ui::TtImageButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* save_btn_;    // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;  // 开启 or 关闭

  // 纯文本 / 16进制切换 / 删除历史消失

  // 消息展示框
  Ui::TtChatView* message_view_;
  // 数据
  Ui::TtChatMessageModel* message_model_;

  bool serial_port_opened{false};
  QThread* worker_thread_{nullptr};
  Core::SerialPortWorker* serial_port_;

  Widget::SerialSetting* serial_setting_;

  //Ui::TtLabelComboBox* framingModel;

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  // 使用开源编辑组件 QScintilla
  QsciScintilla* editor;

  QWidget* original_widget_{nullptr};
  QWidget* edit_widget_{nullptr};
  // QLineEdit* title_edit_ = nullptr;
  Ui::TtLineEdit* title_edit_{nullptr};
  QStackedWidget* stack_{nullptr};

  Ui::TtTableWidget* instruction_table_;

  SerialSaveConfig cfg_;
};

}  // namespace Window

#endif  // WINDOW_SERIAL_WINDOW_H
