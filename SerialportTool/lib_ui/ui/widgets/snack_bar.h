/*****************************************************************/ /**
 * \file   snack_bar.h
 * \brief  
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_WIDGETS_SNACK_BAR_H
#define UI_WIDGETS_SNACK_BAR_H

#include <QFinalState>
//#include <QGuiApplication>
#include <QApplication>
#include <QLabel>
#include <QObject>
#include <QPropertyAnimation>
#include <QQueue>
#include <QScreen>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include <QTimer>

namespace Ui {

class SnackBar : public QLabel {
  Q_OBJECT
 public:
  enum State { Entering, Visible, Leaving };

  explicit SnackBar(const QString& text, int duration = 3000,
                    QWidget* parent = nullptr)
      : QLabel(text, parent), m_duration(duration) {
    initUI();
    initStateMachine();
  }
  ~SnackBar() {}

  void startFlow() { m_machine->start(); }

 signals:
  void finished();
  void enteringFinished();  // 添加缺失的信号
  void leavingFinished();   // 添加缺失的信号
  void timeout();           // 添加缺失的信号

 protected:
  void mousePressEvent(QMouseEvent* event) override {
    Q_UNUSED(event)
    emit finished();
  }

 private slots:
  void onAnimationFinished() {
    if (m_currentState == Leaving)
      emit finished();
  }

 private:
  void initUI() {
    setStyleSheet(
        "background: #323232;"
        "color: white;"
        "border-radius: 4px;"
        "padding: 14px 16px;"
        "font: 12pt 'Microsoft YaHei';");
    setAlignment(Qt::AlignCenter);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    adjustSize();
  }

  void initStateMachine() {
    m_machine = new QStateMachine(this);

    // 状态定义
    QState* entering = new QState(m_machine); // 进入状态
    QState* visible = new QState(m_machine);  // 可视化状态
    QState* leaving = new QState(m_machine);  // 离开状态
    QFinalState* final = new QFinalState(m_machine);  // 最后结束状态

    // 状态属性设置
    entering->assignProperty(this, "pos", calculateEnterPosition());
    visible->assignProperty(this, "pos", calculateVisiblePosition());
    leaving->assignProperty(this, "pos", calculateExitPosition());

    // 创建动画
    QPropertyAnimation* enterAnim = new QPropertyAnimation(this, "pos", this);
    enterAnim->setDuration(220);
    enterAnim->setEasingCurve(QEasingCurve::OutBack);

    QPropertyAnimation* leaveAnim = new QPropertyAnimation(this, "pos", this);
    leaveAnim->setDuration(220);
    leaveAnim->setEasingCurve(QEasingCurve::InQuad);

    // 创建状态转移并绑定动画

    // 完成进入时动画, 进入 visible 状态
    QAbstractTransition* enterTransition =
        entering->addTransition(this, &SnackBar::enteringFinished, visible);
    enterTransition->addAnimation(enterAnim);  // 应用动画

    // 完成离开时动画, 进入 final 状态
    QAbstractTransition* leaveTransition =
        leaving->addTransition(this, &SnackBar::leavingFinished, final);
    leaveTransition->addAnimation(leaveAnim);

    // 定时器配置, 设置 visible 的显示时间 ??? 
    QTimer* timer = new QTimer(visible);
    timer->setInterval(m_duration);
    timer->setSingleShot(true);
    // 当定时器超时时, 进入 leaving 状态
    visible->addTransition(timer, &QTimer::timeout, leaving);

    // 状态进入逻辑
    connect(entering, &QState::entered, [this] {
      m_currentState = Entering;
      // 220 ms 后发射 enteringFinished 信号, 进入状态机
      QTimer::singleShot(220, this, &SnackBar::enteringFinished);
    });

    connect(visible, &QState::entered, [this, timer] {
      m_currentState = Visible;
      // 开启计时器, 显示时间
      timer->start();
    });

    connect(leaving, &QState::entered, [this] {
      m_currentState = Leaving;
      // 进入离开状态, 220ms 后触发状态
      QTimer::singleShot(220, this, &SnackBar::leavingFinished);
    });

    // 初始状态为进入
    m_machine->setInitialState(entering);
    // TODO Bug, 以下语句会导致程序奔溃 testAttribute
    //connect(m_machine, &QStateMachine::finished, this, &SnackBar::deleteLater);
  }

  QPoint calculateEnterPosition() const { /* 进入动画起始位置 */
    //QScreen* screen = QGuiApplication::primaryScreen();
    ////QScreen* screen = QGuiApplication::primaryScreen();
    //return QPoint(screen->geometry().center().x() - width() / 2,
    //              screen->geometry().bottom() + height());

    // 父窗口
    QWidget* parent = parentWidget();
    if (!parent) {
      qDebug() << "active window";
      parent = qApp->activeWindow();
    }

    if (parent) {
      return QPoint(parent->geometry().center().x() - width() / 2,
                    parent->geometry().top() - height()  // 初始位置在窗口上方
      );
    } else {
      QScreen* screen = QGuiApplication::primaryScreen();
      return QPoint(
          screen->geometry().center().x() - width() / 2,
          screen->geometry().center().y() - height() * 2  // 屏幕中心上方
      );
    }
  }
  QPoint calculateVisiblePosition() const { /* 屏幕底部居中位置 */
    //QScreen* screen = QGuiApplication::primaryScreen();
    //return QPoint(screen->geometry().center().x() - width() / 2,
    //              screen->geometry().bottom() - height() - 20);
    QWidget* parent = parentWidget();
    if (!parent)
      parent = qApp->activeWindow();

    if (parent) {
      return QPoint(parent->geometry().center().x() - width() / 2,
                    parent->geometry().top() + 20  // 窗口顶部下方20px
      );
    } else {
      QScreen* screen = QGuiApplication::primaryScreen();
      return QPoint(
          screen->geometry().center().x() - width() / 2,
          screen->geometry().center().y() - height() - 20  // 屏幕中心上方
      );
    }
  }
  QPoint calculateExitPosition() const { /* 隐藏动画结束位置 */
    //return pos() + QPoint(0, -50);
    return pos() + QPoint(0, -height() - 20);  // 向上移出可视区域
  }

  QStateMachine* m_machine;
  int m_duration;
  State m_currentState;
};

class SnackBarController : public QObject {
  Q_OBJECT
 public:
  static SnackBarController* instance() {
    static SnackBarController controller;
    return &controller;
  }

  void showMessage(const QString& text, int duration = 3000) {
    SnackBar* bar = new SnackBar(text, duration);
    connect(bar, &SnackBar::finished, this, &SnackBarController::adjustLayout);
    //m_bars.enqueue(bar);
    m_bars.push_back(bar);

    QTimer::singleShot(0, this, [this, bar] {
      bar->show();
      bar->startFlow();
      adjustLayout();
    });
  }

 protected:
  void adjustLayout() {
    //const int spacing = 8;
    //QScreen* screen = QGuiApplication::primaryScreen();
    //const QRect geometry = screen->availableGeometry();

    //int totalHeight = 0;
    //for (auto* bar : m_bars) {
    //  totalHeight += bar->height() + spacing;
    //}

    //int y = geometry.bottom() - 64;  // 底部留出64px边距
    //for (auto* bar : m_bars) {
    //  y -= bar->height() + spacing;
    //  QPropertyAnimation* anim = new QPropertyAnimation(bar, "pos");
    //  anim->setDuration(300);
    //  anim->setEndValue(QPoint(geometry.center().x() - bar->width() / 2, y));
    //  anim->start(QAbstractAnimation::DeleteWhenStopped);
    //}
    // 清理无效指针
    m_bars.erase(std::remove_if(m_bars.begin(), m_bars.end(),
                                [](SnackBar* bar) {
                                  return bar == nullptr || bar->isHidden();
                                }),
                 m_bars.end());

    const int spacing = 8;
    QWidget* parent = qApp->activeWindow();
    if (!parent)
      return;

    int startY = parent->geometry().top() + 50;  // 从窗口顶部开始布局
    int currentY = startY;

    // 从下往上排列（新消息在上方）
    for (auto* bar : m_bars) {
      if (!bar || !bar->isVisible())
        continue;

      // 确保尺寸有效
      bar->ensurePolished();
      bar->adjustSize();

      QPropertyAnimation* anim = new QPropertyAnimation(bar, "pos");
      anim->setDuration(300);
      anim->setEndValue(
          QPoint(parent->geometry().center().x() - bar->width() / 2, currentY));
      anim->start(QAbstractAnimation::DeleteWhenStopped);

      currentY += bar->height() + spacing;
    }
  }

 private:
  //QQueue<SnackBar*> m_bars;
  QVector<SnackBar*> m_bars;  // 改用QVector便于清理
};

}  // namespace Ui

#endif  //SNACK_BAR_H
