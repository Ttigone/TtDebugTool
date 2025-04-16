#ifndef TTCOLORBUTTONEDITORDIALOG_H
#define TTCOLORBUTTONEDITORDIALOG_H

#include <QColorDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <ui/widgets/buttons.h>
#include "TtColorButton.h"
#include "ui/control/TtLineEdit.h"

class TtColorButtonEditorDialog : public QDialog {
  Q_OBJECT
 public:
  // 构造函数传入要编辑的按钮指针
  TtColorButtonEditorDialog(TtColorButton* button, QWidget* parent = nullptr)
      : QDialog(parent), m_button(button) {
    setWindowTitle(tr("编辑按钮属性"));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 1. 编辑文本
    QHBoxLayout* textLayout = new QHBoxLayout;
    textLayout->addWidget(new QLabel(tr("文本:"), this));
    m_textEdit = new Ui::TtLineEdit(this);
    textLayout->addWidget(m_textEdit);
    mainLayout->addLayout(textLayout);

    Ui::TtLabelLineEdit* FrameHeader =
        new Ui::TtLabelLineEdit(tr("帧头"), this);
    Ui::TtLabelLineEdit* FrameEnd = new Ui::TtLabelLineEdit(tr("帧尾"), this);

    mainLayout->addWidget(FrameHeader);
    mainLayout->addWidget(FrameEnd);

    // 2. 修改按钮颜色
    QHBoxLayout* colorLayout = new QHBoxLayout;
    colorLayout->addWidget(new QLabel(tr("背景色:"), this));
    m_colorButton = new QPushButton(tr("选择颜色"), this);
    // 同步初始颜色
    // 背景色不太相同
    m_colorButton->setStyleSheet(
        QString("background-color: %1").arg(m_button->getColor().name()));

    connect(m_colorButton, &QPushButton::clicked, this, [this]() {
      QColor chosen = QColorDialog::getColor(m_button->getColor(), this,
                                             tr("选择背景颜色"));
      if (chosen.isValid()) {
        m_chosenColor = chosen;
        m_colorButton->setStyleSheet(
            QString("background-color: %1").arg(chosen.name()));
      }
    });
    colorLayout->addWidget(m_colorButton);
    mainLayout->addLayout(colorLayout);

    // 3. 修改勾选区域颜色
    QHBoxLayout* checkBlockLayout = new QHBoxLayout;
    checkBlockLayout->addWidget(new QLabel(tr("勾选块色:"), this));

    // m_checkBlockButton = new QPushButton(tr("选择颜色"), this);
    m_checkBlockButton = new Ui::TtTextButton(tr("选择颜色"), this);

    m_checkBlockButton->setAutoFillBackground(true);
    QPalette pal = m_checkBlockButton->palette();
    pal.setColor(QPalette::Button, m_button->getCheckBlockColor());
    m_checkBlockButton->setPalette(pal);

    connect(m_checkBlockButton, &QPushButton::clicked, this, [this]() {
      QColor chosen = QColorDialog::getColor(m_button->getCheckBlockColor(),
                                             this, tr("选择勾选块颜色"));
      if (chosen.isValid()) {
        QPalette pal = m_checkBlockButton->palette();
        pal.setColor(QPalette::Button, m_chosenCheckBlockColor);
        m_checkBlockButton->setPalette(pal);
      }
    });
    checkBlockLayout->addWidget(m_checkBlockButton);
    mainLayout->addLayout(checkBlockLayout);

    // 4. 底部确定与取消按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton(tr("确定"), this);
    QPushButton* cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this,
            &TtColorButtonEditorDialog::applyChanges);
    connect(cancelButton, &QPushButton::clicked, this,
            &TtColorButtonEditorDialog::reject);
  }

 private slots:
  void applyChanges() {
    if (m_button) {
      // 修改 TtColorButton 的文本
      m_button->setText(m_textEdit->text());

      // 如果选择了新的背景颜色，则更新
      if (m_chosenColor.isValid()) {
        m_button->setColors(m_chosenColor);
      }

      // 如果选择了新的勾选区域颜色，则更新
      if (m_chosenCheckBlockColor.isValid()) {
        m_button->setCheckBlockColor(m_chosenCheckBlockColor);
      }
    }
    accept();
  }

 private:
  TtColorButton* m_button;
  Ui::TtLineEdit* m_textEdit;
  QPushButton* m_colorButton;
  // QPushButton* m_checkBlockButton;
  Ui::TtTextButton* m_checkBlockButton;
  QColor m_chosenColor;
  QColor m_chosenCheckBlockColor;
};

#endif  // TTCOLORBUTTONEDITORDIALOG_H
