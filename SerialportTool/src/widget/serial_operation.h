#ifndef WIDGET_SERIAL_OPERATION_H
#define WIDGET_SERIAL_OPERATION_H

namespace Widget {

class SerialOperation : public QWidget {
  Q_OBJECT
 public:
  SerialOperation(QWidget* parent = nullptr);
  ~SerialOperation();

  void needDisabled();

 signals:
  void openSerialPort();
  void sendData();

 public Q_SLOTS:


 private:
  ///
  /// @brief init
  /// 初始化操作
  void init();

  // 打开串口
  QPushButton *open_serial_;
  // 关闭串口
  QPushButton *close_serial_;
  // 清除接收
  QPushButton *clear_receive_;
  // 保存接收
  QPushButton *save_receive_;
  // 发送数据
  QPushButton *send_datas_;

  QPushButton *send_return_;

  // 显示接收时间
  QCheckBox *show_receive_time_;
  // 显示为 hex
  QCheckBox *show_hex_;
  QCheckBox *word_wrap_;

  // 后缀
  QCheckBox *need_suffix_;
};

}  // namespace Window

#endif  // WINDOW_SERIAL_OPERATION_H
