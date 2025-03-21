#include "customizationtabwidget.h"

namespace Ui {

CustomizationTabWidget::CustomizationTabWidget(QWidget *parent) :
      QTabWidget(parent) {

}

CustomizationTabWidget::~CustomizationTabWidget()
{

}

void CustomizationTabWidget::addTabWidthTitle(const QString &title)
{
  QWidget *tabContent = new QWidget();
  QHBoxLayout *layout = new QHBoxLayout(tabContent);
  layout->setContentsMargins(QMargins(5, 0, 5, 0));

  // 标签内容
  QLabel *label = new QLabel(title);
  QPushButton *closeBtn = new QPushButton("x");

  // 图标 左边标签, 关闭按钮
  layout->addWidget(label);
  layout->addWidget(closeBtn);

  // 父类的 addTab 函数
  int index = addTab(new QWidget(), "");
  tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);
  tabBar()->setTabText(index, title);

  connect(closeBtn, &QPushButton::clicked, this, [=]() {
    removeTab(index);
  });


}


} // namespace Ui
