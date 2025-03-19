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

  void toggleDrawer();

  void openDrawer();
  void closeDrawer();

  bool isDrawerVisible() const { return isDrawerVisible_; }
  bool targetDrawerVisible() const { return targetDrawerVisible_; }

 private slots:
  void updateSplitterLayout(int drawerWidth);

  void onAnimationFinished();

 private:
  // void saveWidgetStates();

  void restoreWidgetStates();

  QSplitter* splitter_;
  QWidget* drawer_;
  QWidget* contentWidget_;
  QPropertyAnimation* animation_;
  int default_width_;            // 当前 Drawer 的宽度
  int current_animation_width_;  // 当前 Drawer 的宽度
  bool isDrawerVisible_;         // Drawer 的可见状态
  bool targetDrawerVisible_;     // 新增目标状态变量

  // // 保存子控件的原始尺寸策略和最小尺寸
  // struct WidgetState {
  //   QSize minimumSize;
  //   QSizePolicy sizePolicy;
  // };
  // QHash<QWidget*, WidgetState> originalStates;
};

}  // namespace Ui

#endif  // UI_EFFECTS_ANIMATED_DRAWER_H
