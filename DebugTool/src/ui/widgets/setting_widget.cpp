#include "setting_widget.h"

#include <ui/layout/vertical_layout.h>

#include <core/download.h>
#include <ui/controls/TtDownloadDialog.h>
#include <ui/widgets/buttons.h>

namespace Ui {

SettingWidget::SettingWidget(QWidget *parent) : QWidget(parent) { init(); }

SettingWidget::~SettingWidget() {
  // qDebug() << "delete SettingWidget";
}

void SettingWidget::downloadProgress(qint64 bytesReveived, qint64 bytesTotal) {

  progress_bar_->setMaximum(bytesTotal);
  progress_bar_->setValue(bytesReveived);
}

void SettingWidget::init() {
  // QWidget* basicWidget = new QWidget(this);
  layout_ = new TtVerticalLayout(this);

  // QFontDialog fontDialog;

  // QLabel info;
  // info.setText();
  QTextBrowser *infoBroswer = new QTextBrowser;
  infoBroswer->setOpenExternalLinks(true);
  infoBroswer->setOpenLinks(true);
  // 设置HTML内容
  infoBroswer->setHtml(R"(
        <h1>TtDebugTool</h1>
        <p>这是一个调试工具，支持：串口 | Tcp | Udp | MQTT | ModBus 调试工具 </p>
    )");

  layout_->addWidget(infoBroswer);

  // progress_bar_ = new QProgressBar;
  // downloader_ = new Core::Downloader;
  // dialog_ = new DownloadDialog(basicWidget);

  // download_btn_ = new TtSvgButton(":/sys/cloud-arrow-down.svg", this);

  // edit = new QPlainTextEdit;

  // layout_->addWidget(download_btn_);
  // layout_->addWidget(progress_bar_);
  // layout_->addWidget(edit);

  // connect(download_btn_, &TtSvgButton::clicked, this, [this]() {
  //   dialog_ = new DownloadDialog(this);
  //   connect(dialog_, &DownloadDialog::accepted, downloader_,
  //           &Core::Downloader::download);
  //   dialog_->exec();
  //   dialog_->deleteLater();
  // });

  // connect(downloader_, &Core::Downloader::errorOccurred, this,
  //         [this](const QString &error) { qDebug() << error; });
  // connect(downloader_, &Core::Downloader::downloadProgress, this,
  //         &SettingWidget::downloadProgress);
  // // connect(downloader_, &Core::Downloader::available,
  // ui->newDownloadButton,
  // // &QPushButton::setEnabled);
  // connect(downloader_, &Core::Downloader::running, progress_bar_,
  //         &QProgressBar::setVisible);
}

} // namespace Ui
