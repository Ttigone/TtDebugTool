#ifndef UI_SETTING_WIDGET_HPP
#define UI_SETTING_WIDGET_HPP

#include <QPlainTextEdit>
#include <QProgressBar>


namespace Core {
class Downloader;
}  // namespace Core

namespace Ui {

class DownloadDialog;
class TtVerticalLayout;
class TtSvgButton;

class SettingWidget : public QWidget {
  Q_OBJECT
 public:
  explicit SettingWidget(QWidget* parent = nullptr);
  ~SettingWidget();

 public slots:
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

 private:
  void init();

  QPlainTextEdit* edit;

  TtVerticalLayout* layout_;

  Core::Downloader* downloader_;

  DownloadDialog* dialog_;

  QProgressBar* progress_bar_;
  TtSvgButton* download_btn_;
};

}  // namespace Ui

#endif  // UI_SETTING_WIDGET_HPP
