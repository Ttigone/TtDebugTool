/*****************************************************************/ /**
 * \file   TtPopUpDrawer.h
 * \brief  弹出抽屉窗口
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_CONTROL_TTPOPUPDRAWER_H
#define UI_CONTROL_TTPOPUPDRAWER_H

#include <QStateMachine>

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

}  // namespace Ui

#endif  // !UI_CONTROL_TTPOPUPDRAWER_H
