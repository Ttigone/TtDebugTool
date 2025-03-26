#ifndef UI_CONTROLS_TTDOWNLOADDIALOG_H
#define UI_CONTROLS_TTDOWNLOADDIALOG_H

#include <QDialog>

namespace Ui {
class TtLabelLineEdit;
class TtSvgButton;
class TtTextButton;
}  // namespace Ui

class DownloadDialog : public QDialog {
  Q_OBJECT
 public:
  DownloadDialog(QWidget* parent = nullptr);
  ~DownloadDialog();

 signals:
  void accepted(const QUrl&, const QString&);

 private slots:
  void acceptedBtnClicked();
  void saveAsBtnClicked();

 private:
  void init();
  void connectSignals();

  Ui::TtLabelLineEdit* url_;
  Ui::TtLabelLineEdit* save_as_;
  Ui::TtSvgButton* save_as_btn_;
  Ui::TtTextButton* ok_btn_;
  Ui::TtTextButton* cancle_btn_;
};

#endif  // UI_CONTROLS_TTDOWNLOADDIALOG_H
