#ifndef UI_CONTROLS_TTQCPITEMRICHTEXT_H
#define UI_CONTROLS_TTQCPITEMRICHTEXT_H

#include <qcustomplot/qcustomplot.h>
#include <QTextDocument>

class TtQCPItemRichText : public QCPItemText {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText)
 public:
  TtQCPItemRichText(QCustomPlot* parentPlot);
  ~TtQCPItemRichText();

  QTextDocument& document() { return doc; }

  void setText(const QString& text);

 protected:
  virtual void draw(QCPPainter* painter);

  QTextDocument doc;
  QMutex m_mutex;
};

#endif  // UI_CONTROLS_TTQCPITEMRICHTEXT_H
