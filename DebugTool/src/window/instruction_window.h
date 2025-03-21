#ifndef WINDOW_INSTRUCTION_WINDOW_H
#define WINDOW_INSTRUCTION_WINDOW_H

#include <QWidget>

namespace Ui {
class TtVerticalLayout;
class TtChatView;
class TtChatMessageModel;

class DataPresentationWidget;
}  // namespace Ui

namespace Window {

class InstructionWindow : public QWidget {
  Q_OBJECT
 public:
  explicit InstructionWindow(QWidget* parent = nullptr);

 signals:

 private:
  void init();

  Ui::TtVerticalLayout* main_layout_;

  // Ui::DataPresentationWidget* chart_;
};

}  // namespace Window

#endif  // WINDOW_INSTRUCTION_WINDOW_H
