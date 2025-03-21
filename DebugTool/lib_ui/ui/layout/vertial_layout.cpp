#include "vertical_layout.h"

#include <QMargins>

namespace Ui {

// TtVerticalLayout::TtVerticalLayout(QWidget* widget) : TtVerticalLayout() {
TtVerticalLayout::TtVerticalLayout(QWidget* widget) : QVBoxLayout(widget) {
  init();
  // widget->setLayout(this);
}

TtVerticalLayout::~TtVerticalLayout()
{

}

void TtVerticalLayout::init()
{
  setContentsMargins(QMargins());
  setSpacing(0);
}


} // namespace Ui
