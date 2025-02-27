#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <QPlainTextEdit>

namespace h {
class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(QWidget *parent = nullptr);
    void appendMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

signals:
    void appendHtml(const QString &htmlMessage);

private slots:
    void _appendHtml(const QString &htmlMessage);

private:
    QPlainTextEdit *m_textEdit;

};
}
#endif // LOGVIEW_H
