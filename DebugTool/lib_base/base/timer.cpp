#include "base/timer.h"

#include <chrono>

namespace base {
Timer::Timer(std::unique_ptr<QThread*> thread, std::function<void()> callback) {
}

Timer::Timer(std::function<void()> callback) {}

void Timer::cancle() {
  if (isActive()) {
    killTimer(timer_id_);
  }
}

void Timer::timerEvent(QTimerEvent* event) { QObject::timerEvent(event); }

void Timer::start(int64 timeout, Qt::TimerType type, Repeat repeat) {
  // 首先取消
  cancle();
  setRepeat(repeat);
  adjusted_ = false;
  setTimerout(timeout);
  timer_id_ = startTimer(timeout, type);
  if (timer_id_) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    // 转换为 ms
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    // timer_id_
    next_ = static_cast<int64>(millis) + timeout_;
  } else {
    // 存储着 下一次触发的开始时间
    next_ = 0;
  }
}

void Timer::setTimerout(int64 timeout) {
  timeout_ = static_cast<unsigned int>(timeout);
}

}  // namespace base
