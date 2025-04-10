#include "core/lua_kernel.h"

namespace Core {

LuaKernel::LuaKernel() {
  // 创建 lua 对象
  lua_state_ = luaL_newstate();

  luaL_openlibs(lua_state_);

}

LuaKernel::~LuaKernel() {}

void LuaKernel::doLuaCode(const QString& code, int data, double& result) {
  if (code.isEmpty()) {
    // 不做处理, 直接返回
    result = data;
    return;
  }
  auto cstrCode = code.toStdString().c_str();
  lua_State* L = this->lua_state_;
  if (luaL_dostring(L, cstrCode)) /* 从字符串中加载LUA脚本 */
  {
    qDebug() << "LUA脚本有误！" << lua_tostring(L, -1);
    lua_pop(L, 1);
    return;
  }

  // 获取变量
  /* 函数入栈 */
  lua_getglobal(L, "getValue");

  // 参数  double 类型
  lua_pushnumber(L, data);

  // /* 第二个函数参数入栈 */
  // lua_pushnumber(L, 200);

  /*
     * 执行函数调用
     * 2表示lua脚本中add函数需要输入两个函数参数
     * 1表示lua脚本中add函数有一个返回值
     * 执行完函数调用后，lua自动出栈函数和参数
     */
  lua_call(L, 1, 1);

  /*
     * 得到add函数执行结果
     * -1表示最后一个返回值，因为lua的函数可以返回多个值的。
     */
  // auto sum = lua_tonumber(L, -1);
  result = lua_tonumber(L, -1);
  // qDebug() << sum;

  /* 出栈一个数据。此时栈中存的是 getValue 函数的执行结果，所以需要出栈 */
  lua_pop(L, 1);
}

}  // namespace Core
