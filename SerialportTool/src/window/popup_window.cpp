#include "popup_window.h"

#include "ui/layout/vertical_layout.h"
#include "ui/layout/horizontal_layout.h"

#include "ui/widgets/labels.h"

namespace Window {

PopUpWindow::PopUpWindow(QWidget *parent) : QWidget(parent) {
  init();
}

void PopUpWindow::setTitle(const QString &title)
{
  title_label_->setText(title);
}

void PopUpWindow::setWidget(QWidget *widget)
{
  if (main_layout_->count() == 1) {
    main_layout_->addWidget(widget);
  } else {
    qDebug() << "已经存在一个 widget";
    // 将原有的 widget delete 掉
    // main_layout_->indexOf()
    // main_layout_->widget()
  }
}

void PopUpWindow::init()
{
  main_layout_ = new Ui::VerticalLayout();
  this->setLayout(main_layout_);


  up_widget_ = new QWidget(this);
  up_widget_->setFixedHeight(40);
  // up_layout_ = new QHBoxLayout(up_widget_);
  up_layout_ = new Ui::HorizontalLayout();
  up_widget_->setLayout(up_layout_);

  // title_label_ = new QLabel(up_widget_);
  title_label_ = new Ui::TtNormalLabel(up_widget_);
  // 添加在左侧
  up_layout_->addWidget(title_label_, Qt::AlignLeft);

  // 添加到最上侧
  main_layout_->addWidget(up_widget_, Qt::AlignTop);

  // body_widget_ = new QWidget();
  // body_widget_->setStyleSheet("background-color: Coral");
  // main_layout_->addWidget(body_widget_);
}

}
