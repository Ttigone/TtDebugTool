#ifndef UI_CONTROL_TTCOMBOBOX_H
#define UI_CONTROL_TTCOMBOBOX_H

#include <QComboBox>

#include "ui/ui_pch.h"

namespace Ui {

class TtComboBoxPrivate;

class Tt_EXPORT TtComboBox : public QComboBox {
  Q_OBJECT
  Q_Q_CREATE(TtComboBox);
  Q_PROPERTY_CREATE_Q_H(int, BorderRadius)
public:
  explicit TtComboBox(QWidget *parent = nullptr);
  ~TtComboBox();

protected:
  void showPopup() override;
  void hidePopup() override;
};

class Tt_EXPORT TtLabelComboBox : public QWidget {
  Q_OBJECT
public:
  TtLabelComboBox(Qt::AlignmentFlag flag, const QString &text = "",
                  QWidget *parent = nullptr);
  TtLabelComboBox(const QString &text = "", QWidget *parent = nullptr);
  ~TtLabelComboBox();

public:
  TtComboBox *body();
  void addItem(const QString &atext, const QVariant &auserData = QVariant());

  QVariant currentData(int role = Qt::UserRole);
  void setCurrentItem(qint8 index);
  QString itemText(int index);
  QString currentText();
  void setCurrentText(const QString &text);
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
  TtLabelBtnComboBox(Qt::AlignmentFlag flag, const QString &text,
                     const QString &image_path, QWidget *parent = nullptr);
  TtLabelBtnComboBox(const QString &text = "", QWidget *parent = nullptr);
  ~TtLabelBtnComboBox();

  void addItem(const QString &atext, const QVariant &auserData = QVariant());

  TtComboBox *body();
  QVariant currentData(int role = Qt::UserRole);
  void setCurrentItem(qint8 index);
  QString itemText(int index);
  QString currentText();
  void setCurrentText(const QString &text);
  int count();

signals:
  void clicked();

private slots:
  void displayCurrentCOMx();

private:
  TtLabelComboBox *part_;
};

} // namespace Ui

#endif // UI_CONTROL_TTCOMBOBOX_H
