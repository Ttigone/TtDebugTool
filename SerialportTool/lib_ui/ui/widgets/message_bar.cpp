#include "ui/widgets/message_bar.h"
#include "ui/widgets/message_bar_p.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QResizeEvent>

#include "ui/TtTheme.h"

namespace Ui {

void TtMessageBar::success(TtMessageBarType::PositionPolicy policy,
                           QString title, QString text, int displayMsec,
                           QWidget* parent) {
  // qDebug() << QApplication::topLevelWidgets();
  // 如果 parent 为空, 就会直接去寻找顶层的窗口, 至于找哪一个, 可以根据属性决定
  if (!parent) {
    QList<QWidget*> widgetList = QApplication::topLevelWidgets();
    for (auto widget : widgetList) {
      // 通过属性获取类名
      // 主窗口 MainWindow
      if (widget->property("TtBaseClassName").toString() == "TtWindow") {
        // qDebug() << "find";
        parent = widget;
      }
    }
    if (!parent) {
      return;
    }
  }
  // 指定父对象是 parent, 检查 delete
  TtMessageBar* bar = new TtMessageBar(policy, TtMessageBarType::Success, title,
                                       text, displayMsec, parent);
  Q_UNUSED(bar);
}

void TtMessageBar::warning(TtMessageBarType::PositionPolicy policy,
                           QString title, QString text, int displayMsec,
                           QWidget* parent) {
  if (!parent) {
    QList<QWidget*> widgetList = QApplication::topLevelWidgets();
    for (auto widget : widgetList) {
      if (widget->property("TtBaseClassName").toString() == "TtWindow") {
        parent = widget;
      }
    }
    if (!parent) {
      return;
    }
  }
  TtMessageBar* bar = new TtMessageBar(policy, TtMessageBarType::Warning, title,
                                       text, displayMsec, parent);
  Q_UNUSED(bar);
}

void TtMessageBar::information(TtMessageBarType::PositionPolicy policy,
                               QString title, QString text, int displayMsec,
                               QWidget* parent) {
  if (!parent) {
    QList<QWidget*> widgetList = QApplication::topLevelWidgets();
    for (auto widget : widgetList) {
      if (widget->property("TtBaseClassName").toString() == "TtWindow") {
        parent = widget;
      }
    }
    if (!parent) {
      return;
    }
  }
  TtMessageBar* bar = new TtMessageBar(policy, TtMessageBarType::Information,
                                       title, text, displayMsec, parent);
  Q_UNUSED(bar);
}

void TtMessageBar::error(TtMessageBarType::PositionPolicy policy, QString title,
                         QString text, int displayMsec, QWidget* parent) {

  if (!parent) {
    QList<QWidget*> widgetList = QApplication::topLevelWidgets();
    for (auto widget : widgetList) {
      if (widget->property("TtBaseClassName").toString() == "TtWindow") {
        parent = widget;
      }
    }
    if (!parent) {
      return;
    }
  }
  TtMessageBar* bar = new TtMessageBar(policy, TtMessageBarType::Error, title,
                                       text, displayMsec, parent);
  Q_UNUSED(bar);
}

void TtMessageBar::paintEvent(QPaintEvent* event) {
  Q_D(TtMessageBar);
  QPainter painter(this);
  painter.setOpacity(d->pOpacity_);
  painter.setRenderHints(QPainter::SmoothPixmapTransform |
                         QPainter::Antialiasing | QPainter::TextAntialiasing);
  // 高性能阴影
  Ui::tTheme->drawEffectShadow(&painter, rect(), d->_shadowBorderWidth,
                               d->_borderRadius);

  // 背景和图标绘制
  painter.save();
  // painter.setPen(d->_themeMode == TtThemeType::Light
  //                    ? QColor(0xBE, 0xBA, 0xBE)
  //                    : QColor(0x52, 0x50, 0x52));

  painter.setPen(QColor(0xBE, 0xBA, 0xBE));
  switch (d->_messageMode) {
    case TtMessageBarType::Success: {
      d->_drawSuccess(&painter);
      break;
    }
    case TtMessageBarType::Warning: {
      d->_drawWarning(&painter);
      break;
    }
    case TtMessageBarType::Information: {
      d->_drawInformation(&painter);
      break;
    }
    case TtMessageBarType::Error: {
      d->_drawError(&painter);
      break;
    }
  }
  // 文字绘制
  // 标题
  QFont font = this->font();
  font.setWeight(QFont::Bold);
  font.setPixelSize(16);
  painter.setFont(font);
  int titleTextWidth = painter.fontMetrics().horizontalAdvance(d->_title) + 1;
  if (titleTextWidth > 100) {
    titleTextWidth = 100;
  }
  int textFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap |
                  Qt::TextWrapAnywhere;
  painter.drawText(QRect(d->_leftPadding + d->_titleLeftSpacing, -1,
                         titleTextWidth, height()),
                   textFlags, d->_title);
  // 正文
  font.setWeight(QFont::Light);
  font.setPixelSize(15);
  painter.setFont(font);
  painter.drawText(
      QRect(d->_leftPadding + d->_titleLeftSpacing + titleTextWidth +
                d->_textLeftSpacing,
            0,
            width() - (d->_leftPadding + d->_titleLeftSpacing + titleTextWidth +
                       d->_textLeftSpacing + d->_closeButtonWidth +
                       d->_closeButtonLeftRightMargin / 2),
            height()),
      textFlags, d->_text);
  int textHeight =
      painter.fontMetrics()
          .boundingRect(
              QRect(d->_leftPadding + d->_titleLeftSpacing + titleTextWidth +
                        d->_textLeftSpacing,
                    0,
                    width() -
                        (d->_leftPadding + d->_titleLeftSpacing +
                         titleTextWidth + d->_textLeftSpacing +
                         d->_closeButtonWidth + d->_closeButtonLeftRightMargin),
                    height()),
              textFlags, d->_text)
          .height();
  if (textHeight >= minimumHeight() - 20) {
    setMinimumHeight(textHeight + 20);
  }
  painter.restore();
}

bool TtMessageBar::eventFilter(QObject* watched, QEvent* event) {
  Q_D(TtMessageBar);
  if (watched == parentWidget()) {
    switch (event->type()) {
      case QEvent::Resize: {
        QResizeEvent* resizeEvent = dynamic_cast<QResizeEvent*>(event);
        QSize offsetSize = parentWidget()->size() - resizeEvent->oldSize();
        if (d->_isNormalDisplay) {
          switch (d->_policy) {
            case TtMessageBarType::Top: {
              this->move(parentWidget()->width() / 2 - minimumWidth() / 2,
                         this->y());
              break;
            }
            case TtMessageBarType::Bottom: {
              this->move(parentWidget()->width() / 2 - minimumWidth() / 2,
                         this->pos().y() + offsetSize.height());
              break;
            }
            case TtMessageBarType::Left:
            case TtMessageBarType::TopLeft: {
              this->move(d->_messageBarHorizontalMargin, this->pos().y());
              break;
            }
            case TtMessageBarType::BottomLeft: {
              this->move(d->_messageBarHorizontalMargin,
                         this->pos().y() + offsetSize.height());
              break;
            }
            case TtMessageBarType::Right:
            case TtMessageBarType::TopRight: {
              this->move(parentWidget()->width() - minimumWidth() -
                             d->_messageBarHorizontalMargin,
                         this->y());
              break;
            }
            case TtMessageBarType::BottomRight: {
              this->move(parentWidget()->width() - minimumWidth() -
                             d->_messageBarHorizontalMargin,
                         this->pos().y() + offsetSize.height());
              break;
            }
          }
        }
        break;
      }
      default: {
        break;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

TtMessageBar::TtMessageBar(TtMessageBarType::PositionPolicy policy,
                           TtMessageBarType::MessageMode messageMode,
                           QString& title, QString& text, int displayMsec,
                           QWidget* parent)
    : QWidget{parent}, d_ptr(new TtMessageBarPrivate()) {

  Q_D(TtMessageBar);
  d->q_ptr = this;
  d->_borderRadius = 6;
  d->_title = title;
  d->_text = text;
  d->_policy = policy;
  d->_messageMode = messageMode;
  // 获取并设置主题
  // d->_themeMode = eTheme->getThemeMode();
  setFixedHeight(60);
  setMouseTracking(true);
  d->pOpacity_ = 1;  // 宏创建
  // setFont(QFont("微软雅黑"));
  parent->installEventFilter(this);  // 安装监听器
  // 关闭按钮
  // d->_closeButton =
  //     new TtIconButton(TtIconType::Xmark, 17, d->_closeButtonWidth, 30, this);
  d->_closeButton = new QPushButton(this);
  switch (d->_messageMode) {
    case TtMessageBarType::Success: {
      // d->_closeButton->setLightHoverColor(QColor(0xE6, 0xFC, 0xE3));
      // d->_closeButton->setDarkHoverColor(QColor(0xE6, 0xFC, 0xE3));
      // d->_closeButton->setDarkIconColor(Qt::black);
      break;
    }
    case TtMessageBarType::Warning: {
      // d->_closeButton->setLightHoverColor(QColor(0x5E, 0x4C, 0x22));
      // d->_closeButton->setDarkHoverColor(QColor(0x5E, 0x4C, 0x22));
      // d->_closeButton->setLightIconColor(Qt::white);
      // d->_closeButton->setDarkIconColor(Qt::white);
      break;
    }
    case TtMessageBarType::Information: {
      // d->_closeButton->setLightHoverColor(QColor(0xEB, 0xEB, 0xEB));
      // d->_closeButton->setDarkHoverColor(QColor(0xEB, 0xEB, 0xEB));
      // d->_closeButton->setDarkIconColor(Qt::black);
      break;
    }
    case TtMessageBarType::Error: {
      // d->_closeButton->setLightHoverColor(QColor(0xF7, 0xE1, 0xE4));
      // d->_closeButton->setDarkHoverColor(QColor(0xF7, 0xE1, 0xE4));
      // d->_closeButton->setDarkIconColor(Qt::black);
      break;
    }
  }
  // d->_closeButton->setBorderRadius(5);
  // connect(d->_closeButton, &TtIconButton::clicked, d,
  //         &TtMessageBarPrivate::onCloseButtonClicked);
  // 处理按钮点击
  connect(d->_closeButton, &QPushButton::clicked, d,
          &TtMessageBarPrivate::onCloseButtonClicked);

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 10, 0);
  mainLayout->addStretch();
  // 右边放置一个关闭按钮
  mainLayout->addWidget(d->_closeButton);
  setObjectName("TtMessageBar");
  setStyleSheet("#TtMessageBar{background-color:transparent;}");
  // 创建消息栏
  d->_messageBarCreate(displayMsec);
}

TtMessageBar::~TtMessageBar() {}

}  // namespace Ui
