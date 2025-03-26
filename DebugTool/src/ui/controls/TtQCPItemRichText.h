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

  void setText(const QString& text);

 protected:
  virtual void draw(QCPPainter* painter);

  QTextDocument doc;
};

#endif  // UI_CONTROLS_TTQCPITEMRICHTEXT_H
