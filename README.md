<div align=center>
<img width=64 src="./common/delegateui_icon.svg">

基于 QtWidget  开发

**支持 串口 | Tcp | Udp | MQTT | ModBus 调试工具 [TtDebugTool](https://github.com/mengps/DelegateUI)**

</div>



## 开发环境

windows 11, Qt 6.4.3 / Qt 6.6.3

## 许可证

使用 `MIT LICENSE`


## 第三方库支持

| 三方库名称   | 版本号 | 应用位置  | 三方库协议类型 |
| ------------ | ---------- | --------------------- | -------------- |
| QWindowKit   | 1.2	| 无边框窗口标题栏            | Apache 2.0 License|
| Qt-Advanced-Stylesheets | 1.0.4	| 切换样式表| LGPL 2.1 |
| QtMqtt| 6.4.3 | 支持 Mqtt 功能|


## Star 历史
[![Star History Chart](https://api.star-history.com/svg?repos=Ttigone/TtDebugTool&type=Date)](https://www.star-history.com/#Ttigone/TtDebugTool&Date)


## TODO

1. TtComboBox 有内存泄漏
2. Drawer 也有内存泄漏
3. 删除对应通道时, 首先确定是否要保存数据, 然后 delete 掉对应的内存
