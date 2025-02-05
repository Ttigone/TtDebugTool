#ifndef TCP_WINDOW_H
#define TCP_WINDOW_H

#include <QWidget>

namespace Window {

class TcpWindow : public QWidget {
  Q_OBJECT
 public:
  explicit TcpWindow(QWidget* parent = nullptr);

 signals:
};

} // namespace Window


#endif  // TCP_WINDOW_H
