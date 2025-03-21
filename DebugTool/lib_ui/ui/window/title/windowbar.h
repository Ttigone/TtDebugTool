#ifndef WINDOW_WINDOWBAR_H
#define WINDOW_WINDOWBAR_H

#include <QFrame>
#include <QMenuBar>
#include <QAbstractButton>
#include <QLabel>

//QT_BEGIN_NAMESPACE
//class QFrame;
//class QMenuBar;
//class QAbstractButton;
//class QLabel;
//QT_END_NAMESPACE
#include "ui/Def.h"

namespace Ui {

class WindowBarPrivate;

class Tt_EXPORT WindowBar : public QFrame {
  Q_OBJECT
  Q_DECLARE_PRIVATE(WindowBar)

 public:
  explicit WindowBar(QWidget *parent = nullptr);
  ~WindowBar();

  QMenuBar *menuBar() const;
  void setMenuBar(QMenuBar *menuBar);

  QLabel *titleLabel() const;
  void setTitleLabel(QLabel *label);

  QAbstractButton *iconButton() const;
  void setIconButton(QAbstractButton *btn);

  QAbstractButton *minButton() const;
  void setMinButton(QAbstractButton *btn);

  QAbstractButton *maxButton() const;
  void setMaxButton(QAbstractButton *btn);

  QAbstractButton *closeButton() const;
  void setCloseButton(QAbstractButton *btn);

  QMenuBar *takeMenuBar();
  QLabel *takeTitleLabel();

  QAbstractButton *takeIconButton();
  QAbstractButton *takeMinButton();
  QAbstractButton *takeMaxButton();
  QAbstractButton *takeCloseButton();

  QWidget *hostWidget() const;
  void setHostWidget(QWidget *w);

  bool titleFollowWindow() const;
  void setTitleFollowWindow(bool value);

  bool iconFollowWindow() const;
  void setIconFollowWindow(bool value);

  // Q_SIGNALS:
 signals:
  void minimizeRequested();
  void maximizeRequested(bool max = false);
  void closeRequested();

 protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

  virtual void titleChanged(const QString &text);
  virtual void iconChanged(const QIcon &icon);

 protected:
  WindowBar(WindowBarPrivate &d, QWidget *parent = nullptr);

  QScopedPointer<WindowBarPrivate> d_ptr;
};

} // namespace Ui

#endif  // WINDOW_WINDOWBAR_H
