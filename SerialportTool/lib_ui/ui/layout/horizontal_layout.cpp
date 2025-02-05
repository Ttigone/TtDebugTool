#include "horizontal_layout.h"

#include <QMargins>

namespace Ui {

HorizontalLayout::HorizontalLayout() {
  init();
}

void HorizontalLayout::init()
{
  setContentsMargins(QMargins(0, 0, 0, 0));
  setSpacing(0);
}

} // namespace Ui

