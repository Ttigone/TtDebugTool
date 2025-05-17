#ifndef CORE_LUA_KERNEL_H
#define CORE_LUA_KERNEL_H

#include "lua-5.4.7/src/lua.hpp"

namespace Core {

class LuaKernel {
 public:
  LuaKernel();
  ~LuaKernel();

  bool doLuaCode(const QString& code, int data, double& result);
  bool doLuaCode(const QString& code, const QVariantList& args, double& result);

 private:
  lua_State* lua_state_;
};

}  // namespace Core

#endif  // CORE_LUA_KERNEL_H
