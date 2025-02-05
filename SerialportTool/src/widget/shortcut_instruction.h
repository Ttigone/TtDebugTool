#ifndef WIDGET_SHORTCUT_INSTRUCTION_H
#define WIDGET_SHORTCUT_INSTRUCTION_H

#include <QListWidget>

namespace Widget {

class HeaderWidget : public QWidget {
 public:
  explicit HeaderWidget(QWidget* parent = nullptr);
  ~HeaderWidget();

 private:
  void init();
};

class InstructionWidget : public QWidget {
 public:
  explicit InstructionWidget(QWidget* parent = nullptr);
  ~InstructionWidget();

 private:
  void init();
};

// class ShortcutInstruction : public QObject {
class ShortcutInstruction : public QListWidget {
 public:
  explicit ShortcutInstruction(QWidget* parent = nullptr);
  ~ShortcutInstruction();

  void addCustomWidget(QWidget* widget);

 private:
  void init();
  void addWidgetItem(QWidget* widget, QSize itemSize);
};

}  // namespace Widget

#endif  // WIDGET_SHORTCUT_INSTRUCTION_H
