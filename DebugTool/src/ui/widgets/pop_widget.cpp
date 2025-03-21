#include "ui/widgets/pop_widget.h"
#include <qtmaterialdrawer.h>
#include <ui/control/TtPopUpDrawer.h>
#include "qtmaterialdrawer_internal.h"

#include <ui/Def.h>

namespace Ui {

PopWidget::PopWidget(QWidget* parent)
    : QWidget(parent),
      drawer_(new TtPopUpDrawer(parent)),
      // drawer_(new QtMaterialDrawer(parent)),
      is_open_(false) {
  init();
}

PopWidget::~PopWidget() {}

void PopWidget::setWidget(QWidget* w) {
  if (!drawer_layout_->widget()) {
    drawer_layout_->addWidget(w);
  } else {
    drawer_layout_->widget()->deleteLater();
    drawer_layout_->addWidget(w);
  }
}

void PopWidget::openDrawer() {
  drawer_->openDrawer();
  is_open_ = true;
}

void PopWidget::closeDrawer() {
  drawer_->closeDrawer();
  is_open_ = false;
}

void PopWidget::triggerSwitch() {
  is_open_ ? closeDrawer() : openDrawer();
}

void PopWidget::resizeEvent(QResizeEvent* event) {
  qDebug() << this->width();
  QWidget::resizeEvent(event);
}
void PopWidget::init() {
  // drawer_ 本身是一个窗口
  // 允许在展开后, 点击面板的其他区域可以关闭
  drawer_->setClickOutsideToClose(true);
  drawer_->setOverlayMode(true);
  drawer_->setAutoRaise(true);
  drawer_->setDrawerSize(300);
  drawer_->setDirection(TtPopUpDirection::Left);

  drawer_layout_ = new QVBoxLayout;
  drawer_layout_->setContentsMargins(QMargins());
  drawer_layout_->setSpacing(0);
  drawer_->setDrawerLayout(drawer_layout_);
  connect(drawer_, &TtPopUpDrawer::willClose, [this]() {
    emit isClosed();
    is_open_ = false;
  });
}

}  // namespace Ui
