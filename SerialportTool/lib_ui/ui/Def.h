#ifndef UI_DEF_H
#define UI_DEF_H

#include <QObject>

#if defined(_WIN32) || defined(_WIN64)

#if defined(Tt_EXPORT)
#define Tt_EXPORT __declspec(dllexport)

#else
#define Tt_EXPORT __declspec(dllimport)

#endif

#else
#define Tt_EXPORT
#endif

namespace TtMessageBarType {
Q_NAMESPACE_EXPORT(Tt_EXPORT)
enum PositionPolicy {
  Left = 0x00,
  Top = 0x01,
  Right = 0x02,
  Bottom = 0x03,

};
Q_ENUM_NS(PositionPolicy)

enum MessageType {
  Success = 0x01,
  Warning = 0x02,
  Infomation = 0x03,
  Error = 0x04,

};
Q_ENUM_NS(MessageType)

}  // namespace TtMessageBarType

#endif  // UI_DEF_H
