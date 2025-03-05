#include "window/tcp_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtTableView.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/fields/customize_fields.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/window/combobox.h>

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>

#include <QTableView>

#include "core/tcp_client.h"
#include "core/tcp_server.h"
#include "widget/shortcut_instruction.h"
#include "widget/tcp_setting.h"

namespace Window {

TcpWindow::TcpWindow(TtProtocolType::ProtocolRole role, QWidget* parent)
    : QWidget(parent), role_(role) {

  init();

  if (role_ == TtProtocolType::Client) {
    connect(tcp_client_, &Core::TcpClient::connected, [this]() {
      // qDebug() << "连接到服务器";
      tcp_opened_ = true;
    });
    connect(tcp_client_, &Core::TcpClient::disconnected, [this]() {
      // qDebug() << "断开连接";
      tcp_opened_ = false;
    });
    connect(tcp_client_, &Core::TcpClient::dataReceived, this,
            &TcpWindow::onDataReceived);
    connect(tcp_client_, &Core::TcpClient::errorOccurred, this,
            [this](const QString& error) {
              Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error,
                                      1500, this);
              // qDebug() << "eeeor ";
              on_off_btn_->setChecked(false);
              tcp_opened_ = false;
            });
  } else {
    connect(tcp_server_, &Core::TcpServer::serverStarted, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::serverStopped, this,
            &TcpWindow::updateServerStatus);
    connect(tcp_server_, &Core::TcpServer::errorOccurred,
            [this](const QString& err) {
              QMessageBox::critical(this, tr("错误"), err);
            });
    connect(tcp_server_, &Core::TcpServer::dataReceived, this,
            &TcpWindow::onDataReceived);
  }
  connectSignals();
}

QString TcpWindow::getTitle() {
  return title_->text();
}

void TcpWindow::switchToEditMode() {
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

void TcpWindow::switchToDisplayMode() {
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

void TcpWindow::updateServerStatus() {
  const bool running = tcp_server_->isRunning();
  auto text =
      (running ? tr("服务运行中 (端口: %1)").arg("111") : tr("服务已停止"));
  qDebug() << text;
}

void TcpWindow::onDataReceived(const QByteArray& data) {
  qDebug() << "Received data:" << data;
  recv_byte_count += data.size();
  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setOutgoing(false);
  tmp->setBubbleColor(QColor("#0ea5e9"));
  QList<Ui::TtChatMessage*> list;
  list.append(tmp);
  message_model_->appendMessages(list);
  recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  message_view_->scrollToBottom();
}

void TcpWindow::init() {
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
  Ui::TtHorizontalLayout* tmpl = new Ui::TtHorizontalLayout(original_widget_);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(title_, 0, Qt::AlignLeft);
  tmpl->addSpacerItem(new QSpacerItem(10, 10));
  tmpl->addWidget(modify_title_btn_);
  tmpl->addStretch();

  // 创建编辑界面
  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout* edit_layout = new Ui::TtHorizontalLayout(edit_widget_);
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
  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // 上方功能按钮
  QWidget* chose_function = new QWidget;
  Ui::TtHorizontalLayout* chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  chose_function_layout->addStretch();
  Ui::TtSvgButton* clear_history =
      new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history->setSvgSize(18, 18);

  //auto bgr = new CustomButtonGroup(chose_function);
  // 选择 text/hex
  //chose_function_layout->addWidget(bgr);

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
  Ui::TtVerticalLayout* cont_layout = new Ui::TtVerticalLayout(cont);

  message_view_ = new Ui::TtChatView(cont);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cont_layout->addWidget(chose_function);
  cont_layout->addWidget(message_view_);

  message_model_ = new Ui::TtChatMessageModel;
  QList<Ui::TtChatMessage*> list;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget* bottomAll = new QWidget;
  Ui::TtVerticalLayout* bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);

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

  editor = new QsciScintilla(messageEdit);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);

  messageEditLayout->addWidget(editor);

  QWidget* bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  //bottomBtnWidget->setStyleSheet("back)ground-color: red");
  Ui::TtHorizontalLayout* bottomBtnWidgetLayout = new Ui::TtHorizontalLayout;
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

  layout->addWidget(messageEdit);
  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, [this, layout](int index) {
    layout->setCurrentIndex(index);
  });

  bottomAllLayout->addWidget(tabs_and_count);
  // bottomAllLayout->addWidget(messageEdit);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(cont);
  VSplitter->addWidget(bottomAll);

  // 左右分区
  mainSplitter->addWidget(VSplitter);

  // 根据不同的角色，选择添加不同的窗口
  if (role_ == TtProtocolType::Client) {
    tcp_client_setting_ = new Widget::TcpClientSetting;
    mainSplitter->addWidget(tcp_client_setting_);
  } else {
    tcp_server_setting_ = new Widget::TcpServerSetting;
    mainSplitter->addWidget(tcp_server_setting_);
  }

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    if (tcp_opened_) {
      if (role_ == TtProtocolType::Client) {
        qDebug() << "test";
        tcp_client_->disconnectFromServer();
      } else if (role_ == TtProtocolType::Server) {
        tcp_server_->close();
      }
      // tcp_opened_ = false;
    } else {
      if (role_ == TtProtocolType::Client) {
        qDebug() << "connectToServer";
        tcp_client_->connectToServer(
            tcp_client_setting_->getTcpClientConfiguration());
      } else if (role_ == TtProtocolType::Server) {

        tcp_server_->startServer(
            tcp_server_setting_->getTcpServerConfiguration());
      }
      // tcp_opened_ = true;
    }
  });

  connect(clear_history, &Ui::TtSvgButton::clicked,
          [this]() { message_model_->clearModelData(); });

  connect(sendBtn, &QtMaterialFlatButton::clicked, [this]() {
    QString data = editor->text();
    send_byte_count += data.size();
    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(data);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage*> list;
    list.append(tmp);
    message_model_->appendMessages(list);

    if (role_ == TtProtocolType::Client) {
      tcp_client_->sendMessage(data.toUtf8());
      // emit requestSendMessage(data.toUtf8());
    } else {
      tcp_server_->sendMessageToClients(data.toUtf8());
    }

    send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
    message_view_->scrollToBottom();
  });
}

void TcpWindow::connectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // 保存配置界面, 但没有历史消息
    // cfg_.obj.insert("WindowTitle", title_->text());
    // cfg_.obj.insert("SerialSetting", serial_setting_->getSerialSetting());
    // cfg_.obj.insert("InstructionTable", instruction_table_->getTableRecord());
    // qDebug() << "yes";
    // Ui::TtMessageBar::success(
    //     TtMessageBarType::Top, "警告",
    //     // "输入框不能为空，请填写完整信息。", 3000, this);
    //     "输入框不能为空，请填写完整信息。", 3000, this);
    emit requestSaveConfig();
    //qDebug() << cfg_.obj;
    // 配置文件保存到文件中
    // 当前的 tabWidget 匹配对应的 QJsonObject
  });
}

}  // namespace Window
