#include "ui/control/TtMaskWidget.h"
#include <qcoreevent.h>
#include <qnamespace.h>
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
  // 也是在父对象之上
  mask_widget_ = new QWidget(parent_widget_);
  mask_widget_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  // 干扰背景色绘制
  // mask_widget_->setAttribute(Qt::WA_TranslucentBackground, true);
  mask_widget_->hide();

  // mask_widget_ 设置为阴影界面
  // 设置背景色
  QPalette palette = mask_widget_->palette();
  // palette.setColor(QPalette::Window, Qt::black);
  palette.setColor(QPalette::Window, QColor(0, 0, 0, 128));
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
  processChildWidget(childWidget);
  // mask_widget_->stackUnder(parent_widget_);
  updateMaskGeometry();
  updateChildPosition();

  mask_widget_->show();
  if (child_widget_) {
    child_widget_->show();
    child_widget_->raise();
  }
  startFadeAnimation(true);
}

void TtMaskWidget::resetChildWidget() {
  if (child_widget_) {
    child_widget_->disconnect(this);
    child_widget_->deleteLater();
    child_widget_ = nullptr;
  }
}

void TtMaskWidget::hide() {
  // if (!mask_widget_ || !mask_widget_->isVisible()) {
  //   return;
  // }
  // startFadeAnimation(false);
  if (!mask_widget_ || !mask_widget_->isVisible()) {
    return;
  }
  qDebug() << "Starting hide anmiation";
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
    qDebug() << "Child widget destroyed externally";
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
    fade_animation_->setStartValue(effect_->opacity());
    fade_animation_->setEndValue(0.0);

    connect(fade_animation_, &QPropertyAnimation::finished, this, [this] {
      mask_widget_->hide();
      emit aboutToClose();
      if (child_widget_) {
        // 当前存在之前设置的显示子控件
        QWidget* originalParent =
            child_widget_->property("originalParent").value<QWidget*>();
        if (originalParent) {
          if (reusable_) {
            // 重用模式：恢复原始父窗口
            qDebug() << "Reusing child widget, restoring parent";
            if (originalParent) {
              disconnect(child_widget_, &QObject::destroyed, this,
                         &TtMaskWidget::handleChildDestroyed);
              child_widget_->setParent(originalParent);
              child_widget_->hide();
            }
          } else {
            // 非重用模式：安全删除子窗口
            qDebug() << "Non-reusable mode, deleting child widget";
            disconnect(child_widget_, &QObject::destroyed, this,
                       &TtMaskWidget::handleChildDestroyed);
            child_widget_->deleteLater();
            child_widget_ = nullptr;
          }
        }
      }
    });
  }

  fade_animation_->start();
}

// void TtMaskWidget::processChildWidget(QWidget* childWidget) {
//   if (!childWidget) {
//     qWarning() << "Attempted to show a null child widget!";
//     return;
//   }
//   if (childWidget == child_widget_) {
//     qDebug() << "Child widget is already set!";
//     return;
//   }
//   if (child_widget_) {
//     if (reusable_) {
//       child_widget_->hide();
//     } else {
//       // 非重用模式
//       // 断开连接以避免触发handleChildDestroyed
//       disconnect(child_widget_, &QObject::destroyed, this,
//                  &TtMaskWidget::handleChildDestroyed);
//       child_widget_->hide();
//       child_widget_->deleteLater();
//       // child_widget_->deleteLater();  // 删除
//     }
//     child_widget_ = nullptr;
//   }
//   child_widget_ = childWidget;
//   if (child_widget_) {
//     child_widget_->setParent(mask_widget_);
//     child_widget_->show();
//     connect(child_widget_, &QObject::destroyed, this,
//             &TtMaskWidget::handleChildDestroyed);
//   }
// }

void TtMaskWidget::processChildWidget(QWidget* childWidget) {
  // 处理显示的窗口
  if (!childWidget) {
    qWarning() << "Attempted to show a null child widget!";
    return;
  }

  // 保存子窗口的原始父窗口
  QWidget* originalParent = childWidget->parentWidget();

  // 如果当前已有显示的子窗口，处理它
  if (child_widget_ && child_widget_ != childWidget) {
    // 断开之前的信号连接
    disconnect(child_widget_, &QObject::destroyed, this,
               &TtMaskWidget::handleChildDestroyed);

    if (reusable_) {
      // 重用模式：将子窗口还原到原始父窗口并隐藏
      child_widget_->hide();
      // 保持原有的父窗口，不删除
    } else {
      // 非重用模式：隐藏子窗口并标记为稍后删除
      child_widget_->hide();
      // 不立即删除，避免在动画过程中访问已删除的窗口
      // 将删除操作延迟到动画结束
    }
  }

  // 设置新的子窗口
  child_widget_ = childWidget;

  // 保存原始父窗口信息，用于恢复
  childWidget->setProperty("originalParent",
                           QVariant::fromValue(originalParent));

  // 设置到mask_widget_
  childWidget->setParent(mask_widget_);

  // 连接销毁信号，以便跟踪窗口是否被外部删除
  connect(childWidget, &QObject::destroyed, this,
          &TtMaskWidget::handleChildDestroyed,
          Qt::UniqueConnection);  // 确保连接唯一
}

QSize TtMaskWidget::calculateChildSize(const QSize& maskSize) const {
  constexpr int minWidth = 400;
  constexpr int minHeight = 300;
  return QSize(qMax(minWidth, maskSize.width() * 2 / 3),
               qMax(minHeight, maskSize.height() * 3 / 4));
}

}  // namespace Ui
