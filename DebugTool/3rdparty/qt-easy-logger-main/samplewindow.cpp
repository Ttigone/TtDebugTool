#include <QVBoxLayout>
#include <QLabel>
#include "samplewindow.h"

SampleWindow::SampleWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(new QLabel("请在命令行查看输出内容，更多特性正在规划中。\n欢迎提建议或参与贡献！", this));
}

SampleWindow::~SampleWindow() {}
