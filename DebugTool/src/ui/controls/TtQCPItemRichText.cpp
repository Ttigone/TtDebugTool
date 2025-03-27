#include "ui/controls/TtQCPItemRichText.h"

TtQCPItemRichText::TtQCPItemRichText(QCustomPlot* parentPlot)
    : QCPItemText(parentPlot) {}

TtQCPItemRichText::~TtQCPItemRichText() {}

void TtQCPItemRichText::setText(const QString& text) {
  if (doc.toHtml() != text) {
    doc.setHtml(text);
    doc.setTextWidth(-1);
    doc.adjustSize();
  }
  QCPItemText::setText(text);
}

void TtQCPItemRichText::draw(QCPPainter* painter) {
  // QMutexLocker locker(&m_mutex);
  QPointF pos(position->pixelPosition());
  if (doc.isEmpty()) {
    return;
  }
  // QTransform transform = painter->transform();
  // transform.translate(pos.x(), pos.y());
  // if (!qFuzzyIsNull(mRotation))
  //   transform.rotate(mRotation);
  // painter->setFont(mainFont());
  // QRect textRect = painter->fontMetrics().boundingRect(
  //     0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
  // QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(),
  //                                       mPadding.right(), mPadding.bottom());
  // QPointF textPos = getTextDrawPoint(
  //     QPointF(0, 0), textBoxRect,
  //     mPositionAlignment);  // 0, 0 because the transform does the translation
  // textRect.moveTopLeft(textPos.toPoint() +
  //                      QPoint(mPadding.left(), mPadding.top()));
  // textBoxRect.moveTopLeft(textPos.toPoint());
  // qreal clipPadF = mainPen().widthF();
  // int clipPad = (int)clipPadF;
  // QRect boundingRect =
  //     textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  // if (transform.mapRect(boundingRect)
  //         .intersects(painter->transform().mapRect(clipRect()))) {
  //   painter->setTransform(transform);
  //   if ((mainBrush().style() != Qt::NoBrush &&
  //        mainBrush().color().alpha() != 0) ||
  //       (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0)) {
  //     painter->setPen(mainPen());
  //     painter->setBrush(mainBrush());
  //     painter->drawRect(textBoxRect);
  //   }
  //   painter->setBrush(Qt::NoBrush);
  //   painter->setPen(QPen(mainColor()));
  //   doc.setDefaultFont(mainFont());
  //   doc.drawContents(painter);
  // }
  // ====== 新的绘制逻辑 ======
  painter->save();

  // 计算文本实际尺寸
  qreal margin = 3;
  qreal width = doc.idealWidth() + 2 * margin;
  qreal height = doc.size().height() + 2 * margin;

  // 转换坐标系
  QTransform transform = painter->transform();
  transform.translate(pos.x(), pos.y());
  if (!qFuzzyIsNull(mRotation))
    transform.rotate(mRotation);
  painter->setTransform(transform);

  // 绘制半透明背景（带圆角）
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(255, 255, 255, 220));  // 半透明白色
  painter->drawRoundedRect(
      QRectF(-margin, -margin, width, height), 5, 5  // X/Y圆角半径
  );

  // 绘制文本内容
  painter->setPen(QPen(mainColor()));
  QRectF textRect(margin, margin, width - 2 * margin, height - 2 * margin);
  doc.drawContents(painter, textRect);

  painter->restore();
  // ====== 结束绘制逻辑 ======
}
