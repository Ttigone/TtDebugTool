#ifndef WINDOW_FUNCTION_SELECTION_WINDOW_H
#define WINDOW_FUNCTION_SELECTION_WINDOW_H

#include <QWidget>

#include "Def.h"

namespace Ui {
class CommonButton;
class TtWordsButton;
class TtNormalLabel;
class TtElidedLabel;
} // namespace Ui

namespace Window {

class FunctionSelectionWindow : public QWidget {
  Q_OBJECT
public:
  explicit FunctionSelectionWindow(QWidget *parent = nullptr);
  ~FunctionSelectionWindow();

signals:
  void switchRequested(TtProtocolRole::Role role);

private:
  void init();

  Ui::TtElidedLabel *label_;
  QGridLayout *function_layout_;
};

class SimulateFunctionSelectionWindow : public QWidget {
  Q_OBJECT
public:
  explicit SimulateFunctionSelectionWindow(QWidget *parent = nullptr);
  ~SimulateFunctionSelectionWindow();

signals:
  void switchRequested(TtProtocolRole::Role role);

private:
  void init();

  Ui::TtElidedLabel *label_;
  QGridLayout *function_layout_;
};

} // namespace Window

class AllFunctionSelectionWindow : public QWidget {
  Q_OBJECT
public:
  explicit AllFunctionSelectionWindow(QWidget *parent = nullptr);
  ~AllFunctionSelectionWindow();

signals:
  void switchRequested(TtProtocolRole::Role role);

private:
  void init();

  Ui::TtElidedLabel *label_;
  QGridLayout *function_layout_;
};

#endif // FUNCTION_SELECTION_WINDOW_H
