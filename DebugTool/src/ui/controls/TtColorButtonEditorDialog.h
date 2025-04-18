#ifndef TTCOLORBUTTONEDITORDIALOG_H
#define TTCOLORBUTTONEDITORDIALOG_H

#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <ui/widgets/buttons.h>
#include "TtColorButton.h"
#include "ui/control/TtLineEdit.h"

class TtColorButtonEditorDialog : public QDialog {
  Q_OBJECT
 public:
  TtColorButtonEditorDialog(TtColorButton* button, QWidget* parent = nullptr);
  ~TtColorButtonEditorDialog();

  void setMetaInfo(const QByteArray& meta);
  QByteArray metaInfo();

 private slots:
  void applyChanges();

 private:
  TtColorButton* m_button;
  Ui::TtLabelLineEdit* title_edit_;
  // Ui::TtFancyButton* m_colorButton;
  Ui::TtFancyButton* m_checkBlockButton;
  QColor m_chosenColor;
  QColor m_chosenCheckBlockColor;

  Ui::TtLabelLineEdit* frame_header_;
  Ui::TtLabelLineEdit* frame_length_;
  Ui::TtLabelLineEdit* frame_end_;
};

#endif  // TTCOLORBUTTONEDITORDIALOG_H
