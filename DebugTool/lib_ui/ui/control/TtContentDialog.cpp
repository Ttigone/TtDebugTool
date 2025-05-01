#include "TtContentDialog.h"
#include "TtContentDialog_p.h"

#include <ui/control/TtTextButton.h>
#include <QApplication>
#include <QCloseEvent>
#include <QScreen>

namespace Ui {

TtContentDialog::TtContentDialog(QWidget* parent)
    : Ui::TtContentDialog(LayoutSelection::TWO_OPTIONS, parent) {}

TtContentDialog::TtContentDialog(LayoutSelection layout, QWidget* parent)
    : QDialog(parent), d_ptr(new TtContentDialogPrivate) {
  Q_D(TtContentDialog);
  // 透明背景
  setAttribute(Qt::WA_TranslucentBackground);
  d->q_ptr = this;
  QList<QWidget*> widgetList = QApplication::topLevelWidgets();
  QWidget* mainWindow = nullptr;
  for (auto widget : widgetList) {
    if (widget->property("TtBaseClassName").toString() == "TtMainWindow") {
      mainWindow = widget;
      break;
    }
  }
  if (mainWindow) {
    d->main_window_ = mainWindow;  // 保存父窗口
    d->_shadowWidget = new QWidget(mainWindow);
    d->_shadowWidget->move(0, 0);
    d->_shadowWidget->setFixedSize(mainWindow->size());
    d->_shadowWidget->setObjectName("TtShadowWidget");
    d->_shadowWidget->setStyleSheet(
        "#TtShadowWidget{background-color:rgba(0,0,0,90);}");
    d->_shadowWidget->setVisible(true);

    mainWindow->installEventFilter(this);
  }
  resize(400, height());
  // setWindowFlags((window()->windowFlags()) | Qt::WindowMinimizeButtonHint |
  //                Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setWindowFlags((window()->windowFlags()) | Qt::WindowMinimizeButtonHint |
                 Qt::FramelessWindowHint);
  // #if (QT_VERSION == QT_VERSION_CHECK(6, 5, 3) || \
//      QT_VERSION == QT_VERSION_CHECK(6, 6, 0))
  //   setWindowModality(Qt::ApplicationModal);
  //   setWindowFlags((window()->windowFlags()) | Qt::WindowMinimizeButtonHint |
  //                  Qt::FramelessWindowHint);
  //   installEventFilter(this);
  //   createWinId();
  //   setShadow((HWND)winId());
  // #endif
  // 安装原始过滤器
  // QGuiApplication::instance()->installNativeEventFilter(this);
  // 堆分配时自动删除
  // setAttribute(Qt::WA_DeleteOnClose);

  if (layout == LayoutSelection::THREE_OPTIONS) {
    d->_leftButton = new TtTextButton("cancel", this);
    d->_leftButton->setMinimumSize(0, 0);
    d->_leftButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_leftButton->setFixedHeight(38);
    d->_leftButton->setBorderRadius(6);
    connect(d->_leftButton, &QPushButton::clicked, this, [this]() {
      Q_EMIT leftButtonClicked();
      onLeftButtonClicked();
      // close();
    });

    d->_middleButton = new TtTextButton("minimum", this);
    d->_middleButton->setMinimumSize(0, 0);
    d->_middleButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_middleButton->setFixedHeight(38);
    d->_middleButton->setBorderRadius(6);
    connect(d->_middleButton, &TtTextButton::clicked, this, [this]() {
      Q_EMIT middleButtonClicked();
      onMiddleButtonClicked();
      // close();
    });

    d->_rightButton = new TtTextButton("exit", this);
    d->_rightButton->setLightDefaultColor(QColor(0x00, 0x66, 0xB4));
    d->_rightButton->setLightHoverColor(QColor(0x00, 0x70, 0xC6));
    d->_rightButton->setLightPressColor(QColor(0x00, 0x7A, 0xD8));
    d->_rightButton->setLightTextColor(Qt::white);
    d->_rightButton->setDarkDefaultColor(QColor(0x4C, 0xA0, 0xE0));
    d->_rightButton->setDarkHoverColor(QColor(0x45, 0x91, 0xCC));
    d->_rightButton->setDarkPressColor(QColor(0x3F, 0x85, 0xBB));
    d->_rightButton->setDarkTextColor(Qt::black);
    d->_rightButton->setMinimumSize(0, 0);
    d->_rightButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_rightButton->setFixedHeight(38);
    d->_rightButton->setBorderRadius(6);
    connect(d->_rightButton, &TtTextButton::clicked, this, [this]() {
      Q_EMIT rightButtonClicked();
      onRightButtonClicked();
      // close();
    });

    d->_centralWidget = new QWidget(this);
    QHBoxLayout* centralVLayout = new QHBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(9, 15, 9, 20);
    QLabel iconLabel;
    iconLabel.setPixmap(QPixmap(":/icon/icon/info-circle.svg"));
    d->content_ = new QLabel();
    centralVLayout->addWidget(&iconLabel);
    centralVLayout->addWidget(d->content_);
    centralVLayout->addStretch();

    d->_mainLayout = new QVBoxLayout(this);
    d->_buttonLayout = new QHBoxLayout();
    d->_buttonLayout->addWidget(d->_leftButton);
    d->_buttonLayout->addWidget(d->_middleButton);
    d->_buttonLayout->addWidget(d->_rightButton);
    d->_mainLayout->addWidget(d->_centralWidget);
    d->_mainLayout->addLayout(d->_buttonLayout);

    // d->_themeMode = ElaApplication::getInstance()->getThemeMode();
    // connect(ElaApplication::getInstance(), &ElaApplication::themeModeChanged,
    //         this, [=](ElaApplicationType::ThemeMode themeMode) {
    //           d->_themeMode = themeMode;
    //         });
  } else if (layout == LayoutSelection::TWO_OPTIONS) {
    d->_leftButton = new TtTextButton("cancel", this);
    d->_leftButton->setMinimumSize(0, 0);
    d->_leftButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_leftButton->setFixedHeight(38);
    d->_leftButton->setBorderRadius(6);

    // connect(d->_leftButton, &TtTextButton::clicked, this, [=]() {
    connect(d->_leftButton, &TtTextButton::clicked, this, [this]() {
      Q_EMIT leftButtonClicked();
      onLeftButtonClicked();
      close();
    });

    d->_rightButton = new TtTextButton("exit", this);
    d->_rightButton->setLightDefaultColor(QColor(0x00, 0x66, 0xB4));
    d->_rightButton->setLightHoverColor(QColor(0x00, 0x70, 0xC6));
    d->_rightButton->setLightPressColor(QColor(0x00, 0x7A, 0xD8));
    d->_rightButton->setLightTextColor(Qt::white);
    d->_rightButton->setDarkDefaultColor(QColor(0x4C, 0xA0, 0xE0));
    d->_rightButton->setDarkHoverColor(QColor(0x45, 0x91, 0xCC));
    d->_rightButton->setDarkPressColor(QColor(0x3F, 0x85, 0xBB));
    d->_rightButton->setDarkTextColor(Qt::black);
    d->_rightButton->setMinimumSize(0, 0);
    d->_rightButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_rightButton->setFixedHeight(38);
    d->_rightButton->setBorderRadius(6);
    connect(d->_rightButton, &TtTextButton::clicked, this, [=]() {
      Q_EMIT rightButtonClicked();
      onRightButtonClicked();
      close();
    });

    d->_centralWidget = new QWidget(this);

    QHBoxLayout* centralVLayout = new QHBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(9, 15, 9, 20);
    QLabel iconLabel;
    iconLabel.setPixmap(QPixmap(":/icon/icon/info-circle.svg"));
    d->content_ = new QLabel();
    centralVLayout->addWidget(&iconLabel);
    centralVLayout->addWidget(d->content_);
    centralVLayout->addStretch();

    d->_mainLayout = new QVBoxLayout(this);
    d->_buttonLayout = new QHBoxLayout();
    d->_buttonLayout->addWidget(d->_leftButton);
    d->_buttonLayout->addWidget(d->_rightButton);
    d->_mainLayout->addWidget(d->_centralWidget);
    d->_mainLayout->addLayout(d->_buttonLayout);

    // d->_themeMode = ElaApplication::getInstance()->getThemeMode();
    // connect(ElaApplication::getInstance(), &ElaApplication::themeModeChanged,
    //         this, [=](ElaApplicationType::ThemeMode themeMode) {
    //           d->_themeMode = themeMode;
    //         });
  }
  if (d->main_window_) {
    QTimer::singleShot(0, this, [=]() {
      setFixedWidth(400);
      adjustPosition();
    });
  }
}

TtContentDialog::~TtContentDialog() {
  qDebug() << "contentdialog delete";
  Q_D(TtContentDialog);
  //   QGuiApplication::instance()->removeNativeEventFilter(this);
  // #if (QT_VERSION == QT_VERSION_CHECK(6, 5, 3) || \
//      QT_VERSION == QT_VERSION_CHECK(6, 6, 0))
  //   removeEventFilter(this);
  // #endif

  if (animation_) {
    animation_->stop();
    delete animation_;
    animation_ = nullptr;
  }
  // 查找并停止所有子动画
  const auto animations = findChildren<QPropertyAnimation*>();
  for (auto anim : animations) {
    anim->stop();
    anim->deleteLater();
  }

  if (d->_shadowWidget) {
    delete d->_shadowWidget;
  }
  if (d->main_window_) {
    d->main_window_->removeEventFilter(this);
  }
  qDebug() << "test";
}

void TtContentDialog::onLeftButtonClicked() {}

void TtContentDialog::onMiddleButtonClicked() {}

void TtContentDialog::onRightButtonClicked() {}

void TtContentDialog::setCentralWidget(QWidget* centralWidget) {
  Q_D(TtContentDialog);
  d->_mainLayout->takeAt(0);
  d->_mainLayout->takeAt(0);
  delete d->_centralWidget;
  d->_mainLayout->addWidget(centralWidget);
  d->_mainLayout->addLayout(d->_buttonLayout);
}

void TtContentDialog::setLeftButtonText(const QString& text) {
  Q_D(TtContentDialog);
  d->_leftButton->setText(text);
}

void TtContentDialog::setMiddleButtonText(const QString& text) {
  Q_D(TtContentDialog);
  d->_middleButton->setText(text);
}

void TtContentDialog::setRightButtonText(const QString& text) {
  Q_D(TtContentDialog);
  d->_rightButton->setText(text);
}

void Ui::TtContentDialog::setCenterText(const QString& text) {
  Q_D(TtContentDialog);
  d->content_->setText(text);
}

void TtContentDialog::setEnablePointOnMouse(bool enable) {
  Q_D(TtContentDialog);
  d->enable_point_on_mouse_ = enable;
}

void TtContentDialog::paintEvent(QPaintEvent* event) {
  Q_D(TtContentDialog);
  QPainter painter(this);
  painter.save();
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(d->_themeMode == TtThemeType::ThemeMode::Light
                       ? Qt::white
                       : QColor(0x2B, 0x2B, 0x2B));

  painter.drawRoundedRect(rect(), 8, 8);  // 统一使用8px圆角半径
                                          // // / 背景绘制
                                          //  painter.drawRect(rect());

  // 按钮栏背景绘制
  painter.setBrush(d->_themeMode == TtThemeType::ThemeMode::Light
                       ? QColor(0xF3, 0xF3, 0xF3)
                       : QColor(0x20, 0x20, 0x20));
  // 圆角
  // painter.drawRoundedRect(QRectF(0, height() - 60, width(), 60), 8, 8);
  painter.restore();
}

void TtContentDialog::showEvent(QShowEvent* event) {
  Q_D(TtContentDialog);
  QDialog::showEvent(event);

  setProperty("closing", false);

  if (d->enable_point_on_mouse_) {
    startShowAnimation();
  }
}

void TtContentDialog::closeEvent(QCloseEvent* event) {
  if (isModal() && !property("closing").toBool()) {
    setProperty("closing", true);
    // 根据用户是 accept 还是 reject，可以用 sender() 来判断，或默认 accept
    QDialog::accept();
    return;
  }

  // 放置重入
  if (property("closing").toBool()) {
    // 截断事件
    qDebug() << "av";
    event->accept();
    return;
  }
  setProperty("closing", true);

  QPointer<TtContentDialog> self(this);  // 弱引用, 避免悬空访问

  QPropertyAnimation* closeAnim = new QPropertyAnimation(this, "windowOpacity");
  closeAnim->setDuration(200);
  closeAnim->setStartValue(1.0);
  closeAnim->setEndValue(0.0);
  connect(closeAnim, &QPropertyAnimation::finished, this, [self, event]() {
    if (self) {
      if (self->result() == QDialog::Rejected) {
        self->QDialog::rejected();
      } else {
        self->QDialog::accept();
      }
    }
  });
  event->ignore();  // 阻止立即关闭

  // 禁用交互，防止在动画期间操作
  setAttribute(Qt::WA_TransparentForMouseEvents, true);
  setEnabled(false);

  closeAnim->start(QAbstractAnimation::DeleteWhenStopped);

  Q_D(TtContentDialog);
  if (d->_shadowWidget) {
    d->_shadowWidget->hide();  // 或者也加个动画
  }
}

bool TtContentDialog::eventFilter(QObject* obj, QEvent* event) {
  Q_D(TtContentDialog);
  if (event->type() == QEvent::Resize && obj == d->main_window_) {
    if (d->_shadowWidget) {
      d->_shadowWidget->setFixedSize(d->main_window_->size());
    }
    adjustPosition();
  }
  return QDialog::eventFilter(obj, event);
}

void TtContentDialog::adjustPosition() {
  Q_D(TtContentDialog);
  if (!d->main_window_ || !isVisible())
    return;

  // 获取父窗口实际显示区域（包含装饰）
  const QRect parentGeo = d->main_window_->frameGeometry();

  // 目标中心点
  const QPoint targetCenter = parentGeo.center();

  // 计算目标位置
  const int x = parentGeo.x() + (parentGeo.width() - width()) / 2;
  const int y = parentGeo.y() + (parentGeo.height() - height()) / 2;

  QRect screenGeo;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  // 当前父窗口所在的屏幕
  QScreen* screen = QGuiApplication::screenAt(targetCenter);
  if (screen) {
    screenGeo = screen->availableGeometry();
  } else {
    screen = QGuiApplication::primaryScreen();
    screenGeo = screen->availableGeometry();
  }
#else
  screenGeo = QApplication::desktop()->availableGeometry(this);
#endif

  // const int finalX = qBound(screenGeo.left(), x, screenGeo.right() - width());
  // const int finalY = qBound(screenGeo.top(), y, screenGeo.bottom() - height());
  const int finalX = qBound(screenGeo.left(), targetCenter.x() - width() / 2,
                            screenGeo.right() - width());
  const int finalY = qBound(screenGeo.top(), targetCenter.y() - height() / 2,
                            screenGeo.bottom() - height());
  move(finalX, finalY);
}

void TtContentDialog::startShowAnimation() {
  Q_D(TtContentDialog);
  if (!d->main_window_) {
    return;
  }
  // 确定窗口尺寸
  this->ensurePolished();
  this->adjustSize();

  const QSize finalSize = size();          // 自身的显示大小
  const QPoint mousePos = QCursor::pos();  // 全局坐标
  const QPoint targetPos = this->pos();    // 本窗口

  setGeometry(QRect(mousePos, QSize(1, 1)));
  setWindowOpacity(0.0);  // 透明状态

  // if (animation_) {

  // }

  // 创建动画组
  QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

  // 缩放动画（带弹性效果） 几何变化
  QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "geometry");
  scaleAnim->setDuration(200);
  scaleAnim->setStartValue(QRect(mousePos, QSize(1, 1)));
  scaleAnim->setEndValue(QRect(targetPos, finalSize));
  scaleAnim->setEasingCurve(QEasingCurve::OutBack);

  // 淡入动画  透明度
  QPropertyAnimation* fadeAnim = new QPropertyAnimation(this, "windowOpacity");
  fadeAnim->setDuration(150);
  fadeAnim->setStartValue(0.0);
  fadeAnim->setEndValue(1.0);

  // 组合动画
  group->addAnimation(scaleAnim);
  group->addAnimation(fadeAnim);

  // 连接信号
  connect(group, &QAnimationGroup::finished, [=]() {
    // 显示动画完成, dialog 接收鼠标事件
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    activateWindow();  // 确保获得焦点
  });

  // 禁用初始交互
  setAttribute(Qt::WA_TransparentForMouseEvents, true);

  // 启动动画
  group->start(QAbstractAnimation::DeleteWhenStopped);

  //   // 获取鼠标位置（相对于父窗口）
  //   QPoint mousePos = d->main_window_->mapFromGlobal(QCursor::pos());

  //   // 转换为全局坐标（考虑多显示器）
  //   mousePos = d->main_window_->mapToGlobal(mousePos);

  //   // 计算目标位置（带边界检查）
  //   QRect parentGeo = d->main_window_->frameGeometry();
  //   QPoint targetPos = parentGeo.center() - QPoint(width() / 2, height() / 2);

  //   QRect screenGeo;
  //   // 边界检查
  // #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  //   QScreen* screen = this->screen();
  //   if (!screen) {
  //     // 获取当前的主窗口
  //     screen = QGuiApplication::primaryScreen();
  //   }
  //   screenGeo = screen->availableGeometry();
  // #elif (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  //   QRect screenGeo = QApplication::desktop()->availableGeometry(this);
  // #endif
  //   // 屏幕边界约束
  //   // QRect screenGeo = QApplication::desktop()->availableGeometry(this);

  //   targetPos.setX(
  //       qMax(screenGeo.left(), qMin(targetPos.x(), screenGeo.right() - width())));
  //   targetPos.setY(qMax(screenGeo.top(),
  //                       qMin(targetPos.y(), screenGeo.bottom() - height())));

  //   // 初始化动画属性
  //   setGeometry(QRect(mousePos, QSize(0, 0)));  // 从0尺寸开始
  //   setWindowOpacity(0.3);

  //   // 组合动画
  //   QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

  //   // 尺寸动画
  //   QPropertyAnimation* sizeAnim = new QPropertyAnimation(this, "size");
  //   sizeAnim->setDuration(350);
  //   sizeAnim->setStartValue(QSize(0, 0));
  //   sizeAnim->setEndValue(size());
  //   sizeAnim->setEasingCurve(QEasingCurve::OutBack);

  //   // 位移动画
  //   QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
  //   posAnim->setDuration(400);
  //   posAnim->setStartValue(mousePos);
  //   posAnim->setEndValue(targetPos);
  //   posAnim->setEasingCurve(QEasingCurve::OutQuad);

  //   // 透明度动画
  //   QPropertyAnimation* opacityAnim =
  //       new QPropertyAnimation(this, "windowOpacity");
  //   opacityAnim->setDuration(300);
  //   opacityAnim->setStartValue(0.3);
  //   opacityAnim->setEndValue(1.0);

  //   group->addAnimation(sizeAnim);
  //   group->addAnimation(posAnim);
  //   group->addAnimation(opacityAnim);

  //   // 连接动画结束信号
  //   connect(group, &QAnimationGroup::finished,
  //           [=]() { setAttribute(Qt::WA_TransparentForMouseEvents, false); });

  //   // 启动动画前禁用鼠标事件
  //   setAttribute(Qt::WA_TransparentForMouseEvents, true);

  //   group->start(QAbstractAnimation::DeleteWhenStopped);
  //   show();
}

TtContentDialogPrivate::TtContentDialogPrivate(QObject* parent)
    : QObject(parent) {}

TtContentDialogPrivate::~TtContentDialogPrivate() {}

}  // namespace Ui
