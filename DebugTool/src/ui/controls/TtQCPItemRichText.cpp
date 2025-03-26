#include "ui/controls/TtQCPItemRichText.h"

TtQCPItemRichText::TtQCPItemRichText(QCustomPlot* parentPlot)
    : QCPItemText(parentPlot) {}

TtQCPItemRichText::~TtQCPItemRichText() {}

void TtQCPItemRichText::setText(const QString& text) {
  QCPItemText::setText(text);
  doc.setHtml(text);
}

void TtQCPItemRichText::draw(QCPPainter* painter) {
  QPointF pos(position->pixelPosition());
  QTransform transform = painter->transform();
  transform.translate(pos.x(), pos.y());
  if (!qFuzzyIsNull(mRotation))
    transform.rotate(mRotation);
  painter->setFont(mainFont());
  QRect textRect = painter->fontMetrics().boundingRect(
      0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
  QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(),
                                        mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(
      QPointF(0, 0), textBoxRect,
      mPositionAlignment);  // 0, 0 because the transform does the translation
  textRect.moveTopLeft(textPos.toPoint() +
                       QPoint(mPadding.left(), mPadding.top()));
  textBoxRect.moveTopLeft(textPos.toPoint());
  qreal clipPadF = mainPen().widthF();
  int clipPad = (int)clipPadF;
  QRect boundingRect =
      textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (transform.mapRect(boundingRect)
          .intersects(painter->transform().mapRect(clipRect()))) {
    painter->setTransform(transform);
    if ((mainBrush().style() != Qt::NoBrush &&
         mainBrush().color().alpha() != 0) ||
        (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0)) {
      painter->setPen(mainPen());
      painter->setBrush(mainBrush());
      painter->drawRect(textBoxRect);
    }
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(mainColor()));
    doc.setDefaultFont(mainFont());
    doc.drawContents(painter);
  }
}
