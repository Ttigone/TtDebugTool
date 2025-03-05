#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

//枚举类导出  兼容QT5低版本
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define Q_BEGIN_ENUM_CREATE_SRC(CLASS) \
  namespace CLASS {                    \
  Q_NAMESPACE
// Q_NAMESPACE_EXPORT(Tt_EXPORT)

#define Q_END_ENUM_CREATE(CLASS) }

#define Q_ENUM_CREATE(CLASS) Q_ENUM_NS(CLASS)

#else
#define Q_BEGIN_ENUM_CREATE(CLASS) \
  class CLASS : public QObject {   \
    Q_OBJECT                       \
   public:

#define Q_END_ENUM_CREATE(CLASS) \
 private:                        \
  Q_DISABLE_COPY(CLASS)          \
  }                              \
  ;

#define Q_ENUM_CREATE(CLASS) Q_ENUM(CLASS)
#endif

Q_BEGIN_ENUM_CREATE_SRC(TtProtocolType)
enum ProtocolRole { Client, Server };
Q_ENUM_CREATE(ProtocolRole)
Q_END_ENUM_CREATE(TtProtocolType)

Q_BEGIN_ENUM_CREATE_SRC(TtFunctionalCategory)
enum Category { Communication, Instruction, Simulate };
Q_ENUM_CREATE(Category)
Q_END_ENUM_CREATE(TtProtocolType)

Q_BEGIN_ENUM_CREATE_SRC(TtUdpMode)
enum Mode { Unicast, Multicast, Broadcast };
Q_ENUM_CREATE(Mode)
Q_END_ENUM_CREATE(TtUdpMode)

#endif  // GLOBAL_DEF_H
