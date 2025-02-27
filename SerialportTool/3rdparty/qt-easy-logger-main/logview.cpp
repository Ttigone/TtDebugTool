#include <QVBoxLayout>
#include <QDateTime>
#include <QScrollBar>
#include "logview.h"

namespace h {
LogView::LogView(QWidget *parent)
    : QWidget{parent}, m_textEdit{new QPlainTextEdit(this)}
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("输出");
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit);
    this->resize(1000, 700);

    QObject::connect(this, &LogView::appendHtml, this, &LogView::_appendHtml, Qt::AutoConnection);
}

void LogView::appendMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    const char *file = context.file ? context.file : "";
    //    const char *function = context.function ? context.function : "";
    QString typeStr = "Unknown";
    QString colorStr = "black";
    switch (type) {
    case QtDebugMsg:
        colorStr = "black";
        typeStr = "Debug";
        break;
    case QtInfoMsg:
        colorStr = "black";
        typeStr = "Info";
        break;
    case QtWarningMsg:
        colorStr = "#ff55ff";
        typeStr = "Warning";
        break;
    case QtCriticalMsg:
        colorStr = "#f44336";
        typeStr = "Critical";
        break;
    case QtFatalMsg:
        colorStr = "#d32f2f";
        typeStr = "Fatal";
        break;
    }

    QString messageHtml = QString("<p style=\"color:%1;\">[%2][%3] %4 (%5:%6)</p>").arg(
        colorStr,
        QDateTime::currentDateTime().toString(Qt::ISODate),
        typeStr,
        msg.toHtmlEscaped(),
        QString::fromStdString(file).mid(18),
        QString::number(context.line)
        );

    Q_EMIT appendHtml(messageHtml);
}

void LogView::_appendHtml(const QString &htmlMessage){
    m_textEdit->appendHtml(htmlMessage);
    m_textEdit->verticalScrollBar()->setValue(m_textEdit->verticalScrollBar()->maximum());
}
}
