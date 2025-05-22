#ifndef WIDGET_MQTT_META_SETTING_WIDGET_H
#define WIDGET_MQTT_META_SETTING_WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QScrollArea;
QT_END_NAMESPACE

namespace Ui {
class TtComboBox;
class TtLineEdit;
class TtLabelComboBox;
class TtLabelLineEdit;
class TtVerticalLayout;
class TtRadioButton;
class TtSvgButton;
class TtTextButton;
class TtSwitchButton;
}  // namespace Ui

namespace Widget {

class PropertyRow;

class PropertyRow : public QWidget {
  Q_OBJECT
 public:
  explicit PropertyRow(QWidget* parent = nullptr);
  QString key() const;
  QString value() const;
  void setData(const QString& k, const QString& v);

 signals:
  void removeRequested(PropertyRow* row);

 private:
  Ui::TtLineEdit* keyEdit_;
  Ui::TtLineEdit* valEdit_;
  Ui::TtSvgButton* delBtn_;
};

class MqttMetaSettingWidget : public QWidget {
  Q_OBJECT
 public:
  explicit MqttMetaSettingWidget(QWidget* parent = nullptr);
  ~MqttMetaSettingWidget();

  void setMetaSettings(const QByteArray& data);
  QByteArray getMetaSettings();

 signals:
  void closed();

 private slots:
  void addProperty();
  void removeProperty(PropertyRow* row);

 private:
  void init();
  void addPropertyRow(const QString& key = "", const QString& value = "");
  void clearProperties();

  Ui::TtVerticalLayout* main_layout_;
  Ui::TtLineEdit* key_;
  Ui::TtLineEdit* value_;
  Ui::TtLabelLineEdit* content_type_;
  Ui::TtLabelLineEdit* message_expiry_interval_;
  Ui::TtLabelLineEdit* topic_alias_;
  Ui::TtLabelLineEdit* response_topic_;
  Ui::TtLabelLineEdit* correlation_data_;
  Ui::TtLabelLineEdit* subscripition_identifier_;
  Ui::TtSwitchButton* payload_format_indicator_;

  QScrollArea* scroll_;
  QWidget* scroll_content_;
  QVBoxLayout* properties_layout_;

  std::vector<PropertyRow*> propertyRows_;

  Ui::TtTextButton* cancle_btn_;
  Ui::TtTextButton* confirm_btn_;
};

}  // namespace Widget
#endif  // MQTT_META_SETTING_WIDGET_H
