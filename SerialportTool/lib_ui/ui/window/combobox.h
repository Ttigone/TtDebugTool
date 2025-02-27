#ifndef UI_WINDOW_COMBOBOX_H
#define UI_WINDOW_COMBOBOX_H

#include <QListView>

#include "ui/Def.h"

namespace Ui {

class TtImageButton;

class Tt_EXPORT TtComboBox : public QComboBox {
 public:
  TtComboBox(QWidget* parent = nullptr);
  ~TtComboBox();
};

class Tt_EXPORT TtLabelComboBox : public QWidget {
  Q_OBJECT
 public:
  TtLabelComboBox(Qt::AlignmentFlag flag, const QString &text = "", QWidget *parent = nullptr);
  TtLabelComboBox(const QString &text = "", QWidget *parent = nullptr);
  ~TtLabelComboBox();

  QComboBox *body();
  void addItem(const QString &atext, const QVariant &auserData = QVariant());

  void setCurrentItem(qint8 index);
  QString itemText(int index);
  QString currentText();
  int count();

  void shortCurrentItemText();

 signals:
  void currentIndexChanged(int index);

 private:
  TtComboBox *combo_box_;
  QLabel *label_;
};

class Tt_EXPORT TtLabelBtnComboBox : public QWidget {
  Q_OBJECT
 public:
  TtLabelBtnComboBox(Qt::AlignmentFlag flag, const QString& text,
                     const QString& image_path, QWidget* parent = nullptr);
  TtLabelBtnComboBox(const QString& text = "", QWidget* parent = nullptr);
  ~TtLabelBtnComboBox();

  void addItem(const QString& atext, const QVariant& auserData = QVariant());

  QComboBox* body();
  // 设置当前项
  void setCurrentItem(qint8 index);

  QString itemText(int index);

  QString currentText();

  ///
  /// @brief count item 个数
  ///
  int count();

 signals:
  void clicked();

 private slots:
  void displayCurrentCOMx();

 private:
  TtLabelComboBox* part_;
  // Ui::TtImageButton* refresh_btn_;
};

}  // namespace Ui

#endif  // UI_WINDOW_COMBOBOX_H
