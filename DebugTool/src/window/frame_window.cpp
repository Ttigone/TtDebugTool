#include "frame_window.h"

FrameWindow::FrameWindow(QWidget* parent) : QWidget{parent} {}

FrameWindow::~FrameWindow() {}

QString FrameWindow::title() const {
  return QString("TtFrameWindow");
}
