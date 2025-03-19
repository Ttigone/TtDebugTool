#include "ui/control/TtDialog.h"
#include "ui/control/TtDialog_p.h"

namespace Ui {

TtDialog::TtDialog(QWidget* parent)
    : QDialog(parent), d_ptr(new TtDialogPrivate) {}

TtDialog::~TtDialog() {}

TtDialogPrivate::TtDialogPrivate(QObject* parent) : QObject(parent) {}

TtDialogPrivate::~TtDialogPrivate() {}

}  // namespace Ui
