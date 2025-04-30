#include "TtContentDialog.h"
#include "TtContentDialog_p.h"

#include <ui/control/TtTextButton.h>
#include <QApplication>
#include <QScreen>

namespace Ui {

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
  setAttribute(Qt::WA_DeleteOnClose);
  if (layout == LayoutSelection::THREE_OPTIONS) {
    d->_leftButton = new TtTextButton("cancel", this);
    d->_leftButton->setMinimumSize(0, 0);
    d->_leftButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_leftButton->setFixedHeight(38);
    d->_leftButton->setBorderRadius(6);
    connect(d->_leftButton, &QPushButton::clicked, this, [=]() {
      Q_EMIT leftButtonClicked();
      onLeftButtonClicked();
      close();
    });

    d->_middleButton = new TtTextButton("minimum", this);
    d->_middleButton->setMinimumSize(0, 0);
    d->_middleButton->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    d->_middleButton->setFixedHeight(38);
    d->_middleButton->setBorderRadius(6);
    connect(d->_middleButton, &TtTextButton::clicked, this, [=]() {
      Q_EMIT middleButtonClicked();
      onMiddleButtonClicked();
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
    QVBoxLayout* centralVLayout = new QVBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(9, 15, 9, 20);
    // ElaText* title = new ElaText("退出", this);
    QLabel* title = new QLabel(tr("退出"), this);
    // title->setTextStyle(ElaTextType::Title);
    // ElaText* subTitle = new ElaText("确定要退出程序吗", this);
    QLabel* subTitle = new QLabel(tr("确定要退出程序吗"), this);
    // subTitle->setTextStyle(ElaTextType::Body);
    centralVLayout->addWidget(title);
    centralVLayout->addWidget(subTitle);
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
    connect(d->_leftButton, &TtTextButton::clicked, this, [=]() {
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
    QVBoxLayout* centralVLayout = new QVBoxLayout(d->_centralWidget);
    centralVLayout->setContentsMargins(9, 15, 9, 20);
    // ElaText* title = new ElaText("退出", this);
    QLabel* title = new QLabel("退出", this);
    // title->setTextStyle(ElaTextType::Title);
    // ElaText* subTitle = new ElaText("确定要退出程序吗", this);
    QLabel* subTitle = new QLabel("确定要退出程序吗", this);
    // subTitle->setTextStyle(ElaTextType::Body);
    centralVLayout->addWidget(title);
    centralVLayout->addWidget(subTitle);
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
    QTimer::singleShot(0, this, [=]() { adjustPosition(); });
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
  if (d->_shadowWidget) {
    delete d->_shadowWidget;
  }
  if (d->main_window_) {
    d->main_window_->removeEventFilter(this);
  }
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
  painter.drawRoundedRect(QRectF(0, height() - 60, width(), 60), 8, 8);
  painter.restore();
}

void TtContentDialog::showEvent(QShowEvent* event) {}

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
  // #if 0
  //   Q_D(TtContentDialog);
  //   if (!d->main_window_ || !isVisible()) {
  //     return;
  //   }
  //   const QSize parentSize = d->main_window_->size();
  //   const QSize mySize = size();

  //   const int x = (parentSize.width() - mySize.width()) / 2;
  //   const int y = (parentSize.height() - mySize.height()) / 2;

  //   move(d->main_window_->mapToGlobal(QPoint(x, y)));
  // #else
  //   Q_D(TtContentDialog);
  //   if (!d->main_window_ || !isVisible()) {
  //     return;
  //   }
  //   QRect parentRect = d->main_window_->frameGeometry();
  //   QPoint parentCenter = parentRect.center();
  //   QPoint targetPos = parentCenter - QPoint(width() / 2, height() / 2);
  //   move(targetPos);
  //   // QPoint center = parentRect.center() - rect().center();
  //   // move(d->main_window_->mapToGlobal(center));
  // #endif

  Q_D(TtContentDialog);
  if (!d->main_window_ || !isVisible())
    return;

  // 获取父窗口实际显示区域（包含装饰）
  const QRect parentGeo = d->main_window_->frameGeometry();

  // 计算目标位置
  const int x = parentGeo.x() + (parentGeo.width() - width()) / 2;
  const int y = parentGeo.y() + (parentGeo.height() - height()) / 2;

  QRect screenGeo;

  // 边界检查
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  QScreen* screen = this->screen();
  if (!screen) {
    // 获取当前的主窗口
    screen = QGuiApplication::primaryScreen();
  }
  screenGeo = screen->availableGeometry();
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  QRect screenGeo = QApplication::desktop()->availableGeometry(this);
#endif
  const int finalX = qBound(screenGeo.left(), x, screenGeo.right() - width());
  const int finalY = qBound(screenGeo.top(), y, screenGeo.bottom() - height());
  move(finalX, finalY);
}

TtContentDialogPrivate::TtContentDialogPrivate(QObject* parent)
    : QObject(parent) {}

TtContentDialogPrivate::~TtContentDialogPrivate() {}

}  // namespace Ui
