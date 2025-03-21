#ifndef UI_DEF_H
#define UI_DEF_H

#include <QObject>

#include "ui/ui_pch.h"

//枚举类导出  兼容QT5低版本
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define Q_BEGIN_ENUM_CREATE(CLASS) \
  namespace CLASS {                \
  Q_NAMESPACE_EXPORT(Tt_EXPORT)

#define Q_END_ENUM_CREATE(CLASS) }

#define Q_ENUM_CREATE(CLASS) Q_ENUM_NS(CLASS)
#else
#define Q_BEGIN_ENUM_CREATE(CLASS)         \
  class Tt_EXPORT CLASS : public QObject { \
    Q_OBJECT                               \
   public:

#define Q_END_ENUM_CREATE(CLASS) \
 private:                        \
  Q_DISABLE_COPY(CLASS)          \
  }                              \
  ;

#define Q_ENUM_CREATE(CLASS) Q_ENUM(CLASS)
#endif

Q_BEGIN_ENUM_CREATE(TtThemeType)
enum ThemeMode {
  Light = 0x0000,
  Dark = 0x0001,
};

Q_ENUM_CREATE(ThemeMode)
enum ThemeColor {
  ScrollBarHandle,
  ToggleSwitchNoToggledCenter,
  WindowBase,
  WindowCentralStackBase,
  PrimaryNormal,
  PrimaryHover,
  PrimaryPress,
  PopupBorder,
  PopupBorderHover,
  PopupBase,
  PopupHover,
  DialogBase,
  DialogLayoutArea,
  BasicText,
  BasicTextInvert,
  BasicDetailsText,
  BasicTextNoFocus,
  BasicTextDisable,
  BasicTextPress,
  BasicBorder,
  BasicBorderDeep,
  BasicBorderHover,
  BasicBase,
  BasicBaseDeep,
  BasicDisable,
  BasicHover,
  BasicPress,
  BasicSelectedHover,
  BasicBaseLine,
  BasicHemline,
  BasicIndicator,
  BasicChute,
  BasicAlternating,
  BasicBaseAlpha,
  BasicBaseDeepAlpha,
  BasicHoverAlpha,
  BasicPressAlpha,
  BasicSelectedAlpha,
  BasicSelectedHoverAlpha,
  StatusDanger,
};
Q_ENUM_CREATE(ThemeColor)
Q_END_ENUM_CREATE(TtThemeType)

Q_BEGIN_ENUM_CREATE(TtMessageBarType)
enum PositionPolicy {
  Top = 0x0000,
  Left = 0x0001,
  Bottom = 0x0002,
  Right = 0x0003,
  TopRight = 0x0004,
  TopLeft = 0x0005,
  BottomRight = 0x0006,
  BottomLeft = 0x0007,
};
Q_ENUM_CREATE(PositionPolicy)

enum MessageMode {
  Success = 0x0000,
  Warning = 0x0001,
  Information = 0x0002,
  Error = 0x0003,
};
Q_ENUM_CREATE(MessageMode)
Q_END_ENUM_CREATE(TtMessageBarType)

Q_BEGIN_ENUM_CREATE(TtIconType)
enum IconName {
  None = 0x0,
  Check = 0xea6c,
};
Q_ENUM_CREATE(IconName)
Q_END_ENUM_CREATE(TtIconType)

Q_BEGIN_ENUM_CREATE(TtPopUpDirection)
enum PopUpDirection { Left, Right, Top, Bottom };
Q_ENUM_CREATE(PopUpDirection)
Q_END_ENUM_CREATE(TtPopUpDirection)

// 等效
// namespace TtMessageBarType {
// Q_NAMESPACE_EXPORT(Tt_EXPORT)
// enum PositionPolicy {
//   Left = 0x00,
//   Top = 0x01,
//   Right = 0x02,
//   Bottom = 0x03,

// };
// //Q_ENUM_NS(PositionPolicy)

// enum MessageType {
//   Success = 0x01,
//   Warning = 0x02,
//   Infomation = 0x03,
//   Error = 0x04,

// };
//Q_ENUM_NS(MessageType)

// }  // namespace TtMessageBarType

#endif  // UI_DEF_H
