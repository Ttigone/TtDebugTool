#ifndef UDP_WINDOW_H
#define UDP_WINDOW_H

#include <QWidget>

namespace Window {

class UdpWindow : public QWidget {
  Q_OBJECT
 public:
  explicit UdpWindow(QWidget* parent = nullptr);

 signals:
};


} // namespace Window


#endif  // UDP_WINDOW_H
