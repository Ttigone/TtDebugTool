#include "ui/widgets/mb_window.h"
#include "ui/platform/platform_window.h"

namespace Ui
{
MbWindow::MbWindow(QWidget* parent)
    : MbWidget(parent),
      helper_(Platform::CreateSpecialBasicWindowHelper(this)) {
  Expects(helper_ != nullptr);
  helper_->initInWindow(this);
  //hide();
}

MbWindow::~MbWindow() = default;

not_null<MbWidget*> MbWindow::body()
{
  return helper_->body();
}

not_null<MbWidget*> MbWindow::body() const { return helper_->body().get(); }

QMargins MbWindow::frameMargins()
{
  return helper_->frameMargins();
}

int MbWindow::addtionalContenPadding() const
{
  return helper_->addtionalContenPadding();
}

int MbWindow::addtionalContentPaddingValue() const
{
  return helper_->addtionalContenPadding();
}

void MbWindow::setTitle(const QString& title)
{
  return helper_->setTitle(title);
}

void MbWindow::setNativeFrame(bool enable)
{
  helper_->setNativeFrame(enable);
}

void MbWindow::setMinimumSize(QSize size)
{
  helper_->setMinimumSize(size);
}

void MbWindow::setFixedSize(QSize size)
{
  helper_->setFixedSize(size);
}

void MbWindow::setGeometry(QRect rect)
{
  helper_->setGeometry(rect);
}

void MbWindow::showFullScreen()
{
  helper_->showFullScreen();
}

void MbWindow::showNormal()
{
  helper_->showNormal();
}

void MbWindow::close()
{
  helper_->close();
}
} // namespace Ui

