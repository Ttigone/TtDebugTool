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

TabWindow *TabWindowManager::findTabWindowWithClosedTab(const QString &uuid) {
  for (TabWindow *window : m_windows) {
    for (const auto &pair : window->closedTabs_) {
      if (pair.first == uuid) {
        return window;
      }
    }
  }
  return nullptr;
}

void TabWindowManager::removeUuidTabPage(const QString &index) {
  if (TabWindow::widgetInstances.contains(index)) {
    auto *item = TabWindow::widgetInstances.value(index);
    // 在处理前先阻断信号
    if (item) {
      item->blockSignals(true);
    }

    int idx = 0;
    for (auto &w : m_windows) {
      if ((idx = w->indexOf(item)) != -1) {
        // qDebug() << "成功找到";
        auto *widget = w->widget(idx);
        // 找到能够删除
        // qDebug() << "remove by leftbutton: " << index;
        TabWindow::widgetInstances.remove(index);
        w->removeTab(idx);
        if (widget) {
          // qDebug() << "删除标签页";
          widget->disconnect();
          widget->setParent(nullptr);
          widget->deleteLater();
        }
        if (w->count() == 0) {
          qDebug() << "TabWindow is empty, removing it from manager.";
          removeWindow(w);
        }
        break;
      }
    }
  }
}

void TabWindowManager::addWindow(TabWindow *window) {
  // main_widow 设置了 root, 已经添加过了
  // BUG, 创建之后,
  qDebug() << "[Manager] addWindow: " << window;
  if (m_windows.contains(window)) {
    return;
  }
  m_windows.append(window);

  // disconnect(window, &TabWindow::requestNewTab, this, nullptr);
  // 删除窗口
  connect(window, &TabWindow::tabCloseRequested, this,
          &TabWindowManager::requestCloseTab);
  connect(window, &TabWindow::requestNewTab, this,
          [this, window]() { emit tabCreateRequested(window); });
}

void TabWindowManager::removeWindow(TabWindow *window) {
  if (!window || !m_windows.contains(window) || window == m_root) {
    return;
  }

  qDebug() << "[Manager] Removing window:" << window;

  // 检查所有TabWindow实例的m_movingWindow是否指向此窗口
  for (TabWindow *w : m_windows) {
    if (w->m_movingWindow == window) {
      // 重置指向即将被删除窗口的指针
      w->m_movingWindow = nullptr;
      w->m_isMoving = false;
    }
  }

  // 确保窗口已经完全清空
  if (window->count() > 0) {
    qWarning() << "尝试删除仍有标签页的窗口！转移标签页...";

    // 获取root窗口作为目标
    TabWindow *root = rootWindow();
    if (root && root != window) {
      // 转移所有标签页到root窗口
      while (window->count() > 0) {
        QWidget *w = window->widget(0);
        QIcon icon = window->tabIcon(0);
        QString txt = window->tabText(0);

        // 从源窗口移除但不删除部件
        window->removeTab(0);

        // 添加到root窗口
        int newIdx = root->addTab(w, icon, txt);
        root->setupCustomTabButton(newIdx);
      }
    } else {
      // 如果无法转移，只能删除标签页
      while (window->count() > 0) {
        window->removeTab(0);
      }
    }
  }

  // 安全移除窗口
  m_windows.removeOne(window);

  // 确保窗口不可见
  window->hide();

  // 断开所有连接
  window->disconnect();

  // // 确保窗口已经完全清空
  // if (window->count() > 0) {
  //   qWarning() << "尝试删除仍有标签页的窗口！正在清空标签页...";
  //   while (window->count() > 0) {
  //     window->removeTab(0);
  //   }
  // }
  // // 安全移除窗口
  // m_windows.removeOne(window);
  // window->hide();
}

TabWindow *TabWindowManager::possibleWindow(TabWindow *src, QPoint globalPos) {
  // for (TabWindow* w : m_windows) {
  for (auto &w : m_windows) {
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
  // qDebug() << "active widgetl";
  // 切换窗口的时候, 会出现问题, 重新显示也会出现问题
  if (!window) {
    return;
  }
  // root 窗口
  TabWindow *rootWindow = m_root;

  TabWindow *targetWindow = nullptr;
  int targetIndex = -1;
  for (int i = 0; i < m_windows.count(); ++i) {
    TabWindow *tabWindow = m_windows.at(i);
    if (tabWindow->windowHandle() == window) {
      targetWindow = tabWindow;
      targetIndex = i;
      break;
    }
  }

  if (!targetWindow) {
    // qDebug() << "未找到目标窗口";
    return;
  }
  // 确保窗口可见
  if (!targetWindow->isVisible()) {
    targetWindow->show();
  }

  // 处理 Z 顺序，将激活窗口提到最前
  // for (TabWindow* w : m_windows) {
  for (auto &w : m_windows) {
    if (w != targetWindow) {
      w->lower();
    }
  }
  // 提升窗口到最前
  targetWindow->raise();
  targetWindow->setFocus();

  // 如果不是 root 窗口，并且当前不在最前（索引不为 0 且非 root）
  // 我们将窗口移到列表首位，但确保 root 窗口位置不变
  if (targetWindow != rootWindow && targetIndex > 0) {
    // 保存 root 窗口位置
    int rootIndex = m_windows.indexOf(rootWindow);

    // 将目标窗口移到首位
    m_windows.move(targetIndex, 0);

    // 如果 root 窗口位置发生变化，需要调整保持 root 窗口的位置不变
    if (rootIndex == 0) {
      // 如果 root 窗口原来在首位，现在将目标窗口和 root 窗口交换位置
      m_windows.move(1, 0);
      m_windows.move(0, 1);
    }
  }

  // 通知应用其他部分窗口已被激活
  // emit windowActivated(targetWindow);
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
  // 但是对应的运行的窗口被移动到了 root 中
  // 执行了析构函数, 对应的所有权没有被转移
  qDebug() << "析构函数" << this;
  m_isMoving = false;
  m_movingWindow = nullptr;
  this->disconnect();
  // 这里会对 窗口部件执行析构函数, 那边如果显示对应的窗口, 会发生问题
  //
  // 确保没有残留的标签页
  while (count() > 0) {
    removeTab(0); // 只移除标签，不删除部件
  }

  // 确保不在TabWindowManager中
  if (TabWindowManager::instance()) {
    // 检查是否需要从管理器中移除
    if (TabWindowManager::instance()->windows().contains(this)) {
      TabWindowManager::instance()->removeWindow(this);
    }
  }

  // 断开所有连接
  // this->disconnect();
  //
  // TabWindowManager::instance()->removeWindow(this);
  // 析构函数被执行了, 显示当前 app 应用时(root) 在的窗口, 发现进程崩溃

  // 如果当前处于运行的窗口, 则需要保存
  // 一般关闭的只有 1 个
  // assert(this->count() <= 1);
  // if (this->count() == 1) {
  //   // 如果处于运行, 则可以保存到
  //   qDebug() << "this";
  // }

  // TabWindowManager::instance()->removeWindow(this);
  // TabWindow *dest = TabWindowManager::instance()->rootWindow();
  // if (dest && dest != this) {

  // }
}

bool TabWindow::eventFilter(QObject *object, QEvent *event) {
  if (object != tabBar()) {
    return QTabWidget::eventFilter(object, event);
  }

  switch (event->type()) {
  case QEvent::MouseMove: {
    if (m_ignoreMouseEvent) {
      return true;
    }
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    // 向某个对象发送鼠标左键的事件
    auto sendFake = [&](QObject *recv, QEvent::Type t) {
      QMouseEvent e(t, mouseEvent->pos(), Qt::LeftButton, mouseEvent->buttons(),
                    mouseEvent->modifiers());
      QCoreApplication::sendEvent(recv, &e);
    };
    if (!m_isMoving) {
      QRect r = tabBar()->rect();
      // 分离标志
      bool detach = (tabBar()->count() > 1 &&
                     distance(r, mouseEvent->pos()) > DISTANCE_TO_DETACH);
      if (detach) {
        // 发送左键鼠标释放事件
        sendFake(object, QEvent::MouseButtonRelease);
        m_isMoving = true;
        // 当前的索引号
        int idx = currentIndex();
        auto tabRect = tabBar()->tabRect(idx);
        m_mouseDelta = tabRect.center() - tabRect.topLeft() +
                       (geometry().topLeft() - pos());

        if (tabBar()->count() >= 2) {
          qDebug() << "生成新的窗口";
          m_movingWindow = new TabWindow;
          if (!m_movingWindow) {
            qDebug() << "Failed to create a new TabWindow";
            m_isMoving = false;
            return false;
          }
          // 执行 close 之后删除
          m_movingWindow->setAttribute(Qt::WA_DeleteOnClose);
          // 获取当前的 widget2
          QWidget *w = widget(idx);
          QIcon icon = tabIcon(idx);
          QString txt = tabText(idx);
          m_movingWindow->setGeometry(rect());
          m_movingWindow->move(QCursor::pos());
          m_movingWindow->show();
          // 实例仍然存在于 WidgetInstace 中
          m_movingWindow->insertTab(0, w, icon, txt);
          m_movingWindow->setupCustomTabButton(0);
          // 注册到管理器 - 让管理器开始跟踪这个窗口
          TabWindowManager::instance()->addWindow(m_movingWindow);
          qDebug() << "插入新的窗口完成";
        } else {
          m_movingWindow = this;
        }
        return true;
      }
    } else if (m_movingWindow) {
      // 不包含当前处于移动的窗口
      if (!TabWindowManager::instance()->windows().contains(m_movingWindow)) {
        qDebug() << "没有找到窗口";
        m_isMoving = false;
        m_movingWindow = nullptr;
        return false;
      }
      // 检查窗口
      QPoint gpos = QCursor::pos();
      auto target =
          TabWindowManager::instance()->possibleWindow(m_movingWindow, gpos);
      if (target) {
        // 合并逻辑（支持单标签重合并）
        if (m_movingWindow && m_movingWindow->count() == 1) {
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
          m_movingWindow->removeTab(0);
          // m_movingWindow.removeTab(0);
          // 目标添加标签页
          target->insertTab(insertIdx, w, icon, txt);
          target->setCurrentIndex(insertIdx);
          target->setupCustomTabButton(insertIdx);
          qDebug() << "TabWindow 删除";

          // 安全删除移动窗口
          QPointer<TabWindow> windowToDelete = m_movingWindow;
          m_movingWindow = nullptr; // 立即重置指针避免悬空

          // 使用计时器延迟删除，确保事件循环完成当前处理
          QTimer::singleShot(0, [windowToDelete]() {
            if (windowToDelete) {
              windowToDelete->deleteLater();
            }
          });
        }
        m_isMoving = false;
        m_ignoreMouseEvent = true;
        // m_movingWindow = nullptr;
        QMouseEvent fakePress(QEvent::MouseButtonPress,
                              target->tabBar()->mapFromGlobal(gpos),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(target->tabBar(), &fakePress);
        return true;
      } else if (m_movingWindow) {
        // 普通拖动
        // QPoint newPos = gpos - m_mouseDelta;
        // m_movingWindow->move(newPos);
        // return true;
        // 安全检查
        if (TabWindowManager::instance()->windows().contains(m_movingWindow)) {
          // 普通拖动
          QPoint newPos = gpos - m_mouseDelta;
          m_movingWindow->move(newPos);
        }
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
  // qDebug() << "close Event";
  TabWindow *root = TabWindowManager::instance()->rootWindow();

  if (root == this) {
    qDebug() << "[ROOT] Closing root window:" << this;
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
    qDebug() << "no root tabwindow";
    int countTabs = this->count();
    for (int i = 0; i < countTabs; ++i) {
      // this->widget()
      QWidget *w = widget(0);
      QIcon ic = tabIcon(0);
      QString txt = tabText(0);

      removeTab(0);
      // 成功插入到了对应的窗口中, 但是显示时, 会出现委托你
      // int newIndex = root->addTab(w, icon, text);
      // root->setupCustomTabButton(newIndex);
      root->insertTab(root->count(), w, ic, txt);
      root->setupCustomTabButton(root->count() - 1);
    }
    event->accept();
    // 调用析构函数
    deleteLater();
    qDebug() << "计划调用析构函数";
    return;
  }
  QTabWidget::closeEvent(event);
}

void TabWindow::addNewTab(const QString &title) {
  // 总数量
  int tabIndex = count();
  QWidget *defaultWidget = createDefaultWidget(tabIndex);
  addTab(defaultWidget, title);
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
  // 删除 functionSelection 窗口
  QWidget *currentWidget = widget(tabIndex);
  if (currentWidget) {
    currentWidget->deleteLater();
  }

  Window::FrameWindow *newWidget = widgetFactories[role]();
  // 从 disk 恢复, 处于保存状态
  // BUG, 为什么有的窗口初始创建时, 没有对应的状态
  newWidget->setSaveState(true);
  widgetInstances[QUuid::createUuid().toString()] = newWidget;
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
  // for (auto *tabWindow : TabWindowManager::instance()->windows()) {
  for (auto &tabWindow : TabWindowManager::instance()->windows()) {
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
  // 处于实例
  // 移动出现后, 没办法切换
  // 根据 uuid 窗口所在的 TabWindow, 切换上升或下降
  auto *findWidget = widgetInstances.value(index);
  if (!findWidget) {
    return;
  }
  // qDebug() << "findWidget: " << findWidget;
  TabWindow *foundTabWindow = nullptr;
  // for (auto *tabWindow : TabWindowManager::instance()->windows()) {
  for (auto &tabWindow : TabWindowManager::instance()->windows()) {
    int count = tabWindow->count();
    // 遍历所有标签页
    for (int i = 0; i < count; i++) {
      // 获取标签页指针
      QWidget *page = tabWindow->widget(i);
      if (page == findWidget) {
        // 这里找到了
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
    // for (auto *tabWindow : TabWindowManager::instance()->windows()) {
    for (auto &tabWindow : TabWindowManager::instance()->windows()) {
      if (tabWindow != foundTabWindow) {
        tabWindow->lower();
      }
    }

    foundTabWindow->raise();
  } else {
    // qDebug() << "没有 tabwindow";
  }
  qDebug() << "sussess";
}

void TabWindow::switchByReadingMem(const QString &index,
                                   TtProtocolRole::Role role) {
  // qDebug() << "reading in mem";
  // for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
  //   qDebug() << it->first;
  //   if (it->first == index) {
  //     qDebug() << "get mem";
  //     restoreClosedTabFromMem(index, it->second);
  //     closedTabs_.erase(it);
  //     break;
  //   }
  // }

  // -------

  // qDebug() << "reading in mem";

  // // 首先在当前 TabWindow 中查找
  // for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
  //   if (it->first == index) {
  //     qDebug() << "found in current TabWindow";
  //     restoreClosedTabFromMem(index, it->second);
  //     closedTabs_.erase(it);
  //     this->raise();  // 确保当前窗口显示在前面
  //     return;
  //   }
  // }

  // // 如果当前 TabWindow 中没有找到，遍历所有 TabWindow 实例
  // for (auto* tabWindow : TabWindowManager::instance()->windows()) {
  //   if (tabWindow == this)
  //     continue;  // 跳过当前窗口，因为已经检查过了

  //   for (auto it = tabWindow->closedTabs_.begin();
  //        it != tabWindow->closedTabs_.end(); ++it) {
  //     if (it->first == index) {
  //       qDebug() << "found in other TabWindow:" << tabWindow;

  //       // 如果窗口隐藏了，先显示它
  //       if (!tabWindow->isVisible()) {
  //         tabWindow->show();
  //       }

  //       // 在找到的 TabWindow 中恢复标签页
  //       tabWindow->restoreClosedTabFromMem(index, it->second);
  //       tabWindow->closedTabs_.erase(it);

  //       // 将找到的窗口提到前面
  //       tabWindow->raise();
  //       tabWindow->activateWindow();
  //       return;
  //     }
  //   }
  // }

  // qDebug() << "UUID" << index << "not found in any TabWindow's closedTabs_";

  // -------

  qDebug() << "reading in mem";

  // 首先在当前 TabWindow 中查找
  for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
    if (it->first == index) {
      // 找到了
      qDebug() << "found in current TabWindow";
      restoreClosedTabFromMem(index, it->second);
      closedTabs_.erase(it);
      this->raise();
      return;
    }
  }

  // 在其他 TabWindow 中查找
  TabWindow *targetWindow =
      TabWindowManager::instance()->findTabWindowWithClosedTab(index);
  if (targetWindow) {
    qDebug() << "found in other TabWindow:" << targetWindow;

    // 如果窗口隐藏了，先显示它
    if (!targetWindow->isVisible()) {
      targetWindow->show();
    }

    // 在目标窗口中查找并恢复
    for (auto it = targetWindow->closedTabs_.begin();
         it != targetWindow->closedTabs_.end(); ++it) {
      if (it->first == index) {
        targetWindow->restoreClosedTabFromMem(index, it->second);
        targetWindow->closedTabs_.erase(it);
        break;
      }
    }

    // 将找到的窗口提到前面
    targetWindow->raise();
    targetWindow->activateWindow();
    return;
  }

  qDebug() << "UUID" << index << "not found in any TabWindow's closedTabs_";
}

void TabWindow::switchByReadingDisk(const QString &index,
                                    TtProtocolRole::Role role,
                                    const QJsonObject &config) {
  switchByAlreadyExistingWidget(count(), index, config, role);
}

void TabWindow::sessionSwitchPage(int tabIndex, TtProtocolRole::Role role) {
  qDebug() << "switch index: " << tabIndex;
  switchByCreateWidget(tabIndex, role); // widgetId = 2 是 widget2
  qDebug() << "切换界面成功";
}

void TabWindow::handleAddNewTab() { addNewTab("新建立"); }

void TabWindow::handleTabCloseRequested(int index) {
  QWidget *widget = this->widget(index);
  const QString uuid = findWidget(widget);
  // qDebug() << uuid;
  if (widget) {
    // widget->blockSingals(true);
    widget->blockSignals(true);
  }

  if (!uuid.isEmpty()) {
    TabWindowManager::instance()->removeUuidTabPage(uuid);
  } else {
    // 仍然需要删除没有UUID的标签页
    removeTab(index);
    if (widget) {
      widget->disconnect();
      widget->setParent(nullptr);
      widget->deleteLater();
    }
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
    // 获取当前点击的标签索引
    int currentIndex = getTabIndexFromButton(closeButton);
    if (currentIndex != -1) {
      // qDebug() << "delete index: " << currentIndex;
      // handleTabClose(currentIndex);

      QString uuid = findWidget(widget(currentIndex));
      qDebug() << "Closing tab with UUID:" << uuid;
      // 使用UUID关闭标签页，避免索引问题
      if (!uuid.isEmpty()) {
        // 先记录总数，用于后续检测
        int tabCountBefore = count();
        handleTabCloseByUuid(uuid);
        qDebug() << "Tabs closed:" << (tabCountBefore - count());
      } else {
        // 作为后备方案使用索引
        handleTabClose(currentIndex);
      }
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

void TabWindow::handleTabCloseByUuid(const QString &uuid) {
  if (widgetInstances.contains(uuid)) {
    QWidget *w = widgetInstances.value(uuid);
    int index = indexOf(w);
    if (index != -1) {
      // 只处理这一个特定的标签页
      handleTabClose(index);
    }
  }
}

void TabWindow::handleTabClose(int index) {
  if (widget(index)->objectName() == "SettingWidget") {
    handleTabCloseRequested(index);
    return;
  }
  // 会影响后面的状态
  Window::FrameWindow *w = qobject_cast<Window::FrameWindow *>(widget(index));
  if (w != nullptr) {
    if (!w->saveState()) {
      qDebug() << "no saved";
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
}

void TabWindow::saveWorkingTabPageToMem(int index) {
  TabData info;
  info.title = tabText(index);
  info.widget = widget(index); // 存储 widget 界面
  info.icon = tabIcon(index);

  // 获取 uuid
  QString removeWidgetUuid = findWidget(widget(index));
  qDebug() << "remove uuid" << removeWidgetUuid;
  // 实例一定存在
  widgetInstances.remove(removeWidgetUuid);
  closedTabs_.prepend(qMakePair((removeWidgetUuid), info));
  removeTab(index);
  if (this->count() <= 0) {
    // 检查是否为 root 窗口
    if (this == TabWindowManager::instance()->rootWindow()) {
      // 如果是root窗口，则创建一个新标签页，保持窗口存在
      // addNewTab(tr("新建立"));
      qDebug() << "新建立窗口";
    } else {
      this->hide();
    }
  }
  qDebug() << "保存到 mem";
}

void TabWindow::restoreClosedTabFromMem(const QString &index,
                                        struct TabData data) {
  // 从内存恢复
  qDebug() << "restore mem";
  // 如果当前窗口是隐藏的，先显示它
  if (!this->isVisible()) {
    qDebug() << "show by hide";
    this->show();
    this->raise();
  }

  // 那么 实例中也许要添加对应的信息, 不然没办法切换
  widgetInstances[index] = data.widget;

  // // icon 消失
  // addNewTab(data.widget, data.title);
  // // 切换到复原的 tab
  // setCurrentIndex(count() - 1);

  // 正确地恢复包含图标的标签页
  int newTabIndex = addTab(data.widget, data.icon, data.title);
  setupCustomTabButton(newTabIndex);
  // 切换到恢复的标签页
  setCurrentIndex(newTabIndex);
  // 如果这是唯一的标签页，确保窗口激活并正常显示
  if (count() == 1) {
    TabWindowManager::instance()->activateWindow(this->windowHandle());
  }

  // 通知窗口被恢复
  if (auto *frameWindow = qobject_cast<Window::FrameWindow *>(data.widget)) {
    // 可以调用窗口的恢复方法，如果有的话
    // frameWindow->onRestoreFromMemory();

    // 确保标题显示正确
    setTabTitle(index, data.title);
  }

  qDebug() << "标签页已从内存恢复: " << data.title;
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
