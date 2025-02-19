#include "window/main_window.h"

#include <QActionGroup>
#include <QApplication>
#include <QCoreApplication>
#include <QDockWidget>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QPushbutton>
#include <QStackedLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QStyle>

#include <QWKWidgets/widgetwindowagent.h>
#include <Qt-Advanced-Stylesheets/src/QtAdvancedStylesheet.h>
#include <qtmaterialsnackbar.h>

#include <ui/window/title/window_button.h>
#include <ui/window/title/windowbar.h>

#include "core/serial_port.h"

#include "ui/widgets/pop_widget.h"

#include "ui/widgets/customizationtabwidget.h"

#include "window/popup_window.h"

#include <ui/layout/vertical_layout.h>
#include <ui/widgets/buttons.h>

#include "ui/control/TtPopUpDrawer.h"
#include "window/function_selection_window.h"

#include "ui/widgets/labels.h"
#include "ui/widgets/snack_bar.h"

#include "window/serial_window.h"
#include "window/tcp_window.h"
#include "window/udp_window.h"

#include "ui/window/window_switcher.h"

#include <ui/effects/animated_drawer.h>
#include <ui/widgets/widget_group.h>

namespace Window {

static inline void emulateLeaveEvent(QWidget* widget) {
  Q_ASSERT(widget);
  if (!widget) {
    return;
  }
  QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QScreen* screen = widget->screen();
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), central_widget_(new QWidget(this)) {
  window_agent_ = new QWK::WidgetWindowAgent(this);
  //installWindowAgent();
  setMinimumWidth(1024);
  setMinimumHeight(720);
  //window_agent_->centralize();

  // this->showMaximized();

  //this->setWindowFlag(Qt::WindowStaysOnTopHint);

  layout_ = new Ui::TtHorizontalLayout;
  central_widget_->setLayout(layout_);
  setCentralWidget(central_widget_);

  //setAttribute(Qt::WA_DontCreateNativeAncestors);
  // 拖拽右边时, 会出现宽度影响高度的情况

  setWindowTitle(tr("TtSerialPort"));

  loadStyleSheet(Theme::Dark);

  setLeftBar();

  function_select_ = new FunctionSelectionWindow();

  // 初始界面是 test111
  tabWidget_ = new Ui::TabManager(function_select_, this);

  // 容纳一定了 widget, 以供外部切换, 直接存放 widget, 而非设定
  auto test = new Window::PopUpWindow();

  QSplitter* mainSplitter = new QSplitter(this);
  //popUpBodyWidget->setParent(mainSpliter);
  //QWidget* testW = new QWidget(mainSplitter);
  //testW->setStyleSheet("background-color: lightblue;");  // Drawer 的背景色
  //mainSplitter->addWidget(testW);
  mainSplitter->addWidget(test);
  //Ui::TtPopUpDrawer* drawer = new Ui::TtPopUpDrawer(mainSpliter);
  //mainSpliter->addWidget(communication_connection_widget);
  mainSplitter->addWidget(tabWidget_);

  mainSplitter->setCollapsible(1, false);
  // 创建 AnimatedDrawer 控制器
  Ui::TtAnimatedDrawer* controller =
      //new Ui::TtAnimatedDrawer(mainSplitter, testW, tabWidget_, this);
      new Ui::TtAnimatedDrawer(mainSplitter, test, tabWidget_, this);

  {

    QWidget* linkList = new QWidget;
    auto linkListLayout = new Ui::TtVerticalLayout(linkList);

    // 顶部
    QWidget* titleWidget = new QWidget(linkList);
    //titleWidget
    //titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //QWidget* titleWidget = new QWidget();
    //titleWidget->setMinimumHeight(40);
    //titleWidget->setMaximumHeight(40);
    Ui::TtNormalLabel* title = new Ui::TtNormalLabel(tr("连接列表"), titleWidget);
    //Ui::TtSvgButton* add =
    //    new Ui::TtSvgButton(":/sys/plus-circle.svg", "", titleWidget);  // 刷新
    Ui::TtImageButton* add =
        new Ui::TtImageButton(":/sys/plus-circle.svg", titleWidget);  // 刷新
    Ui::TtSvgButton* refresh =
        new Ui::TtSvgButton(":/sys/refresh-normal.svg", ":/sys/refresh-hover.svg", titleWidget);  // 新建
    refresh->setEnableToggle(false);
    Ui::TtHorizontalLayout* titleLayout = new Ui::TtHorizontalLayout(titleWidget);
    titleLayout->addWidget(title);
    titleLayout->addStretch();
    titleLayout->addWidget(add);
    titleLayout->addSpacerItem(new QSpacerItem(10, 0));
    titleLayout->addWidget(refresh);

    linkListLayout->addWidget(titleWidget, 0, Qt::AlignTop);


    // 首页界面
    // 显示的图片
    Ui::TtNormalLabel* displayInfo = new Ui::TtNormalLabel();
    displayInfo->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(100, 100)));
    //linkListLayout->addStretch();
    //linkListLayout->addWidget(displayInfo, 0, Qt::AlignCenter);

    // 图片底下的文字
    Ui::TtNormalLabel* displayWords = new Ui::TtNormalLabel(tr("暂时没有可用的连接"));
    //il->addWidget(displayWords, 0, Qt::AlignCenter);
    // 点击后在 tab 弹出一个 新 tab, 用于设置
    auto addLinkBtn = new QPushButton(tr("新建连接"));
    addLinkBtn->setFixedSize(100, 28);


    // 原始和切换逻辑
    // 创建原始界面
    auto original_widget_ = new QWidget();
    //original_widget_->setStyleSheet("background-color : Coral");
    Ui::TtVerticalLayout* tmpl = new Ui::TtVerticalLayout(original_widget_);
    //tmpl->addSpacerItem(new QSpacerItem(10, 10));
    tmpl->addStretch();
    tmpl->addWidget(displayInfo, 0, Qt::AlignHCenter);
    tmpl->addSpacerItem(new QSpacerItem(0, 20));
    tmpl->addWidget(displayWords, 0, Qt::AlignHCenter);
    tmpl->addWidget(addLinkBtn, 0, Qt::AlignHCenter);
    tmpl->addStretch();


    // 存储历史保存设置
    auto historyLinkList = new QWidget();
    historyLinkList->setStyleSheet("background-color: Coral");
    

    // 使用堆叠布局
    auto stack_ = new QStackedWidget(linkList);
    //stack_->setFixedHeight(40);
    stack_->addWidget(original_widget_);
    stack_->addWidget(historyLinkList);
    
    linkListLayout->addWidget(stack_, 1);

    // 新增的弹出框
    connect(addLinkBtn, &QPushButton::clicked, [this, controller]() {
      qDebug() << "add new tab";
      FunctionSelectionWindow* test2 = new FunctionSelectionWindow();
      tabWidget_->addNewTab(test2, tr("新建连接"));
      // 连接信号槽
      QObject::connect(test2, &FunctionSelectionWindow::switchRequested,
                       [this](int widget_id) {
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), widget_id);
                       });
      // 切换到当前新增 tab
      tabWidget_->setCurrentWidget(test2);
    });


    // 注册 0 号窗口
    test->addPairedWidget(0, linkList);

    //// 优化后的信号连接（仅需2个连接点）
    //connect(modify_title_btn_, &Ui::TtImageButton::clicked, this,
    //        &SerialWindow::switchToEditMode);



  }

  {
    QWidget* insidetest = new QWidget;
    auto il = new Ui::TtVerticalLayout(insidetest);

    Ui::TtNormalLabel* imla = new Ui::TtNormalLabel();
    imla->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(100, 100)));
    il->addStretch();
    il->addWidget(imla, 0, Qt::AlignCenter);

    Ui::TtNormalLabel* la_ = new Ui::TtNormalLabel(tr("指令列表为空"));
    il->addWidget(la_, 0, Qt::AlignCenter);
    // 点击后在 tab 弹出一个 新 tab, 用于设置
    auto ib = new QPushButton(tr("新建指令"));
    ib->setFixedSize(100, 28);
    il->addSpacerItem(new QSpacerItem(0, 20));
    il->addWidget(ib, 0, Qt::AlignCenter);
    il->addStretch();

    test->addPairedWidget(1, insidetest);

    // 新增的弹出框
    connect(ib, &QPushButton::clicked, [this, controller]() {
      //qDebug() << "add new tab";
      FunctionSelectionWindow* test2 = new FunctionSelectionWindow();

      tabWidget_->addNewTab(test2, tr("新建连接"));
      // 连接信号槽
      QObject::connect(test2, &FunctionSelectionWindow::switchRequested,
                       [this](int widget_id) {
                         //auto cuI = ;
                         tabWidget_->handleButtonClicked(
                             tabWidget_->currentIndex(), widget_id);
                       });
      // 切换到当前新增 tab
      tabWidget_->setCurrentWidget(test2);
      //controller->closeDrawer();
    });
  }

  {
    QWidget* insidetest = new QWidget;
    auto il = new Ui::TtVerticalLayout(insidetest);

    Ui::TtNormalLabel* imla = new Ui::TtNormalLabel();
    imla->setPixmap(
        QPixmap::fromImage(QImage(":/sys/tmp.png").scaled(100, 100)));
    il->addStretch();
    il->addWidget(imla, 0, Qt::AlignCenter);

    Ui::TtNormalLabel* la_ = new Ui::TtNormalLabel(tr("没有可用的模拟服务"));
    il->addWidget(la_, 0, Qt::AlignCenter);
    // 点击后在 tab 弹出一个 新 tab, 用于设置
    //auto ib = new QPushButton(tr("新建指令"));
    //ib->setFixedSize(100, 28);
    //il->addSpacerItem(new QSpacerItem(0, 20));
    //il->addWidget(ib, 0, Qt::AlignCenter);
    //il->addStretch();

    test->addPairedWidget(2, insidetest);

    //// 新增的弹出框
    //connect(ib, &QPushButton::clicked, [this, controller]() {
    //  qDebug() << "add new tab";
    //  FunctionSelectionWindow* test2 = new FunctionSelectionWindow();
    //  tabWidget_->addNewTab(test2, tr("新建连接"));
    //  // 连接信号槽
    //  QObject::connect(test2, &FunctionSelectionWindow::switchRequested,
    //                   [this](int widget_id) {
    //                     tabWidget_->handleButtonClicked(widget_id);
    //                   });
    //  // 切换到当前新增 tab
    //  tabWidget_->setCurrentWidget(test2);
    //  //controller->closeDrawer();
    //});
  }

  registerTabWidget();

  // 左边栏
  layout_->addWidget(left_bar_, 0, Qt::AlignLeft);

  layout_->addWidget(mainSplitter);

  connect(left_bar_logic_, &Ui::TtWidgetGroup::widgetClicked,
          [this, test, controller](int index) {
            //qDebug() << index;
            // 打开
            if (controller->isDrawerVisible()) {
              // 重复
              if (!test->switchToWidget(index)) {
                //qDebug() << "close";
                controller->closeDrawer();
              }
            } else {
              //qDebug() << "open";
              test->switchToWidget(index);
              controller->openDrawer();
            }
          });

  //connect(communication_connection, &Ui::TtSvgButton::clicked,
  //        [this, test, mainSplitter, controller]() {
  //          controller->toggleDrawer();
  //          //Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //        });

  connectSignals();
}

MainWindow::~MainWindow() {}

void MainWindow::showSnackbar() {
  // snack_bar_->setWindowFlags(snack_bar_->windowFlags() | Qt::WindowStaysOnTopHint);
  // setAttribute(Qt::WA_TranslucentBackground);
  // snack_bar_->addInstantMessage(QString("open serial port!"));
  // snack_bar_->addMessage(QString("open serial port!"));
}

bool MainWindow::event(QEvent* event) {
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

void MainWindow::installWindowAgent() {
  window_agent_->setup(this);

  // 2. Construct your title bar
  auto menuBar = [this]() {
    auto menuBar = new QMenuBar(this);

    // Virtual menu
    auto file = new QMenu(tr("File(&F)"), menuBar);
    file->addAction(new QAction(tr("New(&N)"), menuBar));
    file->addAction(new QAction(tr("Open(&O)"), menuBar));
    file->addSeparator();

    auto edit = new QMenu(tr("Edit(&E)"), menuBar);
    edit->addAction(new QAction(tr("Undo(&U)"), menuBar));
    edit->addAction(new QAction(tr("Redo(&R)"), menuBar));

    // Theme action
    // auto darkAction = new QAction(tr("Enable dark theme"), menuBar);
    // darkAction->setCheckable(true);

    // connect(darkAction, &QAction::triggered, this, [this](bool checked) {
    //     loadStyleSheet(checked ? Dark : Light); //
    // });
    // connect(this, &MainWindow::themeChanged, darkAction, [this, darkAction]() {
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
    // connect(winStyleGroup, &QActionGroup::triggered, this, [this, winStyleGroup](QAction *action) {
    //     // Unset all custom style attributes first, otherwise the style will not display correctly
    //     for (const QAction* _act : winStyleGroup->actions()) {
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
    menuBar->addMenu(edit);
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

#ifdef Q_OS_MAC
  windowAgent->setSystemButtonAreaCallback([](const QSize& size) {
    static constexpr const int width = 75;
    return QRect(QPoint(size.width() - width, 0),
                 QSize(width, size.height()));  //
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

  connect(windowBar, &Ui::WindowBar::minimizeRequested, this,
          &QWidget::showMinimized);
  connect(
      windowBar, &Ui::WindowBar::maximizeRequested, this,
      [this, maxButton](bool max) {
        if (max) {
          showMaximized();
        } else {
          showNormal();
        }

        // It's a Qt issue that if a QAbstractButton::clicked triggers a window's maximization,
        // the button remains to be hovered until the mouse move. As a result, we need to
        // manually send leave events to the button.
        emulateLeaveEvent(maxButton);
      });
  connect(windowBar, &Ui::WindowBar::closeRequested, this, &QWidget::close);
#endif
}

void MainWindow::loadStyleSheet(Theme theme) {

  //qDebug() << "yes";

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

  // now you can set the generated stylesheet
  qApp->setStyleSheet(AdvancedStylesheet.styleSheet());
  //     if (!styleSheet().isEmpty() && theme == currentTheme)
  //         return;
  //     currentTheme = theme;

  //     if (QFile qss(theme == Dark ? QStringLiteral(":/dark-style.qss")
  //                                 : QStringLiteral(":/light-style.qss"));
  //         qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
  //         setStyleSheet(QString::fromUtf8(qss.readAll()));
  // Q_EMIT themeChanged();
  // emit themeChanged();
  //     }
}

void MainWindow::setLeftBar() {
  left_bar_ = new QWidget(this);
  left_bar_logic_ = new Ui::TtWidgetGroup(this);
  left_bar_logic_->setExclusive(true);
  communication_connection = new Ui::TtSvgButton(
      ":/sys/communicating-junctions.svg",
      ":/sys/communicating-junctions_pressed.svg", left_bar_);
  communication_connection->setFixedSize(46, 46);

  communication_instruction = new Ui::TtSvgButton(
      ":/sys/Instruction-configuration.svg",
      ":/sys/Instruction-configuration_pressed.svg", left_bar_);
  communication_instruction->setFixedSize(46, 46);

  realistic_simulation =
      new Ui::TtSvgButton(":/sys/real-time-simulation.svg",
                          ":/sys/real-time-simulation_pressed.svg", left_bar_);
  realistic_simulation->setFixedSize(46, 46);

  communication_connection->setStyleSheet("padding: 5px 10px;");
  communication_instruction->setStyleSheet("padding: 5px 10px;");

  left_bar_logic_->addWidget(communication_connection);
  left_bar_logic_->addWidget(communication_instruction);
  left_bar_logic_->addWidget(realistic_simulation);

  Ui::TtVerticalLayout* left_bar_layout = new Ui::TtVerticalLayout();

  // 添加按钮
  left_bar_layout->addWidget(communication_connection, 0, Qt::AlignTop);
  left_bar_layout->addWidget(communication_instruction);
  left_bar_layout->addWidget(realistic_simulation);
  left_bar_layout->addStretch();

  //left_bar_->addWidget();
  left_bar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  left_bar_->setLayout(left_bar_layout);
  left_bar_->setMinimumWidth(40);

  // 显示 2 次, 出现异常
  //Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //for (int i = 0; i < 100; ++i) {
  //  QTimer::singleShot(i * 100, [] {
  //    Ui::SnackBarController::instance()->showMessage("测试消息");
  //  });
  //}
}

void MainWindow::connectSignals() {
  //connect(communication_connection, &Ui::TtSvgButton::clicked, [this]() {
  //  //communication_connection_widget->triggerSwitch();
  //

  //  Ui::SnackBarController::instance()->showMessage("这是一个测试消息", 2000);
  //});
  // 处理点击 close Pop 时, 按钮恢复, 是否需要切换到新的 tab
  //connect(communication_connection_widget, &Ui::PopWidget::isClosed, [this]() {
  //  communication_connection->setState(true);
  //  qDebug() << "also close";
  //});

  // 点击按钮信号连接
  QObject::connect(
      function_select_, &FunctionSelectionWindow::switchRequested,
      // [this](int widget_id) { tabWidget_->switchToWidget(widget_id); });
      [this](int widget_id) {
        tabWidget_->handleButtonClicked(tabWidget_->currentIndex(), widget_id);
      });
}

void MainWindow::registerTabWidget() {
  // 注册 串口窗口
  tabWidget_->registerWidget(
      0,
      []() {
        // base::DetectRunningTime runtime;
        Window::SerialWindow* d = new Window::SerialWindow();
        // d->setStyleSheet("background-color: Coral");
        // qDebug() << runtime.elapseMilliseconds();
        auto widget = new QWidget();
        // QVBoxLayout* layout = new QVBoxLayout();
        Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout();
        widget->setLayout(layout);
        layout->addWidget(d);
        return widget;
      },
      tr("未命名的串口连接"));
  // 注册 TCP客户端窗口
  tabWidget_->registerWidget(
      1,
      []() {
        Window::TcpWindow* d = new Window::TcpWindow();
        auto widget = new QWidget();
        Ui::TtVerticalLayout* layout = new Ui::TtVerticalLayout(widget);
        layout->addWidget(d);
        return widget;
      },
      tr("未命 TCP 连接"));

  //// 注册 UDP客户端窗口
  //tabWidget_->registerWidget(
  //    2,
  //    []() {
  //      Window::UdpWindow* d = new Window::UdpWindow();
  //      auto widget = new QWidget();
  //      Ui::VerticalLayout* layout = new Ui::VerticalLayout(widget);
  //      layout->addWidget(d);
  //      return widget;
  //    },
  //    []() { return QString(tr("未命名 UDP 连接")); });

  // // 注册 Modbus客户端窗口
  // tabWidget_->registerWidget(4, []() {
  //   Window::SerialWindow *d = new Window::SerialWindow();
  //   auto widget = new QWidget();
  //   QVBoxLayout* layout = new QVBoxLayout(widget);
  //   layout->addWidget(d);
  //   return widget;
  // }, []() {
  //   return QString(tr("未命名串口连接"));
  // });

  // // 注册 MQTT客户端窗口
  // tabWidget_->registerWidget(5, []() {
  //   Window::SerialWindow *d = new Window::SerialWindow();
  //   auto widget = new QWidget();
  //   QVBoxLayout* layout = new QVBoxLayout(widget);
  //   layout->addWidget(d);
  //   return widget;
  // }, []() {
  //   return QString(tr("未命名串口连接"));
  // });

  // tabWidget_->registerWidget(0, []() {
  //   auto widget = new QWidget();
  //   QVBoxLayout* layout = new QVBoxLayout(widget);
  //   layout->addWidget(new QLabel("This is Widget2"));
  //   return widget;
  // }, []() {
  //   return QString("Second Tab");

  // });
}

// void MainWindow::createDockWindows()
// {
// // QMenuBar
// // setMenuBar(new QMenuBar(this));
// // window
// QMenu *viewMenu;
// // viewMenu = menuBar()->addMenu("view");
// viewMenu = qobject_cast<Ui::WindowBar*>(window_agent_->titleBar())->menuBar()->addMenu("view");

// QDockWidget *dock = new QDockWidget(tr("Customers"), this);
// dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
// QListWidget *customerList;
// customerList = new QListWidget(dock);
// customerList->addItems(QStringList()
//                        << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
//                        << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
//                        << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
//                        << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
//                        << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
//                        << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");
// dock->setWidget(customerList);
// addDockWidget(Qt::RightDockWidgetArea, dock);

// viewMenu->addAction(dock->toggleViewAction());

// QListWidget *paragraphsList;
// dock = new QDockWidget(tr("Paragraphs"), this);
// paragraphsList = new QListWidget(dock);
// paragraphsList->addItems(QStringList()
//                          << "Thank you for your payment which we have received today."
//                          << "Your order has been dispatched and should be with you "
//                             "within 28 days."
//                          << "We have dispatched those items that were in stock. The "
//                             "rest of your order will be dispatched once all the "
//                             "remaining items have arrived at our warehouse. No "
//                             "additional shipping charges will be made."
//                          << "You made a small overpayment (less than $5) which we "
//                             "will keep on account for you, or return at your request."
//                          << "You made a small underpayment (less than $1), but we have "
//                             "sent your order anyway. We'll add this underpayment to "
//                             "your next bill."
//                          << "Unfortunately you did not send enough money. Please remit "
//                             "an additional $. Your order will be dispatched as soon as "
//                             "the complete amount has been received."
//                          << "You made an overpayment (more than $5). Do you wish to "
//                             "buy more items, or should we return the excess to you?");
// dock->setWidget(paragraphsList);
// addDockWidget(Qt::RightDockWidgetArea, dock);
// viewMenu->addAction(dock->toggleViewAction());
// }

}  // namespace Window
