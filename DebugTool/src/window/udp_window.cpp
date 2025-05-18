#include "udp_window.h"

#include <io.h>
#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>

#include "core/udp_client.h"
#include "core/udp_server.h"
#include "ui/controls/TtTableView.h"
#include "widget/udp_setting.h"

namespace Window {

UdpWindow::UdpWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    : FrameWindow(parent), role_(role) {
  init();

  if (role_ == TtProtocolType::Client) {

  } else if (role_ == TtProtocolType::Server) {
    connect(udp_server_, &Core::UdpServer::datagramReceived,
            [this](const QString &peerInfo, const quint16 &peerPort,
                   const QByteArray &message) {
              // 显示
              onDataReceived(message);
              // qDebug() << "From" << peerInfo << "Received:" << peerPort;
              // qDebug() << "Data:" << message;
            });

    QObject::connect(udp_server_, &Core::UdpServer::errorOccurred,
                     [this](const QString &error) {
                       Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                                               error, 1500, this);
                     });
  }
  connectSignals();
}

UdpWindow::~UdpWindow() { qDebug() << "Delete UdpWindow"; }

QJsonObject UdpWindow::getConfiguration() const { return config_; }

bool UdpWindow::workState() const { return opened_; }

bool UdpWindow::saveState() { return saved_; }

void UdpWindow::setSaveState(bool state) { saved_ = state; }

void UdpWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitle", title_->text());
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    config_.insert("UdpClientSetting",
                   udp_client_setting_->getUdpClientSetting());
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
    config_.insert("UdpServerSetting",
                   udp_server_setting_->getUdpServerSetting());
  }
  config_.insert("InstructionTable", instruction_table_->getTableRecord());
  saved_ = true;
  emit requestSaveConfig();
}

void UdpWindow::setSetting(const QJsonObject &config) {
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  if (role_ == TtProtocolType::Client) {
    udp_client_setting_->setOldSettings(
        config.value("UdpClientSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    udp_server_setting_->setOldSettings(
        config.value("UdpServerSetting").toObject(QJsonObject()));
  }
  saved_ = true;
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

QString UdpWindow::getTitle() const { return title_->text(); }

void UdpWindow::switchToEditMode() {
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

void UdpWindow::switchToDisplayMode() {
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

void UdpWindow::updateServerStatus() {
  // const bool running = tcp_server_->isRunning();
  // auto text =
  //     (running ? tr("服务运行中 (端口: %1)").arg("111") : tr("服务已停止"));
  // qDebug() << text;
}

void UdpWindow::onDataReceived(const QByteArray &data) {
  // recv_byte_count += data.size();
  recv_byte_count_ += data.size();
  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setOutgoing(false);
  tmp->setBubbleColor(QColor("#0ea5e9"));
  QList<Ui::TtChatMessage *> list;
  list.append(tmp);
  message_model_->appendMessages(list);
  recv_byte_->setText(QString("接收字节数: %1 B").arg(recv_byte_count_));
  message_view_->scrollToBottom();
}

void UdpWindow::init() {

  // main_layout_ = new Ui::TtVerticalLayout(this);

  // if (role_ == TtProtocolType::Client) {
  //   udp_client_ = new Core::UdpClient;
  //   title_ = new Ui::TtNormalLabel(tr("未命名的 UDP 连接"));
  // } else if (role_ == TtProtocolType::Server) {
  //   udp_server_ = new Core::UdpServer;
  //   title_ = new Ui::TtNormalLabel(tr("未命名的 UDP 模拟服务"));
  // }
  // // 编辑命名按钮
  // modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit_name.svg", this);
  // modify_title_btn_->setSvgSize(18, 18);

  // // 创建原始界面
  // original_widget_ = new QWidget(this);
  // Ui::TtHorizontalLayout *tmpl = new
  // Ui::TtHorizontalLayout(original_widget_); tmpl->addSpacerItem(new
  // QSpacerItem(10, 10)); tmpl->addWidget(title_, 0, Qt::AlignLeft);
  // tmpl->addSpacerItem(new QSpacerItem(10, 10));
  // tmpl->addWidget(modify_title_btn_);
  // tmpl->addStretch();

  // // 创建编辑界面
  // edit_widget_ = new QWidget(this);
  // title_edit_ = new Ui::TtLineEdit(this);

  // Ui::TtHorizontalLayout *edit_layout =
  //     new Ui::TtHorizontalLayout(edit_widget_);
  // edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  // edit_layout->addWidget(title_edit_);
  // edit_layout->addStretch();

  // // 使用堆叠布局
  // stack_ = new QStackedWidget(this);
  // stack_->setMaximumHeight(40);
  // stack_->addWidget(original_widget_);
  // stack_->addWidget(edit_widget_);

  // // 优化后的信号连接（仅需2个连接点）
  // connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
  //         &UdpWindow::switchToEditMode);

  // Ui::TtHorizontalLayout *tmpP1 = new Ui::TtHorizontalLayout;
  // tmpP1->addWidget(stack_);

  // Ui::TtHorizontalLayout *tmpAll = new Ui::TtHorizontalLayout;

  // // 保存 lambda 表达式
  // auto handleSave = [this]() {
  //   // qDebug() << "失去";
  //   if (!title_edit_->text().isEmpty()) {
  //     switchToDisplayMode();
  //   } else {
  //     title_edit_->setPlaceholderText(tr("名称不能为空！"));
  //   }
  // };

  // connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  // Ui::TtHorizontalLayout *tmpl2 = new Ui::TtHorizontalLayout;
  // // 保存按钮
  // save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  // save_btn_->setSvgSize(18, 18);
  // // 删除按钮, 是需要保存在 leftbar 才会添加的

  // // 开关按钮
  // on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  // on_off_btn_->setSvgSize(18, 18);
  // on_off_btn_->setColors(Qt::black, Qt::red);

  // tmpl2->addWidget(save_btn_);
  // tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  // tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  // tmpAll->addLayout(tmpP1);
  // tmpAll->addLayout(tmpl2);

  // main_layout_->addLayout(tmpAll);

  // // 左右分隔器
  // QSplitter *mainSplitter = new QSplitter;
  // mainSplitter->setOrientation(Qt::Horizontal);

  // // 上方功能按钮
  // QWidget *chose_function = new QWidget;
  // Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  // chose_function_layout->setSpacing(5);
  // chose_function->setLayout(chose_function_layout);

  // chose_function_layout->addStretch();
  // Ui::TtSvgButton *clear_history =
  //     new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  // clear_history->setSvgSize(18, 18);

  // // auto bgr = new CustomButtonGroup(chose_function);
  // //  选择 text/hex
  // // chose_function_layout->addWidget(bgr);

  // // 清除历史按钮
  // chose_function_layout->addWidget(clear_history);

  // // 中间的弹簧

  // // 上下分隔器
  // QSplitter *VSplitter = new QSplitter;
  // VSplitter->setOrientation(Qt::Vertical);
  // VSplitter->setContentsMargins(QMargins());
  // // VSplitter->setSizes();

  // // 上方选择功能以及信息框
  // QWidget *cont = new QWidget;
  // Ui::TtVerticalLayout *cont_layout = new Ui::TtVerticalLayout(cont);

  // message_view_ = new Ui::TtChatView(cont);
  // message_view_->setResizeMode(QListView::Adjust);
  // message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  // message_view_->setMouseTracking(true);
  // message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  // cont_layout->addWidget(chose_function);
  // cont_layout->addWidget(message_view_);

  // message_model_ = new Ui::TtChatMessageModel;
  // QList<Ui::TtChatMessage *> list;

  // message_view_->setModel(message_model_);
  // message_view_->scrollToBottom();

  // QWidget *bottomAll = new QWidget;
  // Ui::TtVerticalLayout *bottomAllLayout = new
  // Ui::TtVerticalLayout(bottomAll);

  // // 下方自定义指令
  // QWidget *tabs_and_count = new QWidget(this);
  // Ui::TtHorizontalLayout *tacLayout = new Ui::TtHorizontalLayout();
  // tabs_and_count->setLayout(tacLayout);

  // auto m_tabs = new QtMaterialTabs;
  // // m_tabs->setBackgroundColor()
  // m_tabs->addTab(tr("手动"));
  // // m_tabs
  // m_tabs->addTab(tr("片段"));
  // // m_tabs->setBackgroundColor(QColor::fromRgbF(255, 255, 255));
  // m_tabs->setMinimumWidth(80);

  // tacLayout->addWidget(m_tabs);
  // tacLayout->addStretch();

  // // 显示发送字节和接收字节数
  // send_byte = new Ui::TtNormalLabel(tr("发送字节数: 0 B"), tabs_and_count);
  // send_byte->setFixedHeight(30);
  // recv_byte = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_and_count);
  // recv_byte->setFixedHeight(30);

  // tacLayout->addWidget(send_byte);
  // tacLayout->addWidget(recv_byte);

  // QStackedLayout *layout = new QStackedLayout;
  // layout->setContentsMargins(QMargins());
  // layout->setSpacing(0);
  // QWidget *basicWidget = new QWidget(this);
  // basicWidget->setLayout(layout);

  // QWidget *messageEdit = new QWidget(basicWidget);
  // // messageEdit
  // QVBoxLayout *messageEditLayout = new QVBoxLayout;
  // messageEdit->setLayout(messageEditLayout);
  // messageEditLayout->setContentsMargins(3, 3, 3, 3);
  // messageEditLayout->setSpacing(0);

  // editor = new QsciScintilla(messageEdit);
  // editor->setWrapMode(QsciScintilla::WrapWord);
  // editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
  //                            QsciScintilla::WrapFlagInMargin, 0);
  // editor->setCaretWidth(10);
  // editor->setMarginType(1, QsciScintilla::NumberMargin);

  // messageEditLayout->addWidget(editor);

  // QWidget *bottomBtnWidget = new QWidget(messageEdit);
  // bottomBtnWidget->setMinimumHeight(40);
  // // bottomBtnWidget->setStyleSheet("back)ground-color: red");
  // Ui::TtHorizontalLayout *bottomBtnWidgetLayout = new Ui::TtHorizontalLayout;
  // bottomBtnWidgetLayout->setContentsMargins(QMargins());
  // bottomBtnWidgetLayout->setSpacing(0);
  // bottomBtnWidget->setLayout(bottomBtnWidgetLayout);

  // QtMaterialRadioButton *choseText = new
  // QtMaterialRadioButton(bottomBtnWidget); QtMaterialRadioButton *choseHex =
  // new QtMaterialRadioButton(bottomBtnWidget); choseText->setText("TEXT");
  // choseHex->setText("HEX");

  // QtMaterialFlatButton *sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  // sendBtn->setIcon(QIcon(":/sys/send.svg"));
  // bottomBtnWidgetLayout->addWidget(choseText);
  // bottomBtnWidgetLayout->addWidget(choseHex);
  // bottomBtnWidgetLayout->addStretch();
  // bottomBtnWidgetLayout->addWidget(sendBtn);

  // messageEditLayout->addWidget(bottomBtnWidget);

  // instruction_table_ = new Ui::TtTableWidget(basicWidget);

  // layout->addWidget(messageEdit);
  // layout->addWidget(instruction_table_);
  // layout->setCurrentIndex(0);

  // connect(m_tabs, &QtMaterialTabs::currentChanged,
  //         [this, layout](int index) { layout->setCurrentIndex(index); });

  // bottomAllLayout->addWidget(tabs_and_count);
  // bottomAllLayout->addWidget(basicWidget);

  // VSplitter->addWidget(cont);
  // VSplitter->addWidget(bottomAll);

  // // 左右分区
  // mainSplitter->addWidget(VSplitter);

  // // 根据不同的角色，选择添加不同的窗口
  // if (role_ == TtProtocolType::Client) {
  //   udp_client_setting_ = new Widget::UdpClientSetting;
  //   mainSplitter->addWidget(udp_client_setting_);
  // } else {
  //   udp_server_setting_ = new Widget::UdpServerSetting;
  //   mainSplitter->addWidget(udp_server_setting_);
  // }

  // // 主界面是左右分隔
  // main_layout_->addWidget(mainSplitter);

  initUi();

  if (role_ == TtProtocolType::Client) {
    udp_client_ = new Core::UdpClient;
    title_ = new Ui::TtNormalLabel(tr("未命名的 UDP 连接"));
    udp_client_setting_ = new Widget::UdpClientSetting;
    serRightWidget(udp_client_setting_);
  } else if (role_ == TtProtocolType::Server) {
    udp_server_ = new Core::UdpServer;
    title_ = new Ui::TtNormalLabel(tr("未命名的 UDP 模拟服务"));
    udp_server_setting_ = new Widget::UdpServerSetting;
    serRightWidget(udp_client_setting_);
  }
}

void UdpWindow::connectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &UdpWindow::saveSetting);
  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    if (opened_) {
      if (role_ == TtProtocolType::Client) {
        udp_client_->close();
      } else if (role_ == TtProtocolType::Server) {
        udp_server_->close();
      }
    } else {
      if (role_ == TtProtocolType::Client) {
        udp_client_->connect(udp_client_setting_->getUdpClientConfiguration());
      } else if (role_ == TtProtocolType::Server) {
        if (udp_server_->listen(
                udp_server_setting_->getUdpServerConfiguration())) {
          opened_ = true;
        } else {
          opened_ = false;
        }
      }
    }
  });

  connect(clear_history_, &Ui::TtSvgButton::clicked,
          [this]() { message_model_->clearModelData(); });

  connect(send_btn_, &QtMaterialFlatButton::clicked, [this]() {
    QString data = editor_->text();
    send_byte_count_ += data.size();
    auto tmp = new Ui::TtChatMessage();
    tmp->setContent(data);
    tmp->setOutgoing(true);
    tmp->setBubbleColor(QColor("#DCF8C6"));
    QList<Ui::TtChatMessage *> list;
    list.append(tmp);
    message_model_->appendMessages(list);

    if (role_ == TtProtocolType::Client) {
      udp_client_->sendMessage(data.toUtf8());
    } else if (role_ == TtProtocolType::Server) {
      udp_server_->sendMessage(data.toUtf8());
    }

    send_byte_->setText(QString("发送字节数: %1 B").arg(send_byte_count_));
    message_view_->scrollToBottom();
  });
}

} // namespace Window
