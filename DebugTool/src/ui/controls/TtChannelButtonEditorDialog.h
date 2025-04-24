#ifndef TTCHANNELBUTTONEDITORDIALOG_H
#define TTCHANNELBUTTONEDITORDIALOG_H

#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class TtChannelButton;

namespace Ui {
class TtLabelLineEdit;
class TtFancyButton;
}  // namespace Ui

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
  void init();
  // TtColorButton* m_button;
  TtChannelButton* m_button = nullptr;
  Ui::TtLabelLineEdit* title_edit_;
  // Ui::TtFancyButton* m_colorButton;
  Ui::TtFancyButton* m_checkBlockButton;
  QColor m_chosenColor;
  QColor m_chosenCheckBlockColor;

  Ui::TtLabelLineEdit* frame_header_;
  Ui::TtLabelLineEdit* frame_header_length_;
  Ui::TtLabelLineEdit* frame_type_;
  Ui::TtLabelLineEdit* frame_type_offset_;
  Ui::TtLabelLineEdit* frame_length_;
  Ui::TtLabelLineEdit* frame_length_offset_;
  Ui::TtLabelLineEdit* frame_end_;

  bool save_scripit_ = false;
  QString lua_code_;
  QString lua_code_copy_;
  // QStringLiteral()
};

#endif  // TTCHANNELBUTTONEDITORDIALOG_H
