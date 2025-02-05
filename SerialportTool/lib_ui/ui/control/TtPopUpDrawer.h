/*****************************************************************/ /**
 * \file   TtPopUpDrawer.h
 * \brief  弹出抽屉窗口
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_CONTROL_TTPOPUPDRAWER_H
#define UI_CONTROL_TTPOPUPDRAWER_H

#include <qsplitter.h>

#include <QStateMachine>
#include <QTimer>

#include "ui/widgets/overlay_widget.h"

namespace Ui {

class TtPopUpDrawerPrivate;

class TtPopUpDrawer : public TtOverlayWidget {
  Q_OBJECT
 public:
  TtPopUpDrawer(QWidget* parent = nullptr);
  ~TtPopUpDrawer();

  void setDrawerWidth(int width);
  int drawerWidth() const;

  void setDrawerLayout(QLayout* layout);
  QLayout* drawerLayout() const;

  void setClickOutsideToClose(bool state);
  bool clickOutsideToClose() const;

  void setAutoRaise(bool state);
  bool autoRaise() const;

  void setOverlayMode(bool value);
  bool overlayMode() const;

 signals:
  void willClose();

 public slots:
  void openDrawer();
  void closeDrawer();

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* obj, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

  const QScopedPointer<TtPopUpDrawerPrivate> d_ptr;

 private:
  Q_DISABLE_COPY(TtPopUpDrawer)
  Q_DECLARE_PRIVATE(TtPopUpDrawer)
};

class TtPopUpDrawerWidget : public TtOverlayWidget {
  Q_OBJECT
  Q_PROPERTY(int offset WRITE setOffset READ offset)
 public:
  explicit TtPopUpDrawerWidget(QWidget* parent = nullptr);
  ~TtPopUpDrawerWidget();

  void setOffset(int offset);
  int offset() const { return offset_; }

 protected:
  void paintEvent(QPaintEvent* event) override;

  QRect overlayGeometry() const override;

 private:
  int offset_;
};

class TtPopUpDrawerStateMachine : public QStateMachine {
  Q_OBJECT
  Q_PROPERTY(qreal opacity WRITE setOpacity READ opacity)

 public:
  explicit TtPopUpDrawerStateMachine(TtPopUpDrawerWidget* drawer,
                                     TtPopUpDrawer* parent);
  ~TtPopUpDrawerStateMachine();

  // 透明度
  void setOpacity(qreal opacity);
  inline qreal opacity() const;

  bool isInClosedState() const;

  void updatePropertyAssignments();

 signals:
  void signalOpen();
  void signalClose();

 private:
  Q_DISABLE_COPY(TtPopUpDrawerStateMachine)

  TtPopUpDrawerWidget* const drawer_;
  TtPopUpDrawer* const main_;    // 主体
  QState* const opening_state_;  // 处于开启
  QState* const opened_state_;   // 已经开启
  QState* const closing_state_;  // 处于关闭
  QState* const closed_state_;   // 已经关闭
  qreal opacity_;                // 透明度
};

//class DrawerController : public QObject {
//  Q_OBJECT
//
// public:
//  DrawerController(QSplitter* splitter, TtPopUpDrawer* drawer,
//                   QWidget* contentWidget, QObject* parent = nullptr)
//      : QObject(parent),
//        splitter_(splitter),
//        drawer_(drawer),
//        contentWidget_(contentWidget),
//        drawerOpenSize_(250) {  // 默认 Drawer 打开大小
//    // 初始化
//    connect(drawer_, &TtPopUpDrawer::willClose, this,
//            &DrawerController::onDrawerClosed);
//  }
//
//  void toggleDrawer() {
//    if (isDrawerOpen()) {
//      closeDrawer();
//    } else {
//      openDrawer();
//    }
//  }
//
//  bool isDrawerOpen() const {
//    return splitter_->sizes()[0] > 0;  // 判断 Drawer 是否可见
//  }
//
//  void openDrawer() {
//    // 恢复 Drawer 的大小
//    QList<int> sizes = splitter_->sizes();
//    sizes[0] = drawerOpenSize_;  // 显示 Drawer
//    sizes[1] =
//        splitter_->width() - drawerOpenSize_;  // 调整 contentWidget 的大小
//    splitter_->setSizes(sizes);
//
//    drawer_->openDrawer();
//  }
//
//  void closeDrawer() {
//    // 记录当前 Drawer 大小
//    drawerOpenSize_ = splitter_->sizes()[0];
//
//    // 隐藏 Drawer
//    QList<int> sizes = splitter_->sizes();
//    sizes[0] = 0;                   // 隐藏 Drawer
//    sizes[1] = splitter_->width();  // contentWidget 占满
//    splitter_->setSizes(sizes);
//
//    drawer_->closeDrawer();
//  }
//
// private slots:
//  void onDrawerClosed() {
//    // 处理 Drawer 关闭逻辑（如果需要额外操作）
//  }
//
// private:
//  QSplitter* splitter_;
//  TtPopUpDrawer* drawer_;
//  QWidget* contentWidget_;
//  int drawerOpenSize_;  // 记录 Drawer 打开的大小
//};
class DrawerController : public QObject {
  Q_OBJECT

 public:
  DrawerController(QSplitter* splitter, TtPopUpDrawer* drawer,
                   QWidget* contentWidget, QObject* parent = nullptr)
      : QObject(parent),
        splitter_(splitter),
        drawer_(drawer),
        contentWidget_(contentWidget),
        drawerOpenSize_(250),      // 默认 Drawer 打开大小
        isDrawerVisible_(false) {  // 初始状态为隐藏
    // 初始化
    connect(drawer_, &TtPopUpDrawer::willClose, this,
            &DrawerController::onDrawerClosed);
  }

  void toggleDrawer() {
    if (isDrawerVisible_) {
      closeDrawer();
    } else {
      openDrawer();
    }
  }

  bool isDrawerOpen() const { return isDrawerVisible_; }

  void openDrawer() {
    if (isDrawerVisible_)
      return;

    // 把 Drawer 添加回 QSplitter
    if (!splitter_->widget(0)) {
      splitter_->insertWidget(0, drawer_);
    }

    // 恢复 Drawer 的大小
    QList<int> sizes = splitter_->sizes();
    sizes[0] = drawerOpenSize_;  // 显示 Drawer
    sizes[1] =
        splitter_->width() - drawerOpenSize_;  // 调整 contentWidget 的大小
    splitter_->setSizes(sizes);

    drawer_->openDrawer();
    isDrawerVisible_ = true;
  }

  void closeDrawer() {
    if (!isDrawerVisible_)
      return;

    // 记录当前 Drawer 大小
    drawerOpenSize_ = splitter_->sizes()[0];

    // 隐藏 Drawer
    QList<int> sizes = splitter_->sizes();
    sizes[0] = 0;                   // 隐藏 Drawer
    sizes[1] = splitter_->width();  // contentWidget 占满
    splitter_->setSizes(sizes);

    drawer_->closeDrawer();

    // 延迟移除 Drawer，确保动画正常完成
    QTimer::singleShot(220, this, [this]() {
      splitter_->widget(0)->setParent(nullptr);  // 从 splitter 中移除 Drawer
    });

    isDrawerVisible_ = false;
  }

  void handleSplitterMoved() {
    // 避免通过拖动分隔栏强制显示 Drawer
    //if (!isDrawerVisible_ && splitter_->sizes()[0] > 0) {
    //  closeDrawer();
    //}
  }

 private slots:
  void onDrawerClosed() {
    // 处理 Drawer 关闭逻辑（如果需要额外操作）
    qDebug() << "Drawer closed";
  }

 private:
  QSplitter* splitter_;
  TtPopUpDrawer* drawer_;
  QWidget* contentWidget_;
  int drawerOpenSize_;    // 记录 Drawer 打开的大小
  bool isDrawerVisible_;  // 用于记录 Drawer 的显示状态
};



}  // namespace Ui

#endif  // !UI_CONTROL_TTPOPUPDRAWER_H
