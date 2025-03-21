#include "ui/mb_widget.h"

#include <QColorSpace>
#include <QSurfaceFormat>


namespace Ui {

MbWidget::MbWidget(QWidget* parent) : MBWidgetHelper<QWidget>(parent) {
//MbWidget::MbWidget(QWidget* parent) : QWidget(parent) {
  //this->setWindowFlag(Qt::Window);
  //this->setStyleSheet("background-color: #f5f5f5;");
  //  [[maybe_unused]] static const auto Once = [] {
//    [[maybe_unused]] auto format = QSurfaceFormat::defaultFormat();
//    format.setSwapInterval(0);
//
//#ifdef Q_OS_MAC
//    format.setColorSpace(QColorSpace::SRgb);
//#endif
//    QSurfaceFormat::setDefaultFormat(format);
//    return true;
//  }();
}

} // namespace Ui

