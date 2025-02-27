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

void TtAnimatedDrawer::openDrawer() {
  if (animation_->state() == QAbstractAnimation::Running &&
      targetDrawerVisible_) {
    return;
  }

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
  animation_->stop();

  // 保存子控件原始尺寸策略，以便恢复
  QList<QPair<QWidget*, QSizePolicy>> originalPolicies;
  QMap<QWidget*, QByteArray> originalLayouts;  // 保存布局信息

  QList<QWidget*> selfButtons = drawer_->findChildren<QWidget*>(
      "TtSpecialDeleteButton");  // 找到 wwww 控件
  QSet<QWidget*> skipWidgets;
  for (QWidget* button : selfButtons) {
    skipWidgets.insert(button);
    // 封装控件的子节点
    QList<QWidget*> descendants = button->findChildren<QWidget*>();
    for (QWidget* descendant : descendants) {
      skipWidgets.insert(descendant);
    }
  }

  QList<QWidget*> children = drawer_->findChildren<QWidget*>();
  for (QWidget* child : children) {
    originalPolicies.append(qMakePair(child, child->sizePolicy()));
    // 允许水平方向自由缩放
    QSizePolicy sp = child->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Ignored);
    if (skipWidgets.contains(child)) {
      // qDebug() << "跳过 wwww 或其子控件";
      continue;
    }
    child->setSizePolicy(sp);
    // child->setMinimumWidth(0);
    // child->setMaximumWidth(16777215);  // 重置为默认最大值
  }

  // 存储原始策略以便恢复
  QVariant var;
  var.setValue(originalPolicies);
  drawer_->setProperty("originalSizePolicies", var);
  // qDebug() << originalPolicies;

  // // 禁用布局更新
  // if (drawer_->layout()) {
  //   drawer_->layout()->setEnabled(false);
  // }

  targetDrawerVisible_ = false;

  // 直接记录当前宽度，避免在动画过程中获取错误值
  const int currentWidth = splitter_->sizes().value(0, default_width_);
  default_width_ = currentWidth;
  animation_->setStartValue(currentWidth);
  animation_->setEndValue(0);
  animation_->start();

}

void TtAnimatedDrawer::onAnimationFinished() {
  isDrawerVisible_ = targetDrawerVisible_;
  if (!isDrawerVisible_) {
    // 关闭的状态
    // qDebug() << "guanbi";
    drawer_->setVisible(false);
    QVariant var = drawer_->property("originalSizePolicies");
    if (var.isValid()) {
      auto originalPolicies = var.value<QList<QPair<QWidget*, QSizePolicy>>>();
      for (const auto& pair : originalPolicies) {
        pair.first->setSizePolicy(pair.second);
      }
    }

    // // 恢复布局更新
    // if (drawer_->layout()) {
    //   drawer_->layout()->setEnabled(true);
    // }

  } else {
    // qDebug() << "dakai";
  }

  splitter_->update();
  splitter_->widget(0)->updateGeometry();
  splitter_->widget(1)->updateGeometry();
}

}  // namespace Ui
