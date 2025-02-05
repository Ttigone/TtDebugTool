#ifndef CORE_TCP_CLIENT_H
#define CORE_TCP_CLIENT_H

#include <QObject>

namespace Core {

class TcpClient : public QObject {
  Q_OBJECT
 public:
  explicit TcpClient(QObject* parent = nullptr);

 signals:
};

}  // namespace Core

#endif  // TCP_CLIENT_H
