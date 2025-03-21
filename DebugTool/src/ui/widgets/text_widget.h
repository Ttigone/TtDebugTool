#ifndef UI_TEXT_WIDGET_H
#define UI_TEXT_WIDGET_H

#include <QPlainTextEdit>

namespace Ui {

class TextWidget : public QPlainTextEdit {
 public:
  TextWidget(QWidget *parent = nullptr);
  ~TextWidget();
};

} // namespace Ui

#endif // UI_TEXT_WIDGET_H
