/*****************************************************************/ /**
 * \file   TtMaskWidget.h
 * \brief  遮罩界面
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_CONTROL_TTMASKWIDGET_H
#define UI_CONTROL_TTMASKWIDGET_H

QT_BEGIN_NAMESPACE
class QObject;
class QWidget;
class QTimer;
QT_END_NAMESPACE

#include "ui/ui_pch.h"

#include <QGraphicsOpacityEffect>
#include <QPointer>

namespace Ui {

// class TtMaskWidgetPrivate;

class Tt_EXPORT TtMaskWidget : public QObject {
  Q_OBJECT
 public:
  explicit TtMaskWidget(QWidget* parent = nullptr);
  ~TtMaskWidget();

  void initMaskWidget();
  void setMaskColor(const QColor& color, qreal opacity = 0.7);
  void show(QWidget* childWidget = nullptr);

  void setFadeDuration(int ms) { fade_duration_ = ms; }
  void setReusable(bool reusable) { reusable_ = reusable; }
  bool isReusable() const { return reusable_; }

  void resetChildWidget();

  bool isVisible() const { return mask_widget_ && mask_widget_->isVisible(); }
  // void closeMask() { hide(); }

 signals:
  void aboutToClose();

 public slots:
  void handleCloseRequest() { hide(); }

 protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

 private slots:
  void hide();
  void handleChildDestroyed(QObject* obj);  // 子控件销毁时的处理

 private:
  void updateMaskGeometry();
  void updateChildPosition();
  void startFadeAnimation(bool isShow);
  void processChildWidget(QWidget* childWidget);
  QSize calculateChildSize(const QSize& maskSize) const;

  QWidget* parent_widget_;                   // 底层父控件
  QWidget* mask_widget_;                     // 遮罩层
  QWidget* child_widget_;                    // 子控件（由外部传入）
  QPointer<QGraphicsOpacityEffect> effect_;  // 透明效果
  QPointer<QPropertyAnimation> fade_animation_;  // 动画对象
  int fade_duration_ = 220;                      // 默认动画时长
  bool reusable_;                                // 是否可以重用子控件
};

}  // namespace Ui

#endif  // UI_CONTROL_TTMASKWIDGET_H
