#include "window/mqtt_window.h"

#include <QEvent>

#include "Def.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtMaskWidget.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/collapsible_panel.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>

#include "core/mqtt_client.h"
#include "ui/widgets/pop_widget.h"
#include "ui/widgets/subscripition_manager.h"
#include "widget/mqtt_meta_setting_widget.h"
#include "widget/mqtt_setting.h"
#include "widget/subscripition_widget.h"

#include <qtmaterialflatbutton.h>

namespace Window {

MqttWindow::MqttWindow(TtProtocolType::ProtocolRole role, QWidget* parent)
    : QWidget(parent), role_(role) {
  opened_ = false;
  mqtt_client_ = new Core::MqttClient;
  connect(mqtt_client_, &Core::MqttClient::connected, [this]() {
    on_off_btn_->setEnabled(true);
    qDebug() << "conne";
    on_off_btn_->setChecked(true);
  });
  connect(mqtt_client_, &Core::MqttClient::disconnected, [this]() {
    on_off_btn_->setEnabled(true);
    qDebug() << "disconnect";
    on_off_btn_->setChecked(false);
  });
  connect(mqtt_client_, &Core::MqttClient::errorOccurred,
          [this](const QString& error) {
            Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""), error, 1500,
                                    this);
            on_off_btn_->setEnabled(true);
            on_off_btn_->setChecked(false);
          });
  connect(mqtt_client_, &Core::MqttClient::dataReceived,
          [this](const QString& data) { qDebug() << "get: " << data; });

  init();
}

MqttWindow::~MqttWindow() {}

QJsonObject MqttWindow::getConfiguration() const {
  return config_;
}

QString MqttWindow::getTitle() const {
  return title_->text();
}

void MqttWindow::switchToEditMode() {
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

  // 优化后的信号连接（仅需2个连接点）
  connect(modify_title_btn_, &Ui::TtSvgButton::clicked, this,
          &MqttWindow::switchToEditMode);

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

  subscriptionBtn = new Ui::TtSvgButton(":/sys/plus.svg");
  subscriptionBtn->setColors(Qt::black, Qt::blue);
  subscriptionBtn->setText(tr("订阅"));

  tmpl2->addWidget(subscriptionBtn);
  tmpl2->addWidget(save_btn_);
  tmpl2->addWidget(on_off_btn_, 0, Qt::AlignRight);
  tmpl2->addSpacerItem(new QSpacerItem(10, 10));

  tmpAll->addLayout(tmpP1);
  tmpAll->addLayout(tmpl2);

  main_layout_->addLayout(tmpAll);

  QSplitter* mainSplitter = new QSplitter;
  mainSplitter->setOrientation(Qt::Horizontal);

  QWidget* topLevelWidget = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout(topLevelWidget);

  // 在 QWidget 内部嵌入一个 QMainWindow
  QMainWindow* embeddedMainWindow = new QMainWindow();
  embeddedMainWindow->setWindowFlags(Qt::Widget);  // 确保没有窗口装饰
  QDockWidget* dock = new QDockWidget(tr("订阅"), embeddedMainWindow);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setFeatures(QDockWidget::DockWidgetMovable |
                    QDockWidget::DockWidgetFloatable);

  QWidget* cont = new QWidget(embeddedMainWindow);
  Ui::TtVerticalLayout* cont_layout = new Ui::TtVerticalLayout(cont);
  message_view_ = new Ui::TtChatView(cont);
  message_view_->setResizeMode(QListView::Adjust);
  message_view_->setUniformItemSizes(false);  // 允许每个项具有不同的大小
  message_view_->setMouseTracking(true);
  message_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cont_layout->addWidget(message_view_, 1);
  message_model_ = new Ui::TtChatMessageModel;
  message_view_->setModel(message_model_);
  message_view_->scrollToBottom();

  QWidget* sendSetting = new QWidget(cont);
  Ui::TtVerticalLayout* sendSettingLayout =
      new Ui::TtVerticalLayout(sendSetting);

  Ui::TtHorizontalLayout* propertyLayout = new Ui::TtHorizontalLayout();
  fomat_ = new Ui::TtComboBox;
  fomat_->addItem("JSON");
  fomat_->addItem("Base64");
  fomat_->addItem("Hex");
  fomat_->addItem("Plaintext");
  qos_ = new Ui::TtComboBox;
  qos_->addItem("0 (At most once)", 0);
  qos_->addItem("1 (At least once)", 1);
  qos_->addItem("2 (Exactly once)", 2);
  retain_ = new Ui::TtRadioButton("Retain");
  meta_btn_ = new Ui::TtTextButton("Meta");
  propertyLayout->addWidget(fomat_, 0, Qt::AlignLeft);
  propertyLayout->addWidget(qos_, 0, Qt::AlignLeft);
  propertyLayout->addWidget(retain_, 0, Qt::AlignLeft);
  propertyLayout->addWidget(meta_btn_, 0, Qt::AlignLeft);
  propertyLayout->addStretch();

  send_topic_ = new Ui::TtLineEdit(sendSetting);
  editor = new QsciScintilla(sendSetting);
  editor->setWrapMode(QsciScintilla::WrapWord);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagInMargin,
                             QsciScintilla::WrapFlagInMargin, 0);
  editor->setCaretWidth(10);
  editor->setMarginType(1, QsciScintilla::NumberMargin);
  editor->setFrameStyle(QFrame::NoFrame);

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

  sendSettingLayout->addLayout(propertyLayout);
  sendSettingLayout->addWidget(send_topic_);
  sendSettingLayout->addWidget(editor);

  cont_layout->addWidget(sendSetting, 0, Qt::AlignBottom);

  // updateButtonPosition(editor, sendBtn);

  // QWidget* basicWidget = new QWidget;
  // Ui::TtVerticalLayout* basicWidgetLayout =
  //     new Ui::TtVerticalLayout(basicWidget);

  model = new QStandardItemModel;
  subscripition_list_ = new Ui::SubscripitionManager(this);
  subscripition_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(subscripition_list_, &QListView::customContextMenuRequested,
          [this](const QPoint& pos) {
            QMenu menu;
            QAction* deleteAction = menu.addAction("删除订阅");
            connect(deleteAction, &QAction::triggered, [this, pos]() {
              QModelIndex index = subscripition_list_->indexAt(pos);
              model->removeRow(index.row());
            });
            menu.exec(subscripition_list_->viewport()->mapToGlobal(pos));
          });

  for (int i = 0; i < 3; i++) {
    QStandardItem* item = new QStandardItem(QString("项目 %1").arg(i));
    model->appendRow(item);
  }

  subscripition_list_->setModel(model);

  QItemSelectionModel* selectionModel = subscripition_list_->selectionModel();
  QModelIndex firstIndex = model->index(0, 0);

  dock->setWidget(subscripition_list_);
  dock->resize(200, embeddedMainWindow->height());

  embeddedMainWindow->addDockWidget(Qt::RightDockWidgetArea, dock);
  embeddedMainWindow->setCentralWidget(cont);

  layout->addWidget(embeddedMainWindow);
  mainSplitter->addWidget(topLevelWidget);

  if (role_ == TtProtocolType::Client) {
    mqtt_client_setting_ = new Widget::MqttClientSetting;
    mainSplitter->addWidget(mqtt_client_setting_);
  } else {
  }

  main_layout_->addWidget(mainSplitter);
  mainSplitter->setStretchFactor(0, 2);
  mainSplitter->setStretchFactor(1, 1);

  QList<int> initialSizes;
  initialSizes << mainSplitter->width() - 200 << 200;
  mainSplitter->setSizes(initialSizes);

  mask_widget_ = new Ui::TtMaskWidget(this);
  mask_widget_->setReusable(true);

  meta_widget_ = new Widget::MqttMetaSettingWidget();

  connectSignals();
}

void MqttWindow::connectSignals() {

  connect(on_off_btn_, &Ui::TtSvgButton::clicked, [this]() {
    on_off_btn_->setEnabled(false);
    if (mqtt_client_->isConnected()) {
      mqtt_client_->disconnectFromHost();
    } else {
      mqtt_client_->connectToHost(
          mqtt_client_setting_->getMqttClientConfiguration());
    }
  });

  connect(save_btn_, &Ui::TtSvgButton::clicked, [this]() {
    config_.insert("WindowTitile", title_->text());
    if (role_ == TtProtocolType::Client) {
      config_.insert("UdpClientSetting",
                     mqtt_client_setting_->getMqttClientSetting());
    } else if (role_ == TtProtocolType::Server) {
      // config_.insert("UdpServerSetting",
      //                mqtt_->getUdpServerSetting());
    }
    // config_.insert("InstructionTable", instruction_table_->getTableRecord());
    // Ui::TtMessageBar::success(
    //     TtMessageBarType::Top, "警告",
    //     // "输入框不能为空，请填写完整信息。", 3000, this);
    //     "输入框不能为空，请填写完整信息。", 3000, this);
    emit requestSaveConfig();
    //qDebug() << cfg_.obj;
    // 配置文件保存到文件中
    // 当前的 tabWidget 匹配对应的 QJsonObject
  });

  connect(meta_btn_, &Ui::TtTextButton::clicked,
          [this]() { mask_widget_->show(meta_widget_); });

  subscripition_widget = new Widget::SubscripitionWidget();
  connect(subscriptionBtn, &Ui::TtSvgButton::clicked, [this]() {
    if (!mqtt_client_->isConnected()) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                              tr("请先与服务器建立链接"), 1500, this);
    }
    mask_widget_->show(subscripition_widget);
  });

  connect(subscripition_widget,
          &Widget::SubscripitionWidget::saveConfigToManager,
          [this](const QByteArray& config) {
            if (!config.isEmpty()) {
              QDataStream stream(config);
              QString topic, qos, color, alias, identifier, noLocal,
                  retainAsPublished, retainHandling;
              stream >> topic >> qos >> color >> alias >> identifier >>
                  noLocal >> retainAsPublished >> retainHandling;
              qDebug() << topic << qos << color << alias << identifier
                       << noLocal << retainAsPublished << retainHandling;

              QStandardItem* item = new QStandardItem(QString(topic));
              item->setData(config, Qt::UserRole);
              model->appendRow(item);
              mqtt_client_->subscribe(topic);
            }
          });

  connect(send_btn_, &QtMaterialFlatButton::clicked, [&]() {
    if (!mqtt_client_->isConnected()) {
      Ui::TtMessageBar::error(TtMessageBarType::Top, tr(""),
                              tr("客户端未建立链接"), 1500, this);
      return;
    }
    QString data = editor->text();
    mqtt_client_->publishMessage(
        send_topic_->text(), data, qos_->currentData().toInt(),
        retain_->isChecked(), meta_widget_->getMetaSettings());
    // auto tmp = new Ui::TtChatMessage();
    // tmp->setContent(data);
    // tmp->setOutgoing(true);
    // tmp->setBubbleColor(QColor("#DCF8C6"));
    // QList<Ui::TtChatMessage*> list;
    // list.append(tmp);
    // message_model_->appendMessages(list);
    // message_view_->scrollToBottom();
  });

  connect(mqtt_client_setting_, &Widget::MqttClientSetting::mqttVersionChanged,
          [this](TtMqttProcotol::Version version) {
            if (version != TtMqttProcotol::Q5_0) {
              meta_btn_->setEnabled(false);
            } else {
              meta_btn_->setEnabled(true);
            }
          });
}

void updateButtonPosition(QsciScintilla* scrollArea, QPushButton* sendButton) {
  const int margin = 10;  // 边距

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

}  // namespace Window
