#ifndef UI_CONTROL_TTSCROLLBAR_H
#define UI_CONTROL_TTSCROLLBAR_H

#include <QAbstractScrollArea>
#include <QScrollBar>

#include "ui/ui_pch.h"

namespace Ui {

class TtScrollBarPrivate;

class Tt_EXPORT TtScrollBar : public QScrollBar {
  Q_OBJECT
  Q_Q_CREATE(TtScrollBar)
  Q_PROPERTY_CREATE_Q_H(bool, IsAnimation)
  Q_PROPERTY_CREATE_Q_H(qreal, SpeedLimit)
 public:
  explicit TtScrollBar(QWidget* parent = nullptr);
  explicit TtScrollBar(Qt::Orientation orientation, QWidget* parent = nullptr);
  explicit TtScrollBar(QScrollBar* originScrollBar,
                       QAbstractScrollArea* parent = nullptr);
  ~TtScrollBar();

 signals:
  void rangeAnimationFinished();

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTSCROLLBAR_H
