#ifndef UI_CONTROL_TTSWITCHBUTTON_H
#define UI_CONTROL_TTSWITCHBUTTON_H

#include <QObject>

#include "ui/Def.h"

namespace Ui {

class Tt_EXPORT TtSwitchButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal knobPosition READ knobPosition WRITE setKnobPosition)
 public:
  enum TextPosition { TextLeft, TextRight };

  explicit TtSwitchButton(QWidget* parent = nullptr);
  explicit TtSwitchButton(const QString& text, QWidget* parent = nullptr);
  ~TtSwitchButton();

  bool isChecked() const;
  void setText(const QString& text);
  QString text() const;

  void setTextPosition(TextPosition position);
  TextPosition textPosition() const;

 signals:
  void toggled(bool checked);

 public slots:
  void setChecked(bool checked);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

  qreal knobPosition() const;
  void setKnobPosition(qreal position);

 private:
  void updateLayout();
  bool checked;
  qreal m_knobPosition;
  QPropertyAnimation* animation;
  QString text_;
  TextPosition textPosition_;
  QRect switchRect;
  QRect textRect;
  int spacing = 8;  // 文本与开关间距
};

}  // namespace Ui

#endif  // TTSWITCHBUTTON_H
