/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group
  company <info@kdab.com> Author: Nicolas Arnaud-Cormos
  <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QCursor>
#include <QTabWidget>

#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QProxyStyle>
#include <QStackedLayout>
#include <QState>
#include <QStateMachine>
#include <QTabWidget>
#include <qmimedata.h>
#include <qtimer.h>

#include <QToolButton>
#include <functional>

#include "Def.h"

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

namespace Window {
class FrameWindow;
} // namespace Window

namespace Ui {
class TabWindow;
} // namespace Ui

using TabWindow = Ui::TabWindow;

class TabWindowManager : public QObject {
  Q_OBJECT
public:
  static TabWindowManager *instance();

  QList<TabWindow *> windows() const;

  TabWindow *currentWindow() const;
  QWidget *currentWidget() const;

  TabWindow *rootWindow() const;
  void setRootWindow(TabWindow *root);

  TabWindow *findTabWindowWithClosedTab(const QString &uuid);

signals:
  // BUG 这个信号没有使用
  void tabCloseRequested(QWidget *widget, TabWindow *window);
  void tabCreateRequested(TabWindow *sourceWindow); // Window 创建新的 tab

public slots:
  ///
  /// @brief removeUuidTabPage
  /// @param index
  /// 根据提供的 uuid, 删除 tab 窗口, 同时, 如果没有 tab 窗口 将会删除 TabWindow
  void removeUuidTabPage(const QString &index);

protected:
  void addWindow(TabWindow *window);
  void removeWindow(TabWindow *window);

  TabWindow *possibleWindow(TabWindow *currentWindow,
                            QPoint globalPos = QCursor::pos());

private:
  void activateWindow(QWindow *window);
  void requestCloseTab(int index);

private:
  using QObject::QObject;
  TabWindowManager();

  friend class Ui::TabWindow;
  QList<TabWindow *> m_windows;
  TabWindow *m_root = nullptr;
};

namespace Ui {

class TabCloseButton : public QToolButton {
  Q_OBJECT
public:
  TabCloseButton(QWidget *parent = nullptr) : QToolButton(parent) {
    setFixedSize(16, 16);
    setCursor(Qt::PointingHandCursor);
  }
  ~TabCloseButton() = default;

protected:
  virtual void paintEvent(QPaintEvent *event) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    if (underMouse()) {
      painter.setBrush(QColor(200, 200, 200, 150));
      painter.setPen(Qt::NoPen);
      painter.drawEllipse(rect());
    }

    // 绘制关闭图标
    painter.setPen(QPen(isEnabled() ? Qt::black : Qt::gray, 1.5));
    int margin = 4;
    painter.drawLine(margin, margin, width() - margin, height() - margin);
    painter.drawLine(width() - margin, margin, margin, height() - margin);
  }
  void enterEvent(QEnterEvent *event) override {
    QToolButton::enterEvent(event);
  }
  void leaveEvent(QEvent *event) override { QToolButton::leaveEvent(event); }
};

class TabWindow : public QTabWidget {
  Q_OBJECT
public:
  friend TabWindowManager;
  explicit TabWindow(QWidget *parent = nullptr);
  ~TabWindow();

  using WidgetFactory = std::function<Window::FrameWindow *()>;

  static QString SpecialTypeIcon(TtProtocolRole::Role role);

  void addNewTab(const QString &title = "");
  void addNewTab(QWidget *defaultWidget);
  void addNewTab(QWidget *defaultWidget, const QString &title);
  void addNewTab(QWidget *defaultWidget, const QIcon &icon,
                 const QString &title);

  // 注册 Widget 工厂函数
  void registerWidget(TtProtocolRole::Role role, const WidgetFactory &factory,
                      const QString &title);

  //
  void switchToWidget(int tabIndex,
                      TtProtocolRole::Role role); // 创建新的窗口 程序生成 uuid
  void switchByCreateWidget(int tabIndex, TtProtocolRole::Role role);

  void switchByAlreadyExistingWidget(
      int tabIndex, const QString &uuid, const QJsonObject &config,
      TtProtocolRole::Role role); // 存有 uuid 的历史窗口

  QString getCurrentWidgetUUid();
  QString getCurrentWidgetUUid(QWidget *widget);

  void setTabIcon(int index, const QString &iconPath);
  void setTabIcon(int index, const QIcon &icon);

  // 点击了 save 才保存
  void setTabTitle(const QString &title);
  void setTabTitle(const QString &index, const QString &title);

  bool isStoredInMem(const QString &index);
  bool isCurrentDisplayedPage(const QString &index);

  void switchByPage(const QString &index);
  void switchByReadingMem(const QString &index, TtProtocolRole::Role role);
  void switchByReadingDisk(const QString &index, TtProtocolRole::Role role,
                           const QJsonObject &config); // 切换 Page

protected:
  bool eventFilter(QObject *object, QEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

signals:
  void newTabRequested(); // 新建标签信号
  void requestNewTab();
  void widgetDeleted(QWidget *widget);

public slots:
  void sessionSwitchPage(int tabIndex, TtProtocolRole::Role role);
  void handleAddNewTab();                  // 添加标签
  void handleTabCloseRequested(int index); // 删除 Page
  void removeUuidWidget(const QString &index);

private:
  struct TabData;
  void setupCustomTabButton(int index);
  void setupCornerButton(); // 右上角的 button
  int getTabIndexFromButton(QWidget *button) const;
  QWidget *createDefaultWidget(int tabIndex); // 初始化默认的 widget1
  QString findWidget(QWidget *widget);
  void handleTabClose(int index);

  void saveWorkingTabPageToMem(int index); // 保存正在工作的 page 到 Mem 中
  // 从内存中恢复最近关闭的标签页
  void restoreClosedTabFromMem(const QString &index, struct TabData data);
  void saveTabPageToDisk(int index); // 保存配置 page 到磁盘中
  void restoreClosedTabFromDisk();   // 从本地中恢复标签页

  QStringList getClosedTabsList() const; // 获取可恢复的标签页列表
  QJsonObject serializeTab(int index) const;   // 序列化
  void deserializeTab(const QJsonObject &obj); // 反序列化

  struct TabData {
    QString title;
    QWidget *widget;
    QIcon icon;
    QByteArray state; // 用于保存widget状态
  };

  QToolButton *add_button_;
  // 每个实例类保存一个, 处理自己的 show 与 hide 关系, close 直接保存到 root
  QList<QPair<QString, TabData>> closedTabs_;
  // 最大保存历史数量
  const int maxClosedTabs_ = 10;
  QPoint dragStartPosition_;

  // Widget 工厂函数映射 一个类共享
  static QHash<TtProtocolRole::Role, WidgetFactory> widgetFactories;

  // Widget 标题映射
  static QHash<TtProtocolRole::Role, QString> widgetTitles;

  // Widget 实例
  // 是否需要保存 this 实例, 也作为一个 索引, 方便从 tabWindow 遍历
  static QMap<QString, QWidget *> widgetInstances;

  static QMap<TtProtocolRole::Role, QString> type_icon_map_;

private:
  bool m_dragging = false;
  bool m_isMoving = false;
  bool m_ignoreMouseEvent = false;
  TabWindow *m_movingWindow = nullptr;
  QPoint m_mouseDelta;
  TabWindow *m_origin = nullptr;
  TabWindow *m_dragWnd = nullptr;
  QPoint m_mouseOffset;
  const int DETACH_THRESHOLD = 20;
  bool m_pressing = false;
  QPoint m_pressPos;
};

} // namespace Ui

#endif // TABWINDOW_H
