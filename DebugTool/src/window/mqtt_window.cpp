#include "window/mqtt_window.h"

#include <QEvent>
#include <new>

#include "Def.h"

#include <qsizepolicy.h>
#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>

#include "core/mqtt_client.h"
#include "ui/control/TtContentDialog.h"
#include "ui/widgets/subscripition_manager.h"
#include "widget/mqtt_meta_setting_widget.h"
#include "widget/mqtt_setting.h"
#include "widget/subscripition_widget.h"

namespace Window {

MqttWindow::MqttWindow(TtProtocolType::ProtocolRole role, QWidget *parent)
    : FrameWindow(parent), role_(role) {
  opened_ = false;
  mqtt_client_ = new Core::MqttClient;
  connect(mqtt_client_, &Core::MqttClient::connected, this, [this]() {
    on_off_btn_->setEnabled(true);
    // 链接状态
    qDebug() << "conne";
    on_off_btn_->setChecked(true);
  });
  connect(mqtt_client_, &Core::MqttClient::disconnected, this, [this]() {
    on_off_btn_->setEnabled(true);
    qDebug() << "disconnect";
    on_off_btn_->setChecked(false);
  });
  connect(mqtt_client_, &Core::MqttClient::errorOccurred, this,
          [this](const QString &error) {
            Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error, 1500,
                                    this);
            on_off_btn_->setChecked(false);
          });
  connect(mqtt_client_, &Core::MqttClient::dataReceived,
          [this](const QString &data) {
            // BUG 接收到信息后, 需要显示到界面上
            // 能够收到消息
            qDebug() << "get: " << data;
            // 创建一个新的接收消息
            auto incomingMsg = new Ui::TtChatMessage();
            incomingMsg->setContent(data);
            incomingMsg->setOutgoing(false); // 表示接收的消息
            incomingMsg->setBubbleColor(QColor("#F1F0F0")); // 接收消息气泡颜色
            incomingMsg->setTimestamp(QDateTime::currentDateTime());
            // 添加到消息模型
            QList<Ui::TtChatMessage *> list;
            list.append(incomingMsg);
            message_model_->appendMessages(list);
            message_view_->scrollToBottom();
          });

  init();
}

MqttWindow::~MqttWindow() { qDebug() << "delete MqttWindow"; }

QJsonObject MqttWindow::getConfiguration() const { return config_; }

bool MqttWindow::workState() const { return opened_; }

bool MqttWindow::saveState() { return saved_; }

void MqttWindow::setSaveState(bool state) { saved_ = state; }

void MqttWindow::saveSetting() {
  config_.insert("Type", TtFunctionalCategory::Communication);
  config_.insert("WindowTitle", title_->text());
  config_.insert("MqttSetting", mqtt_client_setting_->getMqttClientSetting());
  if (subscripition_widget_) {
    config_.insert("MetaSetting", subscripition_widget_->getMetaSetting());
  }
  // saved_ = true;
  setSaveStatus(true);
  emit requestSaveConfig();
}

void MqttWindow::setSetting(const QJsonObject &config) {
  qDebug() << "set Old Config";
  title_->setText(config.value("WindowTitle").toString(tr("未读取正确的标题")));
  if (role_ == TtProtocolType::Client) {
    config_.insert("Type", TtFunctionalCategory::Communication);
    mqtt_client_setting_->setOldSettings(
        config.value("MqttSetting").toObject(QJsonObject()));
  } else if (role_ == TtProtocolType::Server) {
    config_.insert("Type", TtFunctionalCategory::Simulate);
  }
  if (subscripition_widget_) {
    subscripition_widget_->setOldSettings(
        config.value("MetaSetting").toObject(QJsonObject()));
  }
  setSaveStatus(true);
  Ui::TtMessageBar::success(TtMessageBarType::Top, tr(""), tr("读取配置成功"),
                            1500);
}

QString MqttWindow::getTitle() const { return title_->text(); }

void MqttWindow::switchToEditMode() {
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

void MqttWindow::switchToDisplayMode() {
  title_->setText(title_edit_->text());
  stack_->setCurrentWidget(original_widget_);
}

void MqttWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);
  if (role_ == TtProtocolType::Client) {
    title_ = new Ui::TtNormalLabel(tr("未命名的 MQTT 客户端"));
  } else {
    title_ = new Ui::TtNormalLabel(tr("未命名的 MQTT 服务模拟端"));
  }

  // 编辑命名按钮
  modify_title_btn_ = new Ui::TtSvgButton(":/sys/edit.svg", this);
  modify_title_btn_->setSvgSize(18, 18);

  // 修改遍体的界面
  original_widget_ = new QWidget(this);
  // 原有的展示布局
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
  // 编辑界面布局
  Ui::TtHorizontalLayout *editWidgetLayout =
      new Ui::TtHorizontalLayout(edit_widget_);
  editWidgetLayout->addSpacerItem(new QSpacerItem(10, 10));
  editWidgetLayout->addWidget(title_edit_);
  editWidgetLayout->addStretch();

  // 栈窗口去切换显示与编辑页面
  stack_ = new QStackedWidget(this);
  stack_->setMaximumHeight(40);
  stack_->addWidget(original_widget_);
  stack_->addWidget(edit_widget_);

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &MqttWindow::switchToEditMode);

  // 水平布局
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

  Ui::TtHorizontalLayout *operationButtonLayout = new Ui::TtHorizontalLayout;

  save_btn_ = new Ui::TtSvgButton(":/sys/save_cfg.svg", this);
  save_btn_->setSvgSize(18, 18);
  save_btn_->setColors(QColor("#2196F3"), QColor("#2196F3"));

  // 开关按钮
  on_off_btn_ = new Ui::TtSvgButton(":/sys/start_up.svg", this);
  on_off_btn_->setSvgSize(18, 18);
  on_off_btn_->setColors(Qt::black, Qt::red);

  // subscriptionBtn = new Ui::TtSvgButton(":/sys/plus.svg");
  // subscriptionBtn->setColors(Qt::black, Qt::blue);
  // subscriptionBtn->setText(tr("订阅"));
  subscriptionBtn = new Ui::TtTextButton(tr("订阅"), this);

  operationButtonLayout->addWidget(subscriptionBtn);
  operationButtonLayout->addWidget(save_btn_);
  // 缺少
  operationButtonLayout->addWidget(on_off_btn_, 0, Qt::AlignRight);
  operationButtonLayout->addSpacerItem(new QSpacerItem(10, 10));

  // 初始显示有点问题
  topLayout->addLayout(stackLayout);
  topLayout->addLayout(operationButtonLayout);

  // 添加单独的一个上层操作
  main_layout_->addLayout(topLayout);

  // 左右
  // 左右的展示
  QSplitter *mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  // QWidget *topLevelWidget = new QWidget(this);
  // QVBoxLayout *layout = new QVBoxLayout(topLevelWidget);

  // 左侧的窗口是大的 QMainWindow
  QMainWindow *embeddedMainWindow = new QMainWindow(mainSplitter);
  embeddedMainWindow->setWindowFlags(Qt::Widget); // 确保没有窗口装饰
  QDockWidget *dock = new QDockWidget(tr("订阅"), embeddedMainWindow);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setFeatures(QDockWidget::DockWidgetMovable |
                    QDockWidget::DockWidgetFloatable);

  // 创建一个容器 Widget
  // QWidget *containerWidget = new QWidget(topLevelWidget);
  QWidget *containerWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(containerWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  embeddedMainWindow->setCentralWidget(containerWidget);

  QWidget *messageView = new QWidget(containerWidget);
  Ui::TtHorizontalLayout *messageViewLayout =
      new Ui::TtHorizontalLayout(messageView);
  message_view_ = new Ui::TtChatView(messageView);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false); // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  message_model_ = new Ui::TtChatMessageModel;
  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  messageViewLayout->addWidget(message_view_);

  // QWidget *sendSetting = new QWidget(messageView);
  QWidget *sendSetting = new QWidget(containerWidget);
  Ui::TtVerticalLayout *sendSettingLayout =
      new Ui::TtVerticalLayout(sendSetting);

  // 属性设置布局
  //  操作的水平布局
  Ui::TtHorizontalLayout *propertyLayout = new Ui::TtHorizontalLayout();
  // 测试
  // propertyLayout->setContentsMargins(100, 0, 0, 0);
  propertyLayout->setContentsMargins(0, 0, 0, 0);
  fomat_ = new Ui::TtComboBox(sendSetting);
  fomat_->addItem("JSON");
  fomat_->addItem("Base64");
  fomat_->addItem("Hex");
  fomat_->addItem("Plaintext");
  qos_ = new Ui::TtComboBox(sendSetting);
  qos_->addItem("0 (At most once)", 0);
  qos_->addItem("1 (At least once)", 1);
  qos_->addItem("2 (Exactly once)", 2);
  retain_ = new Ui::TtRadioButton("Retain");
  meta_btn_ = new Ui::TtTextButton("Meta");
  propertyLayout->addWidget(fomat_, 1);
  propertyLayout->addWidget(qos_, 1);
  propertyLayout->addWidget(retain_);
  propertyLayout->addWidget(meta_btn_);
  propertyLayout->addStretch();

  // 发送的底部
  send_topic_ = new Ui::TtLineEdit(sendSetting);
  editor = new QsciScintilla(sendSetting);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);
  editor->setFrameStyle(QFrame::NoFrame);

  sendSettingLayout->addLayout(propertyLayout);
  sendSettingLayout->addWidget(send_topic_);
  sendSettingLayout->addWidget(editor);

  // 放置到 editor 中
  send_btn_ = new QtMaterialFlatButton(tr("发送"), editor);
  // sendBtn->setBaseOpacity(100); // 前景色的透明度
  send_btn_->setBackgroundMode(Qt::OpaqueMode);
  send_btn_->setBackgroundColor(Qt::gray);
  send_btn_->setForegroundColor(Qt::green);
  send_btn_->setRole(Material::Primary);
  send_btn_->setIcon(QIcon(":/sys/send.svg"));
  editor->viewport()->installEventFilter(
      new ScrollAreaEventFilter(editor, send_btn_, sendSetting));
  const auto event = new QResizeEvent(editor->size(), editor->size());
  QCoreApplication::postEvent(editor->viewport(), event);

  model = new QStandardItemModel;
  // list 列表存储 放到 view 的右侧
  // 右侧管理 订阅列表
  // 订阅列表缩小宽度
  subscripition_list_ = new Ui::SubscripitionManager(this);
  subscripition_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(subscripition_list_, &QListView::customContextMenuRequested, this,
          [this](const QPoint &pos) {
            QMenu menu;
            QAction *deleteAction = menu.addAction("删除订阅");
            connect(deleteAction, &QAction::triggered, this, [this, pos]() {
              QModelIndex index = subscripition_list_->indexAt(pos);
              model->removeRow(index.row());
            });
            menu.exec(subscripition_list_->viewport()->mapToGlobal(pos));
          });

  // for (int i = 0; i < 3; i++) {
  //   QStandardItem* item = new QStandardItem(QString("项目 %1").arg(i));
  //   model->appendRow(item);
  // }

  subscripition_list_->setModel(model);
  QItemSelectionModel *selectionModel = subscripition_list_->selectionModel();
  QModelIndex firstIndex = model->index(0, 0);

  // dock widget
  dock->setWidget(subscripition_list_);
  embeddedMainWindow->addDockWidget(Qt::RightDockWidgetArea, dock);

  // 设置初始宽度
  QList<QDockWidget *> dockList;
  dockList << dock;
  QList<int> sizeList;
  sizeList << 200; // 200像素宽度
  embeddedMainWindow->resizeDocks(dockList, sizeList, Qt::Horizontal);

  // containerWidget 是主窗口
  mainLayout->addWidget(messageView);
  mainLayout->addWidget(sendSetting);

  if (role_ == TtProtocolType::Client) {
    mqtt_client_setting_ = new Widget::MqttClientSetting;
    mainSplitter->addWidget(mqtt_client_setting_);
  } else if (role_ == TtProtocolType::Server) {
  }

  // 下面的左右窗口显示有问题
  main_layout_->addWidget(mainSplitter);

  mainSplitter->setStretchFactor(0, 2);
  mainSplitter->setStretchFactor(1, 1);

  QList<int> initialSizes;
  initialSizes << mainSplitter->width() - 200 << 200;
  mainSplitter->setSizes(initialSizes);

  mask_widget_ = new Ui::TtMaskWidget(this);
  // 没有启用重使用标志, 有问题
  mask_widget_->setReusable(true);

  connectSignals();
}

void MqttWindow::connectSignals() {
  connect(on_off_btn_, &Ui::TtSvgButton::clicked, this, [this]() {
    if (opened_) {
      // 断开主机链接
      mqtt_client_->disconnectFromHost();
      on_off_btn_->setChecked(false);
      opened_ = false;
    } else {
      qDebug() << "链接主机";
      mqtt_client_->connectToHost(
          mqtt_client_setting_->getMqttClientConfiguration());
      on_off_btn_->setChecked(true);
      opened_ = true;
    }
  });

  connect(save_btn_, &Ui::TtSvgButton::clicked, this, &MqttWindow::saveSetting);

  connect(meta_btn_, &Ui::TtTextButton::clicked, this, [this]() {
    if (!meta_widget_ ||
        QPointer<Widget::MqttMetaSettingWidget>(meta_widget_).isNull()) {
      qDebug() << "创建 meta widget";
      meta_widget_ = new Widget::MqttMetaSettingWidget(this);
      meta_widget_->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);
    }
    disconnect(meta_widget_, &Widget::MqttMetaSettingWidget::closed, nullptr,
               nullptr);

    connect(meta_widget_, &Widget::MqttMetaSettingWidget::closed, mask_widget_,
            &Ui::TtMaskWidget::handleCloseRequest, Qt::UniqueConnection);

    // 在显示前确保之前显示的组件已关闭
    if (mask_widget_->isVisible()) {
      mask_widget_->handleCloseRequest();
      // QTimer::singleShot(350, this, [this]() {
      //   // 总是会延时出现, 为什么, 并且只有 meta 才会出现
      //   qDebug() << "delay show meta";
      //   mask_widget_->show(meta_widget_);
      // });
      mask_widget_->show(meta_widget_);
    } else {
      mask_widget_->show(meta_widget_);
    }
  });

  connect(subscriptionBtn, &Ui::TtTextButton::clicked, this, [this]() {
    if (!subscripition_widget_ ||
        QPointer<Widget::SubscripitionWidget>(subscripition_widget_).isNull()) {
      // 会被调用析构函数
      subscripition_widget_ = new Widget::SubscripitionWidget(this);
      subscripition_widget_->setSizePolicy(QSizePolicy::Expanding,
                                           QSizePolicy::Expanding);
    }
    // 断开旧连接
    disconnect(subscripition_widget_, &Widget::SubscripitionWidget::closed,
               nullptr, nullptr);
    disconnect(subscripition_widget_,
               &Widget::SubscripitionWidget::saveConfigToManager, nullptr,
               nullptr);

    connect(subscripition_widget_, &Widget::SubscripitionWidget::closed,
            mask_widget_, &Ui::TtMaskWidget::handleCloseRequest,
            Qt::UniqueConnection);

    connect(subscripition_widget_,
            &Widget::SubscripitionWidget::saveConfigToManager, this,
            [this](const QByteArray &config) {
              // 为什么会进入两次
              // qDebug() << "保存订阅配置" << config;
              // 解码
              if (!config.isEmpty()) {
                QDataStream stream(config);
                QString topic, qos, color, alias, identifier, noLocal,
                    retainAsPublished, retainHandling;
                stream >> topic >> qos >> color >> alias >> identifier >>
                    noLocal >> retainAsPublished >> retainHandling;
                qDebug() << topic << qos << color << alias << identifier
                         << noLocal << retainAsPublished << retainHandling;

                // 为什么重复提交
                QStandardItem *item = new QStandardItem(QString(topic));
                item->setData(config, Qt::UserRole);
                model->appendRow(item);
                mqtt_client_->subscribe(topic);
              }
              mask_widget_->handleCloseRequest();
            });

    // 在显示前确保之前显示的组件已关闭
    if (mask_widget_->isVisible()) {
      mask_widget_->handleCloseRequest();
      // 使用单次触发的定时器延迟显示，确保前一个组件已完全关闭
      QTimer::singleShot(300, this, [this]() {
        qDebug() << "延迟显示";
        mask_widget_->show(subscripition_widget_);
      });
    } else {
      mask_widget_->show(subscripition_widget_);
    }
  });

  connect(send_btn_, &QtMaterialFlatButton::clicked, this, [&] {
    if (!mqtt_client_->isConnected()) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                              tr("客户端未建立链接"), 1500, this);
      return;
    }
    // 检查主题是否为空
    QString topic = send_topic_->text().trimmed();
    if (topic.isEmpty()) {
      Ui::TtMessageBar::warning(TtMessageBarType::Top, tr(""),
                                tr("请输入发布主题"), 1500, this);
      send_topic_->setFocus();
      return;
    }
    // 获取消息内容
    QString data = editor->text();
    if (data.isEmpty()) {
      // 如果允许空消息，这行可以删除
      // Ui::TtMessageBar::info(TtMessageBarType::Top, tr(""), tr("发送空消息"),
      //                        1500, this);
      Ui::TtMessageBar::information(TtMessageBarType::Top, tr(""),
                                    tr("发送空消息"), 1500, this);
    }
    // 安全获取 QoS 值
    int qosValue = 0; // 默认值
    QVariant qosData = qos_->currentData();
    if (qosData.isValid()) {
      qosValue = qosData.toInt();
    } else {
      // 备选方案：使用索引
      qosValue = qos_->currentIndex();
    }

    // 安全获取 Meta 数据
    QByteArray metaData;
    try {
      if (meta_widget_ && meta_btn_->isEnabled()) {
        metaData = meta_widget_->getMetaSettings();
      }
    } catch (const std::exception &e) {
      qWarning() << "获取Meta数据出错:" << e.what();
      metaData = QByteArray(); // 确保为空
    }

    // 尝试发送消息
    try {
      // 发送消息失败了
      qint32 msgId = mqtt_client_->publishMessage(
          topic, data, qosValue, retain_->isChecked(), metaData);

      if (msgId > 0) {
        qDebug() << "发送成功，消息ID:" << msgId;

        // 在成功发送后添加消息到聊天视图
        auto outgoingMsg = new Ui::TtChatMessage();
        outgoingMsg->setContent(data + topic);
        outgoingMsg->setOutgoing(true);
        outgoingMsg->setBubbleColor(QColor("#DCF8C6"));
        outgoingMsg->setTimestamp(QDateTime::currentDateTime());
        // outgoingMsg->setTitle(topic); // 在消息中显示主题
        QList<Ui::TtChatMessage *> list;
        list.append(outgoingMsg);
        message_model_->appendMessages(list);
        message_view_->scrollToBottom();
        // 上面能够上发送出去
        // 可选：清空编辑器
        // editor->clear();
      } else {
        Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                                tr("消息发送失败"), 1500, this);
      }
    } catch (const std::exception &e) {
      qWarning() << "发送消息时出现异常:" << e.what();
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                              tr("发送异常: ") + QString::fromUtf8(e.what()),
                              1500, this);
    }
    // // qDebug() << "发送数据:" << data;

    // // QString data = editor->text();
    // // qDebug() << "send Data" << data;

    // // 发送信息
    // mqtt_client_->publishMessage(
    //     send_topic_->text(), data, qos_->currentData().toInt(),
    //     retain_->isChecked(), meta_widget_->getMetaSettings());
    // qDebug() << "succss send";
  });

  connect(mqtt_client_setting_, &Widget::MqttClientSetting::mqttVersionChanged,
          this, [this](TtMqttProcotol::Version version) {
            if (version != TtMqttProcotol::Q5_0) {
              meta_btn_->setEnabled(false);
            } else {
              meta_btn_->setEnabled(true);
            }
          });
  connect(mqtt_client_setting_, &Widget::MqttClientSetting::settingChanged,
          this, [this] { setSaveStatus(false); });

  connect(this, &FrameWindow::savedChanged, this, [this](bool saved) {
    if (saved) {
      save_btn_->setColors(QColor("#2196F3"), QColor("#2196F3")); // 黑色/蓝色
      save_btn_->setToolTip(tr("配置已保存"));
    } else {
      save_btn_->setColors(QColor("#F44336"), QColor("#FF5252")); // 红色系
      save_btn_->setToolTip(tr("有未保存的更改，点击保存"));
    }
  });
}

void updateButtonPosition(QsciScintilla *scrollArea, QPushButton *sendButton) {
  const int margin = 10; // 边距

  // 获取视口（viewport）的几何区域
  QRect viewportRect = scrollArea->viewport()->geometry();

  // 计算按钮的位置（右下角 - 边距 - 按钮尺寸）
  int btnX = viewportRect.right() - sendButton->width() - margin;
  int btnY = viewportRect.bottom() - sendButton->height() - margin;

  // 移动按钮到目标位置
  sendButton->move(btnX, btnY);

  // 确保按钮显示在最上层
  sendButton->raise();
}

} // namespace Window
