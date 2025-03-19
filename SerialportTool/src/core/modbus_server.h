#ifndef CORE_MODBUS_SERVER_H
#define CORE_MODBUS_SERVER_H

#include <QObject>

namespace Core {

class ModbusServer : public QObject {
  Q_OBJECT
 public:
  ModbusServer(QObject* parent = nullptr);
  ~ModbusServer();
};

}  // namespace Core

#endif  // CORE_MODBUS_SERVER_H
