#include "vertical_layout.h"

#include <QMargins>

namespace Ui {

VerticalLayout::VerticalLayout() {
  init();
}

VerticalLayout::VerticalLayout(QWidget* widget) : VerticalLayout() {
  widget->setLayout(this);
}

VerticalLayout::~VerticalLayout()
{

}

void VerticalLayout::init()
{
  setContentsMargins(QMargins());
  setSpacing(0);
}


} // namespace Ui
