#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include "logview.h"

namespace h {
class Logger : public QObject
{
    Q_OBJECT
public:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static Logger *instance();
    static void openLogView();

signals:

private:
    static LogView *s_logView;
};

}
#endif // LOGGER_H
