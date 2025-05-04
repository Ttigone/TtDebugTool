#include "window/frame_window.h"

namespace Window {

FrameWindow::FrameWindow(QWidget* parent) : QWidget{parent} {}

FrameWindow::~FrameWindow() {}

QString FrameWindow::title() const {
  return QString("TtFrameWindow");
}

}  // namespace Window
