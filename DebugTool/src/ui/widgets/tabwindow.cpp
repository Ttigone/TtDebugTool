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

QString TabWindow::SpecialTypeIcon(TtProtocolRole::Role role) {
  return type_icon_map_[role];
}

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

void TabWindowManager::addWindow(TabWindow *window) {
  m_windows.append(window);
  connect(window, &TabWindow::tabCloseRequested, this,
          &TabWindowManager::requestCloseTab);
}

void TabWindowManager::removeWindow(TabWindow *window) {
  if (m_windows.contains(window)) {
    qDebug() << "[Manager] Removing window:" << window;
    m_windows.removeOne(window);
    window->deleteLater(); // 确保对象删除
  }
  // m_windows.removeOne(window);
}

// TabWindow *TabWindowManager::possibleWindow(TabWindow *currentWindow,
//                                             QPoint globalPos) {
//   for (auto w : m_windows) {
//     if (w == currentWindow) {
//       continue;
//     }
//     // 使用 QWidget 的几何区域（不含窗口装饰）
//     QRect widgetRect = w->geometry();
//     if (!widgetRect.contains(globalPos))
//       continue;

//     // 将全局坐标转换为目标窗口标签栏的局部坐标
//     QPoint posInTabBar = w->tabBar()->mapFromGlobal(globalPos);
//     QRect tabBarRect = w->tabBar()->rect();

//     // 扩展标签栏区域宽度至窗口宽度（某些样式下标签栏可能未占满宽度）
//     tabBarRect.setWidth(w->width());

//     if (tabBarRect.contains(posInTabBar)) {
//       return w;
//     }
//   }
//   return nullptr;
// }

TabWindow *TabWindowManager::possibleWindow(TabWindow *src, QPoint globalPos) {
  qDebug() << src << m_root;
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
  qDebug() << "无存在的窗口";
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

TabWindow::TabWindow(QWidget *parent) : QTabWidget(parent) {
  setAttribute(Qt::WA_DeleteOnClose);

  qDebug() << "New TabWindow created:" << this;

  Q_ASSERT(TabWindowManager::instance());
  TabWindowManager::instance()->addWindow(this);
  tabBar()->installEventFilter(this);

  this->setStyleSheet(
      "QTabWidget::pane { border-top: 1px solid #ccc; border-left: 1px solid "
      "#ccc; border-right: none; border-bottom: none; }  QTabWidget { "
      "background: #f0f0f0; }  /* 尝试隐藏或设置角落区域的样式 */ "
      "QTabWidget::corner-widget { border: none; background: #f0f0f0; /* "
      "或者设置为与 TabBar 一致的颜色 */ }  /* 普通未选中的标签样式 */ "
      "QTabBar::tab { background: #f0f0f0; padding: 6px 12px; border-top: "
      "1px solid #ccc; border-left: 1px solid #ccc;  border-right: none; "
      "border-bottom: none; border-top-left-radius: 4px; "
      "border-top-right-radius: 4px; } ");

  setMovable(true);
  setDocumentMode(true);
  setFocusPolicy(Qt::NoFocus);

  setupCornerButton();

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
          // 样式表失效
          // 添加的新窗口
          // 不能指定，否则无法弹出
          m_movingWindow = new TabWindow;
          // m_movingWindow = new TabWindow(this->window());
          m_movingWindow->setAttribute(Qt::WA_DeleteOnClose);
          // 获取当前的 widget2
          QWidget *w = widget(idx);
          QIcon icon = tabIcon(idx);
          QString txt = tabText(idx);
          m_movingWindow->setGeometry(rect());
          qDebug() << "new TabWindow: " << m_movingWindow;
          m_movingWindow->move(QCursor::pos() + QPoint(100, 100));
          m_movingWindow->show();
          m_movingWindow->addTab(w, icon, txt);
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
          qDebug() << "只有一个标签的 window";
          QWidget *w = m_movingWindow->widget(0);
          QIcon icon = m_movingWindow->tabIcon(0);
          QString txt = m_movingWindow->tabText(0);
          QPoint posIn = target->tabBar()->mapFromGlobal(gpos);
          int insertIdx = target->tabBar()->tabAt(posIn);
          if (insertIdx < 0)
            insertIdx = target->count();
          target->insertTab(insertIdx, w, icon, txt);
          target->setCurrentIndex(insertIdx);
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
  qDebug() << "close";
  TabWindow *root = TabWindowManager::instance()->rootWindow();
  qDebug() << "root: " << root;
  if (root == this) {
    qDebug() << "[ROOT] Closing root window:" << this;

    // 获取所有子窗口的拷贝
    const auto children = TabWindowManager::instance()->windows();
    for (TabWindow *w : children) {
      if (w == this)
        continue;
      qDebug() << "[ROOT] Deleting child window:" << w;
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
        dest->addTab(w, ic, txt);
      }
      // 全部的窗口删除了
      event->accept();
      deleteLater();
      return;
    }
  }
  // // 其他窗口关闭
  // TabWindow *dest = TabWindowManager::instance()->rootWindow();
  // if (dest && dest != this) {
  //   int countTabs = count();
  //   for (int i = 0; i < countTabs; ++i) {
  //     QWidget *w = widget(0);
  //     QIcon ic = tabIcon(0);
  //     QString txt = tabText(0);
  //     removeTab(0);
  //     // 这里要修改为其他的 add
  //     dest->addTab(w, ic, txt);
  //   }
  // } else {
  //   // root 窗口不存在或正在关闭
  //   event->accept();
  //   deleteLater();
  //   return;
  // }
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

  widgetInstances[QUuid::createUuid().toString()] = newWidget;

  setTabText(tabIndex, widgetTitles[role]);
  insertTab(tabIndex, newWidget, QIcon(SpecialTypeIcon(role)),
            widgetTitles[role]);
  setupCustomTabButton(tabIndex);
  // updateTabStyle(tabIndex);
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

  qDebug() << "new by has uuid";

  // 创建新的 Widget
  // 这里需要设置对应的 config 配置
  Window::FrameWindow *newWidget = widgetFactories[role]();
  qDebug() << newWidget;
  if (newWidget) {
    // 必须异步调用
    QMetaObject::invokeMethod(newWidget, "setSetting", Qt::QueuedConnection,
                              Q_ARG(const QJsonObject &, config));
    // newWidget->setSetting(config);

    qDebug() << "t1";

    // 可以根据已经有的 uuid 设定
    widgetInstances[uuid] = newWidget;

    // 设置 tab 的文本
    setTabText(tabIndex, widgetTitles[role]);
    // 向 tabIndex 所有的 tab 界面设置界面 newWidget, title 为对应索引值
    // 设置图标
    insertTab(tabIndex, newWidget, QIcon(SpecialTypeIcon(role)),
              widgetTitles[role]);
    setupCustomTabButton(tabIndex);
    qDebug() << "t2";
    // updateTabStyle(tabIndex);

    // 显示当前正在操作的 tab
    // 重复创建 串口时, 发现会跳到之前已经创建好的
    setCurrentIndex(tabIndex);
  } else {
    qDebug() << "error to create widget";
  }
}

QString TabWindow::getCurrentWidgetUUid() {
  return findWidget(currentWidget());
}

void TabWindow::setTabIcon(int index, const QString &iconPath) {
  setTabIcon(index, QIcon(iconPath));
}

void TabWindow::setTabIcon(int index, const QIcon &icon) {
  QTabWidget::setTabIcon(index, icon);
  updateTabStyle(index);
}

void TabWindow::setTabTitle(const QString &title) {
  tabBar()->setTabText(currentIndex(), title);
}

bool TabWindow::isStoredInMem(const QString &index) {
  // for (auto it = closedTabs_.begin())
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
  setCurrentWidget(widgetInstances.value(index));
}

void TabWindow::sessionSwitchPage(int tabIndex, TtProtocolRole::Role role) {
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
    emit widgetDeleted(widget);
    widget->disconnect();
    widget->setParent(nullptr);
    delete widget;
    widget = nullptr;
  }
}

void TabWindow::removeUuidWidget(const QString &index) {
  if (widgetInstances.contains(index)) {
    // 移除标签页
    auto item = widgetInstances.value(index);
    auto i = indexOf(item);
    auto *widget = QTabWidget::widget(i);
    if (widget) {
      widget->disconnect();
      widget->setParent(nullptr);
      widget->deleteLater();
    }
    widgetInstances.remove(index);
    removeTab(i);
  }
}

void TabWindow::setupCustomTabButton(int index) {
  // 当前 index 的 closebutton
  auto *closeButton = new TabCloseButton(this);
  // auto *closeButton = new QToolButton(this);
  tabBar()->setTabButton(index, QTabBar::RightSide, closeButton);

  // 使用按钮找到当前索引，而不是使用固定索引
  connect(closeButton, &QToolButton::clicked, this, [this, closeButton]() {
    int currentIndex = getTabIndexFromButton(closeButton);
    if (currentIndex != -1) {
      qDebug() << "delete index: " << currentIndex;
      handleTabClose(currentIndex);
    }
  });
}

void TabWindow::updateTabStyle(int index) {
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
  for (auto it = widgetInstances.cbegin(); it != widgetInstances.cend(); ++it) {
    if (it.value() == widget) {
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
      // TtContentDialog *dialog = new TtContentDialog(this);
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
      //   // w->saveSetting();
      //   // 保存
      //   QMetaObject::invokeMethod(w, "setSetting", Qt::QueuedConnection);
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
    } else {
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
    // 一般不会进入
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

  QString removeWidgetUuid = findWidget(widget(index));
  qDebug() << "remove uuid" << removeWidgetUuid;
  // 保存到 closedTabs_
  closedTabs_.prepend(qMakePair((removeWidgetUuid), info));
  removeTab(index);
  qDebug() << "保存到 mem";
}

void TabWindow::restoreClosedTabFromMem(struct TabData data) {
  qDebug() << "restore mem";
  // // 恢复 添加到最后
  // addNewTab(data.second.widget, data.second.title);
  addNewTab(data.widget, data.title);
  // addNewTab(data.widget.data(), data.title);
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
