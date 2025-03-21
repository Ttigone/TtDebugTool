# QtEasyLogger 简单易用的Qt格式化日志库

## 使用方式
1. 在项目主路径下执行命令
   ```bash
   git submodule add git@github.com:hanjianqiao/qt-easy-logger.git submodules/qt-easy-logger
   ```
2. 修改CMakeLists.txt，添加下面指令
   ```
   # 假设你的目标名称为：yourTarget
   add_subdirectory(submodules/qt-easy-logger)
   target_include_directories(yourTarget PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/submodules")
   target_link_libraries(yourTarget PRIVATE qt-easy-logger)
   ```
4. 调用`qInstallMessageHandler(h::Logger::messageHandler);`，即可启用功能。

## 例子
### 代码
```c++
#include "samplewindow.h"
#include <QApplication>
#include <qt-easy-logger/logger.h>    // 引用头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qInstallMessageHandler(h::Logger::messageHandler);    // 启用功能

    qDebug() << "这是测试输出";
    qCritical() << "This is a critical message!";

    SampleWindow w;
    w.show();

    return a.exec();
}
```
### 效果
![image](https://github.com/hanjianqiao/qt-easy-logger/assets/7146341/feb20cf7-814e-4176-a728-d0d242d55b0e)

### 在Release模式下输出文件行号
在默认情况下，release模式输出会是这样，去除了敏感的源码位置信息
```
[2024-02-21T12:58:33][Debug] 启动第一个例子 (:0)
[2024-02-21T12:58:34][Debug] 启动第二个例子 (:0)
```
如果，需要在release模式下输出这些，可以在项目CMakeLists.txt中添加以下定义
```cmake
# Release 模式下也输出文件、函数
add_compile_definitions(QT_MESSAGELOGCONTEXT)
```
设置后输出则包含代码位置信息
```
[2024-02-21T13:01:58][Debug] 启动第一个例子 (D:\repos\qt-frameless-window\main.cpp:24)
[2024-02-21T13:01:58][Warning] 启动第二个例子 (D:\repos\qt-frameless-window\main.cpp:28)
```

## 更多
本项目使用MIT协议，欢迎使用、提建议、参与贡献。
