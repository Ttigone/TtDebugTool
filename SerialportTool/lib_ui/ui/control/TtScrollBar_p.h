#ifndef UI_CONTROL_TTSCROLLBAR_P_H
#define UI_CONTROL_TTSCROLLBAR_P_H

#include <QAbstractScrollArea>
#include <QObject>
#include <QScrollBar>

namespace Ui {

class TtScrollBar;

class TtScrollBarPrivate : public QObject {
  Q_OBJECT
  Q_D_CREATE(TtScrollBar)
  Q_PROPERTY_CREATE_D(bool, IsAnimation)
  Q_PROPERTY_CREATE_D(qreal, SpeedLimit)
  // Q_PROPERTY_CREATE(int, TargetMaximum)
  Q_PROPERTY(
      int pTargetMaximum MEMBER pTargetMaximum_ NOTIFY pTargetMaximumChanged)
 public:
  explicit TtScrollBarPrivate(QObject* parent = nullptr);
  ~TtScrollBarPrivate();

  Q_SIGNAL void pTargetMaximumChanged();

  void setTargetMaximum(int M) {
    pTargetMaximum_ = M;
    Q_EMIT pTargetMaximumChanged();
  }
  int getTargetMaximum() const { return pTargetMaximum_; }

 public slots:
  void onRangeChanged(int min, int max);

 private:
  // 映射处理函数
  void initAllConfig();
  void handleScrollBarValueChanged(QScrollBar* scrollBar, int value);
  void handleScrollBarRangeChanged(int min, int max);
  void handleScrollBarGeometry();
  void scroll(Qt::KeyboardModifiers modifiers, int delta);
  int pixelPosToRangeValue(int pos) const;

  QScrollBar* origin_scrollBar_{nullptr};
  QAbstractScrollArea* origin_scrollArea_{nullptr};
  QTimer* expand_timer_{nullptr};  // 展开时间
  bool is_expand_{false};
  QPropertyAnimation* slide_smooth_animation_{nullptr};  // 动画
  int scroll_value_{-1};

 private:
  int pTargetMaximum_;
};

}  // namespace Ui

#endif  // UI_CONTROL_TTSCROLLBAR_P_H
