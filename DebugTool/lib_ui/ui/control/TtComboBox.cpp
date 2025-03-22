#include "ui/control/TtComboBox.h"
#include "ui/control/TtComboBox_p.h"
#include "ui/control/TtScrollBar.h"

#include "ui/widgets/buttons.h"

#include "ui/style/TtComboBoxStyle.h"

#include <QApplication>
#include <QEvent>
#include <QListView>
#include <QMouseEvent>

namespace Ui {

Q_PROPERTY_CREATE_Q_CPP(TtComboBox, int, BorderRadius)

TtComboBox::TtComboBox(QWidget* parent)
    : QComboBox(parent), d_ptr(new TtComboBoxPrivate) {
  Q_D(TtComboBox);
  d->q_ptr = this;
  d->pBorderRadius_ = 3;
  d->theme_mode_ = tTheme->getThemeMode();
  setObjectName("TtComboBox");
  setFixedHeight(35);
  d->comboBox_style_ = new style::TtComboBoxStyle(style());
  setStyle(d->comboBox_style_);

  //调用view 让container初始化
  setView(new QListView(this));
  QAbstractItemView* comboBoxView = this->view();
  comboBoxView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  TtScrollBar* scrollBar = new TtScrollBar(this);
  comboBoxView->setVerticalScrollBar(scrollBar);
  TtScrollBar* floatVScrollBar = new TtScrollBar(scrollBar, comboBoxView);
  floatVScrollBar->setIsAnimation(true);
  comboBoxView->setAutoScroll(false);
  comboBoxView->setSelectionMode(QAbstractItemView::NoSelection);
  comboBoxView->setObjectName("TtComboBoxView");
  comboBoxView->setStyleSheet(
      "#ElaComboBoxView{background-color:transparent;}");
  comboBoxView->setStyle(d->comboBox_style_);
  QWidget* container = this->findChild<QFrame*>();
  if (container) {
    container->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint |
                              Qt::NoDropShadowWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setObjectName("TtComboBoxContainer");
    container->setStyle(d->comboBox_style_);
    QLayout* layout = container->layout();
    while (layout->count()) {
      layout->takeAt(0);
    }
    layout->addWidget(view());
    layout->setContentsMargins(6, 0, 6, 6);
#ifndef Q_OS_WIN
    container->setStyleSheet("background-color:transparent;");
#endif
  }
  QComboBox::setMaxVisibleItems(5);
  connect(
      tTheme, &Ui::TtTheme::themeModeChanged, this,
      [=](TtThemeType::ThemeMode themeMode) { d->theme_mode_ = themeMode; });
}

TtComboBox::~TtComboBox() {}

void TtComboBox::showPopup() {
  Q_D(TtComboBox);
  bool oldAnimationEffects = qApp->isEffectEnabled(Qt::UI_AnimateCombo);
  qApp->setEffectEnabled(Qt::UI_AnimateCombo, false);
  QComboBox::showPopup();
  qApp->setEffectEnabled(Qt::UI_AnimateCombo, oldAnimationEffects);
  if (count() > 0) {
    QWidget* container = this->findChild<QFrame*>();
    if (container) {
      int containerHeight = 0;
      if (count() >= maxVisibleItems()) {
        containerHeight = maxVisibleItems() * 35 + 8;
      } else {
        containerHeight = count() * 35 + 8;
      }
      view()->resize(view()->width(), containerHeight - 8);
      container->move(container->x(), container->y() + 3);
      QLayout* layout = container->layout();
      while (layout->count()) {
        layout->takeAt(0);
      }
      QPropertyAnimation* fixedSizeAnimation =
          new QPropertyAnimation(container, "maximumHeight");
      connect(fixedSizeAnimation, &QPropertyAnimation::valueChanged, this,
              [=](const QVariant& value) {
                container->setFixedHeight(value.toUInt());
              });
      fixedSizeAnimation->setStartValue(1);
      fixedSizeAnimation->setEndValue(containerHeight);
      fixedSizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
      fixedSizeAnimation->setDuration(400);
      fixedSizeAnimation->start(QAbstractAnimation::DeleteWhenStopped);

      QPropertyAnimation* viewPosAnimation =
          new QPropertyAnimation(view(), "pos");
      connect(viewPosAnimation, &QPropertyAnimation::finished, this, [=]() {
        d->is_allow_hide_popup_ = true;
        layout->addWidget(view());
      });
      QPoint viewPos = view()->pos();
      viewPosAnimation->setStartValue(
          QPoint(viewPos.x(), viewPos.y() - view()->height()));
      viewPosAnimation->setEndValue(viewPos);
      viewPosAnimation->setEasingCurve(QEasingCurve::OutCubic);
      viewPosAnimation->setDuration(400);
      viewPosAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    //指示器动画
    QPropertyAnimation* rotateAnimation =
        new QPropertyAnimation(d->comboBox_style_, "pExpandIconRotate");
    connect(rotateAnimation, &QPropertyAnimation::valueChanged, this,
            [=](const QVariant& value) { update(); });
    rotateAnimation->setDuration(300);
    rotateAnimation->setEasingCurve(QEasingCurve::InOutSine);
    rotateAnimation->setStartValue(d->comboBox_style_->getExpandIconRotate());
    rotateAnimation->setEndValue(-180);
    rotateAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    QPropertyAnimation* markAnimation =
        new QPropertyAnimation(d->comboBox_style_, "pExpandMarkWidth");
    markAnimation->setDuration(300);
    markAnimation->setEasingCurve(QEasingCurve::InOutSine);
    markAnimation->setStartValue(d->comboBox_style_->getExpandMarkWidth());
    markAnimation->setEndValue(width() / 2 - d->pBorderRadius_ - 6);
    markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

void TtComboBox::hidePopup() {
  Q_D(TtComboBox);
  if (d->is_allow_hide_popup_) {
    QWidget* container = this->findChild<QFrame*>();
    int containerHeight = container->height();
    if (container) {
      QLayout* layout = container->layout();
      while (layout->count()) {
        layout->takeAt(0);
      }
      QPropertyAnimation* viewPosAnimation =
          new QPropertyAnimation(view(), "pos");
      connect(viewPosAnimation, &QPropertyAnimation::finished, this, [=]() {
        layout->addWidget(view());
        QMouseEvent focusEvent(QEvent::MouseButtonPress, QPoint(-1, -1),
                               QPoint(-1, -1), Qt::NoButton, Qt::NoButton,
                               Qt::NoModifier);
        QApplication::sendEvent(parentWidget(), &focusEvent);
        QComboBox::hidePopup();
        container->setFixedHeight(containerHeight);
      });
      QPoint viewPos = view()->pos();
      connect(viewPosAnimation, &QPropertyAnimation::finished, this,
              [=]() { view()->move(viewPos); });
      viewPosAnimation->setStartValue(viewPos);
      viewPosAnimation->setEndValue(
          QPoint(viewPos.x(), viewPos.y() - view()->height()));
      viewPosAnimation->setEasingCurve(QEasingCurve::InCubic);
      viewPosAnimation->start(QAbstractAnimation::DeleteWhenStopped);

      QPropertyAnimation* fixedSizeAnimation =
          new QPropertyAnimation(container, "maximumHeight");
      connect(fixedSizeAnimation, &QPropertyAnimation::valueChanged, this,
              [=](const QVariant& value) {
                container->setFixedHeight(value.toUInt());
              });
      fixedSizeAnimation->setStartValue(container->height());
      fixedSizeAnimation->setEndValue(1);
      fixedSizeAnimation->setEasingCurve(QEasingCurve::InCubic);
      fixedSizeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      d->is_allow_hide_popup_ = false;
    }
    //指示器动画
    QPropertyAnimation* rotateAnimation =
        new QPropertyAnimation(d->comboBox_style_, "pExpandIconRotate");
    connect(rotateAnimation, &QPropertyAnimation::valueChanged, this,
            [=](const QVariant& value) { update(); });
    rotateAnimation->setDuration(300);
    rotateAnimation->setEasingCurve(QEasingCurve::InOutSine);
    rotateAnimation->setStartValue(d->comboBox_style_->getExpandIconRotate());
    rotateAnimation->setEndValue(0);
    rotateAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    QPropertyAnimation* markAnimation =
        new QPropertyAnimation(d->comboBox_style_, "pExpandMarkWidth");
    markAnimation->setDuration(300);
    markAnimation->setEasingCurve(QEasingCurve::InOutSine);
    markAnimation->setStartValue(d->comboBox_style_->getExpandMarkWidth());
    markAnimation->setEndValue(0);
    markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

TtComboBoxPrivate::TtComboBoxPrivate(QObject* parent) {}

TtComboBoxPrivate::~TtComboBoxPrivate() {}

TtLabelComboBox::TtLabelComboBox(Qt::AlignmentFlag flag, const QString& text,
                                 QWidget* parent)
    : QWidget(parent) {
  combo_box_ = new TtComboBox(this);

  label_ = new QLabel(text, this);
  // 设置 tip
  label_->setToolTip(text);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(5);

  switch (flag) {
    case Qt::AlignLeft:
      layout->addWidget(label_, 0);
      layout->addWidget(combo_box_, 1);  // 添加拉伸因子
      break;
    case Qt::AlignRight:
      layout->addStretch();
      layout->addWidget(label_, 0);
      layout->addWidget(combo_box_, 1);
      break;
    case Qt::AlignHCenter:
      layout->addStretch();
      layout->addWidget(label_, 0);
      layout->addWidget(combo_box_, 1);
      layout->addStretch();
      break;
    default:
      layout->addWidget(label_, 0);
      layout->addWidget(combo_box_, 1);
      break;
  }
  //if (flag & Qt::AlignLeft) {
  //  // 左
  //  // layout->addStretch();
  //  // layout->addWidget(label_, Qt::AlignLeft);
  //  // layout->addWidget(combo_box_, Qt::AlignLeft);
  //  // layout->addStretch();
  //  layout->addWidget(label_);
  //  // layout->addSpacerItem(new QSpacerItem(10, 10));
  //  layout->addWidget(combo_box_);
  //} else if (flag & Qt::AlignRight) {

  //} else if (flag & (Qt::AlignTop | Qt::AlignHCenter)) {
  //  // 水平居中
  //} else if (flag & (Qt::AlignTop | Qt::AlignLeft)) {
  //}
  //setLayout(layout);

  connect(combo_box_, &TtComboBox::currentIndexChanged, this,
          &TtLabelComboBox::currentIndexChanged);
}

TtLabelComboBox::TtLabelComboBox(const QString& text, QWidget* parent)
    : TtLabelComboBox(Qt::AlignLeft, text, parent) {}

TtLabelComboBox::~TtLabelComboBox() {}

TtComboBox* TtLabelComboBox::body() {
  return combo_box_;
}

void TtLabelComboBox::addItem(const QString& atext, const QVariant& auserData) {
  combo_box_->addItem(atext, auserData);
}

QVariant TtLabelComboBox::currentData(int role) {
  return combo_box_->currentData(role);
}

void TtLabelComboBox::setCurrentItem(qint8 index) {
  combo_box_->setCurrentIndex(index);
}

QString TtLabelComboBox::itemText(int index) {
  return combo_box_->itemText(index);
}

QString TtLabelComboBox::currentText() {
  return combo_box_->currentText();
}

int TtLabelComboBox::count() {
  return combo_box_->count();
}

void TtLabelComboBox::shortCurrentItemText() {
  // 虚拟串口一样
  QRegularExpression regex("COM\\d+");
  QRegularExpressionMatch match = regex.match(combo_box_->currentText());
  if (match.hasMatch()) {
    combo_box_->setCurrentText(match.captured(0));
  } else {
  }
}

TtLabelBtnComboBox::TtLabelBtnComboBox(Qt::AlignmentFlag flag,
                                       const QString& text,
                                       const QString& image_path,
                                       QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* layout = new QHBoxLayout(this);
  part_ = new TtLabelComboBox(flag, text, this);
  auto refresh_btn = new Ui::TtSvgButton(":/sys/refresh-normal.svg", this);
  refresh_btn->setEnableHoldToCheck(true);

  layout->addWidget(part_);
  layout->addWidget(refresh_btn);
  connect(refresh_btn, &Ui::TtSvgButton::clicked, [this]() {
    qDebug() << "clicked";
    emit clicked();
  });
}

TtLabelBtnComboBox::TtLabelBtnComboBox(const QString& text, QWidget* parent)
    : QWidget(parent) {
  part_ = new TtLabelComboBox(text, this);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(0);
  auto refresh_btn = new Ui::TtSvgButton(":/sys/refresh-normal.svg", this);
  refresh_btn->setColors(Qt::black, Qt::blue);
  refresh_btn->setEnableHoldToCheck(true);

  layout->addWidget(part_, 1);
  layout->addWidget(refresh_btn, 0);

  connect(part_, &TtLabelComboBox::currentIndexChanged, this,
          &TtLabelBtnComboBox::displayCurrentCOMx);

  connect(refresh_btn, &Ui::TtSvgButton::clicked, [this]() {
    emit clicked();
  });
}

TtLabelBtnComboBox::~TtLabelBtnComboBox() {}

void TtLabelBtnComboBox::addItem(const QString& atext,
                                 const QVariant& auserData) {
  part_->addItem(atext, auserData);
}

TtComboBox* TtLabelBtnComboBox::body() {
  return part_->body();
}

QVariant TtLabelBtnComboBox::currentData(int role) {
  return part_->currentData(role);
}

void TtLabelBtnComboBox::setCurrentItem(qint8 index) {
  part_->setCurrentItem(index);
}

QString TtLabelBtnComboBox::itemText(int index) {
  return part_->itemText(index);
}

QString TtLabelBtnComboBox::currentText() {
  return part_->currentText();
}

int TtLabelBtnComboBox::count() {
  return part_->count();
}

void TtLabelBtnComboBox::displayCurrentCOMx() {
  part_->shortCurrentItemText();
}

}  // namespace Ui
