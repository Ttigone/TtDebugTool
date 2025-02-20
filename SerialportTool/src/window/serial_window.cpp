#include "serial_window.h"

#include <QTableView>

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtTableView.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/snack_bar.h>
#include <ui/window/combobox.h>

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>

#include "widget/serial_setting.h"
#include "widget/shortcut_instruction.h"

namespace Window {

SerialWindow::SerialWindow(QWidget* parent)
    : QWidget(parent),
      serial_port_(new Core::SerialPort()),
      serial_setting_(new Widget::SerialSetting()) {
  init();
  connectSignals();
}

QString SerialWindow::getTitle() {
  return title_->text();
}

QJsonObject SerialWindow::getConfiguration() const {
  return cfg_.obj;
}

void SerialWindow::switchToEditMode() {
  QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  // 预先获取之前的标题
  title_edit_->setText(title_->text());
  // 显示 edit 模式
  stack_->setCurrentWidget(edit_widget_);
  // 获取焦点
  title_edit_->setFocus();  // 自动聚焦输入框
}

void SerialWindow::switchToDisplayMode() {
  //QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(original_widget_);
  //original_widget_->setGraphicsEffect(effect);
  //QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  //anim->setDuration(300);
  //anim->setStartValue(0);
  //anim->setEndValue(1);
  //anim->start(QAbstractAnimation::DeleteWhenStopped);
  // 切换显示模式
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

void SerialWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout();
  setLayout(main_layout_);

  title_ = new Ui::TtNormalLabel(tr("未命名串口连接"));
  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtImageButton(":/sys/edit_name.svg", this);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  original_widget_->setStyleSheet("background-color : Coral");
  Ui::TtHorizontalLayout* tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new QLineEdit(this);
  //save_title_btn_ = new QPushButton(tr("保存"), this);

  Ui::TtHorizontalLayout* edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addWidget(title_edit_);
  edit_widget_->setStyleSheet("background-color : green");
  //edit_layout->addWidget(save_title_btn_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setFixedHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtImageButton::clicked, this,
          &SerialWindow::switchToEditMode);

  Ui::TtHorizontalLayout* tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout* tmpAll = new Ui::TtHorizontalLayout;

  // 保存 lambda 表达式
  auto handleSave = [this]() {
    //qDebug() << "失去";
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout* tmpl2 = new Ui::TtHorizontalLayout;
  // 保存按钮
  save_btn_ = new Ui::TtImageButton(":/sys/save_cfg.svg", this);

  // 删除按钮, 是需要保存在 leftbar 才会添加的

  // 开关按钮
  on_off_btn_ =
      new Ui::TtSvgButton(":/sys/start_up.svg", ":/sys/turn_off.svg", this);

  tmpl2->addWidget(save_btn_);
  tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  //tmpAll->addLayout(tmpl);
  tmpAll->addLayout(tmpP1);
  tmpAll->addLayout(tmpl2);

  //main_layout_->addLayout(tmpl);
  main_layout_->addLayout(tmpAll);

  // 左右分隔器
  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget* chose_function = new QWidget;
  Ui::TtHorizontalLayout* chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  chose_function_layout->addStretch();
  Ui::TtImageButton* clear_history =
      new Ui::TtImageButton(":/sys/trash.svg", chose_function);
  // clear_history->setFixedSize(36, 28);

  //auto bgr = new CustomButtonGroup(chose_function);
  //// 选择 text/hex
  //chose_function_layout->addWidget(bgr);
  QButtonGroup* bgr = new QButtonGroup(this);
  QPushButton* button1 = new QPushButton("TEXT");
  QPushButton* button2 = new QPushButton("HEX");
  bgr->addButton(button1);
  bgr->addButton(button2);
  chose_function_layout->addWidget(button1);
  chose_function_layout->addWidget(button2);

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history);

  // 中间的弹簧

  // 上下分隔器
  QSplitter* VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  // VSplitter->setSizes();

  // 上方选择功能以及信息框
  QWidget* cont = new QWidget;
  Ui::TtVerticalLayout* cont_layout = new Ui::TtVerticalLayout;
  cont->setLayout(cont_layout);

  message_view_ = new Ui::TtChatView(cont);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cont_layout->addWidget(chose_function);
  cont_layout->addWidget(message_view_);

  base::DetectRunningTime runtime;

  message_model_ = new Ui::TtChatMessageModel;
  //QList<Ui::TtChatMessage*> list;

  //Ui::TtChatMessage* msg = new Ui::TtChatMessage;
  //msg->setContent(
  //    "TESTSSSSSSSSSSSSSSSSSSSSSS\r\nSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\r\nsjdsdj"
  //    "skasdj"
  //    "sadsakldjkas");
  //msg->setOutgoing(true);                  // 必须设置方向
  //msg->setBubbleColor(QColor("#DCF8C6"));  // 必须设置颜色
  //msg->setTimestamp(QDateTime::currentDateTime());

  //Ui::TtChatMessage* msg1 = new Ui::TtChatMessage;
  //msg1->setContent("111111111111111111111111111111111111");
  //msg1->setOutgoing(true);                  // 必须设置方向
  //msg1->setBubbleColor(QColor("#9678dd"));  // 必须设置颜色
  //msg1->setTimestamp(QDateTime::currentDateTime());

  //Ui::TtChatMessage* msg2 = new Ui::TtChatMessage;
  //msg2->setContent(
  //    "1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n111111111111111111111111");
  //msg2->setOutgoing(true);                  // 必须设置方向
  //msg2->setBubbleColor(QColor("#ffe292"));  // 必须设置颜色
  //msg2->setTimestamp(QDateTime::currentDateTime());

  //Ui::TtChatMessage* msg3 = new Ui::TtChatMessage;
  //msg3->setContent("蔡韶山");
  //msg3->setOutgoing(true);                  // 必须设置方向
  //msg3->setBubbleColor(QColor("#d9edfd"));  // 必须设置颜色
  //msg3->setTimestamp(QDateTime::currentDateTime());

  //list.append(msg);
  //list.append(msg2);
  //list.append(msg1);
  //list.append(msg3);

  //message_model_->appendMessages(list);

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget* bottomAll = new QWidget;
  Ui::TtVerticalLayout* bottomAllLayout = new Ui::TtVerticalLayout;
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget* tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout* tacLayout = new Ui::TtHorizontalLayout();
  tabs_and_count->setLayout(tacLayout);

  auto m_tabs = new QtMaterialTabs;
  // m_tabs->setBackgroundColor()
  m_tabs->addTab(tr("手动"));
  // m_tabs
  m_tabs->addTab(tr("片段"));
  // m_tabs->setBackgroundColor(QColor::fromRgbF(255, 255, 255));
  m_tabs->setMinimumWidth(80);

  tacLayout->addWidget(m_tabs);
  tacLayout->addStretch();

  // 显示发送字节和接收字节数
  send_byte = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  send_byte->setFixedHeight(30);
  recv_byte = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  recv_byte->setFixedHeight(30);

  tacLayout->addWidget(send_byte);
  tacLayout->addWidget(recv_byte);

  QWidget* la_w = new QWidget(this);
  QStackedLayout* layout = new QStackedLayout(la_w);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QWidget* messageEdit = new QWidget(la_w);
  // messageEdit
  QVBoxLayout* messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 0, 3, 0);
  messageEditLayout->setSpacing(0);

  editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretForegroundColor(QColor("Coral"));
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);

  messageEditLayout->addWidget(editor);

  QWidget* bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  //bottomBtnWidget->setStyleSheet("back)ground-color: red");
  Ui::TtHorizontalLayout* bottomBtnWidgetLayout =
      new Ui::TtHorizontalLayout(bottomBtnWidget);

  QtMaterialRadioButton* choseText = new QtMaterialRadioButton(bottomBtnWidget);
  QtMaterialRadioButton* choseHex = new QtMaterialRadioButton(bottomBtnWidget);
  choseText->setText("TEXT");
  choseHex->setText("HEX");

  QtMaterialFlatButton* sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  sendBtn->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(choseText);
  bottomBtnWidgetLayout->addWidget(choseHex);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(sendBtn);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(la_w);

  layout->addWidget(messageEdit);
  layout->addWidget(instruction_table_);

  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, [this, layout](int index) {
    //qDebug() << index;
    layout->setCurrentIndex(index);
  });

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(cont);
  VSplitter->addWidget(bottomAll);

  // 左右分区
  mainSplitter->addWidget(VSplitter);
  mainSplitter->addWidget(serial_setting_);

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);

  QtMaterialSnackbar* snack_bar_ = new QtMaterialSnackbar(this);
  snack_bar_->setAutoHideDuration(1500);
  snack_bar_->setClickToDismissMode(true);

  //SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //SnackBarController::instance();

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this, snack_bar_]() {
    // 检查是否处于打开状态
    //snack_bar_->addMessage(
    //    "无法打开串口: attempting to open an already opened");
    //Ui::SnackBarController::instance()->showMessage("文件保存成功");
    //Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
    if (serial_port_->isOpened()) {
      serial_port_->closeSerialPort();
      return;
    }
    Core::SerialPort::SerialError error = serial_port_->openSerialPort(
        //serial_setting_->defaultSerialPortConfiguration());
        serial_setting_->getSerialPortConfiguration());
    if (error != Core::SerialPort::NoError) {
      qDebug() << "inside";
      switch (error) {
        case Core::SerialPort::Open:
          // 已经被占用
          snack_bar_->addMessage(
              "无法打开串口: attempting to open an already opened");
          break;
        case Core::SerialPort::Permission:
          snack_bar_->addMessage(
              "无法打开串口: attempting to open an already opened device by "
              "another process or a user");
          break;
        case Core::SerialPort::DeviceNotFound:
          qDebug() << "do it";
          snack_bar_->addInstantMessage(
              "无法打开串口: attempting to open an non-existing device");
          // 没找到设备
          break;
      }
    } else {
    }
  });

  connect(
      serial_port_.get(), &Core::SerialPort::recvData, [this](QByteArray msg) {
        recv_byte_count += msg.size();
        auto tmp = new Ui::TtChatMessage();
        tmp->setContent(msg);
        tmp->setOutgoing(false);
        tmp->setBubbleColor(QColor("#DCF8C6"));
        QList<Ui::TtChatMessage*> list;
        list.append(tmp);
        message_model_->appendMessages(list);
        recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
        message_view_->scrollToBottom();
      });

  connect(clear_history, &Ui::TtImageButton::clicked,
          [this]() { message_model_->clearModelData(); });

  connect(sendBtn, &QtMaterialFlatButton::clicked, [this]() {
    // 发送消息
    auto msg = editor->text();
    send_byte_count += msg.size();
    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(msg);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage*> list;
    list.append(tmp);
    message_model_->appendMessages(list);
    // 串口发送
    serial_port_->sendData(editor->text());

    send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    message_view_->scrollToBottom();
  });
  qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
}

void SerialWindow::setSerialSetting() {
  // 初始 app 时, 随机地设置串口配置
  // 选择可以选择全部, 但是显示的时候, 只显示 COMx
  serial_setting_->setSerialPortsName();
  serial_setting_->setSerialPortsBaudRate();
  serial_setting_->setSerialPortsDataBit();
  serial_setting_->setSerialPortsParityBit();
  serial_setting_->setSerialPortsStopBit();
  serial_setting_->setSerialPortsFluidControl();

  serial_setting_->displayDefaultSetting();
}

void SerialWindow::connectSignals() {
  connect(save_btn_, &Ui::TtImageButton::clicked, [this]() {
    // 保存配置界面, 但没有历史消息
    cfg_.obj.insert("WindowTitle", title_->text());
    cfg_.obj.insert("SerialSetting", serial_setting_->getSerialSetting());
    cfg_.obj.insert("InstructionTable", instruction_table_->getTableRecord());
    emit requestSaveConfig();
    //qDebug() << cfg_.obj;
    // 配置文件保存到文件中
    // 当前的 tabWidget 匹配对应的 QJsonObject
  });
}

}  // namespace Window
