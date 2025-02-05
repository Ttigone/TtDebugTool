#ifndef WINDOW_WINDOWBAR_BUTTON_H
#define WINDOW_WINDOWBAR_BUTTON_H

#include <QPushButton>

namespace Ui {

class WindowbarButtonPrivate;

class WindowbarButton : public QPushButton {
  Q_OBJECT
  Q_DECLARE_PRIVATE(WindowbarButton)
  Q_PROPERTY(QIcon iconNormal READ iconNormal WRITE setIconNormal FINAL)
  Q_PROPERTY(QIcon iconChecked READ iconChecked WRITE setIconChecked FINAL)
  Q_PROPERTY(QIcon iconDisabled READ iconDisabled WRITE setIconDisabled FINAL)

 public:
  explicit WindowbarButton(QWidget *parent = nullptr);
  ~WindowbarButton();

 public:
  QIcon iconNormal() const;
  void setIconNormal(const QIcon &icon);

  QIcon iconChecked() const;
  void setIconChecked(const QIcon &icon);

  QIcon iconDisabled() const;
  void setIconDisabled(const QIcon &icon);

 signals:
  void doubleClicked();

 protected:
  void checkStateSet() override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  // void paintEvent(QPaintEvent* event) override;
  // void mouseMoveEvent(QMouseEvent* event) override;
  // void mousePressEvent(QMouseEvent* event) override;
  // void mouseReleaseEvent(QMouseEvent* event) override;

 protected:
  WindowbarButton(WindowbarButtonPrivate &d, QWidget *parent = nullptr);

  QScopedPointer<WindowbarButtonPrivate> d_ptr;
};

} // namespace Ui

#endif // WINDOW_WINDOWBAR_BUTTON_H
