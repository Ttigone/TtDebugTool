#include "serial_window.h"

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

#include "widget/serial_setting.h"
#include "widget/shortcut_instruction.h"

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>

#include <QTableView>

namespace Window {

// Initialize the static stylesheet
const QString CustomButtonGroup::styleSheet = R"(
QPushButton {
    min-width: 36;
    min-height: 22;
    max-height: 22;
    border: none;
    border-radius: 0px;
    background-color: #0078d4;
    color: white;
    font-size: 12;
    padding: 10px;
}

QPushButton:checked {
    background-color: #005a9e;
}

QPushButton:hover {
    background-color: #005a9e;
}

QPushButton:pressed {
    background-color: #004578;
}
)";

CustomButtonGroup::CustomButtonGroup(QWidget* parent)
    : QWidget(parent),
      button1(new QPushButton("TEXT", this)),
      button2(new QPushButton("HEX", this)),
      buttonGroup(new QButtonGroup(this)),
      animation1(nullptr),
      animation2(nullptr) {
  setupUI();
  initConnections();
  setStyleSheet(styleSheet);

  // 延迟记录初始尺寸，确保布局已完成
  QTimer::singleShot(0, this, [this]() {
    button1InitialSize = button1->size();
    button2InitialSize = button2->size();
  });
}

void CustomButtonGroup::setupUI() {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(QMargins());

  // Configure buttons
  button1->setCheckable(true);
  button1->setChecked(true);
  button2->setCheckable(true);

  // Add buttons to the button group
  buttonGroup->addButton(button1, 1);
  buttonGroup->addButton(button2, 2);

  // Add buttons to the layout
  layout->addWidget(button1);
  layout->addWidget(button2);

  setLayout(layout);
}

void CustomButtonGroup::initConnections() {
  // Connect button group buttonClicked signal to the slot
  connect(buttonGroup,
          QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
          &CustomButtonGroup::buttonClicked);
}

void CustomButtonGroup::buttonClicked(QAbstractButton* button) {
  if (button == button1) {
    emit firstButtonClicked();
  } else if (button == button2) {
    emit secondButtonClicked();
  }
}

void CustomButtonGroup::animateButton(QPushButton* button, qreal scale) {
  QSequentialAnimationGroup** animationPtr = nullptr;
  QSize initialSize;

  // Determine which animation group and initial size to use
  if (button == button1) {
    animationPtr = &animation1;
    initialSize = button1InitialSize;
  } else if (button == button2) {
    animationPtr = &animation2;
    initialSize = button2InitialSize;
  } else {
    return;
  }

  // If an animation is already running, stop and delete it
  if (*animationPtr &&
      (*animationPtr)->state() == QAbstractAnimation::Running) {
    (*animationPtr)->stop();
    delete *animationPtr;
    *animationPtr = nullptr;
  }

  // // 禁用按钮, 放置多次点击
  // button->setEnabled(false);

  // Create a new sequential animation group
  QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

  // Scale down animation
  QPropertyAnimation* scaleDown = new QPropertyAnimation(button, "size");
  scaleDown->setDuration(100);
  scaleDown->setStartValue(button->size());
  scaleDown->setEndValue(initialSize * scale);
  scaleDown->setEasingCurve(QEasingCurve::OutQuad);

  // Scale up animation
  QPropertyAnimation* scaleUp = new QPropertyAnimation(button, "size");
  scaleUp->setDuration(100);
  scaleUp->setStartValue(initialSize * scale);
  scaleUp->setEndValue(initialSize);
  scaleUp->setEasingCurve(QEasingCurve::InQuad);

  // Add animations to the group
  group->addAnimation(scaleDown);
  group->addAnimation(scaleUp);

  // When the animation finishes, reset the animation pointer
  connect(group, &QSequentialAnimationGroup::finished,
          [animationPtr]() { *animationPtr = nullptr; });

  // Store the animation instance
  *animationPtr = group;

  // Start the animation
  group->start(QAbstractAnimation::DeleteWhenStopped);
}

SerialWindow::SerialWindow(QWidget* parent)
    : QWidget(parent),
      serial_port_(new Core::SerialPort()),
      serial_setting_(new Widget::SerialSetting()) {
  init();
}

void SerialWindow::init() {
  main_layout_ = new Ui::VerticalLayout;
  setLayout(main_layout_);

  title_ = new Ui::TtNormalLabel(tr("未命名串口连接"));
  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtImageButton(":/sys/edit_name.svg", this);

  Ui::HorizontalLayout* tmpl = new Ui::HorizontalLayout;

  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  save_btn_ = new Ui::TtImageButton(":/sys/save_cfg.svg", this);

  on_off_btn_ =
      new Ui::TtSvgButton(":/sys/start_up.svg", ":/sys/turn_off.svg", this);

  tmpl->addWidget(save_btn_);
  tmpl->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));

  main_layout_->addLayout(tmpl);

  // 左右分隔器
  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget* chose_function = new QWidget;
  Ui::HorizontalLayout* chose_function_layout = new Ui::HorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  chose_function_layout->addStretch();
  Ui::TtSvgButton* clear_history =
      new Ui::TtSvgButton(":/sys/trash.svg", ":/sys/trash.svg", chose_function);
  // clear_history->setFixedSize(36, 28);

  auto bgr = new CustomButtonGroup(chose_function);
  // 选择 text/hex
  chose_function_layout->addWidget(bgr);

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
  Ui::VerticalLayout* cont_layout = new Ui::VerticalLayout;
  cont->setLayout(cont_layout);

  message_view_ = new Ui::TtChatView(cont);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cont_layout->addWidget(chose_function);
  cont_layout->addWidget(message_view_);

  message_model_ = new Ui::TtChatMessageModel;
  QList<Ui::TtChatMessage*> list;

  Ui::TtChatMessage* msg = new Ui::TtChatMessage;
  msg->setContent(
      "TESTSSSSSSSSSSSSSSSSSSSSSS\r\nSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\r\nsjdsdj"
      "skasdj"
      "sadsakldjkas");
  msg->setOutgoing(true);                  // 必须设置方向
  msg->setBubbleColor(QColor("#DCF8C6"));  // 必须设置颜色
  msg->setTimestamp(QDateTime::currentDateTime());

  Ui::TtChatMessage* msg1 = new Ui::TtChatMessage;
  msg1->setContent("111111111111111111111111111111111111");
  msg1->setOutgoing(true);                  // 必须设置方向
  msg1->setBubbleColor(QColor("#9678dd"));  // 必须设置颜色
  msg1->setTimestamp(QDateTime::currentDateTime());

  Ui::TtChatMessage* msg2 = new Ui::TtChatMessage;
  msg2->setContent(
      "1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n1\n111111111111111111111111");
  msg2->setOutgoing(true);                  // 必须设置方向
  msg2->setBubbleColor(QColor("#ffe292"));  // 必须设置颜色
  msg2->setTimestamp(QDateTime::currentDateTime());

  Ui::TtChatMessage* msg3 = new Ui::TtChatMessage;
  msg3->setContent("蔡韶山");
  msg3->setOutgoing(true);                  // 必须设置方向
  msg3->setBubbleColor(QColor("#d9edfd"));  // 必须设置颜色
  msg3->setTimestamp(QDateTime::currentDateTime());

  list.append(msg);
  list.append(msg2);
  list.append(msg1);
  list.append(msg3);

  message_model_->appendMessages(list);

  message_view_->setModel(message_model_);

  QWidget* bottomAll = new QWidget;
  Ui::VerticalLayout* bottomAllLayout = new Ui::VerticalLayout;
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget* tabs_and_count = new QWidget(this);
  Ui::HorizontalLayout* tacLayout = new Ui::HorizontalLayout();
  tabs_and_count->setLayout(tacLayout);

  auto m_tabs = new QtMaterialTabs;
  // m_tabs->setBackgroundColor()
  m_tabs->addTab(tr("手动"));
  // m_tabs
  m_tabs->addTab(tr("片段"));
  m_tabs->addTab(tr("指令"));
  // m_tabs->setBackgroundColor(QColor::fromRgbF(255, 255, 255));
  m_tabs->setMinimumWidth(80);

  tacLayout->addWidget(m_tabs);
  tacLayout->addStretch();

  // 显示发送字节和接收字节数
  auto send_byte = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  send_byte->setFixedHeight(30);
  auto recv_byte = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  recv_byte->setFixedHeight(30);

  tacLayout->addWidget(send_byte);
  tacLayout->addWidget(recv_byte);

  QStackedLayout* layout = new QStackedLayout;
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);
  QWidget* la_w = new QWidget(this);
  la_w->setLayout(layout);

  QWidget* messageEdit = new QWidget(la_w);
  // messageEdit
  QVBoxLayout* messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 3, 3, 3);
  messageEditLayout->setSpacing(0);

  auto editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);

  editor->setCaretForegroundColor(QColor("Coral"));
  editor->setCaretWidth(10);

  editor->setCaretLineBackgroundColor(QColor("Red"));
  editor->setMarginType(1, QsciScintilla::NumberMargin);

  messageEditLayout->addWidget(editor);

  QWidget* bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  bottomBtnWidget->setStyleSheet("background-color: red");
  Ui::HorizontalLayout* bottomBtnWidgetLayout = new Ui::HorizontalLayout;
  bottomBtnWidgetLayout->setContentsMargins(QMargins());
  bottomBtnWidgetLayout->setSpacing(0);
  bottomBtnWidget->setLayout(bottomBtnWidgetLayout);

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

  Ui::TtTableWidget* table = new Ui::TtTableWidget(la_w);
  // Ui::TtToggleButton* button = new Ui::TtToggleButton();

  // auto te = new Widget::ShortcutInstruction;
  // te->setStyleSheet("background-color: Coral");

  // Insert QPushButton
  // QListWidgetItem* buttonItem = new QListWidgetItem(te);
  // te->setItemWidget(buttonItem, button);

  // Widget::HeaderWidget* ts = new Widget::HeaderWidget;
  // te->addCustomWidget(ts);

  // Widget::InstructionWidget* ttt = new Widget::InstructionWidget;
  // te->addCustomWidget(ttt);

  //layout->addWidget(table);

  layout->addWidget(messageEdit);

  // layout->addWidget(table);

  // // 创建自定义widget
  // // QPushButton* button = new QPushButton("Click Me");
  // Ui::TtToggleButton* button = new Ui::TtToggleButton();
  // button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, [this, layout](int index) {
    qDebug() << index;
    layout->setCurrentIndex(index);
  });

  bottomAllLayout->addWidget(tabs_and_count);
  // bottomAllLayout->addWidget(messageEdit);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(cont);
  VSplitter->addWidget(bottomAll);

  serial_setting_->setSizePolicy(QSizePolicy::Preferred,
                                 QSizePolicy::Preferred);
  setSerialSetting();

  QWidget* contentWidget1 = new QWidget;
  QVBoxLayout* contentLayout1 = new QVBoxLayout(contentWidget1);
  contentLayout1->setSpacing(0);
  contentLayout1->setContentsMargins(QMargins());
  contentLayout1->addWidget(serial_setting_);
  contentWidget1->adjustSize();  // 确保大小正确
  //QToolBox *test = new QToolBox;
  //auto sds = new QComboBox;
  //auto sds = new Ui::TtComboBox;
  //sds->addItem("1");
  //sds->addItem("1");
  //sds->addItem("1");
  //sds->addItem("1");
  //sds->addItem("1");
  //test->addItem(sds, ");
  //test->addItem(new QComboBox(), "test");
  //test->addItem(new QComboBox(), "test");
  Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), contentWidget1);
  //Ui::Drawer* drawer1 = new Ui::Drawer(tr("连接设置"), sds);

  QWidget* contentWidget2 = new QWidget;
  contentWidget2->setStyleSheet("background-color: lightgreen;");
  QVBoxLayout* contentLayout2 = new QVBoxLayout(contentWidget2);
  // contentLayout2->addWidget(new QLabel("Content 2"));
  // contentLayout2->addWidget(modify_title_btn_);
  contentWidget2->adjustSize();  // 确保大小正确
  Ui::Drawer* drawer2 = new Ui::Drawer(tr("变量"), contentWidget2);

  Ui::TtLabelComboBox* c3 = new Ui::TtLabelComboBox(tr("模式: "));
  c3->addItem("换行");
  Ui::Drawer* drawer3 = new Ui::Drawer(tr("分帧"), c3);

  Ui::TtLabelComboBox* c4 = new Ui::TtLabelComboBox(tr("模式: "));
  Ui::Drawer* drawer4 = new Ui::Drawer(tr("心跳"), c4);

  // 滚动区域
  QScrollArea* scr = new QScrollArea(this);
  QWidget* scrollContent = new QWidget(scr);
  //scr->setWidget(scrollContent);
  //scr->setWidgetResizable(true);
  Ui::VerticalLayout* lascr = new Ui::VerticalLayout(scrollContent);

  lascr->addWidget(drawer1, 0, Qt::AlignTop);
  //lascr->addWidget(sds, 0, Qt::AlignTop);
  lascr->addWidget(drawer2);
  lascr->addWidget(drawer3);
  lascr->addWidget(drawer4);
  lascr->addStretch();
  scrollContent->setLayout(lascr);

  scr->setWidget(scrollContent);
  scr->setWidgetResizable(true);

  // 左右分区
  mainSplitter->addWidget(VSplitter);
  mainSplitter->addWidget(scr);

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
        serial_setting_->defaultSerialPortConfiguration());
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

  connect(serial_port_.get(), &Core::SerialPort::recvData,
          [this](QByteArray msg) {
            //m_chatView->addMessage(QString(msg), false);  // 对方消息
            auto tmp = new Ui::TtChatMessage();
            tmp->setContent(msg);
            tmp->setOutgoing(false);
            tmp->setBubbleColor(QColor("#DCF8C6"));
            QList<Ui::TtChatMessage*> list;
            list.append(tmp);
            message_model_->appendMessages(list);
          });

  //connect(clear_history, &Ui::TtSvgButton::clicked, chat_view_,
  //        &Ui::MessageDialog::deleteAllMessage);

  //connect(sendBtn, &QtMaterialFlatButton::clicked, [this, editor]() {
  //  // 发送消息
  //  dialog_->addMessage(editor->text(), true);
  //  serial_port_->sendData(editor->text());
  //});
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

}  // namespace Window
