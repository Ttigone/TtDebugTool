/*****************************************************************//**
 * \file   flags.h
 * \brief  标志 - 模板类
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/
#ifndef BASIC_FLAGS_H
#define BASIC_FLAGS_H

namespace base {

template <typename EnumType>
class flags {
 public:
  using Enum = EnumType;
  // 基础类型, 升级为 标准库的模板类
  using Type = std::underlying_type_t<Enum>;

  constexpr flags() = default;
  constexpr flags(Enum value) noexcept : value_(static_cast<Type>(value)) {}

  // 从二进制数转换为 flags
  static constexpr flags FromRaw(Type value) noexcept {
    return flags(static_cast<Enum>(value));
  }

  constexpr auto value() const noexcept { return value_; }
  constexpr operator Type() const noexcept { return value(); }

  constexpr auto& operator|=(flags other) noexcept {
    value_ |= other.value();
    return *this;
  }
  constexpr auto& operator&=(flags other) noexcept {
    value_ &= other.value();
    return *this;
  }
  constexpr auto& operator^=(flags other) noexcept {
    value_ ^= other.value();
    return *this;
  }
  // 单目运算符
  constexpr auto operator~() const noexcept { return FromRaw(~value()); }
  constexpr auto operator|(flags other) const noexcept {
    return flags(*this) | other;
  }
  constexpr auto operator&(flags other) const noexcept {
    return flags(*this) & other;
  }
  constexpr auto operator^(flags other) const noexcept {
    return flags(*this) ^ other;
  }

  // Enum 参数的构造函数 构造 flags
  constexpr auto operator|(Enum other) const noexcept {
    return flags(*this) |= other;
  }
  constexpr auto operator&(Enum other) const noexcept {
    return flags(*this) &= other;
  }
  constexpr auto operator^(Enum other) const noexcept {
    return flags(*this) ^= other;
  }

  // 对比值是否相等
  constexpr auto operator==(Enum other) const noexcept {
    return value() == static_cast<Type>(other);
  }
  constexpr auto operator!=(Enum other) const noexcept {
    return !(*this == other);
  }
  constexpr auto operator<(Enum other) const noexcept {
    return value() < static_cast<Type>(other);
  }
  constexpr auto operator>(Enum other) const noexcept {
    return *this > other;
  }
  constexpr auto operator<=(Enum other) const noexcept {
    return !(*this > other);
  }
  constexpr auto operator>=(Enum other) const noexcept {
    return !(*this < other);
  }

 private:
  Type value_ = 0;
};


} // namespace base

#endif // BASIC_FLAGS_H
