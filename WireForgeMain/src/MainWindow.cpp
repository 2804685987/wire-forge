#include <iostream>
#include "MainWindow.h"
#include "Router.h"
#include "UI/Window/FluentTitleBar.h"
#include "QFluent/Dialog/MessageDialog.h"
#include "UI/Window/LoginWindow.h"
#include "UI/ProjectEditorWidget.h"

using FIT = Fluent::IconType;   // Fluent图标类型
using NIP = NavigationPanel::ItemPosition;  // 导航栏项位置类型


/**
 * @brief MainWindow::MainWindow 构造函数
 * @param parent 父窗口
 */
MainWindow::MainWindow(QWidget *parent) :
    FluentWidget(parent)
{
    setWindowTitle("WireForge V1.0.0-beta");
    setWindowIcon(QPixmap(":/res/LOGO.png"));
    resize(1024, 768);




    setWindowButtonHints(WindowButtonHint::WindowIcon | WindowButtonHint::Title |
                         WindowButtonHint::Minimize | WindowButtonHint::Maximize |
                         WindowButtonHint::Close | WindowButtonHint::ThemeToggle |
                         WindowButtonHint::StayOnTop);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_navPanel = new NavigationPanel(this);
    m_stacked = new StackedWidget(this);

    layout->addWidget(m_navPanel, 0);
    layout->addWidget(m_stacked, 1);


    setContentsMargins(0,45, 0, 0);


    m_navPanel->setExpandWidth(240);


    initWidget();
    qDebug() << "MainWindow::MainWindow";


    printf("Testing\n");

}

/**
 * @brief MainWindow::initWidget 初始化窗口组件
 */
void MainWindow::initWidget()
{
    auto userCard = m_navPanel->addUserCard("userCard", ":/res/Shizuka.png", "Shizuka", "shizuka@gmail.com",
                                            nullptr, NIP::TOP, false);
    userCard->setTitleFontSize(12);
    userCard->setSubtitleFontSize(10);

    // 注册项目编辑器到导航栏（项目管理模块入口）
    addSubInterface("1",Fluent::icon(FIT::PROJECTOR), "项目", new ProjectEditorWidget(this), true, NIP::TOP, "项目管理");


    [[maybe_unused]] const auto userCardConnection = connect(userCard, &NavigationUserCard::clicked, this, [=](){
        auto loginWindow = new LoginWidget(false);
        loginWindow->setWindowModality(Qt::ApplicationModal);
        loginWindow->setWindowEffect(this->windowEffect());
        loginWindow->show();
    });


    Router::instance()->setDefaultRouteKey(m_stacked, "ProjectEditorWidget");

    m_navPanel->setCurrentItem("1");
}


void MainWindow::showDialog()
{
    auto box = new MessageDialog("你是遇到问题了吗🧐",
                                 "遇到问题？欢迎加入 QQ 群（1084320682）反馈～看到后我会第一时间修复，感谢你让这个项目变得越来越棒！",
                                 this->window());
    box->setIsClosableOnMaskClicked(true);
    box->exec();
}



void MainWindow::addSubInterface(const QString& routeKey, const QIcon& icon, const QString& text,
                                 QWidget* widget, bool selectable,
                                 NavigationPanel::ItemPosition position, const QString& tooltip,
                                 const QString& parentRouteKey)
{
    m_navPanel->addItem(routeKey, icon, text, [=](){Router::instance()->push(m_stacked, widget->objectName());}, selectable, position, tooltip, parentRouteKey);
    m_stacked->addWidget(widget);
}


void MainWindow::setCurrentInterface(const QString &routeKey, int index)
{
    Router::instance()->push(m_stacked, routeKey);
    m_navPanel->setCurrentItem(QString::number(index));
}