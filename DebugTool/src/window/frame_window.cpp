#include "window/frame_window.h"

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

#include "ui/controls/TtTableView.h"

#include <lib/qtmaterialcheckable.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialradiobutton.h>
#include <qtmaterialsnackbar.h>
#include <qtmaterialtabs.h>
#include <ui/controls/TtSerialLexer.h>

#include <QTableView>
#include <Qsci/qsciscintilla.h>

namespace Window {

FrameWindow::FrameWindow(QWidget *parent) : QWidget{parent} {}

FrameWindow::~FrameWindow() {}

QString FrameWindow::title() const { return QString("TtFrameWindow"); }

void FrameWindow::serRightWidget(QWidget *widget) {
  auto *originalWidget = main_splitter_->replaceWidget(1, widget);
  if (originalWidget) {
    originalWidget->setParent(nullptr);
    delete originalWidget;
  }
}

void FrameWindow::addDisplayWidget(Ui::TtSvgButton *btn, QWidget *widget) {
  page_btn_layout_->addWidget(btn);
  page_btn_logical_->addWidget(btn);
  message_stacked_view_->addWidget(widget);
}

void FrameWindow::initUi() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  title_ = new Ui::TtNormalLabel(this);

  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  original_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout *originalWidgetLayout =
      new Ui::TtHorizontalLayout(original_widget_);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(title_, 0, Qt::AlignLeft);
  originalWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  originalWidgetLayout->addWidget(modify_title_btn_);
  originalWidgetLayout->addStretch();

  edit_widget_ = new QWidget(this);
  title_edit_ = new Ui::TtLineEdit(this);

  Ui::TtHorizontalLayout *edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &FrameWindow::switchToEditMode);

  Ui::TtHorizontalLayout *stackLayout = new Ui::TtHorizontalLayout;
  stackLayout->addWidget(stack_);

  Ui::TtHorizontalLayout *topLayout = new Ui::TtHorizontalLayout;

  auto handleSave = [this]() {
    if (!title_edit_->text().isEmpty()) {
      switchToDisplayMode();
    } else {
      title_edit_->setPlaceholderText(tr("名称不能为空！"));
    }
  };

  connect(title_edit_, &QLineEdit::editingFinished, this, handleSave);

  Ui::TtHorizontalLayout *operationButtonLayout = new Ui::TtHorizontalLayout;

  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);

  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setColors(Qt::black, Qt::red);
  on_off_btn_->setSvgSize(18, 18);

  operationButtonLayout->addWidget(save_btn_);
  operationButtonLayout->addWidget(on_off_btn_, 0, Qt::AlignRight);
  operationButtonLayout->addSpacerItem(new QSpacerItem(10, 10));

  topLayout->addLayout(stackLayout);
  topLayout->addLayout(operationButtonLayout);

  main_layout_->addLayout(topLayout);

  main_splitter_ = new QSplitter;
  main_splitter_->setOrientation(Qt::Horizontal);

  QWidget *chose_function = new QWidget;
  Ui::TtHorizontalLayout *chose_function_layout = new Ui::TtHorizontalLayout;
  chose_function_layout->setSpacing(5);
  chose_function->setLayout(chose_function_layout);

  // QWidget *twoBtnForGroup = new QWidget(chose_function);
  // QWidget *pageBtnWidget = new QWidget(chose_function);
  // QHBoxLayout *pageBtnLayout = new QHBoxLayout(pageBtnWidget);
  // 存在 2 层
  page_btn_widget_ = new QWidget(chose_function);
  page_btn_layout_ = new Ui::TtHorizontalLayout(page_btn_widget_);
  Ui::TtSvgButton *terminalButton =
      new Ui::TtSvgButton(":/sys/terminal.svg", page_btn_widget_);
  terminalButton->setSvgSize(18, 18);
  terminalButton->setColors(Qt::black, Qt::blue);

  // leftBtn->setEnableHoldToCheck(true);
  Ui::TtSvgButton *chatButton =
      new Ui::TtSvgButton(":/sys/chat.svg", page_btn_widget_);
  chatButton->setSvgSize(18, 18);
  chatButton->setColors(Qt::black, Qt::blue);
  // rightBtn->setEnableHoldToCheck(true);

  page_btn_layout_->addWidget(terminalButton);
  page_btn_layout_->addWidget(chatButton);

  // 左侧切换逻辑
  page_btn_logical_ = new Ui::TtWidgetGroup(this);
  // showStyle->setHoldingChecked(true);
  page_btn_logical_->setHoldingChecked(true);
  page_btn_logical_->addWidget(terminalButton);
  page_btn_logical_->addWidget(chatButton);
  page_btn_logical_->setExclusive(true);
  page_btn_logical_->setCheckedIndex(0);
  chose_function_layout->addWidget(page_btn_widget_);
  chose_function_layout->addStretch();

  clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history_->setSvgSize(18, 18);

  // 切换显示 text/hex
  Ui::TtWidgetGroup *displayLogic = new Ui::TtWidgetGroup(this);
  displayLogic->setHoldingChecked(true);
  display_text_btn_ = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  display_text_btn_->setCheckedColor(QColor(0, 102, 180));
  display_hex_btn_ = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
  display_hex_btn_->setCheckedColor(QColor(0, 102, 180));

  // styleGroup->addWidget(display_text_btn_);
  // styleGroup->addWidget(display_hex_btn_);
  displayLogic->addWidget(display_text_btn_);
  displayLogic->addWidget(display_hex_btn_);

  displayLogic->setCheckedIndex(0);
  displayLogic->setExclusive(true); // 开启了互斥

  chose_function_layout->addWidget(display_text_btn_);
  chose_function_layout->addWidget(display_hex_btn_);
  display_text_btn_->setChecked(true);

  connect(displayLogic, &Ui::TtWidgetGroup::widgetClicked, this,
          [this](int idx) {
            setDisplayType(idx == 1 ? TtTextFormat::HEX : TtTextFormat::TEXT);
          });
  // 清除历史按钮
  chose_function_layout->addWidget(clear_history_);

  QSplitter *VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  VSplitter->setSizes(QList<int>() << 500 << 200);

  // 上方选择功能以及信息框
  QWidget *contentWidget = new QWidget(this);
  Ui::TtVerticalLayout *contentWidgetLayout =
      new Ui::TtVerticalLayout(contentWidget);

  // 不同类型展示数据栈窗口
  message_stacked_view_ = new QStackedWidget(contentWidget);

  terminal_ = new QPlainTextEdit(this);
  terminal_->setReadOnly(true);
  terminal_->setFrameStyle(QFrame::NoFrame);
  // BUG 内存泄漏
  SerialHighlighter *lexer = new SerialHighlighter(terminal_->document());

  message_stacked_view_->addWidget(terminal_);

  message_view_ = new Ui::TtChatView(message_stacked_view_);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  message_stacked_view_->addWidget(message_view_);

  contentWidgetLayout->addWidget(chose_function);
  contentWidgetLayout->addWidget(message_stacked_view_);

  // 显示的图标
  connect(page_btn_logical_, &Ui::TtWidgetGroup::widgetClicked, this,
          [this](const int &idx) {
            message_stacked_view_->setCurrentIndex(idx);
            if (idx != 0) {
              // 图表
              display_hex_btn_->setVisible(false);
              display_text_btn_->setVisible(false);
            } else {
              display_hex_btn_->setVisible(true);
              display_text_btn_->setVisible(true);
            }
          });

  // base::DetectRunningTime runtime;

  message_model_ = new Ui::TtChatMessageModel;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget *bottomAll = new QWidget(this);
  Ui::TtVerticalLayout *bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  tabs_widget_ = new QWidget(this);
  Ui::TtHorizontalLayout *tacLayout = new Ui::TtHorizontalLayout(tabs_widget_);
  tabs_widget_->setLayout(tacLayout);

  tabs_ = new QtMaterialTabs(tabs_widget_);
  tabs_->addTab(tr("手动"));
  tabs_->addTab(tr("片段"));
  // tabs_->setFixedHeight(30);
  // tabs_->setMinimumWidth(80);
  tabs_->setBackgroundColor(QColor(192, 120, 196));

  tacLayout->addWidget(tabs_);
  tacLayout->addStretch();

  // 显示发送字节和接收字节数
  send_byte_ = new Ui::TtElidedLabel(tr("发送字节数: 0 B"), tabs_widget_);
  send_byte_->setFixedHeight(30);
  recv_byte_ = new Ui::TtNormalLabel(tr("接收字节数: 0 B"), tabs_widget_);
  recv_byte_->setFixedHeight(30);

  tacLayout->addWidget(send_byte_);
  tacLayout->addWidget(recv_byte_);

  display_widget_ = new QStackedWidget(this);

  QWidget *messageEdit = new QWidget(display_widget_);
  QVBoxLayout *messageEditLayout = new QVBoxLayout;
  messageEdit->setLayout(messageEditLayout);
  messageEditLayout->setContentsMargins(3, 0, 3, 0);
  messageEditLayout->setSpacing(0);

  editor_ = new QsciScintilla(messageEdit);
  editor_->setWrapMode(QsciScintilla::WrapWord);
  editor_->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                              QsciScintilla::WrapFlagInMargin, 0);
  editor_->setCaretWidth(10);
  editor_->setMarginType(1, QsciScintilla::NumberMargin);
  editor_->setFrameStyle(QFrame::NoFrame);

  messageEditLayout->addWidget(editor_);

  QWidget *bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  Ui::TtHorizontalLayout *bottomBtnWidgetLayout =
      new Ui::TtHorizontalLayout(bottomBtnWidget);

  // 发送的格式
  chose_text_btn_ = new Ui::TtRadioButton("TEXT", bottomBtnWidget);
  chose_hex_btn_ = new Ui::TtRadioButton("HEX", bottomBtnWidget);
  chose_text_btn_->setChecked(true);

  send_btn_ = new QtMaterialFlatButton(bottomBtnWidget);
  send_btn_->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(chose_text_btn_);
  bottomBtnWidgetLayout->addWidget(chose_hex_btn_);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(send_btn_);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(display_widget_);

  // 栈显示窗口
  display_widget_->addWidget(messageEdit);
  // BUG 缺少显示窗口
  display_widget_->addWidget(instruction_table_);

  display_widget_->setCurrentIndex(0);

  // 显示, 并输入 lua 脚本
  // lua_code_ = new Ui::TtLuaInputBox(this);
  // lua_actuator_ = new Core::LuaKernel;

  // bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(tabs_widget_);
  // bottomAllLayout->addWidget(displayWidget);
  bottomAllLayout->addWidget(display_widget_);

  VSplitter->addWidget(contentWidget);
  VSplitter->addWidget(bottomAll);
  VSplitter->setCollapsible(0, false);

  main_splitter_->addWidget(VSplitter);

  // 设置右侧展示窗口, 放一个置位窗口, 因为后面要设置属性
  QWidget *rightWidget = new QWidget(this);
  main_splitter_->addWidget(rightWidget);

  main_splitter_->setSizes(QList<int>() << 500 << 220);
  main_splitter_->setCollapsible(0, false);
  main_splitter_->setCollapsible(1, true);

  main_splitter_->setStretchFactor(0, 3);
  main_splitter_->setStretchFactor(1, 2);

  main_layout_->addWidget(main_splitter_);

  send_package_timer_ = new QTimer(this);
  send_package_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  send_package_timer_->setInterval(0);
  heartbeat_timer_ = new QTimer(this);
  heartbeat_timer_->setTimerType(Qt::TimerType::PreciseTimer);
  heartbeat_timer_->setInterval(0);
}

void FrameWindow::initSignalsConnection() {
  // Ui 界面的信号槽
  connect(instruction_table_, &Ui::TtTableWidget::rowsChanged, this,
          [this]() { saved_ = false; });
  connect(tabs_, &QtMaterialTabs::currentChanged, this,
          [this](int index) { display_widget_->setCurrentIndex(index); });
}

void FrameWindow::switchToEditMode() {
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

void FrameWindow::switchToDisplayMode() {
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

void FrameWindow::setDisplayType(TtTextFormat::Type type) {
  if (display_type_ != type) {
    display_type_ = type;
    refreshTerminalDisplay();
    saved_ = false;
  }
}

void FrameWindow::refreshTerminalDisplay() {
  terminal_->clear();
  terminal_->setUpdatesEnabled(false);

  // 遍历模型生成内容
  for (int i = 0; i < message_model_->rowCount(); ++i) {
    QModelIndex idx = message_model_->index(i);

    Ui::TtChatMessage *msg = qobject_cast<Ui::TtChatMessage *>(
        idx.data(Ui::TtChatMessageModel::MessageObjectRole).value<QObject *>());

    QString header = msg->isOutgoing() ? "[Tx]" : "[Rx]";
    header.append(' ');
    header.append(msg->timestamp().toString("[yyyy-MM-dd hh:mm:ss.zzz]"));
    header.append(' ');
    //  text 发送, 点击 text 时，文本消失 content 为 0
    QString content;
    if (display_type_ == TtTextFormat::HEX) {
      content = msg->contentAsHex().trimmed();
    } else if (display_type_ == TtTextFormat::TEXT) {
      content = msg->contentAsText();
    }
    header.append(content);
    // qDebug() << header;
    terminal_->appendPlainText(header);
  }

  terminal_->setUpdatesEnabled(true);
}

} // namespace Window
