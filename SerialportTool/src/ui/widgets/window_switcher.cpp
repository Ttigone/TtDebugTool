#include "window_switcher.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QStackedLayout>
#include <QToolButton>
#include <QUuid>

#include "ui/layout/vertical_layout.h"

namespace Ui {

QMap<TtProtocolRole::Role, int> TabManager::type_map_ = {
    {TtProtocolRole::Serial, 0},       {TtProtocolRole::TcpClient, 0},
    {TtProtocolRole::TcpServer, 0},    {TtProtocolRole::UdpClient, 0},
    {TtProtocolRole::UdpServer, 0},    {TtProtocolRole::MqttClient, 0},
    {TtProtocolRole::MqttBroker, 0},   {TtProtocolRole::ModbusClient, 0},
    {TtProtocolRole::ModbusServer, 0}, {TtProtocolRole::BlueTeeth, 0},
};

uint16_t TabManager::SpecialTypeNums(TtProtocolRole::Role role) {
  return type_map_[role];
}

TabManager::TabManager(QWidget* defaultWidget, QWidget* parent)
    : QTabWidget(parent) {
  setTabBar(new ExtTabBar());
  setupCornerButton();
  // setTabBarAutoHide(true);

  setStyleSheet(R"(
    QTabWidget::pane {
        border: none;
        background: transparent;
    }
  )");

  addNewTab(defaultWidget);
  saveState("C:/Users/cssss/Desktop/test.json");

  // 连接关闭信号
  // connect(this, &QTabWidget::tabCloseRequested, this,
  //         [=](int index) { removeTab(index); });
  // connect(this, &QTabWidget::tabCloseRequested, this,
  //         &TabManager::handleTabClose);

  //   // 设置关闭按钮样式
  //   setStyleSheet(R"(
  //     QTabBar::close-button {
  //         image: url(:/sys/delete.svg);
  //         subcontrol-position: right;
  //         subcontrol-origin: padding;
  //         margin-right: 4px;
  //     }
  //     QTabBar::close-button:hover {
  //         image: url(:/sys/delete.svg);
  //     }
  //     QTabBar::close-button:pressed {
  //         image: url(:/sys/link.svg);
  //     }
  // )");
  setupTabBar();
}

TabManager::~TabManager() {
  //for (auto& widget : widgetInstances) {
  //  if (widget) {
  //    widget->deleteLater();
  //  }
  //}
  // qDeleteAll(widgetInstances);
};

void TabManager::addNewTab(const QString& title) {
  // 创建默认的 widget1
  // 总数量
  int tabIndex = count();
  QWidget* defaultWidget = createDefaultWidget(tabIndex);
  // addTab(defaultWidget, "Tab " + QString::number(tabIndex + 1));
  addTab(defaultWidget, title);
  // setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
}

void TabManager::addNewTab(QWidget* defaultWidget) {
  // 创建默认的 widget1
  // int tabIndex = count();
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  QIcon icon(":/sys/unlink.svg");
  addTab(defaultWidget, icon, tr("新增连接"));
  setupCustomTabButton(count() - 1);
}

void TabManager::addNewTab(QWidget* defaultWidget, const QString& title) {
  // 创建默认的 widget1
  int tabIndex = count();
  qDebug() << "tabINdex: " << tabIndex;
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  // 添加的 tab 是一直在最后面的
  QIcon icon(":/sys/unlink.svg");
  addTab(defaultWidget, icon, title);
  setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
}

void TabManager::registerWidget(TtProtocolRole::Role role,
                                const WidgetFactory& factory,
                                const QString& title) {
  // widgetid 注册的窗口标识符 [2]
  widgetFactories[role] = factory;
  widgetTitles[role] = title;
}

void TabManager::switchToWidget(int tabIndex, TtProtocolRole::Role role) {
  // 根据 id 切换目标 widget
  // base::DetectRunningTime runtime;

  // 切换
  if (tabIndex < 0 || tabIndex >= count()) {
    return;  // 无效的 Tab 索引
  }

  if (!widgetFactories.contains(role)) {
    qDebug() << "no register";
    return;  // 未注册的 Widget ID
  }

  // 销毁当前 Widget
  // 实现原地操作
  // 根据 index 去索引 widget, 目标切换窗口!!! 点击的时候对应的 index
  QWidget* currentWidget = widget(tabIndex);
  if (currentWidget) {
    currentWidget->deleteLater();
  }

  // 创建新的 Widget
  QWidget* newWidget = widgetFactories[role]();

  type_map_[role]++;

  // qDebug() << runtime.elapseMilliseconds();
  // 实例应当个性化
  //widgetInstances[newWidget] = ;  // 存储原始指针
  // 根据 uuid 标识
  widgetInstances[QUuid::createUuid().toString()] = newWidget;


  // 设置 tab 的文本
  setTabText(tabIndex, widgetTitles[role]);
  // 向 tabIndex 所有的 tab 界面设置界面 newWidget, title 为对应索引值
  insertTab(tabIndex, newWidget, widgetTitles[role]);

  setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);

  // 显示当前正在操作的 tab
  // 重复创建 串口时, 发现会跳到之前已经创建好的
  setCurrentIndex(tabIndex);
}

void TabManager::setupTabBar() {
  // ExtTabBar* tabBar = new ExtTabBar(this);
  // setTabBar(tabBar);

  //// 连接信号
  //connect(tabBar, &ExtTabBar::tabCloseRequested, this,
  //        &TabManager::handleTabCloseRequested);
  //connect(tabBar, &ExtTabBar::newTabRequested, this, &TabManager::handleAddNewTab);
}

bool TabManager::saveState(const QString& filePath) const {
  QJsonArray tabsArray;

  // 保存当前标签页
  for (int i = 0; i < count(); ++i) {
    tabsArray.append(serializeTab(i));
  }

  // 往文件写数据
  QJsonDocument doc(tabsArray);
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  file.write(doc.toJson());
  return true;
}

bool TabManager::restoreState(const QString& filePath) {
  // 将标签页的数据读取
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  // 先读取成 qdocument
  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isArray()) {
    QJsonArray tabsArray = doc.array();
    for (const auto& tabValue : tabsArray) {
      deserializeTab(tabValue.toObject());
    }
    return true;
  }
  return false;
}

QString TabManager::getCurrentWidgetUUid() {
  return findWidget(currentWidget());
}

void TabManager::setTabIcon(int index, const QString& iconPath) {
  setTabIcon(index, QIcon(iconPath));
}

void TabManager::setTabIcon(int index, const QIcon& icon) {
  QTabWidget::setTabIcon(index, icon);
  updateTabStyle(index);
}

void TabManager::setTabTitle(const QString& title) {
  tabBar()->setTabText(currentIndex(), title);
}

QString TabManager::findWidget(QWidget* widget) {
  for (auto it = widgetInstances.cbegin(); it != widgetInstances.cend(); ++it) {
    if (it.value() == widget) {
      return it.key();
    }
  }
  return QString();
}

void TabManager::handleTabClose(int index) {
  // // 保存标签页信息
  TabData info;
  info.title = tabText(index);
  info.widget = widget(index);
  info.icon = tabIcon(index);
  // info.state

  // 保存widget状态
  if (auto* serializable = dynamic_cast<ISerializable*>(info.widget)) {
    info.state = serializable->saveState();
  }
  // qDebug() << info.state;

  // 添加到历史列表
  // QTabWidget::widget(index);
  closedTabs_.prepend(qMakePair(findWidget(widget(index)), info));
  // qDebug() << closedTabs_.size();

  // 过多则本地化一部分
  // // 限制历史数量
  // while (closedTabs_.size() > maxClosedTabs_) {
  //   TabData& lastInfo = closedTabs_.last();
  //   delete lastInfo.widget;  // 删除过旧的widget
  //   closedTabs_.removeLast();
  // }

  // 从TabWidget中移除，但不删除widget
  removeTab(index);

  // emit tabClosed(index);
}

void TabManager::restoreLastClosedTab() {
  // if (!closedTabs_.isEmpty()) {
  //   TabInfo info = closedTabs_.takeFirst();
  //   int index = addTab(info.widget, info.icon, info.title);
  //   setCurrentIndex(index);
  // }
  if (closedTabs_.isEmpty()) {
    return;
  }

  TabData info = closedTabs_.takeFirst().second;

  // 恢复状态
  if (!info.state.isEmpty()) {
    if (auto* serializable = dynamic_cast<ISerializable*>(info.widget)) {
      serializable->restoreState(info.state);
    }
  }

  int index = addTab(info.widget, info.icon, info.title);
  setCurrentIndex(index);

  // emit tabRestored(index);
}

QStringList TabManager::getClosedTabsList() const {
  QStringList list;
  // for (const TabInfo& info : closedTabs_) {
  //   list << info.title;
  // }
  return list;
}

QJsonObject TabManager::serializeTab(int index) const {
  QJsonObject tabObj;
  tabObj["title"] = tabText(index);
  tabObj["icon"] = tabIcon(index).name();

  // 如果widget支持序列化
  QWidget* w = widget(index);
  if (auto* serializable = dynamic_cast<ISerializable*>(w)) {
    tabObj["state"] = QString(serializable->saveState().toBase64());
  }

  return tabObj;
}

void TabManager::deserializeTab(const QJsonObject& obj) {
  QString title = obj["title"].toString();
  QString iconName = obj["icon"].toString();

  QWidget* page = new QWidget(this);
  // 恢复widget状态
  if (obj.contains("state")) {
    QByteArray state =
        QByteArray::fromBase64(obj["state"].toString().toLatin1());
    if (auto* serializable = dynamic_cast<ISerializable*>(page)) {
      serializable->restoreState(state);
    }
  }

  addTab(page, QIcon(iconName), title);
}

// void TabManager::handleButtonClicked(int tabIndex) {
void TabManager::handleButtonClicked(int tabIndex, TtProtocolRole::Role role) {
  // 假设点击按钮后切换到 widget2
  //qDebug() << 
  // TODO 信号只传递了要切换的 widgetid, 但是没有 tabindex !!!
  switchToWidget(tabIndex, role);  // widgetId = 2 是 widget2
}

void TabManager::handleAddNewTab() {
  addNewTab("新建立");
}

void TabManager::handleTabCloseRequested(int index) {
  if (count() <= 1)
    return;  // 至少保留一个标签

  QWidget* widget = this->widget(index);
  // 彻底删除
  widget->deleteLater();
  removeTab(index);
}

void TabManager::removeUuidWidget(const QString& index) {
  // 移除标签页
  auto item = widgetInstances.value(index);
  // qDebug() << "item: " << item;
  // currentIndex();
  auto i = indexOf(item);
  // qDebug() << "index item: " << i;

  // 获取与选项卡关联的 QWidget
  auto* widget = QTabWidget::widget(i);
  // qDebug() << "index item: " << widget;
  if (widget) {
    // 延迟删除 QWidget
    widget->deleteLater();
  }

  removeTab(i);
}

void TabManager::switchToCurrentIndex(const QString& index) {
  // qDebug() << "get index" << index;
  // uuid QWidget
  // 切换, 以及复原
  // 先查找是否被 remove
  // if (closedTabs_.contains())
  // 以下操作都是整个应用程序没有完全关闭，运行于内存的时候
  // 持久化存储需要整个页面关闭
  for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
    if (it->first == index) {
      qDebug() << "remove";
      // 恢复 添加到最后
      addNewTab(it->second.widget, it->second.title);
      // 切换到复原的 tab
      setCurrentIndex(count() - 1);
      closedTabs_.erase(it);
      return;
    }
  }

  // 切换
  if (widgetInstances.contains(index)) {
    setCurrentWidget(widgetInstances.value(index));
  }
}

void TabManager::setupCustomTabButton(int index) {
  auto* closeButton = new TabCloseButton(this);
  tabBar()->setTabButton(index, QTabBar::RightSide, closeButton);

  // connect(closeButton, &QToolButton::clicked, this, [this, index]() {
  //   // qDebug() << "close";
  //   qDebug() << "index close: " << index;
  //   handleTabClose(index);
  // });
  // 使用按钮找到当前索引，而不是使用固定索引
  connect(closeButton, &QToolButton::clicked, this, [this, closeButton]() {
    int currentIndex = getTabIndexFromButton(closeButton);
    if (currentIndex != -1) {
      handleTabClose(currentIndex);
    }
  });
}

void TabManager::updateTabStyle(int index) {
  // 设置tab的样式
  QString style = R"(
        QTabBar::tab {
            padding: 4px;
            padding-left: 8px;
            padding-right: 8px;
            margin-right: 2px;
            border: none;
            background: transparent;
            min-width: 80px;
        }
        QTabBar::tab:hover {
            background: rgba(0, 0, 0, 0.1);
        }
        QTabBar::tab:selected {
            background: rgba(0, 0, 0, 0.15);
        }
    )";

  tabBar()->setStyleSheet(style);
}

QWidget* TabManager::createDefaultWidget(int tabIndex) {
  // 新建 widget
  QWidget* widget = new QWidget(this);
  //// 布局
  ////QVBoxLayout* layout = new QVBoxLayout(widget);
  //Ui::VerticalLayout* layout = new VerticalLayout(widget);

  //QPushButton* button = new QPushButton("Switch to Widget2", widget);
  ////connect(button, &QPushButton::clicked, this,
  ////        [this, tabIndex]() { handleButtonClicked(tabIndex); });

  //layout->addWidget(button);
  //widget->setLayout(layout);
  return widget;
}

// CustomTabPage::CustomTabPage(QWidget* parent) : QWidget(parent) {}

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
