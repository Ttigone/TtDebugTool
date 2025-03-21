/*****************************************************************//**
 * \file   customize_edit.h
 * \brief  自定义编辑框
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_FIELDS_CUSTOMIZE_EDIT_H
#define UI_FIELDS_CUSTOMIZE_EDIT_H


#include <QTextEdit>
#include <QPainter>
#include <QStyleOption>
#include <QInputMethodEvent>

namespace Ui {

class CustomizeTextEdit : public QTextEdit {
  Q_OBJECT

 public:
  CustomizeTextEdit(QWidget* parent = nullptr) : QTextEdit(parent) {
    setPlaceholderText("Enter your text here...");
  }

 protected:
  void paintEvent(QPaintEvent* event) override {
    QTextEdit::paintEvent(event);

    if ((!has_inputed)) {
      QPainter painter(viewport());
      painter.setPen(Qt::red);  // Set the placeholder text color
      QRect rect = viewport()->rect();
      rect.adjust(4, 4, -4, -4);  // Adjust the position if needed
      painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, placeholderText());
    }
  }

  //
  // 支持有输入法输入, 但是 text 是 empty 的
  void inputMethodEvent(QInputMethodEvent* event) override {
    QTextEdit::inputMethodEvent(event);
    if (!event->preeditString().isEmpty()) {
      has_inputed = true;
      // 选择成功
    } else if (!event->commitString().isEmpty()) {
      has_inputed = true;
    } else {
      has_inputed = false;
    }
  }

  void keyPressEvent(QKeyEvent* event) override {
    QTextEdit::keyPressEvent(event);
    if (document()->isEmpty()) {
      has_inputed = false;
    } else {
      has_inputed = true;
    }
  }

  bool has_inputed = false;
};




} // namespace Ui

#endif // UI_FIELDS_CUSTOMIZE_EDIT_H
