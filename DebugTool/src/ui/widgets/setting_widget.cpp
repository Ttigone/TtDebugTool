#include "setting_widget.h"

#include <ui/layout/vertical_layout.h>

#include <core/download.h>
#include <ui/controls/TtDownloadDialog.h>
#include <ui/widgets/buttons.h>

namespace Ui {

SettingWidget::SettingWidget(QWidget* parent) : QWidget(parent) {
  init();

}

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

  // QLabel info;
  // info.setText();
  QTextBrowser* infoBroswer = new QTextBrowser;
  infoBroswer->setOpenExternalLinks(true);
  infoBroswer->setOpenLinks(true);
  // 设置HTML内容
  infoBroswer->setHtml(R"(
        <h1>TtDebugTool</h1>
        <p>这是一个调试工具，支持：</p>
        <ul>
            <li><b>串口</b>: <a href="#section1">跳转到章节1</a></li>
            <li><b>ModbusRtu/ModbusTcp</b>: <a href="https://www.qt.io">访问Qt官网</a></li>
            <li><b>Tcp 调试</b>: <span style="color:red; font-size:14pt;">彩色文本</span></li>
            <li><b>Udp 调试</b>: <img src="qrc:/images/logo.png" width="50" height="50"/></li>
            <li><b>Mqtt 调试</b>: <a href="https://www.qt.io">访问Qt官网</a></li>
        </ul>
        
        <a name="section1"></a>
        <h2>章节1</h2>
        <p>这是章节1的内容，可以通过内部链接跳转到这里。</p>
        <p>返回 <a href="#">顶部</a></p>
    )");

  layout_->addWidget(infoBroswer);

  progress_bar_ = new QProgressBar;
  downloader_ = new Core::Downloader;
  // dialog_ = new DownloadDialog(basicWidget);

  download_btn_ = new TtSvgButton(":/sys/cloud-arrow-down.svg", this);

  edit = new QPlainTextEdit;

  layout_->addWidget(download_btn_);
  layout_->addWidget(progress_bar_);
  layout_->addWidget(edit);

  connect(download_btn_, &TtSvgButton::clicked, this, [this]() {
    dialog_ = new DownloadDialog(this);
    connect(dialog_, &DownloadDialog::accepted, downloader_,
            &Core::Downloader::download);
    dialog_->exec();
    dialog_->deleteLater();
  });


  connect(downloader_, &Core::Downloader::errorOccurred, this,
          [this](const QString& error) { qDebug() << error; });
  connect(downloader_, &Core::Downloader::downloadProgress, this,
          &SettingWidget::downloadProgress);
  // connect(downloader_, &Core::Downloader::available, ui->newDownloadButton, &QPushButton::setEnabled);
  connect(downloader_, &Core::Downloader::running, progress_bar_,
          &QProgressBar::setVisible);
}

}  // namespace Ui
