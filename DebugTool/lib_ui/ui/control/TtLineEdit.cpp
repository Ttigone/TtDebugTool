#include "ui/control/TtLineEdit.h"
#include "ui/control/TtLineEdit_p.h"

#include "ui/style/TtLineEditStyle.h"

#include <QFocusEvent>

namespace Ui {

Q_PROPERTY_CREATE_Q_CPP(TtLineEdit, int, BorderRadius)

TtLineEdit::TtLineEdit(QWidget* parent)
    : QLineEdit(parent), d_ptr(new TtLineEditPrivate) {

  Q_D(TtLineEdit);
  d->q_ptr = this;
  d->init();
  setObjectName("TtLineEdit");
  // 设置焦点策略为强焦点，允许通过鼠标和键盘获取焦点
  setFocusPolicy(Qt::StrongFocus);
  // 启用鼠标跟踪，以便获取鼠标移动事件
  setMouseTracking(true);
  // 设置自定义样式
  setStyle(new style::TtLineEditStyle(style()));
  // 设置样式表，为输入框添加左内边距
  setStyleSheet("#TtLineEdit{padding-left: 10px;}");
  // 设置字体间距
  QFont textFont = font();
  textFont.setLetterSpacing(QFont::AbsoluteSpacing, d->text_spacing_);
  setFont(textFont);

  // 设置控件可见
  setVisible(true);

  // 根据当前主题模式更新样式
  d->onThemeChanged(tTheme->getThemeMode());

  // 连接主题模式改变信号到更新样式的槽函数
  connect(tTheme, &TtTheme::themeModeChanged, d,
          &TtLineEditPrivate::onThemeChanged);
}

TtLineEdit::TtLineEdit(const QString& text, QWidget* parent)
    : Ui::TtLineEdit(parent) {
  setText(text);
}

TtLineEdit::~TtLineEdit() {}

void TtLineEdit::setReadOnlyNoClearButton(bool enable) {
  Q_D(TtLineEdit);
  d->pIsReadOnlyNoClearButtonEnable_ = enable;
}

// 设置清除按钮是否启用
void TtLineEdit::setIsClearButtonEnable(bool isClearButtonEnable) {
  Q_D(TtLineEdit);
  if (d->pIsReadOnlyNoClearButtonEnable_) {
    d->pIsClearButtonEnable_ = isClearButtonEnable && !isReadOnly();
  } else {
    d->pIsClearButtonEnable_ = isClearButtonEnable;
  }
  // 调用基类的方法设置清除按钮是否启用
  setClearButtonEnabled(isClearButtonEnable);
  // 发射清除按钮启用状态改变的信号
  Q_EMIT pIsClearButtonEnableChanged();
}

// 获取清除按钮是否启用的状态
bool TtLineEdit::getIsClearButtonEnable() const {
  Q_D(const TtLineEdit);
  return d->pIsClearButtonEnable_;
}

void TtLineEdit::focusInEvent(QFocusEvent* event) {
  Q_D(TtLineEdit);
  // 发射获得焦点的信号，并传递当前文本
  Q_EMIT focusIn(this->text());
  if (event->reason() == Qt::MouseFocusReason) {
    if (d->pIsReadOnlyNoClearButtonEnable_) {
      if (d->pIsClearButtonEnable_ && !isReadOnly()) {
        // 如果清除按钮启用，显示清除按钮
        setClearButtonEnabled(true);
      }
    } else {
      if (d->pIsClearButtonEnable_) {
        // 如果清除按钮启用，显示清除按钮
        setClearButtonEnabled(true);
      }
    }
    // 创建一个属性动画，用于动画扩展标记的宽度
    QPropertyAnimation* markAnimation =
        new QPropertyAnimation(d, "pExpandMarkWidth");
    // 连接动画值改变信号到更新界面的槽函数
    connect(markAnimation, &QPropertyAnimation::valueChanged, this,
            [=](const QVariant& value) { update(); });
    // 设置动画持续时间为 300 毫秒
    markAnimation->setDuration(300);
    // 设置动画的缓动曲线为 InOutSine
    markAnimation->setEasingCurve(QEasingCurve::InOutSine);
    // 设置动画的起始值为当前扩展标记的宽度
    markAnimation->setStartValue(d->pExpandMarkWidth_);
    // 设置动画的结束值为输入框宽度的一半减去边框圆角半径的一半
    markAnimation->setEndValue(width() / 2 - d->pBorderRadius_ / 2);
    // 启动动画，并在动画结束后自动删除动画对象
    markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  }

  // 调用基类的焦点获得事件处理函数
  QLineEdit::focusInEvent(event);
}

void TtLineEdit::focusOutEvent(QFocusEvent* event) {
  Q_D(TtLineEdit);
  // 发射失去焦点的信号，并传递当前文本
  Q_EMIT focusOut(this->text());
  if (event->reason() != Qt::PopupFocusReason) {
    if (d->pIsClearButtonEnable_) {
      // 如果清除按钮启用，隐藏清除按钮
      setClearButtonEnabled(false);
    }
    // 创建一个属性动画，用于动画缩小扩展标记的宽度
    QPropertyAnimation* markAnimation =
        new QPropertyAnimation(d, "pExpandMarkWidth");
    // 连接动画值改变信号到更新界面的槽函数
    connect(markAnimation, &QPropertyAnimation::valueChanged, this,
            [=](const QVariant& value) { update(); });
    // 设置动画持续时间为 300 毫秒
    markAnimation->setDuration(300);
    // 设置动画的缓动曲线为 InOutSine
    markAnimation->setEasingCurve(QEasingCurve::InOutSine);
    // 设置动画的起始值为当前扩展标记的宽度
    markAnimation->setStartValue(d->pExpandMarkWidth_);
    // 设置动画的结束值为 0
    markAnimation->setEndValue(0);
    // 启动动画，并在动画结束后自动删除动画对象
    markAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    // 发射失去焦点的自定义信号
    Q_EMIT wmFocusOut(text());
  }
  // 调用基类的焦点失去事件处理函数
  QLineEdit::focusOutEvent(event);
}

void TtLineEdit::paintEvent(QPaintEvent* event) {
  Q_D(TtLineEdit);
  // 调用基类的绘制事件处理函数
  QLineEdit::paintEvent(event);
  QPainter painter(this);
  // 保存当前的绘图状态
  painter.save();
  // 启用抗锯齿和文本抗锯齿
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  // 设置画笔为无，即不绘制边框
  painter.setPen(Qt::NoPen);
  // 设置画刷颜色为当前主题的主要正常颜色
  painter.setBrush(TtThemeColor(d->theme_mode_, PrimaryNormal));
  // 绘制一个圆角矩形，作为扩展标记
  painter.drawRoundedRect(QRectF(width() / 2 - d->pExpandMarkWidth_,
                                 height() - 2.5, d->pExpandMarkWidth_ * 2, 2.5),
                          2, 2);
  // 恢复之前保存的绘图状态
  painter.restore();
}

void TtLineEdit::contextMenuEvent(QContextMenuEvent* event) {}

void TtLineEdit::resizeEvent(QResizeEvent* event) {
  QLineEdit::resizeEvent(event);
  Q_D(TtLineEdit);
  if (hasFocus()) {
    int newTargetWidth = width() / 2 - d->pBorderRadius_ / 2;
    if (d->pMarkAnimation &&
        d->pMarkAnimation->state() == QAbstractAnimation::Running) {
      // 如果动画正在运行，更新结束值
      d->pMarkAnimation->setEndValue(newTargetWidth);
    } else {
      // 否则直接更新宽度并重绘
      d->pExpandMarkWidth_ = newTargetWidth;
      update();
    }
  }
}

TtLineEditPrivate::TtLineEditPrivate(QObject* parent) : QObject(parent) {}

TtLineEditPrivate::~TtLineEditPrivate() {
  // qDebug() << "ttlinedelete";
}

void TtLineEditPrivate::onWMWindowClickedEvent(QVariantMap data) {
  Q_Q(TtLineEdit);
  // TtAppBarType::WMMouseActionType actionType =
  //     data.value("WMClickType").value<ElaAppBarType::WMMouseActionType>();
  // if (actionType == ElaAppBarType::WMLBUTTONDOWN) {
  //   if (q->hasSelectedText() && q->hasFocus()) {
  //     q->clearFocus();
  //   }
  // } else if (actionType == ElaAppBarType::WMLBUTTONUP ||
  //            actionType == ElaAppBarType::WMNCLBUTTONDOWN) {
  //   if (ElaApplication::containsCursorToItem(q) ||
  //       (actionType == ElaAppBarType::WMLBUTTONUP && q->hasSelectedText())) {
  //     return;
  //   }
  //   if (q->hasFocus()) {
  //     q->clearFocus();
  //   }
  // }
}

int TtLineEditPrivate::calculateTargetWidth() const {
  return q_ptr->width() / 2 - pBorderRadius_ / 2;
}

void TtLineEditPrivate::onThemeChanged(TtThemeType::ThemeMode themeMode) {
  Q_Q(TtLineEdit);
  theme_mode_ = themeMode;
  if (themeMode == TtThemeType::Light) {
    QPalette palette;
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::PlaceholderText, QColor(0x00, 0x00, 0x00, 128));
    q->setPalette(palette);
  } else {
    QPalette palette;
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(0xBA, 0xBA, 0xBA));
    q->setPalette(palette);
  }
}

void TtLineEditPrivate::init() {
  // 获取当前的主题模式
  theme_mode_ = tTheme->getThemeMode();
  // 设置边框圆角半径
  pBorderRadius_ = 0;
  // 设置扩展标记的初始宽度
  pExpandMarkWidth_ = 0;
  // 启用清除按钮
  pIsClearButtonEnable_ = true;
  pIsReadOnlyNoClearButtonEnable_ = false;

  // // 事件总线相关设置
  // // 创建一个名为 "WMWindowClicked" 的事件，用于处理窗口点击事件
  // d->_focusEvent = new ElaEvent("WMWindowClicked", "onWMWindowClickedEvent", d);
  // // 注册并初始化该事件
  // d->_focusEvent->registerAndInit();
}

TtLabelLineEdit::TtLabelLineEdit(Qt::AlignmentFlag flag, const QString& text,
                                 QWidget* parent) {
  line_edit_ = new TtLineEdit(this);

  label_ = new QLabel(text, this);
  // 设置 tip
  label_->setToolTip(text);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(QMargins());
  layout->setSpacing(5);

  switch (flag) {
    case Qt::AlignLeft:
      layout->addWidget(label_, 1);
      layout->addWidget(line_edit_, 2);
      break;
    case Qt::AlignRight:
      layout->addStretch();
      layout->addWidget(label_, 1);
      layout->addWidget(line_edit_, 2);
      break;
    case Qt::AlignHCenter:
      layout->addStretch();
      layout->addWidget(label_, 1);
      layout->addWidget(line_edit_, 2);
      layout->addStretch();
      break;
    default:
      layout->addWidget(label_, 1);
      layout->addWidget(line_edit_, 2);
      break;
  }
  connect(line_edit_, &TtLineEdit::textChanged, this,
          &TtLabelLineEdit::currentTextChanged);

  connect(line_edit_, &TtLineEdit::textChanged, this,
          [this](const QString& text) {
            // qDebug() << text.toULongLong();
            emit currentTextToUInt32(text.toULongLong());
          });
}

TtLabelLineEdit::TtLabelLineEdit(const QString& text, QWidget* parent)
    : TtLabelLineEdit(Qt::AlignLeft, text, parent) {}

TtLabelLineEdit::~TtLabelLineEdit() {}

TtLineEdit* TtLabelLineEdit::body() {
  return line_edit_;
}

void TtLabelLineEdit::setText(const QString& text) {
  qDebug() << "TEST: " << text;
  line_edit_->setText(text);
}

QString TtLabelLineEdit::currentText() {
  return line_edit_->text();
}

}  // namespace Ui
