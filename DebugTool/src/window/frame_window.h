#ifndef FRAME_WINDOW_H
#define FRAME_WINDOW_H

#include <QWidget>

class FrameWindow : public QWidget {
  Q_OBJECT
 public:
  explicit FrameWindow(QWidget* parent = nullptr);
  virtual ~FrameWindow();

  virtual QString title() const;

  virtual bool IsWorking() const = 0;
  virtual bool IsSaved() = 0;
};

#endif  // FRAME_WINDOW_H
