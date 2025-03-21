#include "widget/mqtt_meta_setting_widget.h"

#include <ui/control/TtComboBox.h>
#include <ui/control/TtLineEdit.h>
#include <ui/control/TtRadioButton.h>
#include <ui/control/TtSwitchButton.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

namespace Widget {

MqttMetaSettingWidget::MqttMetaSettingWidget(QWidget* parent)
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

void MqttMetaSettingWidget::setMetaSettings(const QByteArray& data) {
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

  // 清除现有动态属性
  QLayout* layout = scroll_content_->layout();
  while (layout->count()) {
    auto item = layout->takeAt(0);
    if (auto w = item->widget()) {
      w->deleteLater();
    }
    delete item;
  }

  // 读取动态属性
  QString key, value;
  while (!stream.atEnd()) {
    stream >> key >> value;
    addPropertyRow(key, value);
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
  QLayout* layout = scroll_content_->layout();
  for (int i = 0; i < layout->count(); ++i) {
    if (auto widget = qobject_cast<QWidget*>(layout->itemAt(i)->widget())) {
      QLayout* rowLayout = widget->layout();
      if (rowLayout && rowLayout->count() >= 2) {
        auto keyEdit =
            qobject_cast<Ui::TtLineEdit*>(rowLayout->itemAt(0)->widget());
        auto valueEdit =
            qobject_cast<Ui::TtLineEdit*>(rowLayout->itemAt(1)->widget());
        if (keyEdit && valueEdit) {
          data << keyEdit->text() << valueEdit->text();
        }
      }
    }
  }

  return byteArray;
}

void MqttMetaSettingWidget::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);
  main_layout_->setContentsMargins(QMargins(3, 3, 3, 3));

  QWidget* setMetaInfoWidget = new QWidget(this);
  QGridLayout* setMetaInfoWidgetLayout = new QGridLayout(setMetaInfoWidget);

  QLabel* propLabel = new QLabel(tr("User Properties"), setMetaInfoWidget);
  Ui::TtSvgButton* plusButton = new Ui::TtSvgButton(":/sys/plus.svg");

  scroll_ = new QScrollArea(this);
  // scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  scroll_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  scroll_->setWidgetResizable(true);
  scroll_->setFrameStyle(QFrame::NoFrame);

  scroll_content_ = new QWidget(scroll_);
  scroll_->setStyleSheet("background-color: white");
  Ui::TtVerticalLayout* scrollContentLayout =
      new Ui::TtVerticalLayout(scroll_content_);

  // Initial property widget
  QWidget* initialProperty = new QWidget(scroll_content_);
  Ui::TtHorizontalLayout* initialPropertyLayout =
      new Ui::TtHorizontalLayout(initialProperty);
  Ui::TtSvgButton* initialDeleteButton =
      new Ui::TtSvgButton(":/sys/trash.svg", initialProperty);
  initialDeleteButton->setSvgSize(18, 18);
  Ui::TtLineEdit* initialKey = new Ui::TtLineEdit(initialProperty);
  initialKey->setPlaceholderText("key");
  Ui::TtLineEdit* initialValue = new Ui::TtLineEdit(initialProperty);
  initialValue->setPlaceholderText("Value");

  initialPropertyLayout->addWidget(initialKey, 1);
  initialPropertyLayout->addWidget(initialValue, 1);
  initialPropertyLayout->addWidget(initialDeleteButton, 0, Qt::AlignRight);
  scrollContentLayout->addWidget(initialProperty);

  // Connect initial delete button
  connect(initialDeleteButton, &Ui::TtSvgButton::clicked, this,
          [this, initialProperty]() {
            scroll_content_->layout()->removeWidget(initialProperty);
            initialProperty->deleteLater();
          });

  scroll_->setWidget(scroll_content_);

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

  Ui::TtTextButton* cancleButton = new Ui::TtTextButton(tr("Cancle"));
  Ui::TtTextButton* saveButton = new Ui::TtTextButton(tr("Save"));

  connect(cancleButton, &Ui::TtTextButton::clicked, this, [this]() {
    emit closed();
    qDebug() << "cancle";
  });

  connect(saveButton, &Ui::TtTextButton::clicked, this, [this]() {
    emit closed();
    qDebug() << "save";
  });

  setMetaInfoWidgetLayout->addWidget(propLabel, 0, 0, 1, 1, Qt::AlignLeft);
  setMetaInfoWidgetLayout->addWidget(plusButton, 0, 2, 1, 1, Qt::AlignRight);
  setMetaInfoWidgetLayout->addWidget(scroll_, 1, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(content_type_, 2, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(message_expiry_interval_, 3, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(topic_alias_, 4, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(response_topic_, 5, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(correlation_data_, 6, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(subscripition_identifier_, 7, 0, 1, 3);
  setMetaInfoWidgetLayout->addWidget(cancleButton, 8, 1, 1, 1, Qt::AlignRight);
  setMetaInfoWidgetLayout->addWidget(saveButton, 8, 2, 1, 1, Qt::AlignLeft);

  // setMetaInfoWidgetLayout->setContentsMargins(0, 0, 0, 0);  // 移除边距
  // setMetaInfoWidgetLayout->setHorizontalSpacing(0);         // 移除水平间距

  // 设置列拉伸因子：第0列固定，第1列扩展，第2列固定
  setMetaInfoWidgetLayout->setColumnStretch(0, 0);  // 第0列不拉伸
  setMetaInfoWidgetLayout->setColumnStretch(1, 1);  // 第1列占剩余空间
  setMetaInfoWidgetLayout->setColumnStretch(2, 0);  // 第2列不拉伸
  setMetaInfoWidgetLayout->setRowStretch(1, 1);     // 第 1 可以拉伸

  // setMetaInfoWidget->setFixedWidth(300);
  // setMetaInfoWidget->setStyleSheet("background-color: Coral");

  // main_layout_->addWidget(scroll);
  main_layout_->addWidget(setMetaInfoWidget);

  connect(plusButton, &Ui::TtSvgButton::clicked, this, [this]() {
    QWidget* newProperty = new QWidget(scroll_content_);
    Ui::TtHorizontalLayout* propertyLayout =
        new Ui::TtHorizontalLayout(newProperty);
    Ui::TtLineEdit* keyEdit = new Ui::TtLineEdit(newProperty);
    keyEdit->setPlaceholderText("key");
    Ui::TtLineEdit* valueEdit = new Ui::TtLineEdit(newProperty);
    valueEdit->setPlaceholderText("Value");
    Ui::TtSvgButton* deleteButton =
        new Ui::TtSvgButton(":/sys/trash.svg", newProperty);
    deleteButton->setSvgSize(18, 18);
    propertyLayout->addWidget(keyEdit, 1);
    propertyLayout->addWidget(valueEdit, 1);
    propertyLayout->addWidget(deleteButton, 0, Qt::AlignRight);
    // Connect delete button
    connect(deleteButton, &Ui::TtSvgButton::clicked, this,
            [this, newProperty]() {
              if (auto layout = newProperty->parentWidget()->layout()) {
                layout->removeWidget(newProperty);
                newProperty->deleteLater();
              }
            });
    scroll_content_->layout()->addWidget(newProperty);
    // 添加新属性后强制更新布局
    scroll_->widget()->adjustSize();
    scroll_->updateGeometry();
  });
  QMetaObject::invokeMethod(
      scroll_->widget(),
      [this]() {
        scroll_->updateGeometry();
        scroll_->adjustSize();
      },
      Qt::QueuedConnection);
}

void MqttMetaSettingWidget::addPropertyRow(const QString& key,
                                           const QString& value) {
  QWidget* row = new QWidget(scroll_content_);
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

}  // namespace Widget
