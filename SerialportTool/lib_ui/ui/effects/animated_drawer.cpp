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
      // isDrawerVisible_(false) {
      isDrawerVisible_(false),
      targetDrawerVisible_(false) {  // 初始化目标状态
  drawer_->setMinimumWidth(0);
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

void TtAnimatedDrawer::toggleDrawer() {
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

void TtAnimatedDrawer::openDrawer() {
  if (animation_->state() == QAbstractAnimation::Running &&
      targetDrawerVisible_) {
    return;
  }
  // 一开始打开时候的状态
  restoreWidgetStates();

  drawer_->setVisible(true);
  animation_->stop();

  targetDrawerVisible_ = true;

  int startValue = splitter_->sizes().value(0, 0);
  //animation_->setStartValue(0);
  animation_->setStartValue(startValue);
  animation_->setEndValue(default_width_);  // 使用固定默认宽度
  animation_->start();

  // 动画完成后恢复状态
  // connect(
  //     animation_, &QPropertyAnimation::finished, this,
  //     [this]() { restoreWidgetStates(); }, Qt::SingleShotConnection);
}

void TtAnimatedDrawer::closeDrawer() {
  if (animation_->state() == QAbstractAnimation::Running &&
      !targetDrawerVisible_) {
    return;
  }

  // 恢复之前可能未完成的策略修改
  restoreWidgetStates();  // 新增：在关闭前恢复控件状态

  animation_->stop();

  // 跳过的控件
  QSet<QWidget*> skipWidgets;
  // 保存子控件原始尺寸策略，以便恢复
  QList<QPair<QWidget*, QSizePolicy>> originalPolicies;
  QMap<QWidget*, QByteArray> originalLayouts;  // 保存布局信息

  QList<QWidget*> selfButtons = drawer_->findChildren<QWidget*>(
      "TtSpecialDeleteButton");  // 找到 wwww 控件
  for (QWidget* button : selfButtons) {
    skipWidgets.insert(button);
    // 封装控件的子节点
    QList<QWidget*> descendants = button->findChildren<QWidget*>();
    for (QWidget* descendant : descendants) {
      skipWidgets.insert(descendant);
    }
  }

  // 添加 refreshBtn 到跳过列表
  QList<QWidget*> refreshBtns = drawer_->findChildren<QWidget*>("TtSvgButton");
  for (QWidget* btn : refreshBtns) {
    skipWidgets.insert(btn);
  }

  QList<QWidget*> children = drawer_->findChildren<QWidget*>();
  for (QWidget* child : children) {
    originalPolicies.append(qMakePair(child, child->sizePolicy()));
    if (!skipWidgets.contains(child)) {
      QSizePolicy sp = child->sizePolicy();
      sp.setHorizontalPolicy(QSizePolicy::Ignored);
      child->setSizePolicy(sp);
    }
  }

  // 存储原始策略以便恢复
  drawer_->setProperty("originalSizePolicies",
                       QVariant::fromValue(originalPolicies));

  targetDrawerVisible_ = false;
  // 直接记录当前宽度，避免在动画过程中获取错误值
  const int currentWidth = splitter_->sizes().value(0, default_width_);
  default_width_ = currentWidth;
  animation_->setStartValue(currentWidth);
  animation_->setEndValue(0);
  animation_->start();

}

void TtAnimatedDrawer::updateSplitterLayout(int drawerWidth) {
  if (!splitter_ || splitter_->count() < 2)
    return;

  const int total = splitter_->width();
  QList<int> sizes{drawerWidth, total - drawerWidth};
  splitter_->setSizes(sizes);
}

void TtAnimatedDrawer::onAnimationFinished() {
  isDrawerVisible_ = targetDrawerVisible_;
  if (!isDrawerVisible_) {
    // 关闭的状态
    drawer_->setVisible(false);
    QVariant var = drawer_->property("originalSizePolicies");
    if (var.isValid()) {
      auto originalPolicies = var.value<QList<QPair<QWidget*, QSizePolicy>>>();
      for (const auto& pair : originalPolicies) {
        pair.first->setSizePolicy(pair.second);
      }
    }
  } else {
  }

  splitter_->update();
  splitter_->widget(0)->updateGeometry();
  splitter_->widget(1)->updateGeometry();
}

// void TtAnimatedDrawer::saveWidgetStates() {
//   if (!drawer_)
//     return;

//   // 保存主容器的状态
//   originalStates[drawer_] = {drawer_->minimumSize(), drawer_->sizePolicy()};

//   // 保存所有子控件的状态
//   for (QWidget* child : drawer_->findChildren<QWidget*>()) {
//     originalStates[child] = {child->minimumSize(), child->sizePolicy()};
//   }
// }

void TtAnimatedDrawer::restoreWidgetStates() {
  QVariant var = drawer_->property("originalSizePolicies");
  if (var.isValid()) {
    auto originalPolicies = var.value<QList<QPair<QWidget*, QSizePolicy>>>();
    for (const auto& pair : originalPolicies) {
      pair.first->setSizePolicy(pair.second);
    }
    drawer_->setProperty("originalSizePolicies",
                         QVariant());  // 清除保存的状态
  }
}

}  // namespace Ui
