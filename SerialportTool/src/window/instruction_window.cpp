#include "window/instruction_window.h"

#include <ui/control/ChatWidget/TtChatMessage.h>
#include <ui/control/ChatWidget/TtChatMessageModel.h>
#include <ui/control/ChatWidget/TtChatView.h>
#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>

#include "ui/widgets/data_presentation_widget.h"

namespace Window {

InstructionWindow::InstructionWindow(QWidget* parent) : QWidget{parent} {
  init();
}

void InstructionWindow::init() {
  main_layout_ = new Ui::TtVerticalLayout(this);

  // chart_ = new Ui::DataPresentationWidget(this);
  // // chart_->setStyleSheet("background-color : Coral");
  // main_layout_->addWidget(chart_);
}

}  // namespace Window
