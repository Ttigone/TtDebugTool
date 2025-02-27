/*****************************************************************/ /**
 * \file   animated_drawer.h
 * \brief  动画抽屉
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_EFFECTS_ANIMATED_DRAWER_H
#define UI_EFFECTS_ANIMATED_DRAWER_H

#include <QSplitter>
#include <QPropertyAnimation>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtAnimatedDrawer : public QObject {
  Q_OBJECT

  Q_PROPERTY(int animationWidth READ animationWidth WRITE
                 setAnimationWidth)  // 修改属性名称

 public:
  TtAnimatedDrawer(QSplitter* splitter, QWidget* drawer, QWidget* contentWidget,
                   QObject* parent = nullptr);
  ~TtAnimatedDrawer() = default;
  int animationWidth() const { return current_animation_width_; }

  void setAnimationWidth(int width) {
    current_animation_width_ = width;
    updateSplitterLayout(width);
  }

  void toggleDrawer() {
    if (animation_->state() == QPropertyAnimation::Running) {
      // 如果动画正在运行，反转方向
      if (targetDrawerVisible_) {
        closeDrawer();
      } else {
        openDrawer();
      }
    } else {
      // 否则根据当前状态切换
      if (isDrawerVisible_) {
        closeDrawer();
      } else {
        openDrawer();
      }
    }
  }

  void openDrawer();
  void closeDrawer();

  bool isDrawerVisible() const { return isDrawerVisible_; }
  bool targetDrawerVisible() const { return targetDrawerVisible_; }

 private slots:
  void updateSplitterLayout(int drawerWidth) {
    if (!splitter_ || splitter_->count() < 2)
      return;

    const int total = splitter_->width();
    QList<int> sizes{drawerWidth, total - drawerWidth};
    splitter_->setSizes(sizes);
  }

  void onAnimationFinished();

 private:
  void saveWidgetStates() {
    if (!drawer_)
      return;

    // 保存主容器的状态
    originalStates[drawer_] = {drawer_->minimumSize(), drawer_->sizePolicy()};

    // 保存所有子控件的状态
    for (QWidget* child : drawer_->findChildren<QWidget*>()) {
      originalStates[child] = {child->minimumSize(), child->sizePolicy()};
    }
  }

  void restoreWidgetStates() {
    if (!drawer_)
      return;

    // 恢复所有保存的状态
    for (auto it = originalStates.begin(); it != originalStates.end(); ++it) {
      if (QWidget* widget = it.key()) {
        widget->setMinimumSize(it.value().minimumSize);
        widget->setSizePolicy(it.value().sizePolicy);
      }
    }

    // 强制更新布局
    if (drawer_->layout()) {
      drawer_->layout()->activate();
    }
    drawer_->updateGeometry();
  }

  QSplitter* splitter_;
  QWidget* drawer_;
  QWidget* contentWidget_;
  QPropertyAnimation* animation_;
  int default_width_;            // 当前 Drawer 的宽度
  int current_animation_width_;  // 当前 Drawer 的宽度
  bool isDrawerVisible_;         // Drawer 的可见状态
  bool targetDrawerVisible_;     // 新增目标状态变量

  // 保存子控件的原始尺寸策略和最小尺寸
  struct WidgetState {
    QSize minimumSize;
    QSizePolicy sizePolicy;
  };
  QHash<QWidget*, WidgetState> originalStates;
};

}  // namespace Ui

#endif  // UI_EFFECTS_ANIMATED_DRAWER_H
