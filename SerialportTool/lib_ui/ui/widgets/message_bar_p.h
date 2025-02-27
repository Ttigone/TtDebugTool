#ifndef UI_WIDGETS_MESSAGE_BAR_P_H
#define UI_WIDGETS_MESSAGE_BAR_P_H

#include <QObject>
#include "ui/Def.h"
#include "ui/singleton.h"
#include "ui/ui_pch.h"

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

enum WorkStatus {
  Idle = 0x0000,                 // 空闲
  CreateAnimation = 0x0001,      // 创建动画
  OtherEventAnimation = 0x0002,  // 其他动画在运行
};

namespace Ui {

class TtMessageBar;

class TtMessageBarManager : public QObject {
  Q_OBJECT
  // 单例模式
  Q_SINGLETON_CREATE_H(TtMessageBarManager)

 public:
  //请求事件堆栈调用
  void requestMessageBarEvent(TtMessageBar* messageBar);
  //发布创建事件
  void postMessageBarCreateEvent(TtMessageBar* messageBar);
  //发布终止事件
  void postMessageBarEndEvent(TtMessageBar* messageBar);
  //强制发布终止事件
  void forcePostMessageBarEndEvent(TtMessageBar* messageBar);
  //获取当前事件数量
  int getMessageBarEventCount(TtMessageBar* messageBar);
  //更新活动序列
  void updateActiveMap(TtMessageBar* messageBar, bool isActive);

 private:
  explicit TtMessageBarManager(QObject* parent = nullptr);
  ~TtMessageBarManager();

 private:
  //
  QMap<TtMessageBar*, QList<QVariantMap>> messagebar_event_map_;
};

class TtMessageBarPrivate : public QObject {
  Q_OBJECT
  // 公有类
  Q_D_CREATE(TtMessageBar)
  Q_PROPERTY_CREATE(qreal, Opacity)  // 透明度属性创建
 public:
  explicit TtMessageBarPrivate(QObject* parent = nullptr);
  ~TtMessageBarPrivate();

  void tryToRequestMessageBarEvent();
  WorkStatus getWorkMode() const;
  Q_INVOKABLE void onOtherMessageBarEnd(QVariantMap eventData);
  Q_INVOKABLE void messageBarEnd(QVariantMap eventData);
  Q_SLOT void onCloseButtonClicked();

 private:
  Q_INVOKABLE void _messageBarCreate(int displayMsec);

  // 初始坐标计算
  void _calculateInitialPos(int& startX, int& startY, int& endX, int& endY);
  //获取总高度和次序信息
  QList<int> _getOtherMessageBarTotalData(bool isJudgeCreateOrder = false);
  //计算目标坐标
  qreal _calculateTargetPosY();

  //创建次序判断
  bool _judgeCreateOrder(TtMessageBar* otherMessageBar);

  // 绘制函数
  void _drawSuccess(QPainter* painter);      // 成功
  void _drawWarning(QPainter* painter);      // 警告
  void _drawInformation(QPainter* painter);  // 提醒
  void _drawError(QPainter* painter);        // 错误

  friend class TtMessageBarManager;
  // TtThemeType::ThemeMode _themeMode; // 主题
  int _borderRadius{6};  // 圆角
  QString _title;        // 文字
  QString _text;         // 标题
  TtMessageBarType::PositionPolicy _policy;    // 摆放策略
  TtMessageBarType::MessageMode _messageMode;  // 显示类型
  qreal create_time_{0};  // 创建时间

  // 位置数据
  int _leftPadding{20};                 // 左边框到图标中心
  int _titleLeftSpacing{30};            // 图标中心到Title左侧
  int _textLeftSpacing{15};             // Title右侧到Text左侧
  int _closeButtonLeftRightMargin{20};  // closeButton左右总Margin
  int _closeButtonWidth{30};
  int _messageBarHorizontalMargin{20};
  int _messageBarVerticalBottomMargin{20};
  int _messageBarVerticalTopMargin{50};
  int _messageBarSpacing{15};
  int _shadowBorderWidth{6};

  // 逻辑数据
  bool _isMessageBarCreateAnimationFinished{false};
  bool _isReadyToEnd{false};
  bool _isNormalDisplay{false};
  bool _isMessageBarEventAnimationStart{false};
  // TtIconButton* _closeButton{nullptr};      // 关闭按钮
  QPushButton* _closeButton{nullptr};  // 关闭按钮
};

}  // namespace Ui

#endif  // UI_WIDGETS_MESSAGE_BAR_P_H
