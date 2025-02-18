#include "ui/effects/animated_drawer.h"

namespace Ui {
TtAnimatedDrawer::TtAnimatedDrawer(QSplitter* splitter, QWidget* drawer,
                                   QWidget* contentWidget, QObject* parent)
    : QObject(parent),
      splitter_(splitter),
      drawer_(drawer),
      contentWidget_(contentWidget),
      animation_(new QPropertyAnimation(this, "animationWidth", this)),
      default_width_(250),  // 默认初始宽度
      current_animation_width_(0),
      //isDrawerVisible_(false) {
      isDrawerVisible_(false),
      targetDrawerVisible_(false) {  // 初始化目标状态
  // 初始化动画
  animation_->setDuration(220);                        // 动画持续时间
  animation_->setEasingCurve(QEasingCurve::OutCubic);  // 缓动曲线

  const int total = splitter_->width();
  QList<int> sizes;
  sizes << 0 << total;
  splitter_->setSizes(sizes);
  drawer->setVisible(false);

  // 确保动画结束时更新布局
  connect(animation_, &QPropertyAnimation::finished, this,
          &TtAnimatedDrawer::onAnimationFinished);
}

void TtAnimatedDrawer::openDrawer() {
  drawer_->setVisible(true);
  animation_->stop();

  targetDrawerVisible_ = true;

  int startValue = splitter_->sizes().value(0, 0);
  //animation_->setStartValue(0);
  animation_->setStartValue(startValue);
  animation_->setEndValue(default_width_);  // 使用固定默认宽度
  animation_->start();

}

void TtAnimatedDrawer::closeDrawer() {
  animation_->stop();

  targetDrawerVisible_ = false;

  // 直接记录当前宽度，避免在动画过程中获取错误值
  const int currentWidth = splitter_->sizes().value(0, default_width_);
  default_width_ = currentWidth;
  animation_->setStartValue(currentWidth);
  animation_->setEndValue(0);
  animation_->start();

}

}  // namespace Ui
