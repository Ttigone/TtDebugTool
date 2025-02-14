/*****************************************************************/ /**
 * \file   animated_drawer.h
 * \brief  动画抽屉
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_EFFECTS_ANIMATED_DRAWER_H
#define UI_EFFECTS_ANIMATED_DRAWER_H

namespace Ui {

class TtAnimatedDrawer : public QObject {
  Q_OBJECT

  Q_PROPERTY(int animationWidth READ animationWidth WRITE
                 setAnimationWidth)  // 修改属性名称

 public:
  TtAnimatedDrawer(QSplitter* splitter, QWidget* drawer, QWidget* contentWidget,
                   QObject* parent = nullptr)
      : QObject(parent),
        splitter_(splitter),
        drawer_(drawer),
        contentWidget_(contentWidget),
        //animation_(new QPropertyAnimation(this, "drawerWidth", this)),
        animation_(new QPropertyAnimation(this, "animationWidth", this)),
        //drawerWidth_(250),  // 默认初始宽度
        default_width_(250),  // 默认初始宽度
        current_animation_width_(0),
        isDrawerVisible_(false) {
    // 初始化动画
    animation_->setDuration(300);                        // 动画持续时间
    animation_->setEasingCurve(QEasingCurve::OutCubic);  // 缓动曲线

    // 确保动画结束时更新布局
    connect(animation_, &QPropertyAnimation::finished, this,
            &TtAnimatedDrawer::onAnimationFinished);
  }

  int animationWidth() const { return current_animation_width_; }

  void setAnimationWidth(int width) {
    current_animation_width_ = width;
    updateSplitterLayout(width);
    //qDebug() << "Drawer width: " << width;
  }

  void toggleDrawer() {
    if (isDrawerVisible_) {
      closeDrawer();
    } else {
      openDrawer();
    }
  }

  void openDrawer() {
    if (isDrawerVisible_)
      return;

    // 确保drawer在splitter中
    if (splitter_->indexOf(drawer_) < 0) {
      qDebug() << "insert";
      splitter_->insertWidget(0, drawer_);
      drawer_->show();
      contentWidget_->show();
    }

    //// 先设置splitter可见区域
    //QList<int> initSizes;
    //initSizes << 0 << contentWidget_->width();
    //splitter_->setSizes(initSizes);
    updateSplitterLayout(0);

    animation_->stop();
    animation_->setStartValue(0);
    animation_->setEndValue(default_width_);  // 使用固定默认宽度
    animation_->start();

    isDrawerVisible_ = true;

  }

  void closeDrawer() {
    if (!isDrawerVisible_)
      return;

    // 直接记录当前宽度，避免在动画过程中获取错误值
    const int currentWidth = splitter_->sizes().value(0, default_width_);
    //drawerWidth_ = currentWidth;
    animation_->stop();
    animation_->setStartValue(currentWidth);
    animation_->setEndValue(0);
    animation_->start();

    connect(
        animation_, &QPropertyAnimation::finished, this,
        [this]() {
          if (splitter_->indexOf(drawer_) >= 0) {
            drawer_->hide();
            splitter_->refresh();  // 强制刷新布局
          }
        },
        Qt::SingleShotConnection);

    isDrawerVisible_ = false;
  }

 private slots:
  void updateSplitterLayout(int drawerWidth) {
    if (!splitter_ || splitter_->count() < 2)
      return;

    const int total = splitter_->width();
    QList<int> sizes;
    sizes << drawerWidth << (total - drawerWidth);
    splitter_->setSizes(sizes);
  }

  void onAnimationFinished() {
    if (!isDrawerVisible_) {
      qDebug() << "Drawer is now hidden.";
    } else {
      qDebug() << "Drawer is now visible.";
    }
  }

 private:
  QSplitter* splitter_;
  QWidget* drawer_;
  QWidget* contentWidget_;
  QPropertyAnimation* animation_;
  int default_width_;            // 当前 Drawer 的宽度
  int current_animation_width_;  // 当前 Drawer 的宽度
  bool isDrawerVisible_;         // Drawer 的可见状态
};

}  // namespace Ui

#endif  // UI_EFFECTS_ANIMATED_DRAWER_H
