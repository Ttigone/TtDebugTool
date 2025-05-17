#include "window/tcp_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/widget_group.h>

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>
#include <ui/controls/TtSerialLexer.h>

#include <QTableView>

#include "core/tcp_client.h"
#include "core/tcp_server.h"
#include "ui/controls/TtTableView.h"
#include "widget/tcp_setting.h"

namespace Window {

TcpWindow::TcpWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    : FrameWindow(parent), role_(role) {

  init();

  if (role_ == TtProtocolType::Client) {
    connect(tcp_client_, &Core::TcpClient::connected,
            [this]() { tcp_opened_ = true; });
    connect(tcp_client_, &Core::TcpClient::disconnected,
            [this]() { tcp_opened_ = false; });
    connect(tcp_client_, &Core::TcpClient::dataReceived, this,
            &TcpWindow::onDataReceived);
    connect(tcp_client_, &Core::TcpClient::errorOccurred, this,
            [this](const QString &error) {
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error,
                                      1500, this);
              on_off_btn_->setChecked(false);
              tcp_opened_ = false;
            });
  } else {
    connect(tcp_server_, &Core::TcpServer::serverStarted, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::serverStopped, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::errorOccurred,
            [this](const QString &err) {
              QMessageBox::critical(this, tr("错误"), err);
            });
    connect(tcp_server_, &Core::TcpServer::dataReceived, this,
            &TcpWindow::onDataReceived);
  }
  connectSignals();
}

TcpWindow::~TcpWindow() { qDebug() << "Delete TCPWindow"; }

QJsonObject TcpWindow::getConfiguration() const { return config_; }

bool TcpWindow::workState() const { return tcp_opened_; }

bool TcpWindow::saveState() { return saved_; }

void TcpWindow::setSaveState(bool state) { saved_ = state; }

void TcpWindow::setSetting(const QJsonObject &config) {
  // 模拟是服务端
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    tcp_client_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    tcp_server_setting_->setOldSettings(
        config.value("TcpClientSetting").toObject(QJsonObject()));
  }
  saved_ = true;
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

void TcpWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitile", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("TcpClientSetting",
                   tcp_client_setting_->getTcpClientSetting());
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("TcpServerSetting",
                   tcp_server_setting_->getTcpServerSetting());
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());
  saved_ = true;
  emit requestSaveConfig();
}

QString TcpWindow::getTitle() const { return title_->text(); }

void TcpWindow::switchToEditMode() {
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(title_edit_);
  title_edit_->setGraphicsEffect(effect);
  QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0);
  anim->setEndValue(1);
  anim->start(QAbstractAnimation::DeleteWhenStopped);

  // 预先获取之前的标题
  title_edit_->setText(title_->text());
  // 显示 edit 模式
  stack_->setCurrentWidget(edit_widget_);
  // 获取焦点
  title_edit_->setFocus(); // 自动聚焦输入框
}

void TcpWindow::switchToDisplayMode() {
  // QGraphicsOpacityEffect* effect = new
  // QGraphicsOpacityEffect(original_widget_);
  // original_widget_->setGraphicsEffect(effect);
  // QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
  // anim->setDuration(300);
  // anim->setStartValue(0);
  // anim->setEndValue(1);
  // anim->start(QAbstractAnimation::DeleteWhenStopped);
  //  切换显示模式
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

void TcpWindow::updateServerStatus() {
  const bool running = tcp_server_->isRunning();
  auto text =
      (running ? tr("服务运行中 (端口: %1)").arg("111") : tr("服务已停止"));
  qDebug() << text;
}

void TcpWindow::onDataReceived(const QByteArray &data) {
  qDebug() << "Received data:" << data;
  recv_byte_count += data.size();
  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setOutgoing(false);
  tmp->setBubbleColor(QColor("#0ea5e9"));
  QList<Ui::TtChatMessage *> list;
  list.append(tmp);
  message_model_->appendMessages(list);
  recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  message_view_->scrollToBottom();
}

void TcpWindow::init() {
  /*
  main_layout_ = new Ui::TtVerticalLayout(this);

  if (role_ == TtProtocolType::Client) {
    tcp_client_ = new Core::TcpClient;
    title_ = new Ui::TtNormalLabel(tr("未命名的 TCP 连接"));
  } else {
    tcp_server_ = new Core::TcpServer;
    title_ = new Ui::TtNormalLabel(tr("未命名的 TCP 服务模拟端"));
  }
  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit_name.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout *tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout *edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &TcpWindow::switchToEditMode);

  Ui::TtHorizontalLayout *tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout *tmpAll = new Ui::TtHorizontalLayout;

  // 保存 lambda 表达式
  auto handleSave = [this]() {
    // qDebug() << "失去";
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout *tmpl2 = new Ui::TtHorizontalLayout;
  // 保存按钮
  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);
  // 删除按钮, 是需要保存在 leftbar 才会添加的

  // 开关按钮
  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setSvgSize(18, 18);
  on_off_btn_->setColors(Qt::black, Qt::red);

  tmpl2->addWidget(save_btn_);
  tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  tmpAll->addLayout(tmpP1);
  tmpAll->addLayout(tmpl2);

  main_layout_->addLayout(tmpAll);

  // 左右分隔器
  QSplitter *mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget *chose_function = new QWidget;
  Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  chose_function_layout->addStretch();
  Ui::TtSvgButton *clear_history =
      new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history->setSvgSize(18, 18);

  // auto bgr = new CustomButtonGroup(chose_function);
  //  选择 text/hex
  // chose_function_layout->addWidget(bgr);

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history);

  // 中间的弹簧

  // 上下分隔器
  QSplitter *VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  // VSplitter->setSizes();

  // 上方选择功能以及信息框
  QWidget *cont = new QWidget;
  Ui::TtVerticalLayout *cont_layout = new Ui::TtVerticalLayout(cont);

  message_view_ = new Ui::TtChatView(cont);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cont_layout->addWidget(chose_function);
  cont_layout->addWidget(message_view_);

  message_model_ = new Ui::TtChatMessageModel;
  // QList<Ui::TtChatMessage *> list;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget *bottomAll = new QWidget;
  Ui::TtVerticalLayout *bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);

  // 下方自定义指令
  QWidget *tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout *tacLayout = new Ui::TtHorizontalLayout();
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

  QStackedLayout *layout = new QStackedLayout;
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);
  QWidget *basicWidget = new QWidget(this);
  basicWidget->setLayout(layout);

  QWidget *messageEdit = new QWidget(basicWidget);
  QVBoxLayout *messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 3, 3, 3);
  messageEditLayout->setSpacing(0);

  editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);

  messageEditLayout->addWidget(editor);

  QWidget *bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  Ui::TtHorizontalLayout *bottomBtnWidgetLayout = new Ui::TtHorizontalLayout;
  bottomBtnWidgetLayout->setContentsMargins(QMargins());
  bottomBtnWidgetLayout->setSpacing(0);
  bottomBtnWidget->setLayout(bottomBtnWidgetLayout);

  // 全部替换过去, 就行了
  // chose_text_ = new Ui::TtRadioButton();
  // chose_text_ = new Ui::TtRadioButton("TEXT", bottomBtnWidget);
  // chose_hex_ = new Ui::TtRadioButton("HEX", bottomBtnWidget);
  // chose_hex_;
  // QtMaterialRadioButton *choseText = new
  // QtMaterialRadioButton(bottomBtnWidget); QtMaterialRadioButton *choseHex =
  // new QtMaterialRadioButton(bottomBtnWidget);
  // chose_text_->setText("TEXT");
  // chose_hex_->setText("HEX");

  QtMaterialFlatButton *sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  sendBtn->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(choseText);
  bottomBtnWidgetLayout->addWidget(choseHex);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(sendBtn);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(basicWidget);

  layout->addWidget(messageEdit);
  layout->addWidget(instruction_table_);
  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, this,
          [this, layout](int index) { layout->setCurrentIndex(index); });

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(basicWidget);

  VSplitter->addWidget(cont);
  VSplitter->addWidget(bottomAll);

  mainSplitter->addWidget(VSplitter);

  if (role_ == TtProtocolType::Client) {
    tcp_client_setting_ = new Widget::TcpClientSetting;
    mainSplitter->addWidget(tcp_client_setting_);
  } else {
    tcp_server_setting_ = new Widget::TcpServerSetting;
    mainSplitter->addWidget(tcp_server_setting_);
  }

  main_layout_->addwidget(mainsplitter);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this] {
    if (tcp_opened_) {
      if (role_ == TtProtocolType::Client) {
        qDebug() << "test";
        tcp_client_->disconnectFromServer();
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->close();
      }
    } else {
      if (role_ == TtProtocolType::Client) {
        qDebug() << "connectToServer";
        tcp_client_->connectToServer(
            tcp_client_setting_->getTcpClientConfiguration());
      } else if (role_ == TtProtocolType::Server) {

        tcp_server_->startServer(
            tcp_server_setting_->getTcpServerConfiguration());
      }
    }
  });

  connect(clear_history, &Ui::TtSvgButton::clicked, this,
          [this]() { message_model_->clearModelData(); });

  connect(sendBtn, &QtMaterialFlatButton::clicked, this, [this] {
    QString data = editor->text();
    send_byte_count += data.size();
    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(data);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage *> list;
    list.append(tmp);
    message_model_->appendMessages(list);

    if (role_ == TtProtocolType::Client) {
      tcp_client_->sendMessage(data.toUtf8());
    } else {
      tcp_server_->sendMessageToClients(data.toUtf8());
    }

    send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    message_view_->scrollToBottom();*/
  // });

  main_layout_ = new Ui::TtVerticalLayout(this);

  if (role_ == TtProtocolType::Client) {
    tcp_client_ = new Core::TcpClient;
    title_ = new Ui::TtNormalLabel(tr("未命名的 TCP 连接"));
  } else if (role_ == TtProtocolType::Server) {
    tcp_server_ = new Core::TcpServer;
    title_ = new Ui::TtNormalLabel(tr("未命名的 TCP 服务模拟端"));
  }

  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 创建原始界面
  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout *originalWidgetLayout =
      new Ui::TtHorizontalLayout(original_widget_);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(title_, 0, Qt::AlignLeft);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(modify_title_btn_);
  originalWidgetLayout->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout *edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  // 链接对应的槽, 虚函数实现
  // BUG
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          // &SerialWindow::switchToEditMode);
          &TcpWindow::switchToEditMode);

  Ui::TtHorizontalLayout *stackLayout = new Ui::TtHorizontalLayout;
  stackLayout->addWidget(stack_);

  // 总布局
  Ui::TtHorizontalLayout *topLayout = new Ui::TtHorizontalLayout;

  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  // // 水平布局
  // Ui::TtHorizontalLayout *tmpl2 = new Ui::TtHorizontalLayout;
  // 右侧水平栏的按钮
  Ui::TtHorizontalLayout *operationButtonLayout = new Ui::TtHorizontalLayout;
  // 保存按钮
  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);

  // 右侧开启按钮
  // 删除按钮, 是需要保存在 leftbar 才会添加的

  // 开关按钮
  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setColors(Qt::black, Qt::red);
  on_off_btn_->setSvgSize(18, 18);

  operationButtonLayout->addWidget(save_btn_);
  operationButtonLayout->addWidget(on_off_btn_, 0, Qt::AlignRight);
  operationButtonLayout->addSpacerItem(new QSpacerItem(10, 10));

  // tmpAll->addLayout(tmpP1);
  topLayout->addLayout(stackLayout);
  topLayout->addLayout(operationButtonLayout);
  // topLayout->addLayout(tmpl2);

  // main_layout_->addLayout(tmpAll);
  main_layout_->addLayout(topLayout);

  // 左右分隔器
  main_splitter_ = new QSplitter;
  main_splitter_->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget *chose_function = new QWidget;
  Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  // 切换
  QWidget *twoBtnForGroup = new QWidget(chose_function);
  QHBoxLayout *layouttest = new QHBoxLayout(twoBtnForGroup);
  Ui::TtSvgButton *terminalButton =
      new Ui::TtSvgButton(":/sys/terminal.svg", twoBtnForGroup);
  terminalButton->setSvgSize(18, 18);
  terminalButton->setColors(Qt::black, Qt::blue);

  // qDebug() << "terminal: " << terminalButton;
  // leftBtn->setEnableHoldToCheck(true);
  Ui::TtSvgButton *chatButton =
      new Ui::TtSvgButton(":/sys/chat.svg", twoBtnForGroup);
  chatButton->setSvgSize(18, 18);
  chatButton->setColors(Qt::black, Qt::blue);
  // qDebug() << "chat: " << chatButton;
  // rightBtn->setEnableHoldToCheck(true);

  Ui::TtSvgButton *graphBtn =
      new Ui::TtSvgButton(":/sys/graph-up.svg", twoBtnForGroup);
  graphBtn->setSvgSize(18, 18);
  graphBtn->setColors(Qt::black, Qt::blue);
  // qDebug() << "graph: " << graphBtn;

  layouttest->addWidget(terminalButton);
  layouttest->addWidget(chatButton);
  layouttest->addWidget(graphBtn);

  // QAction *test = new QAction;
  // 图标颜色显示有问题
  // 互斥
  Ui::TtWidgetGroup *showStyle = new Ui::TtWidgetGroup(this);
  showStyle->setHoldingChecked(true);
  showStyle->addWidget(terminalButton);
  showStyle->addWidget(chatButton);
  showStyle->addWidget(graphBtn);
  showStyle->setExclusive(true);
  showStyle->setCheckedIndex(0);
  chose_function_layout->addWidget(twoBtnForGroup);
  chose_function_layout->addStretch();

  // 点击多次有 bug
  clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history_->setSvgSize(18, 18);

  //// 选择 text/hex
  Ui::TtWidgetGroup *styleGroup = new Ui::TtWidgetGroup(this);
  styleGroup->setHoldingChecked(true);
  // 点击切换后
  // 时间戳消失, 渲染有问题
  Ui::TtTextButton *textBtn = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  // textBtn->setLightDefaultColor(QColor(0, 102, 180));
  // textBtn->setLightHoverColor(QColor(0, 112, 198));
  textBtn->setCheckedColor(QColor(0, 102, 180));
  Ui::TtTextButton *hexBtn = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
  // hexBtn->setLightDefaultColor(QColor(0, 102, 180));
  // hexBtn->setLightHoverColor(QColor(0, 112, 198));
  hexBtn->setCheckedColor(QColor(0, 102, 180));

  styleGroup->addWidget(textBtn);
  styleGroup->addWidget(hexBtn);
  styleGroup->setCheckedIndex(0);
  styleGroup->setExclusive(true);
  chose_function_layout->addWidget(textBtn);
  chose_function_layout->addWidget(hexBtn);
  textBtn->setChecked(true);
  connect(styleGroup, &Ui::TtWidgetGroup::widgetClicked,
          [this](const int &index) {
            // setDisplayType(index == 1 ? TtTextFormat::HEX :
            // TtTextFormat::TEXT);
          });

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history_);

  QSplitter *VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  VSplitter->setSizes(QList<int>() << 500 << 200);

  // 上方选择功能以及信息框
  QWidget *contentWidget = new QWidget;
  Ui::TtVerticalLayout *contentWidgetLayout =
      new Ui::TtVerticalLayout(contentWidget);

  QStackedWidget *messageStackedView = new QStackedWidget(contentWidget);

  terminal_ = new QPlainTextEdit(this);
  terminal_->setReadOnly(true);
  terminal_->setFrameStyle(QFrame::NoFrame);
  SerialHighlighter *lexer = new SerialHighlighter(terminal_->document());

  messageStackedView->addWidget(terminal_);

  message_view_ = new Ui::TtChatView(messageStackedView);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  messageStackedView->addWidget(message_view_);

  // // 需要 ???
  // QSplitter *graphSpiltter = new QSplitter;

  // // 每一个窗口都需要一个 ???

  // // 为什么不同的窗口, 会实现同一份数据
  // serial_plot_ = new Ui::TtSerialPortPlot;
  // graphSpiltter->addWidget(serial_plot_);

  // serialDataList = new QListWidget(graphSpiltter);
  // serialDataList->setContextMenuPolicy(
  //     Qt::CustomContextMenu); // 启用自定义右键菜单
  // connect(
  //     serialDataList, &QListWidget::customContextMenuRequested, this,
  //     [=](const QPoint &pos) {
  //       // 获取当前右键点击的项
  //       QListWidgetItem *item = serialDataList->itemAt(pos);

  //       // 创建菜单
  //       QMenu menu;
  //       QAction *addAction = menu.addAction(tr("添加"));

  //       QAction *editAction = nullptr;
  //       QAction *deleteAction = nullptr;
  //       QAction *renameAction = nullptr;

  //       if (item) {
  //         editAction = menu.addAction(tr("编辑"));
  //         deleteAction = menu.addAction(tr("删除"));
  //         renameAction = menu.addAction(tr("重命名"));
  //       }

  //       // 处理菜单点击
  //       QAction *selectedAction =
  //           menu.exec(serialDataList->viewport()->mapToGlobal(pos));
  //       if (!selectedAction) {
  //         // 点击 menu 的其他区域
  //         return;
  //       }
  //       if (selectedAction == addAction) {
  //         // 此处可以更改颜色
  //         TtChannelButtonEditorDialog editorDialog(this);
  //         if (editorDialog.exec() == QDialog::Accepted) {
  //           saved_ = false;
  //           // 颜色生成
  //           // 创建 button, 添加 channel 信息
  //           // 随机生成的颜色保存在 editorDialog 中
  //           // addChannel(channelBtn->getCheckBlockColor(),
  //           // editorDialog.metaInfo());
  //           addChannel(editorDialog.metaInfo(),
  //           editorDialog.checkBlockColor());
  //         }
  //       } else if (item) {
  //         if (selectedAction == renameAction) {
  //           if (auto *btn = qobject_cast<TtChannelButton *>(
  //                   serialDataList->itemWidget(item))) {
  //             saved_ = false;
  //             btn->modifyText();
  //           }
  //         } else if (selectedAction == deleteAction) {
  //           auto *btn = qobject_cast<TtChannelButton *>(
  //               serialDataList->itemWidget(item));
  //           if (btn) {
  //             // 删除
  //             saved_ = false;
  //             // quint16 channel =
  //             // channel_info_.value(btn->getUuid()).first;
  //             quint16 channel =
  //                 channel_info_.value(btn->getUuid()).channel_num_;
  //             qDebug() << channel;
  //             serial_plot_->removeGraphs(channel);
  //             lua_script_codes_.remove(channel);
  //             // 移出规则
  //             channel_info_.remove(btn->getUuid());
  //             rules_.remove(btn->getUuid());
  //           }
  //           auto *deleteItem =
  //               serialDataList->takeItem(serialDataList->row(item)); //
  //               删除项

  //           qDebug() << "delete";
  //           delete deleteItem;

  //         } else if (selectedAction == editAction) {
  //           if (auto *btn = qobject_cast<TtChannelButton *>(
  //                   serialDataList->itemWidget(item))) {
  //             saved_ = false;
  //             // 数据保存在 info 中
  //             // 根据存在的 btn
  //             // 编辑状态
  //             TtChannelButtonEditorDialog editorDialog(btn, this);
  //             // 设置编辑的数据
  //             editorDialog.setMetaInfo(
  //                 channel_info_.value(btn->getUuid()).data);
  //             if (editorDialog.exec() == QDialog::Accepted) {
  //               // 编辑成功
  //               qDebug() << editorDialog.checkBlockColor();
  //               // BUG 编辑和添加有冲突
  //               // 通道信息中存储的是 0
  //               // 但是修改后, 新增加不同的值后, 变为了 1, 就一直是 1
  //               // BUG 获取的值是错误的 1
  //               // 一直都是同一个 uuid 的 button
  //               // 但是获取的对应的通道号 是 1
  //               qDebug() << "编辑时的(关乎于 button 的特定值) channel uuid: "
  //                        << btn->getUuid()
  //                        << channel_info_.value(btn->getUuid()).channel_num_;

  //               handleDialogData(
  //                   btn->getUuid(),
  //                   channel_info_.value(btn->getUuid()).channel_num_,
  //                   editorDialog.metaInfo(), editorDialog.checkBlockColor());
  //             }
  //           }
  //         }
  //       }
  //     });

  // serialDataList->setContentsMargins(QMargins());
  // serialDataList->setSpacing(0);
  // serialDataList->setStyleSheet(
  //     // 设置列表整体背景色
  //     "QListWidget {"
  //     "   background-color: white;"
  //     "   outline: none;" // 移除焦点虚线框（关键）
  //     "}"
  //     // 项选中状态样式
  //     "QListWidget::item:selected {"
  //     "   background: transparent;" // 强制透明背景
  //     "}"
  //     // 项悬停状态样式
  //     "QListWidget::item:hover {"
  //     "   background: transparent;" // 防止悬停变色
  //     "}"
  //     // 项按下状态样式
  //     "QListWidget::item:pressed {"
  //     "   background: transparent;" // 防止按压变色
  //     "}");

  // // 启用该模式, 会导致有虚线框
  // graphSpiltter->addWidget(serialDataList);
  // graphSpiltter->setSizes(QList<int>{500, 100});
  // graphSpiltter->setCollapsible(0, false);
  // graphSpiltter->setCollapsible(1, false);
  // graphSpiltter->setStretchFactor(0, 1);
  // graphSpiltter->setStretchFactor(1, 0);

  // messageStackedView->addWidget(graphSpiltter);

  contentWidgetLayout->addWidget(chose_function);
  contentWidgetLayout->addWidget(messageStackedView);

  connect(showStyle, &Ui::TtWidgetGroup::widgetClicked, this,
          [this, messageStackedView, textBtn, hexBtn](const int &idx) {
            messageStackedView->setCurrentIndex(idx);
            if (idx != 0) {
              // 图表
              textBtn->setVisible(false);
              hexBtn->setVisible(false);
            } else {
              textBtn->setVisible(true);
              hexBtn->setVisible(true);
            }
          });

  base::DetectRunningTime runtime;

  message_model_ = new Ui::TtChatMessageModel;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget *bottomAll = new QWidget;
  Ui::TtVerticalLayout *bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget *tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout *tacLayout = new Ui::TtHorizontalLayout();
  tabs_and_count->setLayout(tacLayout);

  auto m_tabs = new QtMaterialTabs(tabs_and_count);
  m_tabs->addTab(tr("手动"));
  m_tabs->addTab(tr("片段"));
  // m_tabs->setFixedHeight(30);
  m_tabs->setBackgroundColor(QColor(192, 120, 196));
  // m_tabs->setMinimumWidth(80);

  tacLayout->addWidget(m_tabs);
  tacLayout->addStretch();

  // 显示发送字节和接收字节数
  send_byte_ = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  send_byte_->setFixedHeight(30);
  recv_byte_ = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  recv_byte_->setFixedHeight(30);

  tacLayout->addWidget(send_byte_);
  tacLayout->addWidget(recv_byte_);

  QWidget *la_w = new QWidget(this);
  QStackedLayout *layout = new QStackedLayout(la_w);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);

  QWidget *messageEdit = new QWidget(la_w);
  QVBoxLayout *messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 0, 3, 0);
  messageEditLayout->setSpacing(0);

  editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);
  editor->setFrameStyle(QFrame::NoFrame);

  messageEditLayout->addWidget(editor);

  QWidget *bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  Ui::TtHorizontalLayout *bottomBtnWidgetLayout =
      new Ui::TtHorizontalLayout(bottomBtnWidget);

  // 发送的格式
  chose_text_ = new Ui::TtRadioButton("TEXT", bottomBtnWidget);
  chose_hex_ = new Ui::TtRadioButton("HEX", bottomBtnWidget);
  chose_text_->setChecked(true);

  sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  sendBtn->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(chose_text_);
  bottomBtnWidgetLayout->addWidget(chose_hex_);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(sendBtn);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(la_w);

  layout->addWidget(messageEdit);
  layout->addWidget(instruction_table_);

  // connect(instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
  //         qOverload<const QString &>(&SerialWindow::sendMessageToPort));

  // 表格
  connect(instruction_table_, &Ui::TtTableWidget::sendRowsMsg, this,
          [this](const QVector<QPair<QString, int>> &datas) {
            if (datas.isEmpty()) {
              return;
            }
            foreach (const auto &pair, datas) {
              // BUG
              // sendMessageToPort(pair.first, pair.second);
            }
          });
  connect(instruction_table_, &Ui::TtTableWidget::rowsChanged, this,
          [this]() { saved_ = false; });

  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged,
          [this, layout](int index) { layout->setCurrentIndex(index); });

  // 显示, 并输入 lua 脚本
  // lua_code_ = new Ui::TtLuaInputBox(this);
  // lua_actuator_ = new Core::LuaKernel;

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(contentWidget);
  VSplitter->addWidget(bottomAll);
  VSplitter->setCollapsible(0, false);

  main_splitter_->addWidget(VSplitter);
  if (role_ == TtProtocolType::Client) {
    tcp_client_setting_ = new Widget::TcpClientSetting;
    main_splitter_->addWidget(tcp_client_setting_);
  } else {
    tcp_server_setting_ = new Widget::TcpServerSetting;
    main_splitter_->addWidget(tcp_server_setting_);
  }
  // serial_setting_ = new Widget::SerialSetting;
  // main_splitter_->addWidget(serial_setting_);

  main_splitter_->setSizes(QList<int>() << 500 << 220);
  main_splitter_->setCollapsible(0, false);
  main_splitter_->setCollapsible(1, true);

  main_splitter_->setStretchFactor(0, 3);
  main_splitter_->setStretchFactor(1, 2);

  // 主界面是左右分隔
  main_layout_->addWidget(main_splitter_);

  // send_package_timer_ = new QTimer(this);
  // send_package_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  // send_package_timer_->setInterval(0);
  // heartbeat_timer_ = new QTimer(this);
  // heartbeat_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  // heartbeat_timer_->setInterval(0);
}

void TcpWindow::connectSignals() {
  // connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
  //   config_.insert("WindowTitile", title_->text());
  //   if (role_ == TtProtocolType::Client) {
  //     config_.insert("TcpClientSetting",
  //                    tcp_client_setting_->getTcpClientSetting());
  //   } else if (role_ == TtProtocolType::Server) {
  //     config_.insert("TcpServerSetting",
  //                    tcp_server_setting_->getTcpServerSetting());
  //   }
  //   config_.insert("InstructionTable", instruction_table_->getTableRecord());
  //   emit requestSaveConfig();
  // });
  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &TcpWindow::saveSetting);
}

} // namespace Window
