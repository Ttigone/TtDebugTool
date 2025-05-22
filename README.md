<div align=center>
<!-- <img width=64 src="./common/delegateui_icon.svg"> -->

基于 QtWidget  开发

**支持 串口 | Tcp | Udp | MQTT | ModBus 调试工具 [TtDebugTool](https://github.com/mengps/DelegateUI)**

</div>

Supported Environments
======================

Platforms
---------

* Windows (MINGW, Qt 6.6.3)
* Windows (MSVC, Qt 6.6.3) 



功能
======================

* 支持 串口通信, 自定义数据解析(HEX), 同时能够编写 Lua 代码, 对解析值操作(整数类型), 绘制数据曲线.
* 支持 TCP, UDP 通信.
* 支持 MQTT 客户端通信
* 支持 ModBus 从机通信, 绘制数据曲线.
* 支持配置方案本地存储并在打开界面时恢复配置


## 预览
双语界面(仍有 Bug)

[![image.png](https://pic1.imgdb.cn/item/682f482c58cb8da5c807f4af.png)](https://pic1.imgdb.cn/item/682f482c58cb8da5c807f4af.png)

[![image.png](https://pic1.imgdb.cn/item/68260f6158cb8da5c8f49e70.png)](https://pic1.imgdb.cn/item/68260f6158cb8da5c8f49e70.png)


### 串口

[![image.png](https://pic1.imgdb.cn/item/682f3f7d58cb8da5c807c7b8.png)](https://pic1.imgdb.cn/item/682f3f7d58cb8da5c807c7b8.png)

[![image.png](https://pic1.imgdb.cn/item/682f402958cb8da5c807cc9a.png)](https://pic1.imgdb.cn/item/682f402958cb8da5c807cc9a.png)

保存数据(CSV 格式), 默认在 Data 目录下
[![image.png](https://pic1.imgdb.cn/item/682f404c58cb8da5c807cdad.png)](https://pic1.imgdb.cn/item/682f404c58cb8da5c807cdad.png)
[![image.png](https://pic1.imgdb.cn/item/682f40ab58cb8da5c807cffb.png)](https://pic1.imgdb.cn/item/682f40ab58cb8da5c807cffb.png)

### Tcp

[![image.png](https://pic1.imgdb.cn/item/682f419058cb8da5c807d59b.png)](https://pic1.imgdb.cn/item/682f419058cb8da5c807d59b.png)

[![image.png](https://pic1.imgdb.cn/item/682f41a458cb8da5c807d59c.png)](https://pic1.imgdb.cn/item/682f41a458cb8da5c807d59c.png)

### Udp

[![image.png](https://pic1.imgdb.cn/item/68260fb858cb8da5c8f49ef3.png)](https://pic1.imgdb.cn/item/68260fb858cb8da5c8f49ef3.png)

### MQTT

[![image.png](https://pic1.imgdb.cn/item/682f459758cb8da5c807d967.png)](https://pic1.imgdb.cn/item/682f459758cb8da5c807d967.png)

### ModBus

读取线圈数据, 并显示数据图表
[![image.png](https://pic1.imgdb.cn/item/682f468458cb8da5c807e48a.png)](https://pic1.imgdb.cn/item/682f468458cb8da5c807e48a.png)

读取保持寄存器的数据, 并显示到图表中
[![image.png](https://pic1.imgdb.cn/item/682f46ab58cb8da5c807e696.png)](https://pic1.imgdb.cn/item/682f46ab58cb8da5c807e696.png)

## 许可证

使用 `MIT LICENSE`


## 第三方库支持

| 三方库名称   | 版本号 | 应用位置  | 三方库协议类型 |
| ------------ | ---------- | --------------------- | -------------- |
| QWindowKit   | 1.2	| 无边框窗口标题栏            | Apache 2.0 License|
| Qt-Advanced-Stylesheets | 1.0.4	| 切换样式表| LGPL 2.1 |
| QtMqtt| 6.4.3 | 支持 Mqtt 功能|
| libmodbus | 3.1.11 | 支持 ModBus 功能|
| QCustomPlot | 2.1.0 | 数据可视化支持|



## Star 历史
[![Star History Chart](https://api.star-history.com/svg?repos=Ttigone/TtDebugTool&type=Date)](https://www.star-history.com/#Ttigone/TtDebugTool&Date)


## TODO

* 完善串口部分功能
* 支持 MqttBroker 服务器功能
* 支持 ModBus 主机功能
* 完善 Lua 解析功能, 支持更多数据类型

## BUG

* Udp 通信待修复
* Modbus 数据存储