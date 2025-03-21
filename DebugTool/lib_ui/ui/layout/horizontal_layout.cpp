#include "horizontal_layout.h"

#include <QMargins>

namespace Ui {

TtHorizontalLayout::TtHorizontalLayout() : QHBoxLayout() {
  init();
}

TtHorizontalLayout::TtHorizontalLayout(QWidget* widget) : TtHorizontalLayout() {
  widget->setLayout(this);
}

TtHorizontalLayout::~TtHorizontalLayout() {}

void TtHorizontalLayout::init()
{
  setContentsMargins(QMargins(0, 0, 0, 0));
  setSpacing(0);
}

} // namespace Ui

