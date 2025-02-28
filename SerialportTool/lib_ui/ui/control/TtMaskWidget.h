/*****************************************************************//**
 * \file   TtMaskWidget.h
 * \brief  遮罩界面
 * 
 * \author C3H3_Ttigone
 * \date   February 2025
 *********************************************************************/

#ifndef UI_CONTROL_TTMASKWIDGET_H
#define UI_CONTROL_TTMASKWIDGET_H

QT_BEGIN_NAMESPACE
class QObject;
class QWidget;
class QTimer;
QT_END_NAMESPACE

#include "ui/ui_pch.h"
#include "ui/singleton.h"

namespace Ui {

class TtMaskWidgetPrivate;

class Tt_EXPORT TtMaskWidget : public QWidget {
	Q_OBJECT
	Q_Q_CREATE(TtMaskWidget)
		 public:
    void setMainWidget(QWidget* mainWidget);
          void setDialogNames(const QStringList& dialogNames);

       private:
		TtMaskWidget();
			~TtMaskWidget();


};

} // namespace Ui

#endif  // UI_CONTROL_TTMASKWIDGET_H
