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

namespace Ui {

class TtAnimatedDrawer : public QObject {
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

 private slots:
  void updateSplitterLayout(int drawerWidth) {
    if (!splitter_ || splitter_->count() < 2)
      return;

    const int total = splitter_->width();
    QList<int> sizes{drawerWidth, total - drawerWidth};
    splitter_->setSizes(sizes);
  }

  void onAnimationFinished() {
    isDrawerVisible_ = targetDrawerVisible_;
    if (!isDrawerVisible_) {
      drawer_->setVisible(false);
    }
    splitter_->update();
    splitter_->widget(0)->updateGeometry();
    splitter_->widget(1)->updateGeometry();
  }

 private:
  QSplitter* splitter_;
  QWidget* drawer_;
  QWidget* contentWidget_;
  QPropertyAnimation* animation_;
  int default_width_;            // 当前 Drawer 的宽度
  int current_animation_width_;  // 当前 Drawer 的宽度
  bool isDrawerVisible_;         // Drawer 的可见状态
  bool targetDrawerVisible_;     // 新增目标状态变量
};

}  // namespace Ui

#endif  // UI_EFFECTS_ANIMATED_DRAWER_H
