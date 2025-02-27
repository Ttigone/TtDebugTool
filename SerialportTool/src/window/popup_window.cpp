#include "popup_window.h"

#include "ui/layout/horizontal_layout.h"
#include "ui/layout/vertical_layout.h"

#include "ui/widgets/labels.h"

namespace Window {

PopUpWindow::PopUpWindow(QWidget* parent)
    : QWidget(parent), layout_(new QStackedLayout(this)) {
  init();
}

void PopUpWindow::addPairedWidget(int index, QWidget* widget) {
  widget->setParent(this);
  // 动画完成后恢复状态
  layout_->insertWidget(index, widget);
  //registerWidget(widget);
  registered_widgets_.insert(index, widget);
}

//void PopUpWindow::registerWidget(int index, QWidget* widget) {
//}

//void PopUpWindow::registerWidget(QWidget* widget) {
//  //registered_widgets_.insert()
//}

//void PopUpWindow::setWidget(QWidget* widget) {
//  if (main_layout_->count() == 1) {
//    main_layout_->addWidget(widget);
//  } else {
//    qDebug() << "已经存在一个 widget";
//    // 将原有的 widget delete 掉
//    // main_layout_->indexOf()
//    // main_layout_->widget()
//  }
//}

bool PopUpWindow::switchToWidget(int index) {
  if (layout_->count() - 1 < index) {
    qWarning("index out of range");
  }
  //qDebug() << "cur index: " << layout_->currentIndex();
  if (layout_->currentIndex() == index) {
    // 初始时默认是 0
		return false;
  }
  layout_->setCurrentIndex(index);
  return true;
}

void PopUpWindow::init() {}
}  // namespace Window
