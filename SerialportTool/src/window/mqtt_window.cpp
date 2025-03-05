#include "mqtt_window.h"

namespace Window {

MqttWindow::MqttWindow(TtProtocolType::ProtocolRole role, QWidget* parent)
    : QWidget(parent), role_(role) {}

MqttWindow::~MqttWindow() {}

}  // namespace Window
