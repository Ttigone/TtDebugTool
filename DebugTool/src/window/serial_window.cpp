#include "window/serial_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
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

#include "ui/controls/TtLuaInputBox.h"
#include "ui/controls/TtTableView.h"
#include "widget/serial_setting.h"

namespace Window {

SerialWindow::SerialWindow(QWidget* parent)
    : QWidget(parent),
      worker_thread_(new QThread(this)),
      serial_port_(new Core::SerialPortWorker),
      serial_setting_(new Widget::SerialSetting) {

  init();
  connectSignals();

  // 放在线程中执行
  serial_port_->moveToThread(worker_thread_);

  // 成功 析构函数在工作线程中执行, 否则 serial_port_ 无法执行析构
  connect(worker_thread_, &QThread::finished, serial_port_,
          &QObject::deleteLater);
  connect(worker_thread_, &QThread::finished, worker_thread_,
          &QObject::deleteLater);

  connect(serial_port_, &Core::SerialPortWorker::dataReceived, this,
          &SerialWindow::dataReceived);

  connect(serial_port_, &Core::SerialPortWorker::errorOccurred, this,
          &SerialWindow::showErrorMessage);


  worker_thread_->start();
}

SerialWindow::~SerialWindow() {
  // // qDebug() << "delete SerialWindow";
  // worker_thread_->quit();
  // // worker_thread_->exit();
  // worker_thread_->wait();
  // // 手动触发 SerialPortWorker 的析构（在工作线程中）
  // // 修改为这样调试
  // qDebug() << "准备调用deleteLater";
  // bool success = QMetaObject::invokeMethod(serial_port_, "deleteLater",
  //                                          Qt::QueuedConnection);
  // qDebug() << "调用结果:" << success;
  // // 确保事件得到处理
  // // if (serial_port_) {
  // //   qDebug() << "delete SerialWindow";
  // //   serial_port_->deleteLater();
  // //   // QMetaObject::invokeMethod(serial_port_.get(), "deleteLater",
  // //   // QMetaObject::invokeMethod(serial_port_, "deleteLater",
  // //   //                           Qt::QueuedConnection);
  // // }
  // delete worker_thread_;  // 删除线程对象
  // QMetaObject::invokeMethod(serial_port_, "deleteLater", Qt::QueuedConnection);

  if (worker_thread_) {
    worker_thread_->quit();
    // 3. 等待线程完成(设置超时，避免无限等待)
    if (!worker_thread_->wait(200)) {
      qWarning()
          << "Worker thread did not exit gracefully, forcing termination";
      worker_thread_->terminate();  // 强制终止(不推荐，但作为最后手段)
      worker_thread_->wait();       // 等待强制终止完成
    }
    // 无效
    // delete serial_port_;
    // QMetaObject::invokeMethod(serial_port_, "deleteLater",
    //                           Qt::QueuedConnection);
  }
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

void SerialWindow::setDisplayHex(bool hexMode) {
  qDebug() << hexMode;
  display_hex_ = hexMode;
  refreshTerminalDisplay();
}

void SerialWindow::saveLog() {
  // saveBtn->connect(saveBtn, &QPushButton::clicked, [this]() {
  //   QString fileName = QFileDialog::getSaveFileName(this, tr("保存日志"),
  //                                                   QDir::homePath() + "/serial_log.txt",
  //                                                   tr("文本文件 (*.txt)"));
  //   if (!fileName.isEmpty()) {
  //     QFile file(fileName);
  //     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
  //       QTextStream stream(&file);
  //       stream << terminal_->text();
  //       file.close();
  //     }
  //   }
  // });
}

void SerialWindow::refreshTerminalDisplay() {
  // terminal_->setUpdatesEnabled(false);
  // QString fullContent;

  // // 遍历模型生成内容
  // for (int i = 0; i < message_model_->rowCount(); ++i) {
  //   QModelIndex idx = message_model_->index(i);

  //   Ui::TtChatMessage* msg = qobject_cast<Ui::TtChatMessage*>(
  //       idx.data(Ui::TtChatMessageModel::MessageObjectRole).value<QObject*>());

  //   QString line = msg->timestamp().toString("[yyyy-MM-dd hh:mm:ss.zzz] ");
  //   line += (msg->isOutgoing() ? "<< " : ">> ");
  //   line += display_hex_ ? msg->contentAsHex() : msg->contentAsText();
  //   line += "\n";
  //   fullContent += line;
  // }
  // qDebug() << fullContent;

  // terminal_->setText(fullContent);
  // terminal_->setUpdatesEnabled(true);
}

// void SerialWindow::generateDisplayText() {
//   QString content;

//   // for (const auto &msg : messageHistory) {
//   //   // 1. 时间戳
//   //   content += msg.timestamp.toString("[yyyy-MM-dd hh:mm:ss] ");

//   //   // 2. 方向箭头（<< 或 >>）
//   //   content += (msg.isSend ? "<< " : ">> ");

//   //   // 3. 数据内容（Hex或Text）
//   //   if (displayHex) {
//   //     content += msg.rawData.toHex(' ').toUpper();  // 如 "1A 2B 3C"
//   //   } else {
//   //     content += QString::fromLatin1(msg.rawData);  // 原始文本
//   //   }

//   //   content += "\n";
//   // }
//   // return content;
// }

void SerialWindow::showErrorMessage(const QString& text) {
  Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), text, 1500, this);
  on_off_btn_->setChecked(false);
  serial_port_opened = false;
  serial_setting_->setControlState(true);
}

void SerialWindow::dataReceived(const QByteArray& data) {
  recv_byte_count += data.size();
  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setRawData(data);
  tmp->setOutgoing(false);
  tmp->setBubbleColor(QColor("#0ea5e9"));
  QList<Ui::TtChatMessage*> list;
  list.append(tmp);
  message_model_->appendMessages(list);

  // 获取当前时间并格式化消息
  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  QString formattedMessage = timestamp + " >> " + data + "\n";

  // 添加到终端
  // terminal_->append(formattedMessage);
  terminal_->appendPlainText(formattedMessage);

  recv_byte->setText(QString("接收字节数: %1 B").arg(recv_byte_count));
  message_view_->scrollToBottom();
}

QByteArray SerialWindow::saveState() const {
  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  // 保存需要的数据
  stream << title_->text();
  return state;
}

bool SerialWindow::restoreState(const QByteArray& state) {
  QDataStream stream(state);
  // 恢复数据
  stream >> title;
  // stream >> someData_;
  return true;
}

void SerialWindow::sendMessageToPort() {
  // if (!serial_port_opened) {
  //   Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
  //                           1500, this);
  //   return;
  // }
  QString data = editor->text();
  send_byte_count += data.size();
  // 获取当前时间并格式化消息
  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  QString formattedMessage = timestamp + " << " + "\n" + data + "\n";
  // 添加到终端
  qDebug() << formattedMessage;
  // terminal_->append(formattedMessage);
  terminal_->appendPlainText(formattedMessage);

  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setRawData(data.toUtf8());
  tmp->setTimestamp(now);
  tmp->setOutgoing(true);
  tmp->setBubbleColor(QColor("#DCF8C6"));
  QList<Ui::TtChatMessage*> list;
  list.append(tmp);
  message_model_->appendMessages(list);

  // // 串口发送
  // QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
  //                           Q_ARG(QString, data));
  // send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
  // message_view_->scrollToBottom();
}

void SerialWindow::sendMessageToPort(const QString& data) {
  if (!serial_port_opened) {
    Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), tr("串口未打开"),
                            1500, this);
    return;
  }
  send_byte_count += data.size();
  // 获取当前时间并格式化消息
  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
  QString formattedMessage = timestamp + " << " + data + "\n";
  // 添加到终端
  // terminal_->append(formattedMessage);
  terminal_->appendPlainText(formattedMessage);

  auto tmp = new Ui::TtChatMessage();
  tmp->setContent(data);
  tmp->setRawData(data.toUtf8());
  tmp->setTimestamp(now);
  tmp->setOutgoing(true);
  tmp->setBubbleColor(QColor("#DCF8C6"));
  QList<Ui::TtChatMessage*> list;
  list.append(tmp);
  message_model_->appendMessages(list);

  // 串口发送
  QMetaObject::invokeMethod(serial_port_, "sendData", Qt::QueuedConnection,
                            Q_ARG(QString, data));
  send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
  message_view_->scrollToBottom();
}

void SerialWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  title_ = new Ui::TtNormalLabel(tr("未命名串口连接"));
  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
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

  Ui::TtHorizontalLayout* edit_layout =
      new Ui::TtHorizontalLayout(edit_widget_);
  edit_layout->addSpacerItem(new QSpacerItem(10, 10));
  edit_layout->addWidget(title_edit_);
  edit_layout->addStretch();

  // 使用堆叠布局
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &SerialWindow::switchToEditMode);

  Ui::TtHorizontalLayout* tmpP1 = new Ui::TtHorizontalLayout;
  tmpP1->addWidget(stack_);

  Ui::TtHorizontalLayout* tmpAll = new Ui::TtHorizontalLayout;

  auto handleSave = [this]() {
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
  on_off_btn_->setColors(Qt::black, Qt::red);
  on_off_btn_->setSvgSize(18, 18);

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

  // 切换
  QWidget* twoBtnForGroup = new QWidget(chose_function);
  QHBoxLayout* layouttest = new QHBoxLayout(twoBtnForGroup);
  Ui::TtSvgButton* leftBtn =
      new Ui::TtSvgButton(":/sys/terminal.svg", twoBtnForGroup);
  leftBtn->setSvgSize(18, 18);
  leftBtn->setColors(Qt::black, Qt::blue);
  // leftBtn->setEnableHoldToCheck(true);
  Ui::TtSvgButton* rightBtn =
      new Ui::TtSvgButton(":/sys/chat.svg", twoBtnForGroup);
  rightBtn->setSvgSize(18, 18);
  rightBtn->setColors(Qt::black, Qt::blue);
  // rightBtn->setEnableHoldToCheck(true);
  layouttest->addWidget(leftBtn);
  layouttest->addWidget(rightBtn);
  // 互斥
  Ui::TtWidgetGroup* test_ = new Ui::TtWidgetGroup(this);
  test_->setHoldingChecked(true);
  test_->addWidget(leftBtn);
  test_->addWidget(rightBtn);
  test_->setExclusive(true);
  test_->setCheckedIndex(0);
  chose_function_layout->addWidget(twoBtnForGroup);
  chose_function_layout->addStretch();

  clear_history_ = new Ui::TtSvgButton(":/sys/trash.svg", chose_function);
  clear_history_->setSvgSize(18, 18);

  //// 选择 text/hex
  Ui::TtWidgetGroup* styleGroup = new Ui::TtWidgetGroup(this);
  styleGroup->setHoldingChecked(true);
  Ui::TtTextButton* textBtn = new Ui::TtTextButton(QColor(Qt::blue), "TEXT");
  Ui::TtTextButton* hexBtn = new Ui::TtTextButton(QColor(Qt::blue), "HEX");
  styleGroup->addWidget(textBtn);
  styleGroup->addWidget(hexBtn);
  styleGroup->setCheckedIndex(0);
  styleGroup->setExclusive(true);
  chose_function_layout->addWidget(textBtn);
  chose_function_layout->addWidget(hexBtn);
  connect(
      styleGroup, &Ui::TtWidgetGroup::widgetClicked,
      [this](const int& index) { setDisplayHex(index == 1 ? true : false); });

  // 清除历史按钮
  chose_function_layout->addWidget(clear_history_);

  QSplitter* VSplitter = new QSplitter;
  VSplitter->setOrientation(Qt::Vertical);
  VSplitter->setContentsMargins(QMargins());
  VSplitter->setSizes(QList<int>() << 500 << 200);

  // 上方选择功能以及信息框
  QWidget* contentWidget = new QWidget;
  Ui::TtVerticalLayout* contentWidgetLayout =
      new Ui::TtVerticalLayout(contentWidget);

  QStackedWidget* messageStackedView = new QStackedWidget(contentWidget);

  // terminal_ = new QsciScintilla(messageStackedView);
  terminal_ = new QPlainTextEdit(this);
  terminal_->setReadOnly(true);
  terminal_->setFrameStyle(QFrame::NoFrame);

  SerialHighlighter* lexer = new SerialHighlighter(terminal_->document());

  messageStackedView->addWidget(terminal_);

  message_view_ = new Ui::TtChatView(messageStackedView);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  messageStackedView->addWidget(message_view_);


  contentWidgetLayout->addWidget(chose_function);
  contentWidgetLayout->addWidget(messageStackedView);

  connect(test_, &Ui::TtWidgetGroup::widgetClicked, this,
          [this, messageStackedView](const int& idx) {
            // qDebug() << idx;
            messageStackedView->setCurrentIndex(idx);
          });

  base::DetectRunningTime runtime;

  message_model_ = new Ui::TtChatMessageModel;

  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget* bottomAll = new QWidget;
  Ui::TtVerticalLayout* bottomAllLayout = new Ui::TtVerticalLayout(bottomAll);
  bottomAll->setLayout(bottomAllLayout);

  // 下方自定义指令
  QWidget* tabs_and_count = new QWidget(this);
  Ui::TtHorizontalLayout* tacLayout = new Ui::TtHorizontalLayout();
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
  // editor->setCaretForegroundColor(QColor("Coral"));
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);
  editor->setFrameStyle(QFrame::NoFrame);

  messageEditLayout->addWidget(editor);

  QWidget* bottomBtnWidget = new QWidget(messageEdit);
  bottomBtnWidget->setMinimumHeight(40);
  //bottomBtnWidget->setStyleSheet("back)ground-color: red");
  Ui::TtHorizontalLayout* bottomBtnWidgetLayout =
      new Ui::TtHorizontalLayout(bottomBtnWidget);

  // QtMaterialRadioButton* choseText = new QtMaterialRadioButton(bottomBtnWidget);
  // QtMaterialRadioButton* choseHex = new QtMaterialRadioButton(bottomBtnWidget);
  Ui::TtRadioButton* choseText = new Ui::TtRadioButton(bottomBtnWidget);
  Ui::TtRadioButton* choseHex = new Ui::TtRadioButton(bottomBtnWidget);
  choseText->setText("TEXT");
  choseHex->setText("HEX");

  sendBtn = new QtMaterialFlatButton(bottomBtnWidget);
  sendBtn->setIcon(QIcon(":/sys/send.svg"));
  bottomBtnWidgetLayout->addWidget(choseText);
  bottomBtnWidgetLayout->addWidget(choseHex);
  bottomBtnWidgetLayout->addStretch();
  bottomBtnWidgetLayout->addWidget(sendBtn);

  messageEditLayout->addWidget(bottomBtnWidget);

  instruction_table_ = new Ui::TtTableWidget(la_w);

  layout->addWidget(messageEdit);
  layout->addWidget(instruction_table_);

  // connect(
  //     instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
  //     [this](const QString& data) {
  //       // if (serial_port_->isOpened())
  //       if (serial_port_opened) {
  //         qDebug() << data;
  //         QMetaObject::invokeMethod(serial_port_, "sendData",
  //                                   Qt::QueuedConnection, Q_ARG(QString, data));
  //         send_byte_count += data.size();
  //         send_byte->setText(QString("发送字节数: %1 B").arg(send_byte_count));
  //       }
  //     });

  connect(instruction_table_, &Ui::TtTableWidget::sendRowMsg, this,
          qOverload<const QString&>(&SerialWindow::sendMessageToPort));

  layout->setCurrentIndex(0);

  connect(m_tabs, &QtMaterialTabs::currentChanged, [this, layout](int index) {
    layout->setCurrentIndex(index);
  });

  // 显示, 并输入 lua 脚本
  lua_code_ = new Ui::TtLuaInputBox(this);
  // mask_widget_ = new Ui::TtMaskWidget(this);
  // mask_widget_->show(lua_code_);

  bottomAllLayout->addWidget(tabs_and_count);
  bottomAllLayout->addWidget(la_w);

  VSplitter->addWidget(contentWidget);
  VSplitter->addWidget(bottomAll);

  // 左右分区
  mainSplitter->addWidget(VSplitter);
  mainSplitter->addWidget(serial_setting_);
  mainSplitter->setSizes(QList<int>() << 500 << 200);

  // 主界面是左右分隔
  main_layout_->addWidget(mainSplitter);

  // qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
}

void SerialWindow::setSerialSetting() {
  serial_setting_->setSerialPortsName();
  serial_setting_->setSerialPortsBaudRate();
  serial_setting_->setSerialPortsDataBit();
  serial_setting_->setSerialPortsParityBit();
  serial_setting_->setSerialPortsStopBit();
  serial_setting_->setSerialPortsFluidControl();

  serial_setting_->displayDefaultSetting();
}

void SerialWindow::connectSignals() {
  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    cfg_.obj.insert("WindowTitle", title_->text());
    cfg_.obj.insert("SerialSetting", serial_setting_->getSerialSetting());
    cfg_.obj.insert("InstructionTable", instruction_table_->getTableRecord());
    // Ui::TtMessageBar::success(
    //     TtMessageBarType::Top, "警告",
    //     // "输入框不能为空，请填写完整信息。", 3000, this);
    //     "输入框不能为空，请填写完整信息。", 3000, this);
    emit requestSaveConfig();
    //qDebug() << cfg_.obj;
    // 配置文件保存到文件中
    // 当前的 tabWidget 匹配对应的 QJsonObject
  });

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    // // 检查是否处于打开状态
    // serial_port 已经移动到了 工作线程中
    // 将openSerialPort的调用通过Qt的信号槽机制排队到worker_thread_中执行，而不是直接在主线程调用
    if (serial_port_opened) {
      // 关闭串口时也需跨线程调用
      // QMetaObject::invokeMethod(serial_port_.get(), "closeSerialPort",
      //                           Qt::QueuedConnection);
      QMetaObject::invokeMethod(serial_port_, "closeSerialPort",
                                Qt::QueuedConnection);
      // serial_port_->closeSerialPort();
      serial_port_opened = false;
      serial_setting_->setControlState(true);
    } else {
      // 获取配置后通过 invokeMethod 调用
      Core::SerialPortConfiguration cfg =
          serial_setting_->getSerialPortConfiguration();
      // QMetaObject::invokeMethod(serial_port_.get(), "openSerialPort",
      //                           Qt::QueuedConnection,
      //                           Q_ARG(Core::SerialPortConfiguration, cfg));
      QMetaObject::invokeMethod(serial_port_, "openSerialPort",
                                Qt::QueuedConnection,
                                Q_ARG(Core::SerialPortConfiguration, cfg));
      // serial_port_->openSerialPort(
      //     serial_setting_->getSerialPortConfiguration());
      serial_port_opened = true;
      serial_setting_->setControlState(false);
    }
  });

  connect(clear_history_, &Ui::TtSvgButton::clicked, [this]() {
    message_model_->clearModelData();
    terminal_->clear();
  });

  connect(sendBtn, &QtMaterialFlatButton::clicked, this,
          qOverload<>(&SerialWindow::sendMessageToPort));
  connect(serial_setting_, &Widget::SerialSetting::showScriptSetting,
          [this]() { lua_code_->show(); });
}

}  // namespace Window
