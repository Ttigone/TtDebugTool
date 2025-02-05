#include "window_switcher.h"
#include <QStackedLayout>

namespace Ui {

TabManager::TabManager(QWidget* parent) : QTabWidget(parent) {
  // 默认添加一个 Tab
  addNewTab();
}

TabManager::TabManager(QWidget* defaultWidget, QWidget* parent) : QTabWidget(parent) {
  setTabBar(new ExtTabBar());
  addNewTab(defaultWidget);
}

TabManager::~TabManager() {
  for (auto& widget : widgetInstances) {
    if (widget) {
      widget->deleteLater();
    }
  }
  // qDeleteAll(widgetInstances);
};

void TabManager::addNewTab(const QString& title) {
  // 创建默认的 widget1
  int tabIndex = count();
  QWidget* defaultWidget = createDefaultWidget(tabIndex);
  // addTab(defaultWidget, "Tab " + QString::number(tabIndex + 1));
  addTab(defaultWidget, title);
}

void TabManager::addNewTab(QWidget* defaultWidget) {
  // 创建默认的 widget1
  int tabIndex = count();
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  addTab(defaultWidget, "Tab " + QString::number(tabIndex + 1));
}

void TabManager::addNewTab(QWidget* defaultWidget, const QString& title) {
  // 创建默认的 widget1
  int tabIndex = count();
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  addTab(defaultWidget, title);
}

void TabManager::registerWidget(int widgetId, const WidgetFactory& factory,
                                const QString& title) {
  // widgetid 注册的窗口标识符 [2]
  widgetFactories[widgetId] = factory;
  widgetTitles[widgetId] = title;
}

void TabManager::switchToWidget(int tabIndex, int widgetId) {
  // base::DetectRunningTime runtime;

  // 切换
  if (tabIndex < 0 || tabIndex >= count()) {
    return;  // 无效的 Tab 索引
  }

  if (!widgetFactories.contains(widgetId)) {
    qDebug() << "no register";
    return;  // 未注册的 Widget ID
  }

  // 销毁当前 Widget
  // 实现原地操作
  QWidget* currentWidget = widget(tabIndex);
  if (currentWidget) {
    currentWidget->deleteLater();
  }

  // 创建新的 Widget
  QWidget* newWidget = widgetFactories[widgetId]();

  // qDebug() << runtime.elapseMilliseconds();

  widgetInstances[widgetId] = newWidget;  // 存储原始指针
  // 设置 tab 的文本
  setTabText(tabIndex, widgetTitles[widgetId]);
  // 向 tabIndex 所有的 tab 界面设置界面 newWidget, title 为对应索引值
  insertTab(tabIndex, newWidget, widgetTitles[widgetId]);
  // 显示当前正在操作的 tab
  setCurrentIndex(tabIndex);
}

// void TabManager::handleButtonClicked(int tabIndex) {
void TabManager::handleButtonClicked(int widgetId) {
  // 假设点击按钮后切换到 widget2
  switchToWidget(count() - 1, widgetId);  // widgetId = 2 是 widget2
}

QWidget* TabManager::createDefaultWidget(int tabIndex) {
  // 新建 widget
  QWidget* widget = new QWidget(this);
  // 布局
  QVBoxLayout* layout = new QVBoxLayout(widget);

  QPushButton* button = new QPushButton("Switch to Widget2", widget);
  connect(button, &QPushButton::clicked, this,
          [this, tabIndex]() { handleButtonClicked(tabIndex); });

  layout->addWidget(button);
  widget->setLayout(layout);
  return widget;
}

// // 注册 widget2
// tabManager.registerWidget(2, []() -> QWidget* {
//   QWidget* widget = new QWidget();
//   QVBoxLayout* layout = new QVBoxLayout(widget);
//   layout->addWidget(new QLabel("This is Widget2"));
//   widget->setLayout(layout);
//   return widget;
// }, "Widget2");

// // 注册 widget3（用于新 Tab）
// tabManager.registerWidget(3, []() -> QWidget* {
//   QWidget* widget = new QWidget();
//   QVBoxLayout* layout = new QVBoxLayout(widget);
//   layout->addWidget(new QLabel("This is Widget3"));
//   widget->setLayout(layout);
//   return widget;
// }, "Widget3");

}  // namespace Ui
