#include "core/lua_kernel.h"

namespace Core {

LuaKernel::LuaKernel() {
  // 创建 lua 对象
  lua_state_ = luaL_newstate();

  // 标准库
  luaL_openlibs(lua_state_);
}

LuaKernel::~LuaKernel() { lua_close(lua_state_); }

bool LuaKernel::doLuaCode(const QString &code, int data, double &result) {
  if (code.isEmpty()) {
    // 不做处理, 直接返回
    result = data;
    return false;
  }
  auto cstrCode = code.toStdString().c_str();
  lua_State *L = this->lua_state_;
  if (luaL_dostring(L, cstrCode)) /* 从字符串中加载LUA脚本 */
  {
    qDebug() << "LUA脚本有误！" << lua_tostring(L, -1);
    lua_pop(L, 1);
    return false;
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
  return true;
}

bool LuaKernel::doLuaCode(const QString &code, const QVariantList &args,
                          double &result) {
  if (code.isEmpty()) {
    qDebug() << "code is Empty";
    result = 0;
    return false;
  }
  lua_State *L = lua_state_;
  if (luaL_dostring(L, code.toStdString().c_str())) {
    // 错误逻辑, 需要显示到主界面中
    qDebug() << "LUA ERROR:" << lua_tostring(L, -1);
    lua_pop(L, 1);
    return false;
  }

  // 获取 Lua 函数
  lua_getglobal(L, "getValue");
  // 动态参数入栈
  for (const QVariant &arg : args) {
    switch (arg.userType()) {
    case QMetaType::Int:
      lua_pushinteger(L, arg.toInt());
      break;
    case QMetaType::Double:
      lua_pushnumber(L, arg.toDouble());
      break;
    case QMetaType::QString:
      lua_pushstring(L, arg.toString().toStdString().c_str());
      break;
    case QMetaType::Bool:
      lua_pushboolean(L, arg.toBool());
      break;
    // case QMetaType::QByteArray: {
    //   // 修改参数处理分支, 文本类型
    //   QByteArray bytes = arg.toByteArray();
    //   lua_pushlstring(L, bytes.constData(),
    //                   bytes.size());  // 保留二进制中的 \0 字符
    //   break;
    // function getValue(data)
    //     print(#data)        --> 输出字节长度
    //                          print(data:byte(1)) --> 输出第一个字节的 ASCII
    //                          值
    //     return #data
    //     end
    // }
    case QMetaType::QByteArray: {
      // 二进制数据
      QByteArray bytes = arg.toByteArray();
      lua_createtable(L, bytes.size(), 0); // 创建数组型 table
      for (int i = 0; i < bytes.size(); ++i) {
        // hex
        // 转换为整数
        // 2 ^ 8 =  256 ->  0 ~  255
        lua_pushinteger(L, static_cast<unsigned char>(bytes[i]));
        lua_rawseti(L, -2, i + 1); // Lua 索引从 1 开始
      }
      break;
    }
    default:
      qDebug() << "Unsupported type:" << arg.typeName();
      lua_pop(L, lua_gettop(L));
      result = 0;
      return;
    }
  }
  if (lua_pcall(L, args.size(), 1, 0) != LUA_OK) {
    qDebug() << "Function Error:" << lua_tostring(L, -1);
    lua_pop(L, 1);
    result = 0;
    return;
  }
  // 处理返回值
  result = lua_tonumber(L, -1);
  lua_pop(L, 1);
}

} // namespace Core
