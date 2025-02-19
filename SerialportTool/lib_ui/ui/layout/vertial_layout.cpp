#include "vertical_layout.h"

#include <QMargins>

namespace Ui {

TtVerticalLayout::TtVerticalLayout() {
  init();
}

TtVerticalLayout::TtVerticalLayout(QWidget* widget) : TtVerticalLayout() {
  widget->setLayout(this);
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
