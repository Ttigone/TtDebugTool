#ifndef SAMPLEWINDOW_H
#define SAMPLEWINDOW_H

#include <QWidget>

class SampleWindow : public QWidget
{
    Q_OBJECT

public:
    SampleWindow(QWidget *parent = nullptr);
    ~SampleWindow();
};
#endif // SAMPLEWINDOW_H
