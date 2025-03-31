#include "setting_widget.h"

#include <ui/layout/vertical_layout.h>

#include <core/download.h>
#include <ui/controls/TtDownloadDialog.h>
#include <ui/widgets/buttons.h>

namespace Ui {

SettingWidget::SettingWidget(QWidget* parent) : QWidget(parent) {
  init();

  // 创建 lua 对象
  lua_state_ = luaL_newstate();

  luaL_openlibs(lua_state_);
}

SettingWidget::~SettingWidget() {
  qDebug() << "delete SettingWidget";
}

void SettingWidget::downloadProgress(qint64 bytesReveived, qint64 bytesTotal) {

  // QString test = ";
  // const char* luastring = test.toStdString().c_str();
  lua_State* L = this->lua_state_;
  const char* luastring = edit->toPlainText().toStdString().c_str();
  if (luaL_dostring(L, luastring)) /* 从字符串中加载LUA脚本 */
  {
    qDebug() << "LUA脚本有误！";
    return;
  }

  // 获取变量
  /* 函数入栈 */
  lua_getglobal(L, "add");

  /* 第一个函数参数入栈 */
  lua_pushnumber(L, 100);

  /* 第二个函数参数入栈 */
  lua_pushnumber(L, 200);

  /*
     * 执行函数调用
     * 2表示lua脚本中add函数需要输入两个函数参数
     * 1表示lua脚本中add函数有一个返回值
     * 执行完函数调用后，lua自动出栈函数和参数
     */
  lua_call(L, 2, 1);

  /*
     * 得到add函数执行结果
     * -1表示最后一个返回值，因为lua的函数可以返回多个值的。
     */
  auto sum = lua_tonumber(L, -1);
  qDebug() << sum;

  /* 出栈一个数据。此时栈中存的是add函数的执行结果，所以需要出栈 */
  lua_pop(L, 1);

  progress_bar_->setMaximum(bytesTotal);
  progress_bar_->setValue(bytesReveived);
}

void SettingWidget::init() {
  QWidget* basicWidget = new QWidget(this);
  layout_ = new TtVerticalLayout(basicWidget);

  progress_bar_ = new QProgressBar;
  downloader_ = new Core::Downloader;
  // dialog_ = new DownloadDialog(basicWidget);

  download_btn_ = new TtSvgButton(":/sys/cloud-arrow-down.svg", basicWidget);

  edit = new QPlainTextEdit;

  connect(download_btn_, &TtSvgButton::clicked, this, [this]() {
    dialog_ = new DownloadDialog(this);
    connect(dialog_, &DownloadDialog::accepted, downloader_,
            &Core::Downloader::download);
    dialog_->exec();
    dialog_->deleteLater();
  });

  layout_->addWidget(download_btn_);
  layout_->addWidget(progress_bar_);
  layout_->addWidget(edit);

  connect(downloader_, &Core::Downloader::errorOccurred, this,
          [this](const QString& error) { qDebug() << error; });
  connect(downloader_, &Core::Downloader::downloadProgress, this,
          &SettingWidget::downloadProgress);
  // connect(downloader_, &Core::Downloader::available, ui->newDownloadButton, &QPushButton::setEnabled);
  connect(downloader_, &Core::Downloader::running, progress_bar_,
          &QProgressBar::setVisible);
}

}  // namespace Ui
