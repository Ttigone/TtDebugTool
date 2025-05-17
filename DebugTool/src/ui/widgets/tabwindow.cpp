/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group
  company <info@kdab.com> Author: Nicolas Arnaud-Cormos
  <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "tabwindow.h"

#include <QApplication>
#include <QList>
#include <QMouseEvent>
#include <QTabBar>
#include <QWindow>

#include <ui/control/TtContentDialog.h>

#include "ui/layout/vertical_layout.h"

#include <window/frame_window.h>

namespace {
const constexpr int DISTANCE_TO_DETACH = 20;

int distance(QRect r, QPoint p) {
  if (r.contains(p))
    return 0;

  auto ref = [](int a, int b, int v) {
    if (v < a) // left/top
      return a;
    else if (v > b) // right/bottom
      return b;
    else // in-between
      return v;
  };

  const auto refPoint =
      QPoint(ref(r.left(), r.right(), p.x()), ref(r.top(), r.bottom(), p.y()));

  return (refPoint - p).manhattanLength();
}
} // namespace

TabWindowManager::TabWindowManager() {
  connect(qApp, &QApplication::focusWindowChanged, this,
          &TabWindowManager::activateWindow);
}

TabWindowManager *TabWindowManager::instance() {
  static TabWindowManager manager;
  return &manager;
}

QList<TabWindow *> TabWindowManager::windows() const { return m_windows; }

TabWindow *TabWindowManager::currentWindow() const {
  if (!m_windows.isEmpty())
    return m_windows.first();
  return nullptr;
}

QWidget *TabWindowManager::currentWidget() const {
  if (!m_windows.isEmpty())
    return m_windows.first()->currentWidget();
  return nullptr;
}

TabWindow *TabWindowManager::rootWindow() const { return m_root; }

void TabWindowManager::setRootWindow(TabWindow *root) {
  m_root = root;
  addWindow(root);
}

void TabWindowManager::removeUuidTabPage(const QString &index) {
  // qDebug() << "tabWindow get remove:" << index;
  if (TabWindow::widgetInstances.contains(index)) {
    auto *item = TabWindow::widgetInstances.value(index);
    int idx = 0;
    for (TabWindow *w : m_windows) {
      if ((idx = w->indexOf(item)) != -1) {
        // qDebug() << "成功找到";
        auto *widget = w->widget(idx);
        if (widget) {
          // qDebug() << "删除标签页";
          widget->disconnect();
          widget->setParent(nullptr);
          widget->deleteLater();
        }
        // qDebug() << "remove by leftbutton: " << index;
        TabWindow::widgetInstances.remove(index);
        w->removeTab(idx);
        if (w->count() == 0) {
          // qDebug() << "TabWindow is empty, removing it from manager.";
          removeWindow(w);
        }
        break;
      }
    }
  }
}

void TabWindowManager::addWindow(TabWindow *window) {
  // main_widow 设置了 root, 已经添加过了
  qDebug() << "[Manager] addWindow: " << window;
  if (m_windows.contains(window)) {
    return;
  }
  m_windows.append(window);

  // disconnect(window, &TabWindow::requestNewTab, this, nullptr);
  // 删除窗口
  connect(window, &TabWindow::tabCloseRequested, this,
          &TabWindowManager::requestCloseTab);

  // 新窗口是肯定链接了的
  // 但是切换其他窗口无响应
  connect(window, &TabWindow::requestNewTab, this, [this, window]() {
    // 触发 2 次 ???
    qDebug() << "test";
    emit tabCreateRequested(window);
  });
}

void TabWindowManager::removeWindow(TabWindow *window) {
  // root 不能删除
  if (m_windows.contains(window) && window != m_root) {
    qDebug() << "[Manager] Removing window:" << window;
    m_windows.removeOne(window);
    window->deleteLater(); // 确保对象删除
  }
}

TabWindow *TabWindowManager::possibleWindow(TabWindow *src, QPoint globalPos) {
  for (TabWindow *w : m_windows) {
    if (w == src) {
      continue;
    }
    QWindow *top = w->windowHandle();
    if (!top) {
      continue;
    }
    if (top->frameGeometry().contains(globalPos)) {
      QPoint pos = w->tabBar()->mapFromGlobal(globalPos);
      QRect bar = w->tabBar()->rect();
      bar.setWidth(w->width());
      if (bar.contains(pos)) {
        return w;
      }
    }
  }
  return nullptr;
}

void TabWindowManager::activateWindow(QWindow *window) {
  if (m_windows.count() < 2) {
    // 窗口数量只有 1 个
    return;
  }

  int index = -1;
  // 原始窗口 0 号不作为激活的窗口
  for (int i = 1; i < m_windows.count(); ++i) {
    if (m_windows.at(i)->windowHandle() == window) {
      index = i;
      break;
    }
  }

  if (index != -1) {
    // 位于索引的 index 移动到 0
    m_windows.move(index, 0);
  }
}

void TabWindowManager::requestCloseTab(int index) {
  // 每一个新增的 TabWindow  tabClose 都被关联到该信号
  auto window = qobject_cast<TabWindow *>(sender());
  // 关闭页面的 TabWindow, 关闭页的标签
  emit tabCloseRequested(window->widget(index), window);
}

namespace Ui {

QMap<TtProtocolRole::Role, QString> TabWindow::type_icon_map_ = {
    {TtProtocolRole::Serial, ":/sys/unlink.svg"},
    {TtProtocolRole::TcpClient, ":/sys/netport.svg"},
    {TtProtocolRole::TcpServer, ":/sys/netport.svg"},
    {TtProtocolRole::UdpClient, ":/sys/netport.svg"},
    {TtProtocolRole::UdpServer, ":/sys/netport.svg"},
    {TtProtocolRole::MqttClient, ":/sys/mqtt.svg"},
    {TtProtocolRole::MqttBroker, ":/sys/mqtt.svg"},
    {TtProtocolRole::ModbusClient, ":/sys/modbus-seeklogo.svg"},
    {TtProtocolRole::ModbusServer, ":/sys/modbus-seeklogo.svg"},
    {TtProtocolRole::BlueTeeth, ":/sys/bluetooth-contact.svg"},
};

QHash<TtProtocolRole::Role, TabWindow::WidgetFactory>
    TabWindow::widgetFactories; // Widget 工厂函数映射

QMap<QString, QWidget *> TabWindow::widgetInstances; // Widget 实例

QHash<TtProtocolRole::Role, QString> TabWindow::widgetTitles; // Widget 标题映射

QString TabWindow::SpecialTypeIcon(TtProtocolRole::Role role) {
  return type_icon_map_[role];
}

TabWindow::TabWindow(QWidget *parent) : QTabWidget(parent) {
  setAttribute(Qt::WA_DeleteOnClose);

  qDebug() << "New TabWindow created:" << this;

  // Q_ASSERT(TabWindowManager::instance());
  // 构造函数添加 this
  TabWindowManager::instance()->addWindow(this);
  tabBar()->installEventFilter(this);

  setMovable(true);
  setDocumentMode(true);
  setFocusPolicy(Qt::NoFocus);

  setupCornerButton();

  QString tabBarStyle = R"(
    QTabBar::tab {
        background: transparent;
        padding: 6px 12px;
        border-top: 1px solid #ccc;
        border-left: 1px solid #ccc;
        border-right: none;
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
    }
    QTabBar::tab:selected {
        background: #3C3C3C;
    }
    QTabBar::tab:hover {
        background: #505050;
    }

    /* 关键：确保角落区域样式一致 */
    QTabWidget::corner {
        background-color: transparent;
        border: none;
    }
  )";

  // 将角落区域样式与现有样式合并
  QString currentStyle = this->styleSheet();
  // qDebug() << "currentStyle" << currentStyle;
  this->setStyleSheet(currentStyle + tabBarStyle);

  resize(800, 600);
}

TabWindow::~TabWindow() {
  qDebug() << "析构函数" << this;
  TabWindowManager::instance()->removeWindow(this);
}

bool TabWindow::eventFilter(QObject *object, QEvent *event) {
  if (object != tabBar())
    return QTabWidget::eventFilter(object, event);

  switch (event->type()) {
  case QEvent::MouseMove: {
    if (m_ignoreMouseEvent)
      return true;

    auto mouseEvent = static_cast<QMouseEvent *>(event);
    auto sendFake = [&](QObject *recv, QEvent::Type t) {
      QMouseEvent e(t, mouseEvent->pos(), Qt::LeftButton, mouseEvent->buttons(),
                    mouseEvent->modifiers());
      QCoreApplication::sendEvent(recv, &e);
    };

    if (!m_isMoving) {
      QRect r = tabBar()->rect();
      bool detach = (tabBar()->count() > 1 &&
                     distance(r, mouseEvent->pos()) > DISTANCE_TO_DETACH);
      if (detach) {
        sendFake(object, QEvent::MouseButtonRelease);
        m_isMoving = true;
        int idx = currentIndex();
        auto tabRect = tabBar()->tabRect(idx);
        m_mouseDelta = tabRect.center() - tabRect.topLeft() +
                       (geometry().topLeft() - pos());

        if (tabBar()->count() >= 2) {
          // 样式表问题
          // 样式表失效
          // 添加的新窗口
          // 不能指定父窗口，否则无法 show 弹出
          m_movingWindow = new TabWindow;
          m_movingWindow->setAttribute(Qt::WA_DeleteOnClose);
          // 获取当前的 widget2
          QWidget *w = widget(idx);
          QIcon icon = tabIcon(idx);
          QString txt = tabText(idx);
          m_movingWindow->setGeometry(rect());
          // qDebug() << "new TabWindow: " << m_movingWindow;
          // 为什么要移动 - 可能 show 之后在左上角, 出现闪屏幕
          // m_movingWindow->move(QCursor::pos() + QPoint(100, 100));
          m_movingWindow->move(QCursor::pos());
          m_movingWindow->show();
          // 添加新的标签页
          // m_movingWindow->addTab(w, icon, txt);
          m_movingWindow->insertTab(0, w, icon, txt);
          m_movingWindow->setupCustomTabButton(0);
        } else {
          m_movingWindow = this;
        }
        return true;
      }
    } else if (m_movingWindow) {
      QPoint gpos = QCursor::pos();
      //
      auto target =
          TabWindowManager::instance()->possibleWindow(m_movingWindow, gpos);
      if (target) {
        // 合并逻辑（支持单标签重合并）
        if (m_movingWindow->count() == 1) {
          // 只能够处于 moving 的才有效果
          // qDebug() << "只有一个标签的 window";
          QWidget *w = m_movingWindow->widget(0);
          QIcon icon = m_movingWindow->tabIcon(0);
          QString txt = m_movingWindow->tabText(0);
          QPoint posIn = target->tabBar()->mapFromGlobal(gpos);
          int insertIdx = target->tabBar()->tabAt(posIn);
          if (insertIdx < 0) {
            insertIdx = target->count();
          }
          // 目标添加标签页
          target->insertTab(insertIdx, w, icon, txt);
          target->setCurrentIndex(insertIdx);
          target->setupCustomTabButton(insertIdx);
          qDebug() << "TabWindow 删除";
          m_movingWindow->deleteLater();
        }
        m_movingWindow = nullptr;
        m_isMoving = false;
        m_ignoreMouseEvent = true;
        QMouseEvent fakePress(QEvent::MouseButtonPress,
                              target->tabBar()->mapFromGlobal(gpos),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(target->tabBar(), &fakePress);
        return true;
      } else {
        // 普通拖动
        QPoint newPos = gpos - m_mouseDelta;
        m_movingWindow->move(newPos);
        return true;
      }
    }
    break;
  }
  case QEvent::MouseButtonRelease:
    m_isMoving = false;
    m_movingWindow = nullptr;
    m_ignoreMouseEvent = false;
    break;
  case QEvent::MouseButtonPress:
    m_ignoreMouseEvent = false;
    break;
  default:
    break;
  }
  return QTabWidget::eventFilter(object, event);
}

void TabWindow::closeEvent(QCloseEvent *event) {
  // qDebug() << "close";
  TabWindow *root = TabWindowManager::instance()->rootWindow();
  // qDebug() << "root: " << root;
  if (root == this) {
    // qDebug() << "[ROOT] Closing root window:" << this;

    // 获取所有子窗口的拷贝
    const auto children = TabWindowManager::instance()->windows();
    for (TabWindow *w : children) {
      if (w == this) {
        continue;
      }
      // qDebug() << "[ROOT] Deleting child window:" << w;
      w->setAttribute(Qt::WA_DeleteOnClose, true);
      w->deleteLater(); // 强制异步删除
    }

    event->accept();
    deleteLater();
    return;
  } else {
    // 删除的是 非 root 窗口
    TabWindow *dest = TabWindowManager::instance()->rootWindow();
    if (dest && dest != this) {
      int countTabs = count();
      for (int i = 0; i < countTabs; ++i) {
        QWidget *w = widget(0);
        QIcon ic = tabIcon(0);
        QString txt = tabText(0);
        removeTab(0);
        // dest->addTab(w, ic, txt);
        dest->insertTab(dest->count(), w, ic, txt);
        dest->setupCustomTabButton(dest->count() - 1);
      }
      // 全部的窗口删除了
      event->accept();
      deleteLater();
      return;
    }
  }
  QTabWidget::closeEvent(event);
}

void TabWindow::addNewTab(const QString &title) {
  // 总数量
  int tabIndex = count();
  QWidget *defaultWidget = createDefaultWidget(tabIndex);
  // addTab(defaultWidget, "Tab " + QString::number(tabIndex + 1));
  addTab(defaultWidget, title);
  // setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
}

void TabWindow::addNewTab(QWidget *defaultWidget) {
  // int tabIndex = count();
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  // 改变颜色
  QIcon icon(":/sys/unlink.svg");
  addTab(defaultWidget, icon, tr("新增连接"));
  // 设置对应的 button 样式
  setupCustomTabButton(count() - 1);
}

void TabWindow::addNewTab(QWidget *defaultWidget, const QString &title) {
  // 创建默认的 widget1
  int tabIndex = count();
  // QWidget* defaultWidget = createDefaultWidget(tabIndex);
  // 添加的 tab 是一直在最后面的
  QIcon icon(":/sys/unlink.svg");
  addTab(defaultWidget, icon, title);
  // 对应的 index
  setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
}

void TabWindow::addNewTab(QWidget *defaultWidget, const QIcon &icon,
                          const QString &title) {
  int tabIndex = count();
  addTab(defaultWidget, icon, title);
  setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
}

void TabWindow::registerWidget(TtProtocolRole::Role role,
                               const WidgetFactory &factory,
                               const QString &title) {
  // widgetid 注册的窗口标识符 [2]
  widgetFactories[role] = factory;
  widgetTitles[role] = title;
}

void TabWindow::switchByCreateWidget(int tabIndex, TtProtocolRole::Role role) {
  // 根据 index 去索引
  if (tabIndex < 0 || tabIndex > count()) {
    qDebug() << tabIndex;
    qDebug() << "无效";
    return; // 无效的 Tab 索引
  }

  if (!widgetFactories.contains(role)) {
    qDebug() << "no register";
    return; // 未注册的 Widget ID
  }

  // 销毁当前 Widget
  // 实现原地操作
  // 根据 index 去索引 widget, 目标切换窗口!!! 点击的时候对应的 index
  QWidget *currentWidget = widget(tabIndex);
  if (currentWidget) {
    currentWidget->deleteLater();
  }

  // 创建新的 Widget
  QWidget *newWidget = widgetFactories[role]();

  // 创建 uuid
  widgetInstances[QUuid::createUuid().toString()] = newWidget;
  // 能够找到对应的 uuid, 创建存在 qDebug() << "widget: " << newWidget << "uuid:
  // " << findWidget(newWidget);

  // setTabText(tabIndex, widgetTitles[role]);
  // insertTab(tabIndex, newWidget, QIcon(SpecialTypeIcon(role)),
  //           widgetTitles[role]);
  // 这个地方没有获取到正确的 title, 本地的却能获取到正确的
  qDebug() << widgetTitles[role];
  // 没有标题
  insertTab(tabIndex, newWidget, QIcon(SpecialTypeIcon(role)),
            widgetTitles[role]);
  setupCustomTabButton(tabIndex);
  setCurrentIndex(tabIndex);
}

void TabWindow::switchByAlreadyExistingWidget(int tabIndex, const QString &uuid,
                                              const QJsonObject &config,
                                              TtProtocolRole::Role role) {
  if (tabIndex < 0 || tabIndex > count()) {
    qDebug() << tabIndex;
    qDebug() << "无效";
    return; // 无效的 Tab 索引
  }

  if (!widgetFactories.contains(role)) {
    qDebug() << "no register";
    return; // 未注册的 Widget ID
  }

  // 根据 index 去索引 widget, 目标切换窗口!!! 点击的时候对应的 index
  QWidget *currentWidget = widget(tabIndex);
  if (currentWidget) {
    currentWidget->deleteLater();
  }

  // qDebug() << "new by has uuid";

  // 这里需要设置对应的 config 配置
  Window::FrameWindow *newWidget = widgetFactories[role]();
  // 从 disk 恢复, 处于保存状态
  newWidget->setSaveState(true);

  if (newWidget) {
    // 设置
    // 必须异步调用
    newWidget->setSetting(config);

    // 可以根据已经有的 uuid 设定
    widgetInstances[uuid] = newWidget;

    insertTab(tabIndex, newWidget, QIcon(SpecialTypeIcon(role)),
              config.value("WindowTitle").toString(widgetTitles[role]));

    setupCustomTabButton(tabIndex);
    setCurrentIndex(tabIndex);
  } else {
    qDebug() << "error to create widget";
  }
}

QString TabWindow::getCurrentWidgetUUid(QWidget *widget) {
  return findWidget(widget);
}

QString TabWindow::getCurrentWidgetUUid() {
  // 获取当前 tabWindow 的 uuid
  // 不再当前的 uuid
  // qDebug() << "currWidget: " << currentWidget();
  // 当前的 widget 不同, 获取有问题
  // 移动出去后,  widget 不是当前的 widget, 应该获取点击的 tabWindow 的
  // currentWidget

  return findWidget(currentWidget());
}

void TabWindow::setTabTitle(const QString &title) {

  tabBar()->setTabText(currentIndex(), title);
}

void TabWindow::setTabTitle(const QString &index, const QString &title) {
  auto *findWidget = widgetInstances.value(index);
  if (!findWidget) {
    // 不在实例中
    return;
  }
  TabWindow *foundTabWindow = nullptr;
  for (auto *tabWindow : TabWindowManager::instance()->windows()) {
    int count = tabWindow->count();
    for (int i = 0; i < count; ++i) {
      QWidget *page = tabWindow->widget(i);
      if (page == findWidget) {
        qDebug() << "find";
        foundTabWindow = tabWindow;
        tabWindow->setTabText(i, title);
        break;
      }
    }
    if (foundTabWindow) {
      break;
    }
  }
}

bool TabWindow::isStoredInMem(const QString &index) {
  for (auto it = closedTabs_.constBegin(); it != closedTabs_.end(); ++it) {
    if (it->first == index) {
      return true;
    }
  }
  return false;
}

bool TabWindow::isCurrentDisplayedPage(const QString &index) {
  return widgetInstances.contains(index);
}

void TabWindow::switchByPage(const QString &index) {
  // 移动出现后, 没办法切换
  // 根据 uuid 窗口所在的 TabWindow, 切换上升或下降
  auto *findWidget = widgetInstances.value(index);
  if (!findWidget) {
    return;
  }
  // qDebug() << "findWidget: " << findWidget;
  TabWindow *foundTabWindow = nullptr;
  for (auto *tabWindow : TabWindowManager::instance()->windows()) {
    int count = tabWindow->count();
    // 遍历所有标签页
    for (int i = 0; i < count; i++) {
      // 获取标签页指针
      QWidget *page = tabWindow->widget(i);
      if (page == findWidget) {
        qDebug() << "find";
        foundTabWindow = tabWindow;
        tabWindow->setCurrentWidget(page);
        break;
      }
    }
    if (foundTabWindow) {
      break;
    }
  }

  if (foundTabWindow) {
    for (auto *tabWindow : TabWindowManager::instance()->windows()) {
      if (tabWindow != foundTabWindow) {
        tabWindow->lower();
      }
    }

    foundTabWindow->raise();
  } else {
    // qDebug() << "没有 tabwindow";
  }
}

void TabWindow::switchByReadingMem(const QString &index,
                                   TtProtocolRole::Role role) {
  qDebug() << "reading in mem";
  for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
    qDebug() << it->first;
    if (it->first == index) {
      restoreClosedTabFromMem(index, it->second);
      closedTabs_.erase(it);
      break;
    }
  }
}

void TabWindow::switchByReadingDisk(const QString &index,
                                    TtProtocolRole::Role role,
                                    const QJsonObject &config) {
  // 初始时不存在, 根据 已有的 uuid 创建特定表示窗口
  // qDebug() << "create by disk";
  // qDebug() << "switch uuid: " << index;

  // qDebug() << "TEST1";
  // qDebug() << currentIndex() + 1;
  // 当前索引加 1?
  // switchByAlreadyExistingWidget(currentIndex() + 1, index, config, role);
  // 当前的数量加 1
  // switchByAlreadyExistingWidget(count() + 1, index, config, role);
  // 当前 count() 作为索引
  switchByAlreadyExistingWidget(count(), index, config, role);
  // qDebug() << "TEST2";
}

void TabWindow::sessionSwitchPage(int tabIndex, TtProtocolRole::Role role) {
  // 进入的是这个
  // 根据鼠标位置, 获取当前的 tabWindow
  // 切换处于后, 将会导致 tabIndex 不匹配
  qDebug() << "switch index: " << tabIndex;

  // 索引对应的 tabWidget_ ???
  // switchToWidget(tabIndex, role);  // widgetId = 2 是 widget2
  switchByCreateWidget(tabIndex, role); // widgetId = 2 是 widget2
}

void TabWindow::handleAddNewTab() { addNewTab("新建立"); }

void TabWindow::handleTabCloseRequested(int index) {
  qDebug() << "handle delete index: " << index;
  // 关闭按钮的 close, 要从 widgetInstance 中删除掉
  QWidget *widget = this->widget(index);

  // 功能widget实例
  const QString uuid = findWidget(widget);
  if (widgetInstances.contains(uuid)) {
    widgetInstances.remove(uuid);
  }

  // 有问题, remove, 导致 多一个
  removeTab(index);

  // widgetInstances.removeIf();
  // widgetInstances.contains(findWidget(widget))
  // 彻底删除
  if (widget) {
    // 原有的窗口调用了析构函数
    // qDebug() << "delete";
    // 似乎没有删除 function select widget
    emit widgetDeleted(widget);
    widget->disconnect();
    widget->setParent(nullptr);
    delete widget;
    widget = nullptr;
  }
}

void TabWindow::removeUuidWidget(const QString &index) {
  // 关闭对应的标签页, bug
  // 实例中获取对应的 id
  qDebug() << "tabWindow get remove:" << index;
  if (widgetInstances.contains(index)) {
    // 实例全部都有, 但不是都存储在同一个 tabWindow 中
    // 根据 uuid 获取对应的 tabWindow
    // findWidget();
    // 获取 QTabWindow

    auto *item = widgetInstances.value(index);

    // 每一个 window 都有不同
    auto i = indexOf(item);

    // 调用的是 tabWidget_ 的查找 widget, 跨窗口无用
    auto *widget = QTabWidget::widget(i);
    if (widget) {
      qDebug() << "删除标签页";
      widget->disconnect();
      widget->setParent(nullptr);
      widget->deleteLater();
    }
    qDebug() << "remove by leftbutton: " << index;
    widgetInstances.remove(index);
    removeTab(i);
  }
}

void TabWindow::setupCustomTabButton(int index) {
  auto *closeButton = new TabCloseButton(this);
  tabBar()->setTabButton(index, QTabBar::RightSide, closeButton);

  // 使用按钮找到当前索引，而不是使用固定索引
  connect(closeButton, &QToolButton::clicked, this, [this, closeButton]() {
    int currentIndex = getTabIndexFromButton(closeButton);
    if (currentIndex != -1) {
      // qDebug() << "delete index: " << currentIndex;
      handleTabClose(currentIndex);
    }
  });
}

void TabWindow::setupCornerButton() {
  add_button_ = new QToolButton(this);
  add_button_->setIcon(QIcon(":/sys/plus-circle.svg"));
  add_button_->setToolTip(tr("New Tab"));
  add_button_->setStyleSheet(R"(
        QToolButton {
            border: none;
            padding: 2px;
            background: transparent;
        }
        QToolButton:hover {
            background: rgba(0, 0, 0, 0.1);
        }
        QToolButton:pressed {
            background: rgba(0, 0, 0, 0.15);
        }
    )");

  setCornerWidget(add_button_, Qt::TopRightCorner);

  // 对应肯定发出了信号, 但是没有人链接
  // 衍生的 tabWindow 没有, 信号没有给到正确的 tabWindow
  // 如果本窗口创建, 然后移动了出去, 对应的 tabWindow 会生成新的一个
  connect(add_button_, &QPushButton::clicked, this, &TabWindow::requestNewTab);
}

int TabWindow::getTabIndexFromButton(QWidget *button) const {
  QTabBar *bar = tabBar();
  for (int i = 0; i < bar->count(); ++i) {
    if (bar->tabButton(i, QTabBar::RightSide) == button) {
      return i;
    }
  }
  return -1;
}

QWidget *TabWindow::createDefaultWidget(int tabIndex) {
  // 新建 widget
  QWidget *widget = new QWidget(this);
  return widget;
}

QString TabWindow::findWidget(QWidget *widget) {
  // 全部实例保存到 widgetInstance 中, 每个类都有一个 widgetInstaces 中
  for (auto it = widgetInstances.cbegin(); it != widgetInstances.cend(); ++it) {
    if (it.value() == widget) {
      // error
      qDebug() << "处于实例组中";
      return it.key();
    }
  }
  return QString();
}

void TabWindow::handleTabClose(int index) {
  if (widget(index)->objectName() == "SettingWidget") {
    handleTabCloseRequested(index);
    return;
  }
  // 先判断是否保存, 再判断是否 当前的状态
  // 先判断当前的工作状态, 处于开始工作状态, 以及无法改变了
  Window::FrameWindow *w = qobject_cast<Window::FrameWindow *>(widget(index));
  if (w != nullptr) {
    if (!w->saveState()) {
      qDebug() << "no saved";

      // Ui::TtContentDialog *dialog = new Ui::TtContentDialog(this);
      // dialog->setLeftButtonText(tr("取消"));
      // dialog->setRightButtonText(tr("确定"));
      // dialog->setCenterText(tr("通讯链接配置已修改, 是否保存"));

      // connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this, [&]() {
      //   dialog->reject();
      //   qDebug() << "取消保存";
      //   if (w->workState()) {
      //     // 保存
      //     saveWorkingTabPageToMem(index);
      //   } else {
      //     // 删除
      //     handleTabCloseRequested(index);
      //   }
      // });
      // connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this, [&]() {
      //   dialog->accept();
      //   qDebug() << "保存最新变动";
      //   w->saveSetting();
      //   // 主动保存能成功
      //   if (w->workState()) {
      //     qDebug() << "save to mem";
      //     saveWorkingTabPageToMem(index);
      //   } else {
      //     qDebug() << "close";
      //     handleTabCloseRequested(index);
      //   }
      // });
      // dialog->exec();
      // delete dialog;

      // 非模态对话框
      Ui::TtContentDialog *dialog = new Ui::TtContentDialog(
          Qt::ApplicationModal, true,
          Ui::TtContentDialog::LayoutSelection::TWO_OPTIONS, this);
      dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动销毁
      dialog->setLeftButtonText(tr("取消"));
      dialog->setRightButtonText(tr("确定"));
      dialog->setCenterText(tr("通讯链接配置已修改, 是否保存"));
      connect(dialog, &Ui::TtContentDialog::leftButtonClicked, dialog,
              &QDialog::reject);
      connect(dialog, &Ui::TtContentDialog::rightButtonClicked, dialog,
              &QDialog::accept);
      const int result = dialog->exec();
      if (result == QDialog::Accepted) {
        // right 摁下
        qDebug() << "保存最新变动";
        w->saveSetting();
        // 主动保存能成功
        if (w->workState()) {
          qDebug() << "save to mem";
          saveWorkingTabPageToMem(index);
        } else {
          qDebug() << "close";
          handleTabCloseRequested(index);
        }
      } else {
        // left 摁下
        if (w->workState()) {
          // 保存
          saveWorkingTabPageToMem(index);
        } else {
          // 删除
          handleTabCloseRequested(index);
        }
      }

      // // 非模态对话框
      // Ui::TtContentDialog *dialog = new Ui::TtContentDialog(
      //     Qt::NonModal, true,
      //     Ui::TtContentDialog::LayoutSelection::TWO_OPTIONS, this);
      // dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动销毁
      // dialog->setLeftButtonText(tr("取消"));
      // dialog->setRightButtonText(tr("确定"));
      // dialog->setCenterText(tr("通讯链接配置已修改, 是否保存"));
      // // 连接按钮信号到业务逻辑
      // connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this, [=] {
      //   if (w->workState()) {
      //     // 保存
      //     saveWorkingTabPageToMem(index);
      //   } else {
      //     // 删除
      //     handleTabCloseRequested(index);
      //   }
      //   qDebug() << "操作已取消";
      //   dialog->close(); // 非必须，WA_DeleteOnClose会触发销毁
      // });

      // connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this, [=] {
      //   qDebug() << "保存最新变动";
      //   w->saveSetting();
      //   // 主动保存能成功
      //   if (w->workState()) {
      //     qDebug() << "save to mem";
      //     saveWorkingTabPageToMem(index);
      //   } else {
      //     qDebug() << "close";
      //     handleTabCloseRequested(index);
      //   }
      //   dialog->close();
      // });

      // dialog->show();
    } else {
      // 直接处于保存状态
      qDebug() << "save";
      if (w->workState()) {
        saveWorkingTabPageToMem(index);
      } else {
        // 保存到本地
        saveTabPageToDisk(index);
        handleTabCloseRequested(index);
      }
    }
  } else {
    // 选择窗口, 直接 delete
    // function Select Widget 是否删除
    handleTabCloseRequested(index);
    qDebug() << "nullptr";
  }
  // }

  /*
  TabData info;
  info.title = tabText(index);
  info.widget = widget(index);  // 存储 widget 界面
  info.icon = tabIcon(index);

  // // 保存widget状态
  // if (auto* serializable = dynamic_cast<ISerializable*>(info.widget)) {
  //   info.state = serializable->saveState();
  // }

  // 获取 widget 实例的 uuid
  // 删除了 uuid 对应的 widget, 还有什么用呢 uuid ?

  QString removeWidgetUuid = findWidget(widget(index));
  // closedTabs_.prepend(qMakePair(findWidget(widget(index)), info));
  closedTabs_.prepend(qMakePair((removeWidgetUuid), info));
  removeTab(index);
  */

  // 移出实例
  // widgetInstances.remove(removeWidgetUuid);

  // --------

  // 过多则本地化一部分
  // // 限制历史数量
  // while (closedTabs_.size() > maxClosedTabs_) {
  //   TabData& lastInfo = closedTabs_.last();
  //   delete lastInfo.widget;  // 删除过旧的widget
  //   closedTabs_.removeLast();
  // }

  // qDebug() << "close index: " << index;
  // qDebug() << info.title;
  // qDebug() << info.widget;
  // qDebug() << info.icon;

  // emit tabClosed(index);
}

void TabWindow::saveWorkingTabPageToMem(int index) {
  TabData info;
  info.title = tabText(index);
  info.widget = widget(index); // 存储 widget 界面
  info.icon = tabIcon(index);

  // 获取 uuid
  QString removeWidgetUuid = findWidget(widget(index));
  qDebug() << "remove uuid" << removeWidgetUuid;
  widgetInstances.remove(removeWidgetUuid);
  closedTabs_.prepend(qMakePair((removeWidgetUuid), info));
  removeTab(index);
  qDebug() << "保存到 mem";
}

void TabWindow::restoreClosedTabFromMem(const QString &index,
                                        struct TabData data) {
  qDebug() << "restore mem";
  // 那么 实例中也许要添加对应的信息, 不然没办法切换
  widgetInstances[index] = data.widget;
  // icon 消失
  addNewTab(data.widget, data.title);
  // 切换到复原的 tab
  setCurrentIndex(count() - 1);

  // 恢复状态
  // if (!info.state.isEmpty()) {
  //   if (auto* serializable = dynamic_cast<ISerializable*>(info.widget)) {
  //     serializable->restoreState(info.state);
  //   }
  // }
  // 重新构造 widget
  // int index = addTab(info.widget, info.icon, info.title);
  // setCurrentIndex(index);
}

void TabWindow::saveTabPageToDisk(int index) { qDebug() << "保存到 disk"; }

void TabWindow::restoreClosedTabFromDisk() {}

QStringList TabWindow::getClosedTabsList() const {
  QStringList list;
  // for (const TabInfo& info : closedTabs_) {
  //   list << info.title;
  // }
  return list;
}

QJsonObject TabWindow::serializeTab(int index) const {
  QJsonObject tabObj;
  tabObj["title"] = tabText(index);
  tabObj["icon"] = tabIcon(index).name();

  // 如果widget支持序列化
  // QWidget *w = widget(index);
  // if (auto *serializable = dynamic_cast<ISerializable *>(w)) {
  //   tabObj["state"] = QString(serializable->saveState().toBase64());
  // }

  return tabObj;
}

void TabWindow::deserializeTab(const QJsonObject &obj) {
  QString title = obj["title"].toString();
  QString iconName = obj["icon"].toString();

  QWidget *page = new QWidget(this);
  // 恢复widget状态
  // if (obj.contains("state")) {
  //   QByteArray state =
  //       QByteArray::fromBase64(obj["state"].toString().toLatin1());
  //   if (auto *serializable = dynamic_cast<ISerializable *>(page)) {
  //     serializable->restoreState(state);
  //   }
  // }

  addTab(page, QIcon(iconName), title);
}

} // namespace Ui
