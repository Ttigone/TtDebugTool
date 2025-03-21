#ifndef UI_PCH_H
#define UI_PCH_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include <QJsonObject>

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>

#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QSvgWidget>
#include <QWidget>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <any>

#include "base/basic_types.h"
#include "base/flags.h"
#include "base/timer.h"

#if defined(_WIN32) || defined(_WIN64)
// // 宏在 CMakeLists.txt 中定义
#if defined(TTLIBUI_LIBRARY)
#define Tt_EXPORT __declspec(dllexport)
// #define Tt_EXPORT Q_DECL_EXPORT
#else
#define Tt_EXPORT __declspec(dllimport)
// #define Tt_EXPORT Q_DECL_IMPORT
#endif

#else
#define Tt_EXPORT
#endif

// PIMPL 设计模式 在公共类中使用
#define Q_Q_CREATE(CLASS)                                      \
 protected:                                                    \
  explicit CLASS(CLASS##Private& dd, CLASS* parent = nullptr); \
  QScopedPointer<CLASS##Private> d_ptr;                        \
                                                               \
 private:                                                      \
  Q_DISABLE_COPY(CLASS)                                        \
  Q_DECLARE_PRIVATE(CLASS);

#define Q_D_CREATE(CLASS) \
 protected:               \
  CLASS* q_ptr;           \
                          \
 private:                 \
  Q_DECLARE_PUBLIC(CLASS);

// 属性成员创建
#define Q_PROPERTY_CREATE(TYPE, M)                          \
  Q_PROPERTY(TYPE p##M MEMBER p##M##_ NOTIFY p##M##Changed) \
 public:                                                    \
  void set##M(TYPE M) {                                     \
    if (p##M##_ != M) {                                     \
      p##M##_ = M;                                          \
      Q_EMIT p##M##Changed();                               \
    }                                                       \
  }                                                         \
  TYPE get##M() const {                                     \
    return p##M##_;                                         \
  }                                                         \
  Q_SIGNAL void p##M##Changed();                            \
                                                            \
 private:                                                   \
  TYPE p##M##_;

// Q_D Q_Q普通属性快速创建
// 公共类头文件使用, 只有声明
#define Q_PROPERTY_CREATE_Q_H(TYPE, M)                                \
  Q_PROPERTY(TYPE p##M READ get##M WRITE set##M NOTIFY p##M##Changed) \
 public:                                                              \
  Q_SIGNAL void p##M##Changed();                                      \
  void set##M(TYPE M);                                                \
  TYPE get##M() const;

// 完成公共类中属性成员的 set 和 get, 在 CPP 中使用
#define Q_PROPERTY_CREATE_Q_CPP(CLASS, TYPE, M) \
  void CLASS::set##M(TYPE M) {                  \
    Q_D(CLASS);                                 \
    d->p##M##_ = M;                             \
    Q_EMIT p##M##Changed();                     \
  }                                             \
  TYPE CLASS::get##M() const {                  \
    return d_ptr->p##M##_;                      \
  }

// 私有类头文件使用, 定义属性成员
#define Q_PROPERTY_CREATE_D(TYPE, M) \
 private:                            \
  TYPE p##M##_;

// 私有类, 定义私有成员
#define Q_PRIVATE_CREATE_D(TYPE, M) \
 private:                           \
  TYPE p##M##_;

// 私有成员创建和定义
#define Q_PRIVATE_CREATE(TYPE, M) \
 public:                          \
  void set##M(TYPE M) {           \
    p##M##_ = M;                  \
  }                               \
  TYPE get##M() const {           \
    return p##M##_;               \
  }                               \
                                  \
 private:                         \
  TYPE p##M##_;

#endif  // UI_PCH_H
