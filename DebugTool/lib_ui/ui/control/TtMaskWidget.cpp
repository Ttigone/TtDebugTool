#include "ui/control/TtMaskWidget.h"
#include <qcoreevent.h>
#include <qpropertyanimation.h>
#include "ui/control/TtMaskWidget_p.h"

#include <QApplication>
#include <QEvent>

namespace Ui {

TtMaskWidget::TtMaskWidget(QWidget* parent)
    : QObject(parent),
      parent_widget_(parent),
      mask_widget_(nullptr),
      child_widget_(nullptr),
      reusable_(false) {
  // fade_duration_(nullptr) {
  if (!parent_widget_) {
    qWarning() << "Parent widget must be provided!";
    return;
    // parent_widget_ = QApplication::activeWindow();  // 备用方案
    // for (QWidget* widget : QApplication::allWidgets()) {
    //   if (widget->objectName() == "CentralWidget") {
    //     parent_widget_ = widget;
    //     break;
    //   }
    // }
  }

  // if (parent_widget_) {
  //   // parent_widget_ 是 mqttwindow 类
  //   initMaskWidget();
  // } else {
  //   qWarning() << "Parent widget not found!";
  // }
  initMaskWidget();
}

TtMaskWidget::~TtMaskWidget() {
  qDebug() << "TtMaskWidget delete";
  if (mask_widget_) {
    mask_widget_->deleteLater();
  }
}

void TtMaskWidget::initMaskWidget() {
  mask_widget_ = new QWidget(parent_widget_);
  mask_widget_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  mask_widget_->hide();

  // 设置背景色
  QPalette palette = mask_widget_->palette();
  palette.setColor(QPalette::Window, Qt::black);
  mask_widget_->setAutoFillBackground(true);
  mask_widget_->setPalette(palette);

  // 透明效果
  effect_ = new QGraphicsOpacityEffect(mask_widget_);
  effect_->setOpacity(0.7);
  mask_widget_->setGraphicsEffect(effect_);

  parent_widget_->installEventFilter(this);
  mask_widget_->setGeometry(parent_widget_->rect());
}

void TtMaskWidget::setMaskColor(const QColor& color, qreal opacity) {
  QPalette palette = mask_widget_->palette();
  palette.setColor(QPalette::Window, color);
  mask_widget_->setPalette(palette);

  if (effect_) {
    effect_->setOpacity(opacity);
  }
}

void TtMaskWidget::show(QWidget* childWidget) {
  if (!parent_widget_ || !mask_widget_) {
    qWarning() << "Mask widget not initialized";
    return;
  }

  mask_widget_->raise();
  // mask_widget_->stackUnder(parent_widget_);

  updateMaskGeometry();
  updateChildPosition();
  mask_widget_->show();
  if (child_widget_) {
    child_widget_->show();
  }
  startFadeAnimation(true);

  // processChildWidget(childWidget);
  // // 立即执行布局计算
  // QMetaObject::invokeMethod(
  //     this,
  //     [this]() {
  //       updateMaskGeometry();
  //       updateChildPosition();
  //       mask_widget_->show();
  //       if (child_widget_) {
  //         child_widget_->show();
  //       }
  //       startFadeAnimation(true);
  //     },
  //     Qt::QueuedConnection);
}

void TtMaskWidget::resetChildWidget() {
  if (child_widget_) {
    child_widget_->disconnect(this);
    child_widget_->deleteLater();
    child_widget_ = nullptr;
  }
}

void TtMaskWidget::hide() {
  if (!mask_widget_ || !mask_widget_->isVisible()) {
    return;
  }
  startFadeAnimation(false);
}

bool TtMaskWidget::eventFilter(QObject* obj, QEvent* event) {
  // if (obj == parent_widget_ && event->type() == QEvent::Resize) {
  //   updateMaskGeometry();
  // }
  if (obj == parent_widget_ && event->type() == QEvent::Resize ||
      event->type() == QEvent::Move || event->type() == QEvent::Show) {
    updateMaskGeometry();
  }
  return QObject::eventFilter(obj, event);
}

void TtMaskWidget::handleChildDestroyed(QObject* obj) {
  if (obj == child_widget_) {
    child_widget_ = nullptr;
  }
}

void TtMaskWidget::updateMaskGeometry() {
  if (parent_widget_ && mask_widget_) {
    mask_widget_->setGeometry(parent_widget_->rect());
    updateChildPosition();
  }
}

void TtMaskWidget::updateChildPosition() {
  if (!child_widget_) {
    return;
  }

  const QSize maskSize = mask_widget_->size();
  const QSize childSize = calculateChildSize(maskSize);

  child_widget_->resize(childSize);
  child_widget_->move((maskSize.width() - childSize.width()) / 2,
                      (maskSize.height() - childSize.height()) / 2);
  // child_widget_->updateGeometry();
}

void TtMaskWidget::startFadeAnimation(bool isShow) {
  if (!mask_widget_) {
    qWarning() << "Mask widget is null!";
    return;
  }
  // if (!effect_ || effect_->parent() != mask_widget_) {
  //   effect_ = new QGraphicsOpacityEffect(mask_widget_);
  //   mask_widget_->setGraphicsEffect(effect_);
  // }
  // if (!effect_->property("opacity").isValid()) {
  //   qCritical() << "Opacity property not found!";
  //   return;
  // }

  // QPropertyAnimation* anim = new QPropertyAnimation(effect_, "opacity");
  // anim->setDuration(fade_duration_);

  // if (isShow) {
  //   effect_->setOpacity(0.0);
  //   mask_widget_->show();
  //   anim->setStartValue(0.0);
  //   anim->setEndValue(1.0);
  // } else {
  //   anim->setStartValue(1.0);
  //   anim->setEndValue(0.0);

  //   connect(anim, &QPropertyAnimation::finished, [this] {
  //     mask_widget_->hide();
  //     emit aboutToClose();
  //     if (child_widget_ && !reusable_) {
  //       child_widget_->deleteLater();
  //       child_widget_ = nullptr;
  //     }
  //   });
  // }

  // anim->start(QAbstractAnimation::DeleteWhenStopped);
  // 懒加载模式
  if (!fade_animation_) {
    fade_animation_ = new QPropertyAnimation(effect_, "opacity", this);
    fade_animation_->setDuration(fade_duration_);
  }
  fade_animation_->stop();
  fade_animation_->disconnect();

  if (isShow) {
    effect_->setOpacity(0.0);
    mask_widget_->show();
    fade_animation_->setStartValue(0.0);
    fade_animation_->setEndValue(1.0);
  } else {
    fade_animation_->setStartValue(1.0);
    fade_animation_->setEndValue(0.0);

    connect(fade_animation_, &QPropertyAnimation::finished, this, [this] {
      mask_widget_->hide();
      emit aboutToClose();
      if (child_widget_ && !reusable_) {
        child_widget_->deleteLater();
        child_widget_ = nullptr;
      }
    });
  }

  fade_animation_->start();
}

void TtMaskWidget::processChildWidget(QWidget* childWidget) {
  if (childWidget == child_widget_) {
    return;
  }
  if (child_widget_) {
    if (reusable_) {
      child_widget_->hide();
    } else {
      child_widget_->deleteLater();  // 删除
    }
  }
  child_widget_ = childWidget;
  if (child_widget_) {
    child_widget_->setParent(mask_widget_);
    child_widget_->show();
    connect(child_widget_, &QObject::destroyed, this,
            &TtMaskWidget::handleChildDestroyed);
    // if (auto* closable = qobject_cast<QWidget*>(child_widget_)) {
    //   disconnect(closable, SIGNAL(closed()), this,
    //              SLOT(hide()));  // 确保断开旧连接
    //   connect(closable, SIGNAL(closed()), this, SLOT(hide()));
    // }
  }
}

QSize TtMaskWidget::calculateChildSize(const QSize& maskSize) const {
  constexpr int minWidth = 400;
  constexpr int minHeight = 300;
  return QSize(qMax(minWidth, maskSize.width() * 2 / 3),
               qMax(minHeight, maskSize.height() * 3 / 4));
}

}  // namespace Ui
