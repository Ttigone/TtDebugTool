#ifndef SERIAL_WINDOW_H
#define SERIAL_WINDOW_H

namespace Ui {
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class VerticalLayout;

class TtChatView;
class TtChatMessageModel;
}  // namespace Ui

namespace Widget {
class SerialSetting;
}  // namespace Widget

namespace Core {
class SerialPort;
};

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QParallelAnimationGroup>
#include <QPlainTextEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBlock>
#include <QToolBox>
#include <QWidget>


#include <Qsci/qsciscintilla.h>

namespace Window {

class CustomButtonGroup : public QWidget {
  Q_OBJECT

 public:
  explicit CustomButtonGroup(QWidget* parent = nullptr);

 signals:
  void firstButtonClicked();
  void secondButtonClicked();

 private slots:
  void buttonClicked(QAbstractButton* button);
  void animateButton(QPushButton* button, qreal scale);

 private:
  void setupUI();
  void initConnections();

  QPushButton* button1;
  QPushButton* button2;
  QButtonGroup* buttonGroup;

  // Animation instances
  QSequentialAnimationGroup* animation1;
  QSequentialAnimationGroup* animation2;

  // Initial sizes
  QSize button1InitialSize;
  QSize button2InitialSize;

  // Stylesheet
  static const QString styleSheet;
};

class SerialWindow : public QWidget {
  Q_OBJECT
 public:
  explicit SerialWindow(QWidget* parent = nullptr);

 signals:

 private:
  ///
  /// @brief init
  /// 初始化
  void init();

  ///
  /// @brief setSerialSetting 设置串口参数
  ///
  void setSerialSetting();

  Ui::VerticalLayout* main_layout_;

  Ui::TtNormalLabel* title_;             // 名称
  Ui::TtImageButton* modify_title_btn_;  // 修改连接名称
  Ui::TtImageButton* save_btn_;          // 保存连接记录
  Ui::TtSvgButton* on_off_btn_;          // 开启 or 关闭

  // 纯文本 / 16进制切换 / 删除历史消失

  // 消息展示框
  Ui::TtChatView *message_view_;
  // 数据 
  Ui::TtChatMessageModel *message_model_;

  std::shared_ptr<Core::SerialPort> serial_port_;

  Widget::SerialSetting* serial_setting_;

  // 使用开源编辑组件 QScintilla
  QsciScintilla* editor;
};

}  // namespace Window

#endif  // SERIAL_WINDOW_H
