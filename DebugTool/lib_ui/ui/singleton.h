#ifndef UI_SINGLETON_H
#define UI_SINGLETON_H

#include <QMutex>

template <typename T>
class Singleton {
 public:
  static T* getInstance();

 private:
  Q_DISABLE_COPY(Singleton)
};

template <typename T>
T* Singleton<T>::getInstance() {
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  static T* instance = nullptr;
  if (instance == nullptr) {
    instance = new T();
  }
  return instance;
}

#define Q_SINGLETON_CREATE(Class)           \
 private:                                   \
  friend class Singleton<Class>;            \
                                            \
 public:                                    \
  static Class* getInstance() {             \
    return Singleton<Class>::getInstance(); \
  }

#define Q_SINGLETON_CREATE_H(Class) \
 private:                           \
  static Class* instance_;          \
                                    \
 public:                            \
  static Class* getInstance();

#define Q_SINGLETON_CREATE_CPP(Class) \
  Class* Class::instance_ = nullptr;  \
  Class* Class::getInstance() {       \
    static QMutex mutex;              \
    QMutexLocker locker(&mutex);      \
    if (instance_ == nullptr) {       \
      instance_ = new Class();        \
    }                                 \
    return instance_;                 \
  }

#endif  // UI_SINGLETON_H
