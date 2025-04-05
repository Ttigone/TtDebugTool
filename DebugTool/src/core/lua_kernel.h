#ifndef CORE_LUA_KERNEL_H
#define CORE_LUA_KERNEL_H

#include "lua-5.4.7/src/lua.hpp"

namespace Core {

class LuaKernel {
 public:
  LuaKernel();
  ~LuaKernel();

  void doLuaCode(const char* code, int data);

 private:
  lua_State* lua_state_;
};

}  // namespace Core

#endif  // CORE_LUA_KERNEL_H
