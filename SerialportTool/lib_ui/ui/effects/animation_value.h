// ****************************************************************//**
//  * \file   animation_value.h
//  * \brief  动画属性值
//  *
//  * \author C3H3_Ttigone
//  * \date   August 2024
//  ********************************************************************

#ifndef UI_EFFECTS_ANIMATION_VALUE_H
#define UI_EFFECTS_ANIMATION_VALUE_H


namespace Ui
{
namespace Animation {
enum class Type : uchar {
  normal,
  instant,
};

enum class Activation : uchar {
  normal,
  background,
};

enum class Repeat : uchar {
  loop,
  once,
};

using transition = std::function<float64(float64 delta, float64 dt)>;

extern transition linear;
extern transition sineInOut;
extern transition halfSine;
extern transition easeOutBack;
extern transition easeInCirc;
extern transition easeOutCirc;
extern transition easeInCubic;
extern transition easeOutCubic;
extern transition easeInQuint;
extern transition easeOutQuint;

class Value {
 public:
  using ValueType = float64;
  Value() = default;
};

} // namespace Aniamtion


} // namespace Ui

#endif  // UI_EFFECTS_ANIMATION_VALUE_H
