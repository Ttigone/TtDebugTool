#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

// 枚举类导出  兼容QT5低版本
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define Q_BEGIN_ENUM_CREATE_SRC(CLASS)                                         \
  namespace CLASS {                                                            \
  Q_NAMESPACE
// Q_NAMESPACE_EXPORT(Tt_EXPORT)

#define Q_END_ENUM_CREATE(CLASS) }

#define Q_ENUM_CREATE(CLASS) Q_ENUM_NS(CLASS)

#else
#define Q_BEGIN_ENUM_CREATE(CLASS)                                             \
  class CLASS : public QObject {                                               \
    Q_OBJECT                                                                   \
  public:

#define Q_END_ENUM_CREATE(CLASS)                                               \
private:                                                                       \
  Q_DISABLE_COPY(CLASS)                                                        \
  }                                                                            \
  ;

#define Q_ENUM_CREATE(CLASS) Q_ENUM(CLASS)
#endif

const QString SerialPrefix = "Serial+";       // Define the prefix
const QString TcpClientPrefix = "TcpClient+"; // Define the prefix
const QString TcpServerPrefix = "TcpServer+"; // Define the prefix
const QString UdpClientPrefix = "UdpClient+"; // Define the prefix
const QString UdpServerPrefix = "UdpServer+"; // Define the prefix
const QString MqttPrefix = "MQTT+";           // Define the prefix
const QString ModbusPrefix = "Modbus+";       // Define the prefix

Q_BEGIN_ENUM_CREATE_SRC(TtProtocolRole)
enum Role {
  Serial = 0x0,
  TcpClient,
  TcpServer,
  UdpClient,
  UdpServer,
  MqttClient,
  MqttBroker,
  ModbusClient,
  ModbusServer,
  BlueTeeth,
};
Q_ENUM_CREATE(Role)
Q_END_ENUM_CREATE(TtProtocolRole)

Q_BEGIN_ENUM_CREATE_SRC(TtProtocolType)
enum ProtocolRole { Client, Server };
Q_ENUM_CREATE(ProtocolRole)
Q_END_ENUM_CREATE(TtProtocolType)

Q_BEGIN_ENUM_CREATE_SRC(TtFunctionalCategory)
enum Category { Communication, Instruction, Simulate, None };
Q_ENUM_CREATE(Category)
Q_END_ENUM_CREATE(TtFunctionalCategory)

Q_BEGIN_ENUM_CREATE_SRC(TtUdpMode)
enum Mode { Unicast, Multicast, Broadcast };
Q_ENUM_CREATE(Mode)
Q_END_ENUM_CREATE(TtUdpMode)

Q_BEGIN_ENUM_CREATE_SRC(TtMqttProcotol)
enum Version { Q3_1 = 3, Q3_1_1, Q5_0 };
Q_ENUM_CREATE(Version)
Q_END_ENUM_CREATE(TtMqttProcotol)

Q_BEGIN_ENUM_CREATE_SRC(TtModbusProcotol)
enum Type { RTU, RTU_ASCLL, TCP };
Q_ENUM_CREATE(Type)
Q_END_ENUM_CREATE(TtModbusProcotol)

Q_BEGIN_ENUM_CREATE_SRC(TtModbusRegisterType)
enum Type {
  Coils,
  HoldingRegisters,
  DiscreteInputs,
  InputRegisters,
};
Q_ENUM_CREATE(Type)
Q_END_ENUM_CREATE(TtModbusRegisterType)

#endif // GLOBAL_DEF_H
