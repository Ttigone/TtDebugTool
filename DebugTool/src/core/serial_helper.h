#ifndef CORE_SERIAL_HELPER_H
#define CORE_SERIAL_HELPER_H

#include <QObject>

class SerialHelper : public QObject {
  Q_OBJECT
 public:
  // 帧结构体
  // QVariant

  enum class FrameState {
    STATE_READY = 0x01,
    STATE_HEAD,  // 会不会有多个头部
    STATE_TYPE,
    STATE_NUM,
    STATE_DATA,
    STATE_TAIL,
  };
  enum class FrameType {
    Type1 = 0x01,
  };

  explicit SerialHelper(QObject* parent = nullptr);

  FrameState state() const { return state_; }
  inline void setState(FrameState state) { state_ = state; }

  FrameType dataType() const { return type_; }
  inline void setDataType(FrameType type) { type_ = type; }

  unsigned char frameLength() const { return frame_length_; }
  inline void setFrameLength(unsigned char length) { frame_length_ = length; }

  unsigned char* data() { return raw_data; }
  inline void setData(unsigned char index, unsigned char data) {
    raw_data[index] = data;
  }

  quint16 currentBuffLength() const { return recv_length_; }
  inline void setCurrentRecvLength(quint16 length) { recv_length_ = length; }
  inline void currentRecvLengthPlus() { ++recv_length_; }

  void clearCache() { std::memset(raw_data, 0, 8); }

 signals:

 private:
  FrameState state_;
  FrameType type_;

  unsigned char frame_type_;
  unsigned char frame_length_;  // 数据长度
  quint16 recv_length_;         // 接收的数据长度
  unsigned char raw_data[8];
};

#endif  // CORE_SERIAL_HELPER_H
