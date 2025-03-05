#ifndef MQTT_WINDOW_H
#define MQTT_WINDOW_H

#include <Qsci/qsciscintilla.h>
#include <QWidget>

#include "Def.h"

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

namespace Ui {
class TtNormalLabel;
class CommonButton;
class TtImageButton;
class TtSvgButton;
class MessageDialog;
class TtVerticalLayout;
class TtLineEdit;

class TtChatView;
class TtChatMessageModel;
}  // namespace Ui

namespace Widget {
class TcpServerSetting;
class TcpClientSetting;
}  // namespace Widget

namespace Core {
class TcpServer;
class TcpClient;
}  // namespace Core

namespace Window {

class MqttWindow : public QWidget {
  Q_OBJECT
 public:
  explicit MqttWindow(TtProtocolType::ProtocolRole role,
                      QWidget* parent = nullptr);
  ~MqttWindow();

 signals:

 private:
  TtProtocolType::ProtocolRole role_;
};

}  // namespace Window

#endif  // MQTT_WINDOW_H
