#include "window/main_window.h"

#include <QActionGroup>
#include <QApplication>
#include <QCoreApplication>
#include <QDockWidget>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QMargins>
#include <QPushbutton>
#include <QStackedLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QStyle>
#include <qtconcurrentrun.h>

#include <QWKWidgets/widgetwindowagent.h>
#include <Qt-Advanced-Stylesheets/src/QtAdvancedStylesheet.h>
#include <qtmaterialsnackbar.h>

#include <ui/effects/animated_drawer.h>
#include <ui/layout/vertical_layout.h>

#include <ui/control/TtContentDialog.h>
#include <ui/control/buttonbox/TtButtonBox.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/customizationtabwidget.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/message_bar.h>
#include <ui/widgets/tabwindow.h>
#include <ui/widgets/widget_group.h>

#include <ui/layout/horizontal_layout.h>
#include <ui/layout/vertical_layout.h>

#include <ui/window/title/window_button.h>
#include <ui/window/title/windowbar.h>

#include "ui/widgets/tabwindow.h"
#include "window/function_selection_window.h"
#include "window/modbus_window.h"
#include "window/mqtt_window.h"
#include "window/popup_window.h"
#include "window/serial_window.h"
#include "window/tcp_window.h"
#include "window/udp_window.h"

#include "ui/widgets/session_manager.h"

#include "storage/configs_manager.h"
#include "storage/setting_manager.h"
#include <ui/window/title/window_button.h>

#include "window/instruction_window.h"
#include <ui/widgets/widget_group.h>

#include "lang/translation_manager.h"
#include "ui/widgets/setting_widget.h"

namespace Window {

QMap<TtProtocolRole::Role, QString> configMappingTable = {
    {TtProtocolRole::Serial, SerialPrefix},
    {TtProtocolRole::TcpClient, TcpClientPrefix},
    {TtProtocolRole::UdpClient, UdpClientPrefix},
    {TtProtocolRole::TcpServer, TcpServerPrefix},
    {TtProtocolRole::UdpServer, UdpServerPrefix},
    {TtProtocolRole::MqttClient, MqttPrefix},
    {TtProtocolRole::ModbusClient, ModbusPrefix},
};

static inline void emulateLeaveEvent(QWidget *widget) {
  Q_ASSERT(widget);
  if (!widget) {
    return;
  }
  QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QScreen *screen = widget->screen();
#else
        const QScreen *screen = widget->windowHandle()->screen();
#endif
    const QPoint globalPos = QCursor::pos(screen);
    if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size())
             .contains(globalPos)) {
      QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
      if (widget->testAttribute(Qt::WA_Hover)) {
        const QPoint localPos = widget->mapFromGlobal(globalPos);
        const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
        static constexpr const auto oldPos = QPoint{};
        const Qt::KeyboardModifiers modifiers =
            QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        const auto event = new QHoverEvent(QEvent::HoverLeave, scenePos,
                                           globalPos, oldPos, modifiers);
        Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
        QCoreApplication::postEvent(widget, event);
      }
    }
  });
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

  // // qDebug() << "MainWindow constructor thread:" <<
  // QThread::currentThread();
  // // statusBar()->addWidget(new Ui::TtSvgButton());

  window_agent_ = new QWK::WidgetWindowAgent(this);
  installWindowAgent();
  setProperty("TtBaseClassName", "TtMainWindow");
  setObjectName("TtMainWindow");
  resize(980, 720);
  window_agent_->centralize();
  setWindowTitle(tr("TtDebugTool"));

  layout_ = new Ui::TtHorizontalLayout;
  central_widget_ = new QWidget(this);
  central_widget_->setLayout(layout_);
  central_widget_->setProperty("TtBaseClassName", "CentralWidget");
  central_widget_->setObjectName("CentralWidget");
  setCentralWidget(central_widget_);

  // setAttribute(Qt::WA_DontCreateNativeAncestors);
  loadStyleSheet(Theme::Dark);

  setLeftBar();
  function_select_ = new FunctionSelectionWindow();

  tabWidget_ = new Ui::TabWindow(this);
  tabWidget_->addNewTab(function_select_);
  // addWindow 已经添加了 root
  TabWindowManager::instance()->setRootWindow(tabWidget_);

  // // 容纳一定了 widget, 以供外部切换, 直接存放 widget, 而非设定
  auto *leftPopUp = new Window::PopUpWindow();

  QSplitter *mainSplitter = new QSplitter(this);
  mainSplitter->setContentsMargins(QMargins(0, 0, 0, 0));
  mainSplitter->addWidget(leftPopUp);
  leftPopUp->setMinimumWidth(1);
  mainSplitter->addWidget(tabWidget_);

  mainSplitter->setStretchFactor(0, 1);
  mainSplitter->setStretchFactor(1, 3);

  mainSplitter->setCollapsible(0, true);
  mainSplitter->setCollapsible(1, false);
  // 创建 AnimatedDrawer 控制器
  Ui::TtAnimatedDrawer *controller =
      new Ui::TtAnimatedDrawer(mainSplitter, leftPopUp, tabWidget_, this);

  {
    QWidget *linkList = new QWidget;
    auto linkListLayout = new QGridLayout(linkList);

    Ui::TtNormalLabel *title = new Ui::TtNormalLabel(tr("连接列表"), linkList);

    QWidget *btnWidget = new QWidget(linkList);
    Ui::TtHorizontalLayout *btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    Ui::TtSvgButton *addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget); // 刷新
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton *refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget); // 新建
    refreshBtn->setEnableHoldToCheck(true);
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    linkListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    linkListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);
    Ui::TtNormalLabel *displayInfo = new Ui::TtNormalLabel();
    displayInfo->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(88, 88)));
    Ui::TtNormalLabel *displayWords =
        new Ui::TtNormalLabel(tr("暂时没有可用的连接"));
    auto addLinkBtn = new QPushButton(tr("新建连接"));
    addLinkBtn->setMinimumSize(100, 28);
    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout *tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();
    history_link_list_ = new Ui::SessionManager();
    history_link_list_->setSelectionMode(QAbstractItemView::NoSelection);
    auto stack_ = new QStackedWidget(linkList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_link_list_);
    linkListLayout->addWidget(stack_, 1, 0, 1, 2);
    connect(history_link_list_->model(), &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_link_list_->count() == 1) { // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_link_list_->model(), &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_link_list_->count() == 0) { // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    connect(addLinkBtn, &QPushButton::clicked, this,
            &MainWindow::addSelectToolPage);
    connect(addBtn, &Ui::TtSvgButton::clicked, this,
            &MainWindow::addSelectToolPage);

    connect(refreshBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "无",
                                    tr("连接列表已刷新"), 1000, this);
    });
    leftPopUp->addPairedWidget(0, linkList);
  }

  {
    QWidget *instructionList = new QWidget;
    instructionList->setObjectName("instructionList");
    auto instructionListLayout = new QGridLayout(instructionList);

    Ui::TtNormalLabel *title =
        new Ui::TtNormalLabel(tr("指令"), instructionList);

    QWidget *btnWidget = new QWidget(instructionList);
    Ui::TtHorizontalLayout *btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    Ui::TtSvgButton *addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget);
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton *refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget);
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    refreshBtn->setEnableHoldToCheck(true);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    instructionListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    instructionListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);

    Ui::TtNormalLabel *displayWords = new Ui::TtNormalLabel(tr("指令列表未空"));
    auto addLinkBtn = new QPushButton(tr("新建指令"));
    addLinkBtn->resize(100, 28);

    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout *tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();
    history_instruction_list_ = new Ui::SessionManager();
    history_instruction_list_->setSelectionMode(QAbstractItemView::NoSelection);
    auto stack_ = new QStackedWidget(instructionList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_instruction_list_);
    instructionListLayout->addWidget(stack_, 1, 0, 1, 2);
    connect(history_instruction_list_->model(),
            &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_instruction_list_->count() == 1) { // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_instruction_list_->model(),
            &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_instruction_list_->count() == 0) { // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      Window::InstructionWindow *instruction = new Window::InstructionWindow;
      tabWidget_->addNewTab(instruction, tr("未命名的指令"));
      tabWidget_->setCurrentWidget(instruction);
    });
    connect(addBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Window::InstructionWindow *instruction = new Window::InstructionWindow;
      tabWidget_->addNewTab(instruction, tr("未命名的指令"));
      tabWidget_->setCurrentWidget(instruction);
    });
    connect(refreshBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      Ui::TtMessageBar::information(TtMessageBarType::Top, "无",
                                    tr("连接列表已刷新"), 1000);
    });
    leftPopUp->addPairedWidget(1, instructionList);
  }

  {
    QWidget *mockList = new QWidget;
    mockList->setObjectName("mockList");
    auto mockListLayout = new QGridLayout(mockList);

    Ui::TtNormalLabel *title = new Ui::TtNormalLabel(tr("模拟"), mockList);

    QWidget *btnWidget = new QWidget(mockList);
    Ui::TtHorizontalLayout *btnWidgetLayout =
        new Ui::TtHorizontalLayout(btnWidget);
    // // 以下两个会消失
    Ui::TtSvgButton *addBtn =
        new Ui::TtSvgButton(":/sys/plus-circle.svg", btnWidget); // 刷新
    addBtn->setSvgSize(QSize(18, 18));
    Ui::TtSvgButton *refreshBtn =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", btnWidget); // 新建
    refreshBtn->setSvgSize(QSize(18, 18));
    refreshBtn->setColors(Qt::black, Qt::blue);
    btnWidgetLayout->addWidget(addBtn);
    btnWidgetLayout->addSpacerItem(new QSpacerItem(5, 0));
    btnWidgetLayout->addWidget(refreshBtn);

    mockListLayout->addWidget(title, 0, 0, Qt::AlignTop);
    mockListLayout->addWidget(btnWidget, 0, 1, Qt::AlignRight);

    // 首页界面
    // 显示的图片
    Ui::TtNormalLabel *displayInfo = new Ui::TtNormalLabel();
    displayInfo->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(88, 88)));

    // 图片底下的文字
    Ui::TtNormalLabel *displayWords =
        new Ui::TtNormalLabel(tr("没有可用的模拟服务"));
    auto addLinkBtn = new QPushButton(tr("新建连接"));
    addLinkBtn->setMinimumSize(100, 28);

    // 创建原始界面
    auto original_widget_ = new QWidget();
    Ui::TtVerticalLayout *tmpl = new Ui::TtVerticalLayout(original_widget_);
    tmpl->addStretch();
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();

    // 存储历史保存设置
    history_mock_list_ = new Ui::SessionManager();
    history_mock_list_->setSelectionMode(QAbstractItemView::NoSelection);

    // 使用堆叠布局
    auto stack_ = new QStackedWidget(mockList);
    stack_->addWidget(original_widget_);
    stack_->addWidget(history_mock_list_);

    mockListLayout->addWidget(stack_, 1, 0, 1, 2);

    // 监听 model 的行数变化
    connect(history_mock_list_->model(), &QAbstractItemModel::rowsInserted,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_mock_list_->count() == 1) { // 从0变为1
                stack_->setCurrentIndex(1);
              }
            });
    connect(history_mock_list_->model(), &QAbstractItemModel::rowsRemoved,
            [=](const QModelIndex &parent, int first, int last) {
              if (history_mock_list_->count() == 0) { // 从1变为0
                stack_->setCurrentIndex(0);
              }
            });
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      SimulateFunctionSelectionWindow *simulate =
          new SimulateFunctionSelectionWindow();
      tabWidget_->addNewTab(simulate, tr("创建模拟器"));
      QObject::connect(
          simulate, &SimulateFunctionSelectionWindow::switchRequested,
          [this](TtProtocolRole::Role role) {
            // 在 tab 上切换不同的 功能 widget
            tabWidget_->sessionSwitchPage(tabWidget_->currentIndex(), role);
          });
      tabWidget_->setCurrentWidget(simulate);
    });
    connect(addBtn, &Ui::TtSvgButton::clicked, [this, controller]() {
      SimulateFunctionSelectionWindow *simulate =
          new SimulateFunctionSelectionWindow();
      tabWidget_->addNewTab(simulate, tr("创建模拟器"));
      QObject::connect(
          simulate, &SimulateFunctionSelectionWindow::switchRequested,
          [this](TtProtocolRole::Role role) {
            tabWidget_->sessionSwitchPage(tabWidget_->currentIndex(), role);
          });
      tabWidget_->setCurrentWidget(simulate);
    });

    leftPopUp->addPairedWidget(2, mockList);
  }

  registerTabWidget();

  // 左边栏
  layout_->addWidget(left_bar_, 0, Qt::AlignLeft);

  layout_->addWidget(mainSplitter);

  connect(left_bar_logic_, &Ui::TtWidgetGroup::widgetClicked,
          [leftPopUp, controller](int index) {
            // 打开
            if (controller->targetDrawerVisible()) {
              // 重复
              if (!leftPopUp->switchToWidget(index)) {
                controller->closeDrawer();
              }
            } else {
              leftPopUp->switchToWidget(index);
              controller->openDrawer();
            }
          });

  buttonGroup = new Ui::WidgetGroup(this);

  connectSignals();

  initLanguageMenu();
  readingProjectConfiguration();

  qDebug() << "MainWindow Success";
}

MainWindow::~MainWindow() {
  qDebug() << "Mainwindow :析构";
  // tabWidget_->close();
  // tabWidget_->deleteLater();

  // 先关闭所有 TabWindow
  // auto manager = TabWindowManager::instance();
  // for (auto w : manager->windows()) {
  //   w->close(); // 触发 TabWindow 的 closeEvent
  // }

  // // 再执行默认关闭逻辑
  // QMainWindow::closeEvent(event);

  // // 强制退出应用
  // QApplication::quit();
}

void MainWindow::initLanguageMenu() {
  // 指定 .ts 文件路径，搜索获取 .ts 文件列表
  QDir languageDir("F:/MyProject/DebugTool/DebugTool/res/language/");
  QStringList tsFileList =
      languageDir.entryList(QStringList("*.ts"), QDir::Files);
  QStringList tsFilePathList;
  for (const QString &tsFile : tsFileList) {
    // 绝对路径
    tsFilePathList.append(languageDir.filePath(tsFile));
  }

  // 为每个TS文件单独调用lrelease
  for (const QString &tsFile : tsFileList) {
    QString tsFilePath = languageDir.absoluteFilePath(tsFile);
    QString qmFilePath = tsFilePath;
    qmFilePath.replace(QRegularExpression("\\.ts$"), ".qm");

    QStringList args;
    args << tsFilePath << "-qm" << qmFilePath;

    QProcess *process = new QProcess(this);
    process->start("F:/MyProject/DebugTool/DebugTool/res/lrelease/lrelease.exe",
                   args);

    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, process, tsFile, tsFileList]() {
              if (process->exitStatus() == QProcess::NormalExit &&
                  process->exitCode() == 0) {
                // qDebug() << "Successfully processed:" << tsFile;
                // 检查是否所有文件都处理完毕
                static int processedCount = 0;
                if (++processedCount >= tsFileList.size()) {
                  compileTsFilesFinished();
                  processedCount = 0; // 重置计数器
                }
              }
              process->deleteLater();
            });
  }
}

void MainWindow::closeWindow() {

  // QRect windowRect = window()->geometry();
  // // 抵消setFixedSize导致的移动偏航
  // window()->setMinimumSize(0, 0);
  // window()->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  // QPropertyAnimation* closeGeometryAnimation =
  //     new QPropertyAnimation(window(), "geometry");
  // QPropertyAnimation* closeOpacityAnimation =
  //     new QPropertyAnimation(window(), "windowOpacity");
  // connect(closeGeometryAnimation, &QPropertyAnimation::finished, this,
  //         [=]() { window()->close(); });
  // closeGeometryAnimation->setStartValue(windowRect);
  // // qreal minmumWidth = (d->_titleLabel->width() + 320);
  // qreal minmumWidth = 320;
  // closeGeometryAnimation->setEndValue(QRect(
  //     windowRect.x() + (windowRect.width() - minmumWidth) / 2,
  //     windowRect.y() + (windowRect.height() / 2) - 145, minmumWidth, 290));
  // closeGeometryAnimation->setDuration(300);
  // closeGeometryAnimation->setEasingCurve(QEasingCurve::InOutSine);

  // closeOpacityAnimation->setStartValue(1);
  // closeOpacityAnimation->setEndValue(0);
  // closeOpacityAnimation->setDuration(300);
  // closeOpacityAnimation->setEasingCurve(QEasingCurve::InOutSine);
  // closeGeometryAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  // closeOpacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
  QWidget::close();
}

void MainWindow::compileTsFilesFinished() {
  QMenuBar *menuBar =
      qobject_cast<Ui::WindowBar *>(window_agent_->titleBar())->menuBar();
  QActionGroup *actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);
  // 固定插入到最后
  QMenu *languageMenu = menuBar->addMenu(tr("语言"));
  // 搜索获取 .qm 文件列表 qmFiles
  QDir languageDir("F:/MyProject/DebugTool/DebugTool/res/language/");
  if (!languageDir.exists()) {
    qWarning() << "Language directory not exists!";
    return;
  }

  QString currentLoadedFile =
      Lang::TtTranslationManager::instance().currentLanguage();
  qDebug() << currentLoadedFile;
  // 搜索 .qm
  QStringList qmFiles = languageDir.entryList(QStringList("*.qm"), QDir::Files);
  // 动态生成语言菜单
  for (const QString &qmFile : qmFiles) {
    QString languageName = extractLanguageName(qmFile); // 解析语言名称
    QAction *action = new QAction(languageName, this);
    action->setCheckable(true);
    languageMenu->addAction(action);
    actionGroup->addAction(action);
    qDebug() << qmFile;
    if (currentLoadedFile == qmFile) {
      action->setChecked(true);
    }
    connect(action, &QAction::triggered, this,
            [this, qmFile]() { changeLanguage(qmFile); });
  }
  // qDebug() <<
}

void MainWindow::saveCsvFile() {
  // 弹出一个串口, 选择你要保存的 csv 文件.
  // 当前的界面的 csv
  // 获取的是当前的 窗口, 当如果是跨越窗口呢 ?
  // 弹出一个 dialog, 里面有多个选择, 你要保存哪一个窗口的 csv
  Ui::TtContentDialog choseDialog(this);
  // choseDialog;
  // 非模态的
  QWidget *selectSaveCsvData = new QWidget;

  auto widget =
      qobject_cast<Window::SerialWindow *>(tabWidget_->currentWidget());
  if (widget) {
    qDebug() << "保存窗口 csv";
    // qDebug() << widget->getTitle();
    widget->saveWaveFormData();
  }
}

void MainWindow::switchToOtherTabPage(const QString &uuid, const int &type) {
  if (tabWidget_->isCurrentDisplayedPage(uuid)) {
    tabWidget_->switchByPage(uuid);
  } else if (tabWidget_->isStoredInMem(uuid)) {
    tabWidget_->switchByReadingMem(uuid,
                                   static_cast<TtProtocolRole::Role>(type));
  } else {
    base::DetectRunningTime test;
    // 只有第一次运行才会慢, 包行了创建首个窗口的缓存
    qDebug() << "switch uuid " << uuid;

    tabWidget_->switchByReadingDisk(
        uuid, static_cast<TtProtocolRole::Role>(type),

        getSpecificConfiguration(uuid,
                                 static_cast<TtProtocolRole::Role>(type)));
    qDebug() << test.elapseMilliseconds();
  }
}

void MainWindow::addSelectToolPage() {
  FunctionSelectionWindow *selectTool = new FunctionSelectionWindow();
  tabWidget_->addNewTab(selectTool, tr("新建连接"));
  QObject::connect(
      selectTool, &FunctionSelectionWindow::switchRequested, selectTool,
      [this, selectTool](TtProtocolRole::Role role) {
        if (selectTool) {
          if (selectTool->parentWidget()) {
            auto *parent = selectTool->parentWidget();

            while (parent) {
              if (auto *tabWindow = qobject_cast<TabWindow *>(parent)) {
                tabWindow->sessionSwitchPage(tabWindow->currentIndex(), role);
                return;
              }
              parent = parent->parentWidget();
            }
          }
        }
      });
  tabWidget_->setCurrentWidget(selectTool);
}

void MainWindow::addSelectAllToolPage() {
  AllFunctionSelectionWindow *selectAllTool = new AllFunctionSelectionWindow();
  tabWidget_->addNewTab(selectAllTool, tr("新建连接"));
  QObject::connect(
      selectAllTool, &AllFunctionSelectionWindow::switchRequested,
      selectAllTool, [this, selectAllTool](TtProtocolRole::Role role) {
        if (selectAllTool) {
          // 根据 父对象 获取 TabWindow
          if (selectAllTool->parentWidget()) {
            auto *parent = selectAllTool->parentWidget();

            while (parent) {
              if (auto *tabWindow = qobject_cast<TabWindow *>(parent)) {
                // 在父对象中显示对应的 tabWidget
                tabWindow->sessionSwitchPage(tabWindow->currentIndex(), role);
                return;
              }
              parent = parent->parentWidget();
            }
          }
        }
      });
  tabWidget_->setCurrentWidget(selectAllTool);
}

bool MainWindow::event(QEvent *event) {
  switch (event->type()) {
  case QEvent::WindowActivate: {
    auto menu = menuWidget();
    if (menu) {
      menu->setProperty("bar-active", true);
      style()->polish(menu);
    }
    break;
  }
  case QEvent::WindowDeactivate: {
    auto menu = menuWidget();
    if (menu) {
      menu->setProperty("bar-active", false);
      style()->polish(menu);
    }
    break;
  }

  default:
    break;
  }
  return QMainWindow::event(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // 关闭所有窗口
  // 写入文件
  // Storage::SettingsManager::instance().saveSettings();
  // 强制保存
  Storage::SettingsManager::instance().forceSave();

  // 先关闭所有 TabWindow
  auto manager = TabWindowManager::instance();
  for (auto w : manager->windows()) {
    w->close(); // 触发 TabWindow 的 closeEvent
  }

  // 再执行默认关闭逻辑
  QMainWindow::closeEvent(event);

  // 强制退出应用
  QApplication::quit();
}

void MainWindow::installWindowAgent() {
  window_agent_->setup(this);

  // 2. Construct your title bar
  auto menuBar = [this]() {
    auto menuBar = new QMenuBar(this);

    auto file = new QMenu(tr("文件(&F)"), menuBar);
    file->addAction(new QAction(tr("新建"), menuBar));
    file->addAction(new QAction(tr("打开"), menuBar));

    file->addSeparator();
    auto saveToCsv = new QAction(tr("保存到 CSV"), menuBar);
    file->addAction(saveToCsv);
    connect(saveToCsv, &QAction::triggered, this, [this] { saveCsvFile(); });

    file->addAction(new QAction(tr("保存到 Sqlite 数据库"), menuBar));
    file->addSeparator();

    // auto edit = new QMenu(tr("Edit(&E)"), menuBar);
    // edit->addAction(new QAction(tr("Undo(&U)"), menuBar));
    // edit->addAction(new QAction(tr("Redo(&R)"), menuBar));

    // 创建帮助菜单
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    QAction *aboutQtAction = new QAction(tr("关于 Qt"), menuBar);
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    // 添加"关于应用"选项
    // QAction *aboutAction = helpMenu->addAction(tr("&About Application"));
    // connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    // helpMenu->addAction(new QAction(tr("联系作者"), menuBar));

    // Theme action
    // auto darkAction = new QAction(tr("Enable dark theme"), menuBar);
    // darkAction->setCheckable(true);

    // connect(darkAction, &QAction::triggered, this, [this](bool checked) {
    //     loadStyleSheet(checked ? Dark : Light); //
    // });
    // connect(this, &MainWindow::themeChanged, darkAction, [this, darkAction]()
    // {
    //     darkAction->setChecked(currentTheme == Dark); //
    // });

#ifdef Q_OS_WIN
    // auto noneAction = new QAction(tr("None"), menuBar);
    // noneAction->setData(QStringLiteral("none"));
    // noneAction->setCheckable(true);
    // noneAction->setChecked(true);

    // auto dwmBlurAction = new QAction(tr("Enable DWM blur"), menuBar);
    // dwmBlurAction->setData(QStringLiteral("dwm-blur"));
    // dwmBlurAction->setCheckable(true);

    // auto acrylicAction = new QAction(tr("Enable acrylic material"), menuBar);
    // acrylicAction->setData(QStringLiteral("acrylic-material"));
    // acrylicAction->setCheckable(true);

    // auto micaAction = new QAction(tr("Enable mica"), menuBar);
    // micaAction->setData(QStringLiteral("mica"));
    // micaAction->setCheckable(true);

    // auto micaAltAction = new QAction(tr("Enable mica alt"), menuBar);
    // micaAltAction->setData(QStringLiteral("mica-alt"));
    // micaAltAction->setCheckable(true);

    // auto winStyleGroup = new QActionGroup(menuBar);
    // winStyleGroup->addAction(noneAction);
    // winStyleGroup->addAction(dwmBlurAction);
    // winStyleGroup->addAction(acrylicAction);
    // winStyleGroup->addAction(micaAction);
    // winStyleGroup->addAction(micaAltAction);
    // connect(winStyleGroup, &QActionGroup::triggered, this, [this,
    // winStyleGroup](QAction *action) {
    //     // Unset all custom style attributes first, otherwise the style will
    //     not display correctly for (const QAction* _act :
    //     winStyleGroup->actions()) {
    //         const QString data = _act->data().toString();
    //         if (data.isEmpty() || data == QStringLiteral("none")) {
    //             continue;
    //         }
    //         window_agent_->setWindowAttribute(data, false);
    //     }
    //     const QString data = action->data().toString();
    //     if (data == QStringLiteral("none")) {
    //         setProperty("custom-style", false);
    //     } else if (!data.isEmpty()) {
    //         window_agent_->setWindowAttribute(data, true);
    //         setProperty("custom-style", true);
    //     }
    //     style()->polish(this);
    // });

#elif defined(Q_OS_MAC)
    auto darkBlurAction = new QAction(tr("Dark blur"), menuBar);
    darkBlurAction->setCheckable(true);
    connect(darkBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "dark")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", true);
        style()->polish(this);
      }
    });

    auto lightBlurAction = new QAction(tr("Light blur"), menuBar);
    lightBlurAction->setCheckable(true);
    connect(lightBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "light")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", true);
        style()->polish(this);
      }
    });

    auto noBlurAction = new QAction(tr("No blur"), menuBar);
    noBlurAction->setCheckable(true);
    connect(noBlurAction, &QAction::toggled, this, [this](bool checked) {
      if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"),
                                           "none")) {
        return;
      }
      if (checked) {
        setProperty("custom-style", false);
        style()->polish(this);
      }
    });

    auto macStyleGroup = new QActionGroup(menuBar);
    macStyleGroup->addAction(darkBlurAction);
    macStyleGroup->addAction(lightBlurAction);
    macStyleGroup->addAction(noBlurAction);
#endif

    // Real menu
    // auto settings = new QMenu(tr("Settings(&S)"), menuBar);
    // settings->addAction(darkAction);

#ifdef Q_OS_WIN
    // settings->addSeparator();
    // settings->addAction(noneAction);
    // settings->addAction(dwmBlurAction);
    // settings->addAction(acrylicAction);
    // settings->addAction(micaAction);
    // settings->addAction(micaAltAction);
#elif defined(Q_OS_MAC)
    settings->addAction(darkBlurAction);
    settings->addAction(lightBlurAction);
    settings->addAction(noBlurAction);
#endif

    menuBar->addMenu(file);
    menuBar->addMenu(helpMenu);
    // menuBar->addMenu(edit);
    // menuBar->addMenu(settings);
    return menuBar;
  }();

  menuBar->setObjectName(QStringLiteral("win-menu-bar"));

  auto titleLabel = new QLabel();
  titleLabel->setAlignment(Qt::AlignCenter);
  titleLabel->setObjectName(QStringLiteral("win-title-label"));

#ifndef Q_OS_MAC
  auto iconButton = new Ui::WindowbarButton();
  iconButton->setObjectName(QStringLiteral("icon-button"));
  iconButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto topButton = new Ui::WindowbarButton();
  topButton->setCheckable(true);
  topButton->setObjectName(QStringLiteral("top-button"));
  topButton->setProperty("system-button", true);
  topButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto minButton = new Ui::WindowbarButton();
  minButton->setObjectName(QStringLiteral("min-button"));
  minButton->setProperty("system-button", true);
  minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto maxButton = new Ui::WindowbarButton();
  maxButton->setCheckable(true);
  maxButton->setObjectName(QStringLiteral("max-button"));
  maxButton->setProperty("system-button", true);
  maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  auto closeButton = new Ui::WindowbarButton();
  closeButton->setObjectName(QStringLiteral("close-button"));
  closeButton->setProperty("system-button", true);
  closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#endif

  auto windowBar = new Ui::WindowBar();
#ifndef Q_OS_MAC
  windowBar->setIconButton(iconButton);
  windowBar->setTopButton(topButton);
  windowBar->setMinButton(minButton);
  windowBar->setMaxButton(maxButton);
  windowBar->setCloseButton(closeButton);
#endif
  windowBar->setMenuBar(menuBar);
  windowBar->setTitleLabel(titleLabel);
  windowBar->setHostWidget(this);

  window_agent_->setTitleBar(windowBar);
#ifndef Q_OS_MAC
  window_agent_->setSystemButton(QWK::WindowAgentBase::WindowIcon, iconButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
  window_agent_->setSystemButton(QWK::WindowAgentBase::Close, closeButton);
#endif
  window_agent_->setHitTestVisible(menuBar, true);
  window_agent_->setHitTestVisible(topButton, true);

#ifdef Q_OS_MAC
  windowAgent->setSystemButtonAreaCallback([](const QSize &size) {
    static constexpr const int width = 75;
    return QRect(QPoint(size.width() - width, 0),
                 QSize(width, size.height())); //
  });
#endif

  setMenuWidget(windowBar);

#ifndef Q_OS_MAC
  connect(iconButton, &QAbstractButton::clicked, window_agent_,
          [this, iconButton] {
            iconButton->setProperty("double-click-close", false);

            // WindowBar's icon show a menu to choose
            QTimer::singleShot(75, window_agent_, [this, iconButton]() {
              if (iconButton->property("double-click-close").toBool()) {
                return;
              }
              window_agent_->showSystemMenu(
                  iconButton->mapToGlobal(QPoint(0, iconButton->height())));
            });
          });

  connect(iconButton, &Ui::WindowbarButton::doubleClicked, this,
          [iconButton, this]() {
            iconButton->setProperty("double-click-close", true);
            close();
          });

  connect(windowBar, &Ui::WindowBar::topRequest, this, [this](bool top) {
    auto *window = this->windowHandle();

    if (window) {
#ifdef Q_OS_WIN
      HWND hwnd = reinterpret_cast<HWND>(window->winId());
      if (top) {
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      } else {
        ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE);
      }
#else
        window->setFlag(Qt::WindowStaysOnTopHint, top);
#endif
    }
  });

  connect(windowBar, &Ui::WindowBar::minimizeRequested, this,
          &QWidget::showMinimized);
  connect(windowBar, &Ui::WindowBar::maximizeRequested, this,
          [this, maxButton](bool max) {
            qDebug() << "max:" << max;
            if (max) {
              showMaximized();
            } else {
              showNormal();
            }

            // It's a Qt issue that if a QAbstractButton::clicked triggers a
            // window's maximization, the button remains to be hovered until the
            // mouse move. As a result, we need to manually send leave events to
            // the button.
            emulateLeaveEvent(maxButton);
          });
  // connect(windowBar, &Ui::WindowBar::closeRequested, this, &QWidget::close);

  // connect(windowBar, &Ui::WindowBar::closeRequested, this, [this]() {
  //   // 模态窗口
  //   Ui::TtContentDialog *dialog = new Ui::TtContentDialog(
  //       true, Ui::TtContentDialog::LayoutSelection::THREE_OPTIONS, this);
  //   QPointer<Ui::TtContentDialog> dialogPtr(dialog);
  //   dialog->setCenterText(tr("确定要退出程序吗"));
  //   // 样式表有时会失效
  //   connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this,
  //           [this, dialogPtr] { dialogPtr->reject(); });
  //   connect(dialog, &Ui::TtContentDialog::middleButtonClicked, this,
  //           [this, dialogPtr] {
  //             dialogPtr->accept();
  //             MainWindow::showMinimized();
  //           });
  //   connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this,
  //           [this, dialogPtr] {
  //             dialogPtr->accept();
  //             MainWindow::closeWindow();
  //           });
  //   dialog->exec();
  //   delete dialog;
  //   qDebug() << "finale test";
  // });
  connect(windowBar, &Ui::WindowBar::closeRequested, this, [this]() {
    // QScopedPointer<Ui::TtContentDialog> dialog(new Ui::TtContentDialog(
    //     Qt::ApplicationModal, true,
    //     Ui::TtContentDialog::LayoutSelection::THREE_OPTIONS, this));

    Ui::TtContentDialog *dialog = new Ui::TtContentDialog(
        Qt::ApplicationModal, true,
        Ui::TtContentDialog::LayoutSelection::THREE_OPTIONS, this);

    dialog->setCenterText(tr("确定要退出程序吗"));
    dialog->setAttribute(Qt::WA_DeleteOnClose); // 二次删除的情况
    connect(dialog, &Ui::TtContentDialog::leftButtonClicked, dialog,
            &QDialog::reject);
    connect(dialog, &Ui::TtContentDialog::middleButtonClicked, dialog,
            &QDialog::accept); // 中间按钮 -> accept()
    connect(dialog, &Ui::TtContentDialog::rightButtonClicked, dialog,
            &QDialog::accept); // 右侧按钮 -> accept()
    const int result = dialog->exec();
    if (result == QDialog::Accepted) {
      // 获取最后点击的按钮类型
      if (dialog->clickButtonType() == Ui::TtContentDialog::MiddleButton) {
        // 最小化时, 背后显示的阴影还是存在
        MainWindow::showMinimized();
      } else {
        MainWindow::closeWindow();
      }
    } else {
      qDebug() << "用户取消操作";
    }
  });
  // 有bug, 进程崩溃

#endif
}

void MainWindow::loadStyleSheet(Theme theme) {

#if 0
  //QString AppDir = qApp->applicationDirPath();
  QString AppDir("F:/MyProject/SerialportTool/out/build");
  //qDebug() << AppDir;

  acss::QtAdvancedStylesheet AdvancedStylesheet;

  // first set the directory that contains all your styles
  AdvancedStylesheet.setStylesDirPath(AppDir +
                                      "/../../SerialportTool/res/styles");
  //qDebug() << AdvancedStylesheet.currentStylePath();

  // now set the output folder where the processed styles are stored. The
  // style manager will create a sub directory for each style
  AdvancedStylesheet.setOutputDirPath(AppDir + "/output");

  // set the current style and select a theme. After these two calls, the output
  // folder will contains the generated resoruces and stylesheet.
  // AdvancedStylesheet.setCurrentStyle("qt_material");
  AdvancedStylesheet.setCurrentStyle("modify_qt_material");
  AdvancedStylesheet.setCurrentTheme("dark_teal");

  AdvancedStylesheet.updateStylesheet();
  
  qApp->setStyleSheet(AdvancedStylesheet.styleSheet());
#endif

  // now you can set the generated stylesheet
  // if (!styleSheet().isEmpty() && theme == currentTheme)
  //   return;
  // currentTheme = theme;

  // if (QFile qss(theme == Dark ? QStringLiteral(":/dark-style.qss")
  //                             : QStringLiteral(":/light-style.qss"));
  QFile qss(":/theme/dark-style.qss");
  qss.open(QIODevice::ReadOnly | QIODevice::Text);
  // 应用与全局
  qApp->setStyleSheet(QString::fromUtf8(qss.readAll()));
  // setStyleSheet(QString::fromUtf8(qss.readAll()));
  // emit themeChanged();
}

void MainWindow::setLeftBar() {
  left_bar_ = new QWidget(this);
  left_bar_logic_ = new Ui::TtWidgetGroup(this);
  left_bar_logic_->setExclusive(true);
  communication_connection_ =
      new Ui::TtSvgButton(":/sys/communicating-junctions.svg", left_bar_);
  communication_connection_->setColors(Qt::black, Qt::blue);
  communication_connection_->setEnableHoldToCheck(false);

  communication_instruction_ =
      new Ui::TtSvgButton(":/sys/Instruction-configuration.svg", left_bar_);
  communication_instruction_->setColors(Qt::black, Qt::blue);
  communication_instruction_->setEnableHoldToCheck(false);

  realistic_simulation_ =
      new Ui::TtSvgButton(":/sys/real-time-simulation.svg", left_bar_);
  realistic_simulation_->setColors(Qt::black, Qt::blue);
  realistic_simulation_->setEnableHoldToCheck(false);

  setting_ = new Ui::TtSvgButton(":/sys/setting.svg", left_bar_);
  setting_->setColors(Qt::black, Qt::blue);
  setting_->setEnableHoldToCheck(true);

  left_bar_logic_->addWidget(communication_connection_);
  left_bar_logic_->addWidget(communication_instruction_);
  left_bar_logic_->addWidget(realistic_simulation_);

  Ui::TtVerticalLayout *left_bar_layout = new Ui::TtVerticalLayout(left_bar_);

  left_bar_layout->addSpacerItem(new QSpacerItem(0, 10));
  left_bar_layout->addWidget(communication_connection_, 0, Qt::AlignTop);
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 20));
  left_bar_layout->addWidget(communication_instruction_, 0, Qt::AlignTop);
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 20));
  left_bar_layout->addWidget(realistic_simulation_, 0, Qt::AlignTop);
  left_bar_layout->addStretch();
  left_bar_layout->addWidget(setting_, 0, Qt::AlignBottom);
  left_bar_layout->addSpacerItem(new QSpacerItem(0, 20));

  left_bar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  left_bar_->setMinimumWidth(40);
}

void MainWindow::connectSignals() {
  // 删除的时候, 底层也删除
  connect(history_link_list_, &Ui::SessionManager::uuidsChanged, buttonGroup,
          &Ui::WidgetGroup::updateUuid);
  connect(history_link_list_, &Ui::SessionManager::uuidsChanged,
          [](const QString &index, TtProtocolRole::Role role) {
            // 删除 tab 页面
            TabWindowManager::instance()->removeUuidTabPage(index);
            // 本地删除
            Storage::SettingsManager::instance().removeOneSetting(
                configMappingTable[role] + index);
          });
  connect(history_mock_list_, &Ui::SessionManager::uuidsChanged, buttonGroup,
          &Ui::WidgetGroup::updateUuid);

  connect(history_mock_list_, &Ui::SessionManager::uuidsChanged,
          [](const QString &index) {
            TabWindowManager::instance()->removeUuidTabPage(index);
          });

  connect(function_select_, &FunctionSelectionWindow::switchRequested,
          [this](TtProtocolRole::Role role) {
            tabWidget_->sessionSwitchPage(tabWidget_->currentIndex(), role);
          });
  connect(setting_, &Ui::TtSvgButton::clicked, this, [this]() {
    if (!setting_widget_) {
      setting_widget_ = new Ui::SettingWidget;
      setting_widget_->setObjectName(QStringLiteral("SettingWidget"));
      tabWidget_->addNewTab(setting_widget_, QIcon(":/sys/settings.svg"),
                            tr("设置"));
    }
    tabWidget_->setCurrentWidget(setting_widget_);
  });

  connect(buttonGroup, &Ui::WidgetGroup::currentIndexChanged, this,
          &MainWindow::switchToOtherTabPage);

  connect(TabWindowManager::instance(), &TabWindowManager::tabCreateRequested,
          this, [this](TabWindow *sourceWindow) {
            AllFunctionSelectionWindow *selectAllTool =
                new AllFunctionSelectionWindow;
            sourceWindow->addNewTab(selectAllTool, tr("新建连接"));
            sourceWindow->setCurrentWidget(selectAllTool);

            QObject::connect(
                selectAllTool, &AllFunctionSelectionWindow::switchRequested,
                selectAllTool, [selectAllTool](TtProtocolRole::Role role) {
                  if (selectAllTool) {
                    if (selectAllTool->parentWidget()) {
                      auto *parent = selectAllTool->parentWidget();
                      while (parent) {
                        if (auto *tabWindow =
                                qobject_cast<TabWindow *>(parent)) {
                          int index = tabWindow->indexOf(selectAllTool);
                          if (index != -1) {
                            tabWindow->sessionSwitchPage(index, role);
                            break;
                          }
                        }
                        parent = parent->parentWidget();
                      }
                    }
                  }
                });
          });

  connect(
      tabWidget_, &TabWindow::widgetDeleted, this,
      [this](QWidget *deleteWidget) {
        if (setting_widget_ == deleteWidget) {
          setting_widget_ = nullptr;
        }
      },
      Qt::DirectConnection);
}

void MainWindow::registerTabWidget() {
  // 注册 串口窗口
  // tabWidget 分离出去了
  tabWidget_->registerWidget(
      TtProtocolRole::Serial,
      [this]() {
        // base::DetectRunningTime runtime;

        // auto widget = new QWidget();
        // Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(widget);

        //// 创建并显示加载动画
        // QLabel* loadingLabel = new QLabel(widget);
        // QMovie* loadingMovie = new QMovie(":/sys/loading.gif");
        // loadingLabel->setMovie(loadingMovie);
        // layout->addWidget(loadingLabel);
        // loadingMovie->start();

        //// 使用 QtConcurrent::run 在后台线程中创建 SerialWindow
        // QFuture<Window::SerialWindow*> future =
        //     QtConcurrent::run([this]() { return new Window::SerialWindow();
        //     });
        //// 使用 QFutureWatcher 监听线程完成
        // QFutureWatcher<Window::SerialWindow*>* watcher =
        //     new QFutureWatcher<Window::SerialWindow*>(this);

        // connect(watcher, &QFutureWatcher<Window::SerialWindow*>::finished,
        //         [this, watcher, layout, loadingLabel, loadingMovie]() {
        //           Window::SerialWindow* d = watcher->result();
        //           connect(d, &Window::SerialWindow::requestSaveConfig,
        //                   [this, d]() {
        //                     Storage::SettingsManager::instance().setSetting(
        //                         "Serial", d->getConfiguration());
        //                     // 向 ListWidget 添加 button
        //                     addDifferentConfiguration(1);
        //                     qDebug() << "save";
        //                     // 只有关闭整个 app 的时候才会保存到本地
        //                   });

        //          // 移除加载动画并添加 SerialWindow
        //          layout->removeWidget(loadingLabel);
        //          loadingMovie->stop();
        //          loadingLabel->deleteLater();
        //          layout->addWidget(d);

        //          qDebug() << "Create SerialWindow finished";
        //          watcher->deleteLater();
        //        });
        // watcher->setFuture(future);

        // qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
        // return widget;

        Window::SerialWindow *serial = new Window::SerialWindow();
        // qDebug() << "Create SerialWindow: " << runtime.elapseMilliseconds();
        connect(serial, &Window::SerialWindow::requestSaveConfig,
                [this, serial]() {
                  qDebug() << "request save";
                  // 能够找到 widget, 本窗口
                  // tabWidget_.find

                  // 所有的 uuid 保存在
                  // 当前 tabWidget_ 不是所处的 tabWindow
                  // 获取当前所在 tabWindow
                  // 这样才能获取正确的 uuid

                  // 有成功保存到 leftbar 中, 但是 uuid 获取失败
                  // 获取当前窗口的独特值(uuid), 是否已经保存到 listwidget,

                  // 获取的 uuid 全部有问题
                  // 应该使用实例获取

                  // 有 widget, 寻找 uuid

                  const auto &uuid = tabWidget_->getCurrentWidgetUUid(serial);

                  addDifferentConfiguration(
                      TtFunctionalCategory::Communication,
                      TtProtocolRole::Serial, serial->getTitle(),
                      // tabWidget_->getCurrentWidgetUUid());
                      // tabWidget_->getCurrentWidgetUUid(serial));
                      uuid);
                  // 设置对应的 tabTitle
                  // 设置的也有问题

                  // 有 uuid

                  // tabWidget_->setTabTitle(serial->getTitle());
                  tabWidget_->setTabTitle(uuid, serial->getTitle());

                  // 直接保存到本地
                  // 从磁盘读取，又从磁盘写入, 怎么区分 ?
                  // 重新开启应用时, 从磁盘读取内容, 有 uuid, 则不需要创建
                  // 直接保存到磁盘
                  Storage::SettingsManager::instance().setSetting(
                      // "Serial+" + tabWidget_->getCurrentWidgetUUid(),
                      "Serial+" + uuid, serial->getConfiguration());

                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000, this);
                  qDebug() << "save sucess";
                });
        return serial;
      },
      tr("未命名的串口连接"));
  tabWidget_->registerWidget(
      TtProtocolRole::TcpClient,
      [this]() {
        Window::TcpWindow *tcpClient =
            new Window::TcpWindow(TtProtocolType::Client);
        connect(tcpClient, &Window::TcpWindow::requestSaveConfig,
                [this, tcpClient]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(tcpClient);

                  addDifferentConfiguration(
                      TtFunctionalCategory::Communication,
                      TtProtocolRole::TcpClient, tcpClient->getTitle(),
                      // tabWidget_->getCurrentWidgetUUid());
                      uuid);
                  // tabWidget_->setTabTitle(tcpClient->getTitle());
                  tabWidget_->setTabTitle(uuid, tcpClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      // "TcpClient+" + tabWidget_->getCurrentWidgetUUid(),
                      "TcpClient+" + uuid, tcpClient->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return tcpClient;
      },
      tr("未命 TCP 客户端连接"));
  tabWidget_->registerWidget(
      TtProtocolRole::UdpClient,
      [this]() {
        Window::UdpWindow *udpClient =
            new Window::UdpWindow(TtProtocolType::Client);
        connect(udpClient, &Window::UdpWindow::requestSaveConfig,
                [this, udpClient]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(udpClient);
                  addDifferentConfiguration(TtFunctionalCategory::Communication,
                                            TtProtocolRole::UdpClient,
                                            udpClient->getTitle(), uuid);
                  tabWidget_->setTabTitle(uuid, udpClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "UdpClient+" + uuid, udpClient->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return udpClient;
      },
      tr("未命名 UDP 连接"));
  tabWidget_->registerWidget(
      // 有问题, 会影响主窗口的变化
      TtProtocolRole::MqttClient,
      [this]() {
        Window::MqttWindow *mqttClient =
            new Window::MqttWindow(TtProtocolType::Client);
        connect(mqttClient, &Window::MqttWindow::requestSaveConfig,
                [this, mqttClient]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(mqttClient);
                  addDifferentConfiguration(
                      TtFunctionalCategory::Communication,
                      TtProtocolRole::MqttClient, mqttClient->getTitle(),
                      // tabWidget_->getCurrentWidgetUUid());
                      uuid);
                  tabWidget_->setTabTitle(mqttClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      // "MqttClient+" + tabWidget_->getCurrentWidgetUUid(),
                      "MqttClient+" + uuid, mqttClient->getConfiguration());

                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000, this);
                });
        // bug
        qDebug() << "create";
        return mqttClient;
      },
      tr("未命名的 MQTT 客户端"));
  tabWidget_->registerWidget(
      TtProtocolRole::ModbusClient,
      [this]() {
        Window::ModbusWindow *modbusClient =
            new Window::ModbusWindow(TtProtocolType::Client);
        connect(modbusClient, &Window::ModbusWindow::requestSaveConfig,
                [this, modbusClient]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(modbusClient);
                  addDifferentConfiguration(TtFunctionalCategory::Communication,
                                            TtProtocolRole::ModbusClient,
                                            modbusClient->getTitle(), uuid);
                  tabWidget_->setTabTitle(modbusClient->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      "ModbusClient+" + uuid, modbusClient->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return modbusClient;
      },
      tr("未命名的 Modbus 主机"));
  tabWidget_->registerWidget(
      TtProtocolRole::TcpServer,
      [this]() {
        Window::TcpWindow *tcpServer =
            new Window::TcpWindow(TtProtocolType::Server);
        connect(tcpServer, &Window::TcpWindow::requestSaveConfig,
                [this, tcpServer]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(tcpServer);
                  addDifferentConfiguration(
                      TtFunctionalCategory::Simulate, TtProtocolRole::TcpServer,
                      tcpServer->getTitle(),
                      // tabWidget_->getCurrentWidgetUUid());
                      uuid);
                  tabWidget_->setTabTitle(uuid, tcpServer->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      // "TcpServer+" + tabWidget_->getCurrentWidgetUUid(),
                      "TcpServer+" + uuid, tcpServer->getConfiguration());
                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000);
                });
        return tcpServer;
      },
      tr("未命名的 TCP 服务模拟端"));

  // 注册 UDP 服务端窗口
  tabWidget_->registerWidget(
      TtProtocolRole::UdpServer,
      [this]() {
        Window::UdpWindow *udpServer =
            new Window::UdpWindow(TtProtocolType::Server);
        connect(udpServer, &Window::UdpWindow::requestSaveConfig,
                [this, udpServer]() {
                  const auto &uuid =
                      tabWidget_->getCurrentWidgetUUid(udpServer);
                  addDifferentConfiguration(
                      TtFunctionalCategory::Simulate, TtProtocolRole::UdpServer,
                      udpServer->getTitle(),
                      // tabWidget_->getCurrentWidgetUUid());
                      uuid);
                  tabWidget_->setTabTitle(uuid, udpServer->getTitle());
                  Storage::SettingsManager::instance().setSetting(
                      // "UdpServer+" + tabWidget_->getCurrentWidgetUUid(),
                      "UdpServer+" + uuid, udpServer->getConfiguration());

                  Ui::TtMessageBar::success(TtMessageBarType::Top, "",
                                            tr("保存成功"), 1000, this);
                });
        return udpServer;
      },
      tr("未命名的 UDP 服务模拟端"));
  // tabWidget_->registerWidget(
  //     TtProtocolRole::MqttBroker,
  //     [this]() {
  //       Window::MqttWindow* mqttServer =
  //           new Window::MqttWindow(TtProtocolType::Server);
  //       connect(
  //           mqttServer, &Window::MqttWindow::requestSaveConfig,
  //           [this, mqttServer]() {
  //             addDifferentConfiguration(TtFunctionalCategory::Simulate,
  //                                       mqttServer->getTitle(),
  //                                       tabWidget_->getCurrentWidgetUUid());
  //             tabWidget_->setTabTitle(mqttServer->getTitle());
  //             Storage::SettingsManager::instance().setSetting(
  //                 "MqttServer" + QString::number(tabWidget_->SpecialTypeNums(
  //                                    TtProtocolRole::MqttBroker)),
  //                 mqttServer->getConfiguration());
  //             Ui::TtMessageBar::success(TtMessageBarType::Top, "",
  //                                       tr("保存成功"), 1000, this);
  //           });
  //       return mqttServer;
  //     },
  //     tr("未命名的 MQTT 服务模拟端"));
}

void MainWindow::addDifferentConfiguration(TtFunctionalCategory::Category type,
                                           TtProtocolRole::Role role,
                                           const QString &title,
                                           const QString &uuid) {
  Ui::TtSpecialDeleteButton *button = new Ui::TtSpecialDeleteButton(
      title, ":/sys/displayport.svg", ":/sys/delete.svg", this);

  switch (type) {
  case TtFunctionalCategory::Communication: {
    history_link_list_->addAdaptiveWidget(title, std::make_pair(uuid, role),
                                          button);
    break;
  }
  case TtFunctionalCategory::Instruction: {
    break;
  }
  case TtFunctionalCategory::Simulate: {
    history_mock_list_->addAdaptiveWidget(title, std::make_pair(uuid, role),
                                          button);
    break;
  }
  default:
    break;
  }
  buttonGroup->addButton(uuid, static_cast<int>(role), button);
}

QString MainWindow::extractLanguageName(const QString &qmFile) {
  QMap<QString, QString> mapLanguage = {{"_zh", tr("简体中文")},
                                        {"_en", tr("英文")},
                                        {"_fr", tr("法语")},
                                        {"_de", tr("德语")},
                                        {"_ja", tr("日语")}};
  for (auto it = mapLanguage.begin(); it != mapLanguage.end(); ++it) {
    if (qmFile.contains(it.key())) {
      return it.value();
    }
  }
  return qmFile;
}

void MainWindow::changeLanguage(const QString &qmFile) {

  // 如果一直切换语言, 则以最后一次切换为准
  // 保存到配置文件
  saveLanguageSetting(qmFile);

  // Ui::TtContentDialog* dialog = new Ui::TtContentDialog(
  //     Ui::TtContentDialog::LayoutSelection::TWO_OPTIONS, this);
  // dialog->setLeftButtonText(tr("立马重启"));
  // dialog->setRightButtonText(tr("自己稍后重启"));
  // connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this, [this]() {
  //   // 2. 重启应用
  //   restartApplication();
  // });
  // connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this, [this]() {
  //   // 保持当前语言
  // });

  // dialog->show();

  Ui::TtContentDialog *dialog = new Ui::TtContentDialog(this);
  dialog->setLeftButtonText(tr("立马重启"));
  dialog->setRightButtonText(tr("稍后重启"));
  dialog->setCenterText(tr("已切换语言, 是否立马重启"));

  QPointer<Ui::TtContentDialog> dialogPtr(dialog);

  connect(dialog, &Ui::TtContentDialog::leftButtonClicked, this,
          [dialogPtr, this]() {
            if (dialogPtr) {
              dialogPtr->accept();
            }
            // 2. 重启应用
            restartApplication();
          });
  connect(dialog, &Ui::TtContentDialog::rightButtonClicked, this,
          [dialogPtr, this]() {
            if (dialogPtr) {
              dialogPtr->reject();
            }
          });

  dialog->exec();
  // 需要手动释放
  delete dialog;

  // if (translator_) {
  //   qApp->removeTranslator(translator_);
  //   delete translator_;
  // }
  // translator_ = new QTranslator(this);
  // // 安装的时候, 这些会放在哪里 ?
  // if (translator_->load("F:/MyProject/DebugTool/DebugTool/res/language/" +
  //                       qmFile)) {
  //   qApp->installTranslator(translator_);
  // }
}

void MainWindow::saveLanguageSetting(const QString &language) {
  // QSettings settings("MyCompany", "MyApp");
  // language 是 xxx.qm 前缀
  // QSettings settings(QCoreApplication::applicationDirPath() + "config.ini");
  // settings.setValue("Language", language);
  Storage::TtConfigsManager::instance().setConfigVaule("Language", language);
  qDebug() << "save: " << language;
}

void MainWindow::restartApplication() {
  // 获取当前应用的可执行文件路径
  QString program = QApplication::applicationFilePath();

  // 获取命令行参数（可选）
  QStringList arguments = QApplication::arguments();
  if (arguments.contains("--lang")) {
    // 移除旧的 "--lang" 参数（如果有）
    arguments.removeAll("--lang");
  }
  // 添加新的语言参数（例如 "--lang zh_CN"）
  arguments << "--lang"
            << savedLanguage_; // savedLanguage_ 是成员变量，保存当前语言

  // 启动新进程
  QProcess::startDetached(program, arguments);

  // 关闭当前应用
  QApplication::quit();
}

void MainWindow::readingProjectConfiguration() {
  // base::DetectRunningTime timer;
  // qDebug() << timer.elapseMilliseconds();
  const auto configs =
      Storage::SettingsManager::instance().getHistorySettings();
  // qDebug() << configs;

  // 创建三个哈希表来存储不同前缀的配置，以UUID为键
  QHash<QString, QJsonObject> serialConfigs;
  QHash<QString, QJsonObject> tcpClientConfigs;
  QHash<QString, QJsonObject> tcpServerConfigs;
  QHash<QString, QJsonObject> udpClientConfigs;
  QHash<QString, QJsonObject> udpServerConfigs;
  QHash<QString, QJsonObject> mqttConfigs;
  QHash<QString, QJsonObject> modbusConfigs;

  // 使用迭代器高效遍历
  for (auto it = configs.constBegin(); it != configs.constEnd(); ++it) {
    const QString &key = it.key();

    // 使用引用避免创建临时字符串
    if (key.startsWith(SerialPrefix)) {
      // uuid
      QString uuid = key.mid(SerialPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        // 配置项
        serialConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(TcpClientPrefix)) {
      QString uuid = key.mid(TcpClientPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        tcpClientConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(UdpClientPrefix)) {
      QString uuid = key.mid(UdpClientPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        udpClientConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(TcpServerPrefix)) {
      QString uuid = key.mid(TcpServerPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        tcpServerConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(UdpServerPrefix)) {
      QString uuid = key.mid(UdpServerPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        udpServerConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(MqttPrefix)) {
      QString uuid = key.mid(MqttPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        mqttConfigs[uuid] = it.value().toObject();
      }
    } else if (key.startsWith(ModbusPrefix)) {
      QString uuid = key.mid(ModbusPrefix.length());
      if (!uuid.isEmpty() && it.value().isObject()) {
        modbusConfigs[uuid] = it.value().toObject();
      }
    }
  }
  processConfigsByType(serialConfigs, TtProtocolRole::Serial);
  processConfigsByType(tcpClientConfigs, TtProtocolRole::TcpClient);
  processConfigsByType(tcpServerConfigs, TtProtocolRole::TcpServer);
  processConfigsByType(udpClientConfigs, TtProtocolRole::UdpClient);
  processConfigsByType(udpServerConfigs, TtProtocolRole::UdpServer);
  processConfigsByType(mqttConfigs, TtProtocolRole::MqttClient);
  processConfigsByType(modbusConfigs, TtProtocolRole::ModbusClient);
}

QJsonObject MainWindow::getSpecificConfiguration(const QString index,
                                                 TtProtocolRole::Role role) {
  const auto configs =
      Storage::SettingsManager::instance().getHistorySettings();
  auto prefix = configMappingTable.value(role);
  for (const QString &key : configs.keys()) {
    if (key.startsWith(prefix)) {
      QString uuid = key.sliced(prefix.length());
      if (uuid == index) {
        QJsonValue value = configs.value(key);
        if (value.isObject()) {
          return value.toObject();
        }
      }
    }
  }
  return QJsonObject();
}

void MainWindow::processConfigsByType(
    const QHash<QString, QJsonObject> &configs,
    TtProtocolRole::Role protocolRole) {

  // 不同类型的配置项
  // uuid, config
  for (auto it = configs.constBegin(); it != configs.constEnd(); ++it) {
    qDebug() << "uuid";
    const QString &uuid = it.key();      // uuid
    const QJsonObject &obj = it.value(); // obj

    // 分开不同的 leftbar
    // 获取功能分类
    QJsonValue typeValue = obj.value("Type");
    TtFunctionalCategory::Category category = TtFunctionalCategory::None;

    if (typeValue.isDouble()) {
      int value = typeValue.toInt(-1);
      if (value >= TtFunctionalCategory::Communication &&
          value <= TtFunctionalCategory::Simulate) {
        category = static_cast<TtFunctionalCategory::Category>(value);
      }
    }
    // 添加了对应的信息
    // 这里值解析了 WindowTitle 属性
    addDifferentConfiguration(category, protocolRole,
                              obj.value("WindowTitle").toString(), uuid);
  }
}

} // namespace Window
