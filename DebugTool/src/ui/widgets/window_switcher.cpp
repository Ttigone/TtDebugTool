#include "window_switcher.h"

#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStackedLayout>
#include <QToolButton>
#include <QUuid>
#include <QWindow>
#include <ui/control/TtContentDialog.h>

#include "ui/layout/vertical_layout.h"

#include <window/frame_window.h>

namespace Ui {

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

QString TabWindow::SpecialTypeIcon(TtProtocolRole::Role role) {
  return type_icon_map_[role];
}

TabWindow::TabWindow(QWidget *parent) : QTabWidget(parent) {
  Q_ASSERT(TabWindowManager::instance());
  TabWindowManager::instance()->addWindow(this);
  // setTabBar(new ExtTabBar());
  // 处理的是 tabbar 的事件
  tabBar()->installEventFilter(this);

  setMovable(true);
  setDocumentMode(true);
  setFocusPolicy(Qt::NoFocus);
}

// TabWindow::TabWindow(QWidget *defaultWidget, QWidget *parent)
//     : QTabWidget(parent) {
//   Q_ASSERT(TabWindowManager::instance());
//   // 添加的是整个 QTabWidget ???
//   TabWindowManager::instance()->addWindow(this);

//   // setTabBar(new ExtTabBar());
//   // 处理的是 tabbar 的事件
//   tabBar()->installEventFilter(this);

//   setMovable(true);
//   setDocumentMode(true);
//   setFocusPolicy(Qt::NoFocus);

//   // setupCornerButton();

//   // setTabBarAutoHide(true);

//   // addNewTab(defaultWidget);
//   // saveState("C:/Users/cssss/Desktop/test.json");
// }

TabWindow::~TabWindow() {
  // for (auto& widget : widgetInstances) {
  //  if (widget) {
  //    widget->deleteLater();
  //  }
  //}
  // qDeleteAll(widgetInstances);
  qDebug() << "析构函数" << this;
  TabWindowManager::instance()->removeWindow(this);
};

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

bool TabWindow::saveState(const QString &filePath) const {
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

bool TabWindow::restoreState(const QString &filePath) {
  // 将标签页的数据读取
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  // 先读取成 qdocument
  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isArray()) {
    QJsonArray tabsArray = doc.array();
    for (const auto &tabValue : tabsArray) {
      deserializeTab(tabValue.toObject());
    }
    return true;
  }
  return false;
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
      TtContentDialog *dialog = new TtContentDialog(this);
      dialog->setLeftButtonText(tr("取消"));
      dialog->setRightButtonText(tr("确定"));
      dialog->setCenterText(tr("通讯链接配置已修改, 是否保存"));

      connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this, [&]() {
        dialog->reject();
        qDebug() << "取消保存";
        if (w->workState()) {
          // 保存
          saveWorkingTabPageToMem(index);
        } else {
          // 删除
          handleTabCloseRequested(index);
        }
      });
      connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this, [&]() {
        dialog->accept();
        qDebug() << "保存最新变动";
        // w->saveSetting();
        // 保存
        QMetaObject::invokeMethod(w, "setSetting", Qt::QueuedConnection);
        if (w->workState()) {
          qDebug() << "save to mem";
          saveWorkingTabPageToMem(index);
        } else {
          qDebug() << "close";
          handleTabCloseRequested(index);
        }
      });
      dialog->exec();
      delete dialog;
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
  QWidget *w = widget(index);
  if (auto *serializable = dynamic_cast<ISerializable *>(w)) {
    tabObj["state"] = QString(serializable->saveState().toBase64());
  }

  return tabObj;
}

void TabWindow::deserializeTab(const QJsonObject &obj) {
  QString title = obj["title"].toString();
  QString iconName = obj["icon"].toString();

  QWidget *page = new QWidget(this);
  // 恢复widget状态
  if (obj.contains("state")) {
    QByteArray state =
        QByteArray::fromBase64(obj["state"].toString().toLatin1());
    if (auto *serializable = dynamic_cast<ISerializable *>(page)) {
      serializable->restoreState(state);
    }
  }

  addTab(page, QIcon(iconName), title);
}

// void TabWindow::handleButtonClicked(int tabIndex, TtProtocolRole::Role role)
// {
void TabWindow::sessionSwitchPage(int tabIndex, TtProtocolRole::Role role) {
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

// bool TabWindow::eventFilter(QObject *object, QEvent *event) {
//   if (object != tabBar()) {
//     return QTabWidget::eventFilter(object, event);
//   }

//   switch (event->type()) {
//   case QEvent::MouseMove: {
//     qDebug() << "mouse move";
//     if (m_ignoreMouseEvent) {
//       qDebug() << "ignoe";
//       return true;
//     }
//     auto mouseEvent = static_cast<QMouseEvent *>(event);

//     auto sendFakeEvent = [mouseEvent](QObject *receiver, QEvent::Type type) {
//       if (!receiver) {
//         return;
//       }
//       QMouseEvent newEvent(type, mouseEvent->pos(), Qt::LeftButton,
//                            mouseEvent->buttons(), mouseEvent->modifiers());
//       QCoreApplication::sendEvent(receiver, &newEvent);
//     };

//     if (m_isMoving) {
//       qDebug() << "inside";
//       if (m_movingWindow) {
//         auto globalPos = QCursor::pos();
//         auto window = TabWindowManager::instance()->possibleWindow(
//             m_movingWindow, globalPos);
//         if (window) {
//           // re-attach
//           const auto pos = window->mapFromGlobal(globalPos);
//           const int index = tabBar()->tabAt(pos);
//           const auto w = m_movingWindow->widget(0);
//           const auto text = m_movingWindow->tabText(0);
//           window->raise();
//           window->activateWindow();
//           window->setCurrentIndex(window->insertTab(index, w, text));
//           m_movingWindow->deleteLater();
//           m_movingWindow = nullptr;
//           m_isMoving = false;
//           m_ignoreMouseEvent = true;
//           sendFakeEvent(object, QEvent::MouseButtonRelease);
//           sendFakeEvent(window->tabBar(), QEvent::MouseButtonPress);
//         } else {
//           auto newPos = globalPos - m_mouseDelta;
//           m_movingWindow->move(newPos);
//           qDebug() << "this moving";
//         }
//         return true;
//       }
//     } else {
//       qDebug() << "no move";
//       auto r = tabBar()->rect();
//       if (tabBar()->count() == 1 ||
//           distance(r, mouseEvent->pos()) > DISTANCE_TO_DETACH) {

//         // 清理旧窗口
//         if (m_movingWindow && m_movingWindow != this) {
//           m_movingWindow->deleteLater();
//           m_movingWindow = nullptr;
//         }

//         sendFakeEvent(object, QEvent::MouseButtonRelease);
//         qDebug() << "fake";

//         m_isMoving = true;
//         const int index = currentIndex();

//         auto tabRect = tabBar()->tabRect(index);
//         m_mouseDelta = tabRect.center() - tabRect.topLeft() +
//                        (geometry().topLeft() - pos());

//         if (tabBar()->count() >= 2) {
//           // m_movingWindow = new TabWindow;
//           // m_movingWindow = new TabWindow(nullptr);
//           m_movingWindow = new TabWindow();
//           const auto w = widget(index);
//           const auto text = tabText(index);
//           const auto icon = tabIcon(index);
//           m_movingWindow->setGeometry(rect());
//           const auto globalPos = QCursor::pos();
//           m_movingWindow->move(globalPos.x() + 100, globalPos.y() + 100);
//           // BUG: it would be better to add the tab before showing, but on
//           // Windows with poor openGL driver, there is actually a crash when
//           the
//           // tab is a QQuickWidget
//           m_movingWindow->show();
//           m_movingWindow->addTab(w, icon, text);
//         } else {
//           m_movingWindow = this;
//         }
//         return true;
//       }
//     }
//     break;
//   }
//   case QEvent::MouseButtonRelease:
//     m_isMoving = false;
//     m_movingWindow = nullptr;
//     m_ignoreMouseEvent = false;
//     break;
//   case QEvent::MouseButtonPress:
//     m_ignoreMouseEvent = false;
//     break;
//   default:
//     // Remove warning
//     break;
//   }
//   return QTabWidget::eventFilter(object, event);
// }

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
          m_movingWindow = new TabWindow;
          QWidget *w = widget(idx);
          QIcon icon = tabIcon(idx);
          QString txt = tabText(idx);
          m_movingWindow->setGeometry(rect());
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
  }
  if (count() == 0) {
    event->accept();
    deleteLater();
    return;
  }
  QTabWidget::closeEvent(event);
}

void TabWindow::switchByReadingMem(const QString &index,
                                   TtProtocolRole::Role role) {
  for (auto it = closedTabs_.begin(); it != closedTabs_.end(); ++it) {
    qDebug() << it->first;
    if (it->first == index) {
      restoreClosedTabFromMem(it->second);
      closedTabs_.erase(it);
      break;
    }
  }
}

void TabWindow::switchByReadingDisk(const QString &index,
                                    TtProtocolRole::Role role,
                                    const QJsonObject &config) {
  // 删除后, 在点击 button, 没有对应的窗口
  // 处理点击左侧后, 不存在, 需要重新创建
  // 初始时是有一个默认 widget, 所以 currentIndex = 0, 会切换当前
  // 为什么还会进来 ?
  qDebug() << "no find to create";
  // qDebug() << "currentIndex: " << currentIndex();
  // switchToWidget(currentIndex() + 1, role);
  // 初始时不存在, 根据 已有的 uuid 创建特定表示窗口

  qDebug() << "TEST1";
  qDebug() << currentIndex() + 1;
  switchByAlreadyExistingWidget(currentIndex() + 1, index, config, role);
  qDebug() << "TEST2";
}

void TabWindow::setupCustomTabButton(int index) {
  // 当前 index 的 closebutton
  auto *closeButton = new TabCloseButton(this);
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

TabWindowManager *TabWindowManager::instance() {
  static TabWindowManager manager;
  return &manager;
}

QList<TabWindow *> TabWindowManager::windows() const { return windows_; }

TabWindow *TabWindowManager::currentWindow() const {
  if (!windows_.isEmpty()) {
    return windows_.first();
  }
  return nullptr;
}

QWidget *TabWindowManager::currentWidget() const {
  if (!windows_.isEmpty()) {
    return windows_.first()->currentWidget();
  }
  return nullptr;
}

TabWindow *TabWindowManager::rootWindow() const { return m_root; }

void TabWindowManager::setRootWindow(TabWindow *root) {
  m_root = root;
  addWindow(root);
}

void TabWindowManager::addWindow(TabWindow *window) {
  windows_.append(window);
  // 关联 QTabWidget 的 tab 关闭
  connect(window, &TabWindow::tabCloseRequested, this,
          &TabWindowManager::requestCloseTab);
}

void TabWindowManager::removeWindow(TabWindow *window) {
  windows_.removeOne(window);
}

// TabWindow *TabWindowManager::possibleWindow(TabWindow *currentWindow,
//                                             QPoint globalPos) {
// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//   for (auto tabWindow : std::as_const(windows_))
// #else
//   for (auto tabWindow : qAsConst(m_windows))
// #endif
//   {
//     if (tabWindow == currentWindow)
//       continue;
//     // Get the possible drop rectangle, which is the rectangle at the top
//     // containing the tabbar
//     if (tabWindow->frameGeometry().contains(globalPos)) {
//       auto pos = tabWindow->tabBar()->mapFromGlobal(globalPos);
//       auto r = tabWindow->tabBar()->rect();
//       r.setWidth(tabWindow->rect().width());
//       if (r.contains(pos))
//         return tabWindow;
//       return nullptr;
//     }
//   }
//   return nullptr;
// }

TabWindow *TabWindowManager::possibleWindow(TabWindow *src, QPoint globalPos) {
  // qDebug() << src << m_root;
  // for (TabWindow *w : windows_) {
  //   if (w == src) {
  //     continue;
  //   }
  //   QWindow *top = w->windowHandle();
  //   if (!top) {
  //     continue;
  //   }
  //   if (top->frameGeometry().contains(globalPos)) {
  //     QPoint pos = w->tabBar()->mapFromGlobal(globalPos);
  //     QRect bar = w->tabBar()->rect();
  //     bar.setWidth(w->width());
  //     if (bar.contains(pos)) {
  //       return w;
  //     }
  //   }
  // }
  // qDebug() << "无存在的窗口";
  // return nullptr;
  for (TabWindow *w : windows_) {
    if (w == src) {
      continue;
    }
    QRect widgetRect = w->geometry();
    qDebug() << "widget Rect: " << widgetRect;
    if (!widgetRect.contains(globalPos)) {
      continue;
    }
    // 将全局坐标转换为标签栏的局部坐标
    QPoint posInTabBar = w->tabBar()->mapFromGlobal(globalPos);
    QRect tabBarRect = w->tabBar()->rect();
    tabBarRect.setWidth(w->width()); // 扩展标签栏宽度
  }
}

void TabWindowManager::activateWindow(QWindow *window) {
  // 个数只有 1 个, 不能拖拽, 大于 2 个才能
  if (windows_.count() < 2) {
    return;
  }

  int index = -1;
  for (int i = 1; i < windows_.count(); ++i) {
    if (windows_.at(i)->windowHandle() == window) {
      index = i;
      break;
    }
  }

  if (index != -1) {
    windows_.move(index, 0);
  }
}

void TabWindowManager::requestCloseTab(int index) {
  auto window = qobject_cast<TabWindow *>(sender());
  emit tabCloseRequested(window->widget(index), window);
}

TabWindowManager::TabWindowManager() {
  connect(qApp, &QApplication::focusWindowChanged, this,
          &TabWindowManager::activateWindow);
}

// CustomTabPage::CustomTabPage(QWidget* parent) : QWidget(parent) {}

// // 注册 widget2
// TabWindow.registerWidget(2, []() -> QWidget* {
//   QWidget* widget = new QWidget();
//   QVBoxLayout* layout = new QVBoxLayout(widget);
//   layout->addWidget(new QLabel("This is Widget2"));
//   widget->setLayout(layout);
//   return widget;
// }, "Widget2");

// // 注册 widget3（用于新 Tab）
// TabWindow.registerWidget(3, []() -> QWidget* {
//   QWidget* widget = new QWidget();
//   QVBoxLayout* layout = new QVBoxLayout(widget);
//   layout->addWidget(new QLabel("This is Widget3"));
//   widget->setLayout(layout);
//   return widget;
// }, "Widget3");

} // namespace Ui
