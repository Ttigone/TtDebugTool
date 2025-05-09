#ifndef WINDOW_MQTT_WINDOW_H
#define WINDOW_MQTT_WINDOW_H

#include <QScrollArea>
#include <QWidget>
#include <Qsci/qsciscintilla.h>

#include "Def.h"
#include "ui/widgets/window_switcher.h"
#include "window/frame_window.h"

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QStandardItemModel;
QT_END_NAMESPACE

namespace Ui {
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class TtVerticalLayout;
class TtLineEdit;
class TtChatView;
class TtChatMessageModel;
class TtMaskWidget;
class TtComboBox;
class TtRadioButton;
class TtTextButton;

class SubscripitionManager;
} // namespace Ui

namespace Widget {
class MqttServerSetting;
class MqttClientSetting;
class SubscripitionWidget;
class MqttMetaSettingWidget;
} // namespace Widget

namespace Core {
class MqttServer;
class MqttClient;
} // namespace Core

class QtMaterialFlatButton;

namespace Window {

void updateButtonPosition(QsciScintilla *scrollArea, QPushButton *sendButton);

class ScrollAreaEventFilter : public QObject {
public:
  ScrollAreaEventFilter(QsciScintilla *scrollArea, QPushButton *button,
                        QObject *parent = nullptr)
      : QObject(parent), m_scrollArea(scrollArea), m_button(button) {}

protected:
  bool eventFilter(QObject *watched, QEvent *event) override {
    // 响应视口的尺寸变化或滚动事件
    if (event->type() == QEvent::Resize || event->type() == QEvent::Scroll) {
      updateButtonPosition(m_scrollArea, m_button);
    }
    return QObject::eventFilter(watched, event);
  }

private:
  QsciScintilla *m_scrollArea;
  QPushButton *m_button;
};

class MqttWindow : public FrameWindow {
  Q_OBJECT
public:
  explicit MqttWindow(TtProtocolType::ProtocolRole role,
                      QWidget *parent = nullptr);
  ~MqttWindow();

  QString getTitle() const;
  QJsonObject getConfiguration() const;

  bool workState() const override;
  bool saveState() override;
  void setSaveState(bool state) override;

  void saveSetting() override;
  void setSetting(const QJsonObject &config) override;

signals:
  void requestSaveConfig();

private slots:
  void switchToEditMode();
  void switchToDisplayMode();

private:
  void init();
  void connectSignals();

  Ui::TtVerticalLayout *main_layout_;

  Ui::TtNormalLabel *title_;          // 名称
  Ui::TtSvgButton *modify_title_btn_; // 修改连接名称
  Ui::TtSvgButton *save_btn_;         // 保存连接记录
  Ui::TtSvgButton *on_off_btn_;       // 开启 or 关闭

  Ui::TtComboBox *fomat_;
  Ui::TtComboBox *qos_;
  Ui::TtRadioButton *retain_;
  Ui::TtLineEdit *send_topic_;

  QsciScintilla *editor;

  Ui::TtMaskWidget *m_overlay = nullptr;
  // 消息展示框
  Ui::TtChatView *message_view_;
  // 数据
  Ui::TtChatMessageModel *message_model_;

  Widget::MqttClientSetting *mqtt_client_setting_;

  QWidget *original_widget_ = nullptr;
  QWidget *edit_widget_ = nullptr;
  Ui::TtLineEdit *title_edit_ = nullptr;
  QStackedWidget *stack_ = nullptr;

  Ui::TtSvgButton *subscriptionBtn;

  QtMaterialFlatButton *send_btn_;

  Core::MqttClient *mqtt_client_;

  QStandardItemModel *model;
  Ui::SubscripitionManager *subscripition_list_;

  Widget::SubscripitionWidget *subscripition_widget_ = nullptr;
  Widget::MqttMetaSettingWidget *meta_widget_ = nullptr;

  Ui::TtTextButton *meta_btn_;

  bool opened_;

  TtProtocolType::ProtocolRole role_;

  Ui::TtMaskWidget *mask_widget_ = nullptr; // 遮罩层
  QJsonObject config_;
};

} // namespace Window

#endif // WINDOW_MQTT_WINDOW_H
