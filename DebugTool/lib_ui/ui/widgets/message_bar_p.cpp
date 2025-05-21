#include "ui/widgets/message_bar_p.h"
#include "ui/control/TtIconButton.h"
#include "ui/widgets/message_bar.h"

#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>

namespace Ui {

Q_SINGLETON_CREATE_CPP(TtMessageBarManager)

// 策略 | 该策略的在暂存显示消息栏
QMap<TtMessageBarType::PositionPolicy, QList<TtMessageBar *> *>
    _messageBarActiveMap;

TtMessageBarManager::TtMessageBarManager(QObject *parent) {}

TtMessageBarManager::~TtMessageBarManager() {}

void TtMessageBarManager::requestMessageBarEvent(TtMessageBar *messageBar) {
  if (!messageBar) {
    return;
  }
  if (messagebar_event_map_.contains(messageBar)) {
    QList<QVariantMap> eventList = messagebar_event_map_.value(messageBar);
    QVariantMap eventData = eventList.last();
    eventList.removeLast();
    if (eventList.isEmpty()) {
      messagebar_event_map_.remove(messageBar);
    } else {
      messagebar_event_map_[messageBar] = eventList;
    }
    // 触发事件
    QString functionName = eventData.value("EventFunctionName").toString();
    QVariantMap functionData = eventData.value("EventFunctionData").toMap();
    QMetaObject::invokeMethod(
        messageBar->d_func(), functionName.toLocal8Bit().constData(),
        Qt::AutoConnection, Q_ARG(QVariantMap, functionData));
  }
}

void TtMessageBarManager::postMessageBarCreateEvent(TtMessageBar *messageBar) {
  if (!messageBar) {
    return;
  }
  updateActiveMap(messageBar, true); // 计算坐标前增加
  if (!messagebar_event_map_.contains(messageBar)) {
    QList<QVariantMap> eventList;
    QVariantMap eventData;
    eventData.insert("EventFunctionName", "messageBarEnd");
    eventList.append(eventData);
    messagebar_event_map_.insert(messageBar, eventList);
  }
}

void TtMessageBarManager::postMessageBarEndEvent(TtMessageBar *messageBar) {
  if (!messageBar) {
    return;
  }
  updateActiveMap(messageBar, false);
  // Other MessageBar事件入栈 记录同一策略事件
  TtMessageBarType::PositionPolicy policy = messageBar->d_ptr->_policy;
  foreach (auto otherMessageBar, *_messageBarActiveMap.value(policy)) {
    if (otherMessageBar->d_ptr->_judgeCreateOrder(messageBar)) {
      QList<QVariantMap> eventList = messagebar_event_map_[otherMessageBar];
      // 优先执行先触发的事件 End事件保持首位
      QVariantMap eventData;
      eventData.insert("EventFunctionName", "onOtherMessageBarEnd");
      QVariantMap functionData;
      functionData.insert("TargetPosY",
                          otherMessageBar->d_ptr->_calculateTargetPosY());
      eventData.insert("EventFunctionData", functionData);
      // 若处于创建动画阶段  则合并事件动画
      if (otherMessageBar->d_ptr->getWorkMode() ==
          WorkStatus::CreateAnimation) {
        while (eventList.count() > 1) {
          eventList.removeLast();
        }
      }
      eventList.insert(1, eventData);
      messagebar_event_map_[otherMessageBar] = eventList;
      otherMessageBar->d_ptr->tryToRequestMessageBarEvent();
    }
  }
}

void TtMessageBarManager::forcePostMessageBarEndEvent(
    TtMessageBar *messageBar) {
  if (!messageBar) {
    return;
  }
  // 清除事件堆栈记录
  messagebar_event_map_.remove(messageBar);
  // 发布终止事件
  postMessageBarEndEvent(messageBar);
}

int TtMessageBarManager::getMessageBarEventCount(TtMessageBar *messageBar) {
  if (!messageBar) {
    return -1;
  }
  if (!messagebar_event_map_.contains(messageBar)) {
    return -1;
  }
  QList<QVariantMap> eventList = messagebar_event_map_[messageBar];
  return eventList.count();
}

void TtMessageBarManager::updateActiveMap(TtMessageBar *messageBar,
                                          bool isActive) {
  if (!messageBar) {
    return;
  }
  TtMessageBarType::PositionPolicy policy = messageBar->d_ptr->_policy;
  if (isActive) {
    if (_messageBarActiveMap.contains(policy)) {
      _messageBarActiveMap[policy]->append(messageBar);
    } else {
      QList<TtMessageBar *> *messageBarList = new QList<TtMessageBar *>();
      messageBarList->append(messageBar);
      _messageBarActiveMap.insert(policy, messageBarList);
    }
  } else {
    if (_messageBarActiveMap.contains(policy)) {
      if (_messageBarActiveMap[policy]->count() > 0) {
        _messageBarActiveMap[policy]->removeOne(messageBar);
      }
    }
  }
}

TtMessageBarPrivate::TtMessageBarPrivate(QObject *parent) : QObject(parent) {
  setProperty("MessageBarClosedY", 0);
  setProperty("MessageBarFinishY", 0);
  create_time_ = QDateTime::currentMSecsSinceEpoch();
}

TtMessageBarPrivate::~TtMessageBarPrivate() {}

void TtMessageBarPrivate::tryToRequestMessageBarEvent() {
  Q_Q(TtMessageBar);
  if (!_isMessageBarCreateAnimationFinished ||
      _isMessageBarEventAnimationStart) {
    return;
  }
  TtMessageBarManager::getInstance()->requestMessageBarEvent(q);
}

WorkStatus TtMessageBarPrivate::getWorkMode() const {
  if (!_isMessageBarCreateAnimationFinished) {
    return WorkStatus::CreateAnimation;
  }
  if (_isMessageBarEventAnimationStart) {
    return WorkStatus::OtherEventAnimation;
  }
  return WorkStatus::Idle;
}

void TtMessageBarPrivate::onOtherMessageBarEnd(QVariantMap eventData) {
  Q_Q(TtMessageBar);
  _isMessageBarEventAnimationStart = true;
  qreal targetPosY = eventData.value("TargetPosY").toReal();
  QPropertyAnimation *closePosAnimation =
      new QPropertyAnimation(this, "MessageBarClosedY");
  connect(
      closePosAnimation, &QPropertyAnimation::valueChanged, this,
      [=](const QVariant &value) { q->move(q->pos().x(), value.toUInt()); });
  connect(closePosAnimation, &QPropertyAnimation::finished, this, [=]() {
    _isMessageBarEventAnimationStart = false;
    if (TtMessageBarManager::getInstance()->getMessageBarEventCount(q) > 1) {
      TtMessageBarManager::getInstance()->requestMessageBarEvent(q);
    }
    if (_isReadyToEnd) {
      TtMessageBarManager::getInstance()->requestMessageBarEvent(q);
    }
  });
  closePosAnimation->setEasingCurve(QEasingCurve::InOutSine);
  closePosAnimation->setDuration(200);
  closePosAnimation->setStartValue(q->pos().y());
  closePosAnimation->setEndValue(targetPosY);
  closePosAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TtMessageBarPrivate::messageBarEnd(QVariantMap eventData) {
  Q_Q(TtMessageBar);
  TtMessageBarManager::getInstance()->postMessageBarEndEvent(q);
  QPropertyAnimation *barFinishedOpacityAnimation =
      new QPropertyAnimation(this, "pOpacity");
  connect(barFinishedOpacityAnimation, &QPropertyAnimation::valueChanged, this,
          [=]() {
            // _closeButton->setOpacity(_pOpacity);
            q->update();
          });
  connect(barFinishedOpacityAnimation, &QPropertyAnimation::finished, this,
          [=]() { q->deleteLater(); });
  barFinishedOpacityAnimation->setDuration(300);
  barFinishedOpacityAnimation->setEasingCurve(QEasingCurve::InOutSine);
  barFinishedOpacityAnimation->setStartValue(1);
  barFinishedOpacityAnimation->setEndValue(0);
  barFinishedOpacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TtMessageBarPrivate::onCloseButtonClicked() {
  Q_Q(TtMessageBar);
  if (_isReadyToEnd) {
    return;
  }
  _isReadyToEnd = true;
  _isNormalDisplay = false;
  TtMessageBarManager::getInstance()->forcePostMessageBarEndEvent(q);
  QPropertyAnimation *opacityAnimation =
      new QPropertyAnimation(this, "pOpacity");
  connect(opacityAnimation, &QPropertyAnimation::valueChanged, this, [=]() {
    // _closeButton->setOpacity(_pOpacity);
    q->update();
  });
  connect(opacityAnimation, &QPropertyAnimation::finished, q,
          [=]() { q->deleteLater(); });
  opacityAnimation->setStartValue(pOpacity_); // 属性值, 使用宏创建
  opacityAnimation->setEndValue(0);
  opacityAnimation->setDuration(220);
  opacityAnimation->setEasingCurve(QEasingCurve::InOutSine);
  opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TtMessageBarPrivate::_messageBarCreate(int displayMsec) {
  Q_Q(TtMessageBar);
  q->show();
  QFont font = q->font();
  font.setPixelSize(16);
  font.setWeight(QFont::Bold);
  q->setFont(font);
  int titleWidth = q->fontMetrics().horizontalAdvance(_title);
  font.setPixelSize(14);
  font.setWeight(QFont::Medium);
  q->setFont(font);
  int textWidth = q->fontMetrics().horizontalAdvance(_text);
  int fixedWidth = _closeButtonLeftRightMargin + _leftPadding +
                   _titleLeftSpacing + _textLeftSpacing + _closeButtonWidth +
                   titleWidth + textWidth + 2 * _shadowBorderWidth;
  q->setFixedWidth(fixedWidth > 500 ? 500 : fixedWidth);
  TtMessageBarManager::getInstance()->postMessageBarCreateEvent(q);
  int startX = 0;
  int startY = 0;
  int endX = 0;
  int endY = 0;
  _calculateInitialPos(startX, startY, endX, endY);
  // 滑入动画
  QPropertyAnimation *barPosAnimation = new QPropertyAnimation(q, "pos");
  connect(barPosAnimation, &QPropertyAnimation::finished, q, [=]() {
    _isNormalDisplay = true;
    _isMessageBarCreateAnimationFinished = true;
    if (TtMessageBarManager::getInstance()->getMessageBarEventCount(q) > 1) {
      TtMessageBarManager::getInstance()->requestMessageBarEvent(q);
    }
    QTimer::singleShot(displayMsec, q, [=]() {
      _isReadyToEnd = true;
      TtMessageBarManager::getInstance()->requestMessageBarEvent(q);
    });
  });
  switch (_policy) {
  case TtMessageBarType::Top:
  case TtMessageBarType::Bottom: {
    barPosAnimation->setDuration(250);
    break;
  }
  default: {
    barPosAnimation->setDuration(450);
    break;
  }
  }
  barPosAnimation->setStartValue(QPoint(startX, startY));
  barPosAnimation->setEndValue(QPoint(endX, endY));
  barPosAnimation->setEasingCurve(QEasingCurve::InOutSine);
  barPosAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TtMessageBarPrivate::_calculateInitialPos(int &startX, int &startY,
                                               int &endX, int &endY) {
  Q_Q(TtMessageBar);
  QList<int> resultList = _getOtherMessageBarTotalData();
  int minimumHeightTotal = resultList[0];
  int indexLessCount = resultList[1];
  switch (_policy) {
  case TtMessageBarType::Top: {
    // 25动画距离
    startX = q->parentWidget()->width() / 2 - q->minimumWidth() / 2;
    startY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
             _messageBarVerticalTopMargin - 25;
    endX = startX;
    endY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
           _messageBarVerticalTopMargin;
    break;
  }
  case TtMessageBarType::Left: {
    startX = -q->minimumWidth();
    startY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
             q->parentWidget()->height() / 2;
    endX = _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  case TtMessageBarType::Bottom: {
    startX = q->parentWidget()->width() / 2 - q->minimumWidth() / 2;
    startY = q->parentWidget()->height() - q->minimumHeight() -
             minimumHeightTotal - _messageBarSpacing * indexLessCount -
             _messageBarVerticalBottomMargin - 25;
    endX = startX;
    endY = q->parentWidget()->height() - q->minimumHeight() -
           minimumHeightTotal - _messageBarSpacing * indexLessCount -
           _messageBarVerticalBottomMargin;
    break;
  }
  case TtMessageBarType::Right: {
    startX = q->parentWidget()->width();
    startY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
             q->parentWidget()->height() / 2;
    endX = q->parentWidget()->width() - q->minimumWidth() -
           _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  case TtMessageBarType::TopRight: {
    startX = q->parentWidget()->width();
    startY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
             _messageBarVerticalTopMargin;
    endX = q->parentWidget()->width() - q->minimumWidth() -
           _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  case TtMessageBarType::TopLeft: {
    startX = -q->minimumWidth();
    startY = minimumHeightTotal + _messageBarSpacing * indexLessCount +
             _messageBarVerticalTopMargin;
    endX = _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  case TtMessageBarType::BottomRight: {
    startX = q->parentWidget()->width();
    startY = q->parentWidget()->height() - q->minimumHeight() -
             minimumHeightTotal - _messageBarSpacing * indexLessCount -
             _messageBarVerticalBottomMargin;
    endX = q->parentWidget()->width() - q->minimumWidth() -
           _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  case TtMessageBarType::BottomLeft: {
    startX = -q->minimumWidth();
    startY = q->parentWidget()->height() - q->minimumHeight() -
             minimumHeightTotal - _messageBarSpacing * indexLessCount -
             _messageBarVerticalBottomMargin;
    endX = _messageBarHorizontalMargin;
    endY = startY;
    break;
  }
  }
  if (endY < _messageBarVerticalTopMargin ||
      endY > q->parentWidget()->height() - _messageBarVerticalBottomMargin -
                 q->minimumHeight()) {
    TtMessageBarManager::getInstance()->updateActiveMap(q, false);
    q->deleteLater();
  }
}

QList<int>
TtMessageBarPrivate::_getOtherMessageBarTotalData(bool isJudgeCreateOrder) {
  Q_Q(TtMessageBar);
  QList<int> resultList;
  int minimumHeightTotal = 0;
  int indexLessCount = 0;
  // 弹出的消息栏列表
  QList<TtMessageBar *> *messageBarList = _messageBarActiveMap[_policy];
  for (auto messageBar : *messageBarList) {
    if (messageBar == q) {
      continue;
    }
    if (!isJudgeCreateOrder ||
        (isJudgeCreateOrder && _judgeCreateOrder(messageBar))) {
      indexLessCount++;
      minimumHeightTotal += messageBar->minimumHeight();
    }
  }
  resultList.append(minimumHeightTotal);
  resultList.append(indexLessCount);
  return resultList;
}

qreal TtMessageBarPrivate::_calculateTargetPosY() {
  Q_Q(TtMessageBar);
  QList<int> resultList = _getOtherMessageBarTotalData(true);
  int minimumHeightTotal = resultList[0];
  int indexLessCount = resultList[1];
  switch (_policy) {
  case TtMessageBarType::Top:
  case TtMessageBarType::TopRight:
  case TtMessageBarType::TopLeft: {
    return minimumHeightTotal + _messageBarSpacing * indexLessCount +
           _messageBarVerticalTopMargin;
  }
  case TtMessageBarType::Left:
  case TtMessageBarType::Right: {
    return minimumHeightTotal + _messageBarSpacing * indexLessCount +
           q->parentWidget()->height() / 2;
  }
  case TtMessageBarType::Bottom:
  case TtMessageBarType::BottomRight:
  case TtMessageBarType::BottomLeft: {
    return q->parentWidget()->height() - q->minimumHeight() -
           minimumHeightTotal - _messageBarSpacing * indexLessCount -
           _messageBarVerticalBottomMargin;
  }
  }
  return 0;
}

bool TtMessageBarPrivate::_judgeCreateOrder(TtMessageBar *otherMessageBar) {
  if (otherMessageBar->d_ptr->create_time_ < create_time_) {
    // otherMessageBar先创建
    return true;
  } else {
    return false;
  }
}

void TtMessageBarPrivate::_drawSuccess(QPainter *painter) {
  Q_Q(TtMessageBar);
  painter->setBrush(QColor(0xE0, 0xF6, 0xDD));
  QRect foregroundRect(_shadowBorderWidth, _shadowBorderWidth,
                       q->width() - 2 * _shadowBorderWidth,
                       q->height() - 2 * _shadowBorderWidth);
  painter->drawRoundedRect(foregroundRect, _borderRadius, _borderRadius);
  // 图标绘制
  painter->save();
  painter->setPen(Qt::white);
  QPainterPath textPath;
  textPath.addEllipse(QPoint(_leftPadding + 6, q->height() / 2), 9, 9);
  painter->setClipPath(textPath);
  painter->fillPath(textPath, QColor(0x11, 0x77, 0x10));
  QFont iconFont = QFont("ElaAwesome");
  iconFont.setPixelSize(12);
  painter->setFont(iconFont);
  // painter->drawText(_leftPadding, 0, q->width(), q->height(),
  // Qt::AlignVCenter,
  //                   QChar((unsigned short)TtIconType::Check));
  painter->restore();
  // 文字颜色
  painter->setPen(QPen(Qt::black));
}

void TtMessageBarPrivate::_drawWarning(QPainter *painter) {
  Q_Q(TtMessageBar);
  painter->setBrush(QColor(0x6B, 0x56, 0x27));
  QRect foregroundRect(_shadowBorderWidth, _shadowBorderWidth,
                       q->width() - 2 * _shadowBorderWidth,
                       q->height() - 2 * _shadowBorderWidth);
  painter->drawRoundedRect(foregroundRect, _borderRadius, _borderRadius);
  // 图标绘制
  // exclamation
  painter->save();
  painter->setPen(Qt::black);
  QPainterPath textPath;
  textPath.addEllipse(QPoint(_leftPadding + 6, q->height() / 2), 9, 9);
  painter->setClipPath(textPath);
  painter->fillPath(textPath, QColor(0xF8, 0xE2, 0x23));
  painter->drawText(_leftPadding + 4, 0, q->width(), q->height(),
                    Qt::AlignVCenter, "!");
  painter->restore();
  // 文字颜色
  painter->setPen(QColor(0xFA, 0xFA, 0xFA));
}

void TtMessageBarPrivate::_drawInformation(QPainter *painter) {
  Q_Q(TtMessageBar);
  painter->setBrush(QColor(0xF4, 0xF4, 0xF4));
  QRect foregroundRect(_shadowBorderWidth, _shadowBorderWidth,
                       q->width() - 2 * _shadowBorderWidth,
                       q->height() - 2 * _shadowBorderWidth);
  painter->drawRoundedRect(foregroundRect, _borderRadius, _borderRadius);
  // 图标绘制
  painter->save();
  painter->setPen(Qt::white);
  QPainterPath textPath;
  textPath.addEllipse(QPoint(_leftPadding + 6, q->height() / 2), 9, 9);
  painter->setClipPath(textPath);
  painter->fillPath(textPath, QColor(0x00, 0x66, 0xB4));
  painter->drawText(_leftPadding + 4, 0, q->width(), q->height(),
                    Qt::AlignVCenter, "i");
  painter->restore();
  // 文字颜色
  painter->setPen(Qt::black);
}

void TtMessageBarPrivate::_drawError(QPainter *painter) {
  Q_Q(TtMessageBar);
  painter->setBrush(QColor(0xFE, 0xE7, 0xEA));
  QRect foregroundRect(_shadowBorderWidth, _shadowBorderWidth,
                       q->width() - 2 * _shadowBorderWidth,
                       q->height() - 2 * _shadowBorderWidth);
  painter->drawRoundedRect(foregroundRect, _borderRadius, _borderRadius);
  // 图标绘制
  painter->save();
  painter->setPen(Qt::white);
  QPainterPath textPath;
  textPath.addEllipse(QPoint(_leftPadding + 6, q->height() / 2), 9, 9);
  painter->setClipPath(textPath);
  painter->fillPath(textPath, QColor(0xBA, 0x2D, 0x20));
  QFont iconFont = QFont("ElaAwesome");
  iconFont.setPixelSize(13);
  painter->setFont(iconFont);
  // painter->drawText(_leftPadding + 1, 0, q->width(), q->height(),
  //                   Qt::AlignVCenter,
  //                   QChar((unsigned short)TtIconType::Xmark));
  painter->restore();
  // 文字颜色
  painter->setPen(Qt::black);
}

} // namespace Ui
