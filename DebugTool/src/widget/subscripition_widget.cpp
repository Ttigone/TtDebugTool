#include "widget/subscripition_widget.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

namespace Widget {

SubscripitionWidget::SubscripitionWidget(QWidget* parent) : QWidget{parent} {
  setObjectName("SubscripitionWidget");
  setAttribute(Qt::WA_StyledBackground, true);
  setStyleSheet(
      "#SubscripitionWidget { border-radius: 10px; background-color: white; }");
  init();
}

SubscripitionWidget::~SubscripitionWidget() {
  // hide 的时候, 会被析构
  qDebug() << "delete SubscripitionWidget";
}

void SubscripitionWidget::onCloseButtonClicked() {
  emit closed();  // 发射关闭信号
}

void SubscripitionWidget::validateTopicInput() {
  const bool hasError = topic_edit_->toPlainText().isEmpty();

  // 设置动态属性
  topic_edit_->setProperty("error", hasError);
  topic_edit_->style()->unpolish(topic_edit_);
  topic_edit_->style()->polish(topic_edit_);

  // // 显示/隐藏错误提示
  if (hasError) {
    error_label_->show();
    error_animation_->start();
  } else {
    error_label_->hide();
  }

  // 强制重绘
  topic_edit_->update();
  error_label_->update();
}

void SubscripitionWidget::confirmSub() {
  // 已经有的订阅照样传, 根据订阅主题修改
  // 订阅为空停止
  if (topic_edit_->toPlainText().isEmpty()) {
    return;
  }

  QByteArray byteArray;
  QDataStream data(&byteArray, QIODevice::WriteOnly);

  data << QString(topic_edit_->toPlainText()) << QString(qos_->currentText())
       << QString(color_->text()) << QString(alias_edit_->toPlainText())
       << QString(subscripition_identifier_->currentText())
       << QString(retain_handling_->currentText());

  {
    auto button =
        qobject_cast<Ui::TtRadioButton*>(no_local_flag_bgr_->checkedButton());
    if (button) {
      // qDebug() << "y";
      // qDebug() << QString(button->text());
      data << QString(button->text());
    }
  }
  {
    auto button = qobject_cast<Ui::TtRadioButton*>(
        retain_as_pub_flag_bgr_->checkedButton());
    if (button) {
      // qDebug() << "y";
      // qDebug() << QString(button->text());
      data << QString(button->text());
    }
  }

  emit saveConfigToManager(byteArray);

  QMetaObject::invokeMethod(
      this, [this]() { emit closed(); }, Qt::QueuedConnection);
}

void SubscripitionWidget::init() {
  // 主容器设置不透明背景
  main_layout_ = new Ui::TtVerticalLayout(this);
  main_layout_->setContentsMargins(QMargins(3, 3, 3, 3));

  QWidget* basicWidget = new QWidget(this);
  basicWidget->setStyleSheet("background-color: white");
  Ui::TtVerticalLayout* basicWidgetLayout =
      new Ui::TtVerticalLayout(basicWidget);

  QWidget* titleBar = new QWidget(basicWidget);
  titleBar->setStyleSheet("QWidget { border-bottom: 1px solid #cccccc; }");
  Ui::TtHorizontalLayout* titleBarLayout = new Ui::TtHorizontalLayout(titleBar);

  Ui::TtElidedLabel* title = new Ui::TtElidedLabel(tr("新建订阅"));
  titleBarLayout->addWidget(title, 1, Qt::AlignLeft);
  close_button_ = new Ui::TtSvgButton(":/sys/delete.svg");
  close_button_->setSvgSize(18, 18);
  close_button_->setColors(QColor(144, 157, 132), Qt::red);
  close_button_->setEnableHoldToCheck(true);

  // 绑定关闭按钮点击事件
  connect(close_button_, &Ui::TtSvgButton::clicked, this,
          &SubscripitionWidget::onCloseButtonClicked);
  titleBarLayout->addWidget(close_button_, 0, Qt::AlignRight);

  QLabel* topic = new QLabel(basicWidget);
  topic->setText("<font color='red'>*</font> 主题");
  topic->setTextFormat(Qt::RichText);  // 确保HTML被正确解析

  QLabel* infoIcon = new QLabel(basicWidget);
  QPixmap pixmap = style()->standardPixmap(QStyle::SP_MessageBoxInformation);
  infoIcon->setPixmap(pixmap);
  infoIcon->setToolTip(
      tr("订阅一个或多个主题，当订阅多个主题时,使用(,)分隔每一个主题, 例如: "
         "test1,test2"));

  topic_edit_ = new QPlainTextEdit(basicWidget);
  topic_edit_->setObjectName("topicEdit");
  qApp->setStyleSheet(R"(
    QPlainTextEdit#topicEdit {
        border: 1px solid #cccccc;
        border-radius: 4px;
        padding: 4px;
    }
    QPlainTextEdit#topicEdit[error="true"] {
        border: 1px solid red;
    }
  )");
  topic_edit_->setPlainText(QString("testtopic/#"));
  connect(topic_edit_, &QPlainTextEdit::textChanged, this,
          &SubscripitionWidget::validateTopicInput);

  error_label_ = new QLabel(basicWidget);
  error_label_->setStyleSheet("color: red; font-size: 12px;");
  error_label_->setText("必须输入主题");
  error_label_->hide();
  error_animation_ = new QPropertyAnimation(error_label_, "opacity", this);
  error_animation_->setDuration(250);
  error_animation_->setStartValue(0.0);
  error_animation_->setEndValue(1.0);

  QGridLayout* layout1 = new QGridLayout();
  layout1->addWidget(topic, 0, 0, Qt::AlignLeft);
  layout1->addWidget(infoIcon, 0, 1, Qt::AlignRight);
  layout1->addWidget(topic_edit_, 1, 0, 1, 2);
  layout1->addWidget(error_label_, 2, 0, 1, 2);  // 新增错误提示行

  QLabel* qosLabel = new QLabel(basicWidget);
  qosLabel->setText("<font color='red'>*</font> QoS");
  qosLabel->setTextFormat(Qt::RichText);  // 确保HTML被正确解析
  qos_ = new Ui::TtComboBox(basicWidget);
  qos_->addItem("0 (At most once)");
  qos_->addItem("1 (At least once)");
  qos_->addItem("2 (Exactly once)");

  QLabel* colorLabel = new QLabel(tr("Color"), basicWidget);
  color_ = new Ui::TtLineEdit(basicWidget);

  QGridLayout* layout2 = new QGridLayout();
  layout2->addWidget(qosLabel, 0, 0, Qt::AlignLeft);
  layout2->addWidget(colorLabel, 0, 1, Qt::AlignRight);
  layout2->addWidget(qos_, 1, 0, Qt::AlignLeft);
  layout2->addWidget(color_, 1, 1, Qt::AlignRight);

  QLabel* aliasLabel = new QLabel(tr("Alias"), basicWidget);
  QLabel* infoIcon1 = new QLabel(basicWidget);
  QPixmap pixmap1 = style()->standardPixmap(QStyle::SP_MessageBoxInformation);
  infoIcon1->setPixmap(pixmap1);
  infoIcon1->setToolTip(
      tr("Comma separator (,) is also used when setting aliases for multiple "
         "topics"));

  alias_edit_ = new QPlainTextEdit(basicWidget);
  subscripition_identifier_ =
      new Ui::TtLabelLineEdit(tr("Subscripition Identifier"), basicWidget);
  QLabel* no = new QLabel("No Local Flag", basicWidget);
  QLabel* reta = new QLabel("Retain as Published Flag", basicWidget);

  no_local_flag_.append(new Ui::TtRadioButton("true"));
  no_local_flag_.append(new Ui::TtRadioButton("false"));

  retain_as_pub_flag_.append(new Ui::TtRadioButton("true"));
  retain_as_pub_flag_.append(new Ui::TtRadioButton("false"));

  retain_handling_ = new Ui::TtLabelComboBox("Retain Handling");
  retain_handling_->addItem("0");
  retain_handling_->addItem("1");
  retain_handling_->addItem("2");

  cancle_button_ = new Ui::TtTextButton("Cancle", this);
  connect(cancle_button_, &QPushButton::clicked, this,
          &SubscripitionWidget::onCloseButtonClicked);

  // 点击 confirm 后, 该 widget 被销毁
  confirm_button_ = new Ui::TtTextButton(Qt::blue, "Confirm", this);
  connect(confirm_button_, &QPushButton::clicked, this,
          &SubscripitionWidget::confirmSub);

  QGridLayout* layout3 = new QGridLayout();
  layout3->addWidget(aliasLabel, 0, 0, 1, 1, Qt::AlignLeft);
  layout3->addWidget(infoIcon1, 0, 2, 1, 1, Qt::AlignRight);
  layout3->addWidget(alias_edit_, 1, 0, 1, 3);
  layout3->addWidget(subscripition_identifier_, 2, 0, 1, 3);
  layout3->addWidget(no, 3, 0, 1, 1);
  no_local_flag_bgr_ = new QButtonGroup;
  no_local_flag_bgr_->setExclusive(true);
  no_local_flag_bgr_->addButton(no_local_flag_[0]);
  no_local_flag_bgr_->addButton(no_local_flag_[1]);
  no_local_flag_[0]->setCheckable(true);
  no_local_flag_[1]->setCheckable(true);
  no_local_flag_[0]->setChecked(true);
  layout3->addWidget(no_local_flag_[0], 3, 1, 1, 1);
  layout3->addWidget(no_local_flag_[1], 3, 2, 1, 1);
  layout3->addWidget(reta, 4, 0, 1, 1);
  retain_as_pub_flag_bgr_ = new QButtonGroup;
  retain_as_pub_flag_bgr_->setExclusive(true);
  retain_as_pub_flag_bgr_->addButton(retain_as_pub_flag_[0]);
  retain_as_pub_flag_bgr_->addButton(retain_as_pub_flag_[1]);
  retain_as_pub_flag_[0]->setCheckable(true);
  retain_as_pub_flag_[1]->setCheckable(true);
  retain_as_pub_flag_[0]->setChecked(true);
  layout3->addWidget(retain_as_pub_flag_[0], 4, 1, 1, 1);
  layout3->addWidget(retain_as_pub_flag_[1], 4, 2, 1, 1);
  layout3->addWidget(retain_handling_, 5, 0, 1, 3);
  layout3->addWidget(cancle_button_, 6, 1, 1, 1);
  layout3->addWidget(confirm_button_, 6, 2, 1, 1);

  basicWidgetLayout->setContentsMargins(QMargins(5, 5, 5, 5));
  basicWidgetLayout->addWidget(titleBar);
  basicWidgetLayout->addLayout(layout1);
  basicWidgetLayout->addLayout(layout2);
  basicWidgetLayout->addLayout(layout3);

  main_layout_->addWidget(basicWidget);
}

}  // namespace Widget
