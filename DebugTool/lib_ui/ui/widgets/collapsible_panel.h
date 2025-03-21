#ifndef UI_WIDGETS_COLLAPSIBLE_PANEL_H
#define UI_WIDGETS_COLLAPSIBLE_PANEL_H

#include <QEvent>
#include <QIcon>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QToolBox>
#include <QTreeWidget>
#include <QWidget>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT DrawerButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(qreal arrowRotation READ arrowRotation WRITE setArrowRotation)

 public:
  explicit DrawerButton(const QString &title, QWidget *parent = nullptr);

  qreal arrowRotation() const;
  void setArrowRotation(qreal rotation);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void initializeArrow();
  qreal m_arrowRotation; // 箭头旋转角度
  // QPolygon m_arrowPolygon;
  QPainterPath m_arrowPath;
};

class Tt_EXPORT Drawer : public QWidget {
  Q_OBJECT
 public:
  Drawer(const QString &title, QWidget *contentWidget, QWidget *parent = nullptr);

protected:
 bool eventFilter(QObject* obj, QEvent* event) override;
 void resizeEvent(QResizeEvent* event) override;

private slots:
  void toggle();

private:
  void initializeLayout(const QString& title);
  void initializeAnimations();

  void connectSignals();

  void openDrawer();
  void closeDrawer();

  void startAnimation(int start, int end);
  void startArrowAnimation(qreal start, qreal end);

  void updateAnimationEndValue();

  int contentHeight() const;

private:
 // QPushButton *toggleButton;
 DrawerButton* toggleButton;
 QWidget* contentWidget;
 QWidget* wrapperWidget;  // 包装 contentWidget 的容器
 QPropertyAnimation* animation;
 QPropertyAnimation* arrowAnimation;
 bool isOpen;
};

} // namespace Ui


#endif  // COLLAPSIBLE_PANEL_H
