#include "ui/control/TtListView.h"
#include "ui/control/TtListView_p.h"

#include "ui/control/TtScrollBar.h"
#include "ui/style/TtListViewStyle.h"

namespace Ui {

TtListView::TtListView(QWidget* parent)
    : QListView(parent), d_ptr(new TtListViewPrivate) {
  Q_D(TtListView);
  d->q_ptr = this;
  setObjectName("TtListView");
  setStyleSheet("#TtListView{background-color:transparent;}");
  d->listView_style_ = new style::TtListViewStyle(style());
  setStyle(d->listView_style_);
  setMouseTracking(true);
  setVerticalScrollBar(new TtScrollBar(this));
  setHorizontalScrollBar(new TtScrollBar(this));
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
}

TtListView::~TtListView() {}

void TtListView::setItemHeight(int itemHeight) {
  Q_D(TtListView);
  if (itemHeight > 0) {
    d->listView_style_->setItemHeight(itemHeight);
    doItemsLayout();
  }
}

int TtListView::getItemHeight() const {
  Q_D(const TtListView);
  return d->listView_style_->getItemHeight();
}

void TtListView::setIsTransparent(bool isTransparent) {
  Q_D(TtListView);
  d->listView_style_->setIsTransparent(isTransparent);
  update();
}

bool TtListView::getIsTransparent() const {
  Q_D(const TtListView);
  return d->listView_style_->getIsTransparent();
}

TtListViewPrivate::TtListViewPrivate(QObject* parent) : QObject(parent) {}

TtListViewPrivate::~TtListViewPrivate() {}

}  // namespace Ui
