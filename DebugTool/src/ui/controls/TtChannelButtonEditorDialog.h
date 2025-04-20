#ifndef TTCHANNELBUTTONEDITORDIALOG_H
#define TTCHANNELBUTTONEDITORDIALOG_H

#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <ui/widgets/buttons.h>
#include "TtChannelButton.h"
#include "ui/control/TtLineEdit.h"

class TtChannelButtonEditorDialog : public QDialog {
  Q_OBJECT
 public:
  TtChannelButtonEditorDialog(QWidget* parent = nullptr);
  TtChannelButtonEditorDialog(TtChannelButton* button,
                              QWidget* parent = nullptr);
  ~TtChannelButtonEditorDialog();

  void setMetaInfo(const QByteArray& meta);
  QByteArray metaInfo();

  QString title() const;

  QColor checkBlockColor() const;

 private slots:
  void applyChanges();

 private:
  // TtColorButton* m_button;
  TtChannelButton* m_button = nullptr;
  Ui::TtLabelLineEdit* title_edit_;
  // Ui::TtFancyButton* m_colorButton;
  Ui::TtFancyButton* m_checkBlockButton;
  QColor m_chosenColor;
  QColor m_chosenCheckBlockColor;

  Ui::TtLabelLineEdit* frame_header_;
  Ui::TtLabelLineEdit* frame_header_length_;
  Ui::TtLabelLineEdit* frame_type_offset_;
  Ui::TtLabelLineEdit* frame_length_offset_;
  Ui::TtLabelLineEdit* frame_length_;
  Ui::TtLabelLineEdit* frame_end_;
};

#endif  // TTCHANNELBUTTONEDITORDIALOG_H
