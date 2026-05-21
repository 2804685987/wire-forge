#pragma once

#include "UI/Window/FluentWidget.h"
#include "QFluent/StackedWidget.h"
#include "QFluent/Navigation/NavigationPanel.h"
#include "QFluent/Navigation/NavigationWidget.h"

/**
 * @brief MainWindow 主窗口类
 * @details 主窗口类，负责显示和管理应用的主界面。
 */
class MainWindow : public FluentWidget {
Q_OBJECT
public:
    /**
     * @brief MainWindow::MainWindow 构造函数
     * @param parent 父窗口
     */
    explicit MainWindow(QWidget *parent = nullptr);  // 构造函数

    /**
     * @brief MainWindow::setCurrentInterface 设置当前界面
     * @param routeKey 路由键
     * @param index 索引
     * @note 索引从 1 开始
     */
    void setCurrentInterface(const QString &routeKey, int index);

private:
    /**
     * @brief MainWindow::m_stacked 主内容区域堆栈
     * @details 主内容区域堆栈，用于显示应用的主界面。
     * @note 主内容区域堆栈的子界面数量必须与导航栏的项数量相同
     * @note 主内容区域堆栈的子界面必须是 FluentWidget 类的实例
     */
    StackedWidget *m_stacked;
    /**
     * @brief MainWindow::m_navPanel 导航栏
     * @details 导航栏，用于显示应用的主界面。
     * @note 导航栏的项数量必须与主内容区域堆栈的子界面数量相同
     */
    NavigationPanel *m_navPanel;

    /**
     * @brief MainWindow::initWidget 初始化窗口组件
     */
    void initWidget();

    /**
     * @brief MainWindow::showDialog 显示对话框
     */
    void showDialog();

    /**
     * @brief MainWindow::addSubInterface 添加子界面
     * @param routeKey 路由键
     * @param icon 图标
     * @param text 文本
     * @param widget 子界面小部件
     * @param selectable 是否可选
     * @param position 导航栏项位置
     * @param tooltip 提示
     * @param parentRouteKey 父路由键
     * @note 父路由键为空时，子界面将作为根界面添加
     * @note 子界面的路由键必须是唯一的
     * @note 子界面的路由键不能与根界面的路由键相同
     * @note 子界面的路由键不能与已存在的子界面的路由键相同
     */
    void addSubInterface(const QString& routeKey, const QIcon& icon, const QString& text,
                         QWidget* widget, bool selectable = true,
                         NavigationPanel::ItemPosition position = NavigationPanel::ItemPosition::TOP,
                         const QString& tooltip = QString(), const QString& parentRouteKey = QString());

};

