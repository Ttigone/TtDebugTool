#ifndef TCP_WINDOW_H
#define TCP_WINDOW_H

#include <QWidget>
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

#include <Qsci/qsciscintilla.h>

namespace Ui {
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;

class TtChatView;
class TtChatMessageModel;
}  // namespace Ui

namespace Widget {
class TcpServerSetting;
}  // namespace Widget

namespace Window {

class TcpWindow : public QWidget {
  Q_OBJECT
 public:
  explicit TcpWindow(QWidget* parent = nullptr);

 signals:

 private slots:
  void switchToEditMode();
  void switchToDisplayMode();

 private:
  void init();

  Ui::TtVerticalLayout* main_layout_;

  Ui::TtNormalLabel* title_;             // 名称
  Ui::TtImageButton* modify_title_btn_;  // 修改连接名称
  Ui::TtImageButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;          // 开启 or 关闭

  // 消息展示框
  Ui::TtChatView* message_view_;
  // 数据
  Ui::TtChatMessageModel* message_model_;

  Widget::TcpServerSetting *tcp_server_setting_;

  Ui::TtNormalLabel *send_byte;
  Ui::TtNormalLabel *recv_byte;
  quint64 send_byte_count = 0;
  quint64 recv_byte_count = 0;

  // 使用开源编辑组件 QScintilla
  QsciScintilla* editor;

  QWidget* original_widget_ = nullptr;
  QWidget* edit_widget_ = nullptr;
  QLineEdit* title_edit_ = nullptr;
  QStackedWidget* stack_ = nullptr;
};

} // namespace Window


#endif  // TCP_WINDOW_H
