#include <QDateTime>
#include <QGuiApplication>
#include "logger.h"

namespace h {
LogView *Logger::s_logView = nullptr;

void Logger::openLogView(){
    if(!s_logView){
        s_logView = new LogView();
        s_logView->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
        QObject::connect(s_logView, &LogView::destroyed, qApp, [](){
            s_logView = nullptr;
        });
    }
    s_logView->show();
    s_logView->activateWindow();
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    {
        const char *file = context.file ? context.file : "";
        //    const char *function = context.function ? context.function : "";
        QString typeStr = "Unknown";
        switch (type) {
        case QtDebugMsg:
            typeStr = "Debug";
            break;
        case QtInfoMsg:
            typeStr = "Info";
            break;
        case QtWarningMsg:
            typeStr = "Warning";
            break;
        case QtCriticalMsg:
            typeStr = "Critical";
            break;
        case QtFatalMsg:
            typeStr = "Fatal";
            break;
        }
        QString message = QString("[%1][%2] %3 (%4:%5)").arg(
            QDateTime::currentDateTime().toString(Qt::ISODate),
            typeStr,
            msg,
            QString::fromStdString(file),
            QString::number(context.line)
            );
        switch (type) {
        case QtDebugMsg:
            fprintf(stdout, "%s\n", message.toLocal8Bit().data());
            break;
        case QtInfoMsg:
            fprintf(stdout, "%s\n", message.toLocal8Bit().data());
            break;
        case QtWarningMsg:
            fprintf(stderr, "%s\n", message.toLocal8Bit().data());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "%s\n", message.toLocal8Bit().data());
            break;
        case QtFatalMsg:
            fprintf(stderr, "%s\n", message.toLocal8Bit().data());
            break;
        }
        fflush(stdout);
    }

    if(s_logView){
        s_logView->appendMessage(type, context, msg);
    }
}
}
