#ifndef WINDOW_FRAME_WINDOW_H
#define WINDOW_FRAME_WINDOW_H

#include <QWidget>

namespace Window {

class FrameWindow : public QWidget {
  Q_OBJECT
public:
  explicit FrameWindow(QWidget *parent = nullptr);
  virtual ~FrameWindow();

  virtual QString title() const;
  virtual bool workState() const = 0;
  virtual bool saveState() = 0;
  virtual void setSaveState(bool state) = 0;
  Q_INVOKABLE virtual void saveSetting() = 0;
  Q_INVOKABLE virtual void setSetting(const QJsonObject &config) = 0;

protected:
  bool saved_ = false;
};

} // namespace Window

#endif // FRAME_WINDOW_H
