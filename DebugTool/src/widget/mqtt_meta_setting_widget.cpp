#include "widget/mqtt_meta_setting_widget.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/control/TtTextButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

namespace Widget {

PropertyRow::PropertyRow(QWidget *parent) : QWidget(parent) {
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  keyEdit_ = new Ui::TtLineEdit(this);
  keyEdit_->setPlaceholderText("Key");

  valEdit_ = new Ui::TtLineEdit(this);
  valEdit_->setPlaceholderText("Value");

  delBtn_ = new Ui::TtSvgButton(":/sys/trash.svg", this);
  delBtn_->setSvgSize(18, 18);

  layout->addWidget(keyEdit_, 1);
  layout->addWidget(valEdit_, 1);
  layout->addWidget(delBtn_, 0, Qt::AlignRight);

  connect(delBtn_, &Ui::TtSvgButton::clicked, this,
          [this]() { emit removeRequested(this); });
}

QString PropertyRow::key() const { return keyEdit_->text(); }

QString PropertyRow::value() const { return valEdit_->text(); }

void PropertyRow::setData(const QString &k, const QString &v) {
  keyEdit_->setText(k);
  valEdit_->setText(v);
}

MqttMetaSettingWidget::MqttMetaSettingWidget(QWidget *parent)
    : QWidget(parent) {
  setObjectName("MqttMetaSettingWidget");
  setAttribute(Qt::WA_StyledBackground, true);
  setStyleSheet(
      "#MqttMetaSettingWidget{ border-radius: 10px; background-color: white; "
      "}");
  init();
}

MqttMetaSettingWidget::~MqttMetaSettingWidget() {
  qDebug() << "delete MqttMeta";
}

void MqttMetaSettingWidget::setMetaSettings(const QByteArray &data) {
  QDataStream stream(data);

  // 读取固定属性
  QString contentType, msgExpiry, topicAlias, respTopic, corrData, subsId,
      payload;
  stream >> contentType >> msgExpiry >> topicAlias >> respTopic >> corrData >>
      subsId >> payload;
  content_type_->setText(contentType);
  message_expiry_interval_->setText(msgExpiry);
  topic_alias_->setText(topicAlias);
  response_topic_->setText(respTopic);
  correlation_data_->setText(corrData);
  subscripition_identifier_->setText(subsId);
  payload_format_indicator_->setChecked(payload.compare("1") ? true : false);

  // 清除属性
  clearProperties();
  // // 清除现有动态属性
  // QLayout *layout = scroll_content_->layout();
  // while (layout->count()) {
  //   auto item = layout->takeAt(0);
  //   if (auto w = item->widget()) {
  //     w->deleteLater();
  //   }
  //   delete item;
  // }

  // 读取动态属性
  QString key, value;
  while (!stream.atEnd()) {
    QString key, value;
    stream >> key >> value;
    // addPropertyRow(key, value);
    addProperty();
    propertyRows_.back()->setData(key, value);
  }
}

QByteArray MqttMetaSettingWidget::getMetaSettings() {
  QByteArray byteArray;
  QDataStream data(&byteArray, QIODevice::WriteOnly);

  data << QString(content_type_->currentText())
       << QString(message_expiry_interval_->currentText())
       << QString(topic_alias_->currentText())
       << QString(response_topic_->currentText())
       << QString(correlation_data_->currentText())
       << QString(subscripition_identifier_->currentText())
       << QString(payload_format_indicator_->isChecked() ? "1" : "0");

  qDebug() << "get data: " << byteArray;

  // 序列化动态属性
  QLayout *layout = scroll_content_->layout();
  for (int i = 0; i < layout->count(); ++i) {
    if (auto widget = qobject_cast<QWidget *>(layout->itemAt(i)->widget())) {
      QLayout *rowLayout = widget->layout();
      if (rowLayout && rowLayout->count() >= 2) {
        auto keyEdit =
            qobject_cast<Ui::TtLineEdit *>(rowLayout->itemAt(0)->widget());
        auto valueEdit =
            qobject_cast<Ui::TtLineEdit *>(rowLayout->itemAt(1)->widget());
        if (keyEdit && valueEdit) {
          data << keyEdit->text() << valueEdit->text();
        }
      }
    }
  }

  return byteArray;
}

void MqttMetaSettingWidget::addProperty() {
  // 添加行, 能否动态调整
  // 创建一行
  auto *row = new PropertyRow(scroll_content_);
  // 布局添加
  properties_layout_->addWidget(row);
  propertyRows_.push_back(row);

  connect(row, &PropertyRow::removeRequested, this,
          [this, row]() { removeProperty(row); });
  // 太小了
  scroll_->updateGeometry();
  scroll_->adjustSize();
}

void MqttMetaSettingWidget::removeProperty(PropertyRow *row) {
  properties_layout_->removeWidget(row);
  propertyRows_.erase(
      std::remove(propertyRows_.begin(), propertyRows_.end(), row),
      propertyRows_.end());
  row->deleteLater();
}

void MqttMetaSettingWidget::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);
  // main_layout_->setContentsMargins(QMargins(3, 3, 3, 3));
  main_layout_->setContentsMargins(0, 0, 0, 0);

  QWidget *setMetaInfoWidget = new QWidget(this);
  QGridLayout *setMetaInfoWidgetLayout = new QGridLayout(setMetaInfoWidget);

  QLabel *propLabel = new QLabel(tr("User Properties"), setMetaInfoWidget);
  Ui::TtSvgButton *plusButton = new Ui::TtSvgButton(":/sys/plus.svg");
  plusButton->setFixedSize(30, 30);
  plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  scroll_ = new QScrollArea(this);
  // scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  scroll_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  scroll_->setWidgetResizable(true);
  scroll_->setFrameStyle(QFrame::NoFrame);

  scroll_content_ = new QWidget(scroll_);
  scroll_->setStyleSheet("background-color: white");

  // 添加 row 的布局
  properties_layout_ = new QVBoxLayout(scroll_content_);
  scroll_content_->setLayout(properties_layout_);
  scroll_->setWidget(scroll_content_);
  // 添加的布局
  addProperty();

  content_type_ = new Ui::TtLabelLineEdit(tr("Content Type"));
  message_expiry_interval_ =
      new Ui::TtLabelLineEdit(tr("Message Expiry Interval(s)"));
  topic_alias_ = new Ui::TtLabelLineEdit(tr("Topic Alias"));
  response_topic_ = new Ui::TtLabelLineEdit(tr("Response Topic"));
  correlation_data_ = new Ui::TtLabelLineEdit(tr("Correlation Data"));
  subscripition_identifier_ =
      new Ui::TtLabelLineEdit(tr("Subscripition Identifier"));
  payload_format_indicator_ =
      new Ui::TtSwitchButton(tr("Payload Format Indicator"));

  // Ui::TtTextButton *cancleButton = new Ui::TtTextButton(tr("Cancle"));
  // Ui::TtTextButton *saveButton = new Ui::TtTextButton(tr("Save"));
  // connect(cancleButton, &Ui::TtTextButton::clicked, this, [this]() {
  //   emit closed();
  //   qDebug() << "cancle";
  // });

  // connect(saveButton, &Ui::TtTextButton::clicked, this, [this]() {
  //   emit closed();
  //   qDebug() << "save";
  // });

  setMetaInfoWidgetLayout->addWidget(propLabel, 0, 0, 1, 1, Qt::AlignLeft);
  setMetaInfoWidgetLayout->addWidget(plusButton, 0, 2, 1, 1, Qt::AlignRight);
  setMetaInfoWidgetLayout->addWidget(scroll_, 1, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(content_type_, 2, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(message_expiry_interval_, 3, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(topic_alias_, 4, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(response_topic_, 5, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(correlation_data_, 6, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(subscripition_identifier_, 7, 0, 1, 3);

  // setMetaInfoWidgetLayout->addWidget(cancleButton, 8, 1, 1, 1,
  // Qt::AlignRight); setMetaInfoWidgetLayout->addWidget(saveButton, 8, 2, 1, 1,
  // Qt::AlignLeft);

  // setMetaInfoWidgetLayout->setContentsMargins(0, 0, 0, 0);  // 移除边距
  // setMetaInfoWidgetLayout->setHorizontalSpacing(0);         //
  // 移除水平间距

  // 设置列拉伸因子：第0列固定，第1列扩展，第2列固定
  setMetaInfoWidgetLayout->setColumnStretch(0, 0); // 第0列不拉伸
  setMetaInfoWidgetLayout->setColumnStretch(1, 1); // 第1列占剩余空间
  setMetaInfoWidgetLayout->setColumnStretch(2, 0); // 第2列不拉伸
  setMetaInfoWidgetLayout->setRowStretch(1, 1);    // 第 1 可以拉伸

  // setMetaInfoWidget->setFixedWidth(300);
  // setMetaInfoWidget->setStyleSheet("background-color: Coral");

  // main_layout_->addWidget(scroll);
  main_layout_->addWidget(setMetaInfoWidget);

  connect(plusButton, &Ui::TtSvgButton::clicked, this,
          [this]() { addProperty(); });

  // 调整大小
  QMetaObject::invokeMethod(
      scroll_->widget(),
      [this]() {
        scroll_->updateGeometry();
        scroll_->adjustSize();
      },
      Qt::QueuedConnection);
}

void MqttMetaSettingWidget::addPropertyRow(const QString &key,
                                           const QString &value) {
  // 出现内存泄漏
  // 每个行都是 row
  QWidget *row = new QWidget(scroll_content_);
  auto layout = new Ui::TtHorizontalLayout(row);

  auto keyEdit = new Ui::TtLineEdit(row);
  keyEdit->setText(key);
  auto valueEdit = new Ui::TtLineEdit(row);
  valueEdit->setText(value);

  auto deleteBtn = new Ui::TtSvgButton(":/sys/trash.svg", row);
  deleteBtn->setSvgSize(18, 18);

  layout->addWidget(keyEdit, 1);
  layout->addWidget(valueEdit, 1);
  layout->addWidget(deleteBtn, 0, Qt::AlignRight);

  connect(deleteBtn, &Ui::TtSvgButton::clicked, this, [row]() {
    row->parentWidget()->layout()->removeWidget(row);
    row->deleteLater();
  });

  scroll_content_->layout()->addWidget(row);
  scroll_->widget()->adjustSize();
}

void MqttMetaSettingWidget::clearProperties() {
  for (auto row : propertyRows_) {
    row->deleteLater();
  }
  propertyRows_.clear();
}

} // namespace Widget
