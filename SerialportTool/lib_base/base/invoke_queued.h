#ifndef INVOKE_QUEUED_H
#define INVOKE_QUEUED_H

#include <QEvent>
#include <QCoreApplication>

#include "basic_types.h"

/// @brief 调用一个函数，该函数会被放入队列中，等待下一帧调用

namespace base {

// 队列事件
// 自定义事件类
class InvokeQueuedEvent : public QEvent {
 public:
	/// 
	/// @param func 执行的动作函数
	explicit InvokeQueuedEvent(std::function<void()> &&func)
      : QEvent(Type()), func_(std::move(func)) {}

	/// @brief 获取事件类型
  static QEvent::Type Type() {
    // 静态变量，只会初始化一次
    // 只会执行一次, 确保返回的结果是唯一的
    static auto result = static_cast<QEvent::Type>(QEvent::registerEventType());
    return result;
  }

  /// @brief 调用方法
  void invoke() {
    // 调用函数
    func_();
  }

 private:
  // 函数对象
  std::function<void()> func_;
};

}  // namespace base


/// 
/// @tparam T 
/// @param context 要发送的目的地
/// @param func lambda 表达式
template <typename T>
inline void InvokeQueued(const QObject *context, T &&func) {
  // std::forward : 完美转发
  QCoreApplication::postEvent(
      const_cast<QObject *>(context),
      new base::InvokeQueuedEvent(std::forward<T>(func)));
}


#endif