#include "ProjectEditorWidget.h"
#include "../Graphics/BluePrintView.h"
#include "../Graphics/BluePrintScene.h"
#include "../UI/Attribute/PropertyPanel.h"
#include "../Graphics/ConnectorItem.h"
#include "../Graphics/WireItem.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QUndoStack>
#include <QTimer>
#include <QRandomGenerator>

ProjectEditorWidget::ProjectEditorWidget(QWidget* parent) : QWidget(parent)
{
    m_undoStack = new QUndoStack(this);
    setupUI();
    setupToolbar();
    setupConnections();

    // 测试：初始添加一个 Connector
    QTimer::singleShot(100, this, &ProjectEditorWidget::addTestComponent);
}

ProjectEditorWidget::~ProjectEditorWidget() = default;

void ProjectEditorWidget::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧：画布
    m_view = new BluePrintView(this);
    m_scene = new BluePrintScene(this);
    m_view->setScene(m_scene);

    m_mainSplitter->addWidget(m_view);

    // 右侧：属性面板
    m_rightSplitter = new QSplitter(Qt::Vertical);
    m_propertyPanel = new PropertyPanel(this);
    m_rightSplitter->addWidget(m_propertyPanel);

    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setStretchFactor(0, 7);   // 画布占大比例
    m_mainSplitter->setStretchFactor(1, 3);

    mainLayout->addWidget(m_mainSplitter);
}

void ProjectEditorWidget::setupToolbar()
{
    m_toolBar = new QToolBar("Tools", this);

    auto addConnectorAct = new QAction("添加连接器", this);
    addConnectorAct->setIcon(QIcon::fromTheme("list-add"));
    connect(addConnectorAct, &QAction::triggered, this, &ProjectEditorWidget::addTestComponent);
    m_toolBar->addAction(addConnectorAct);

    auto addWireAct = new QAction("添加测试线束", this);
    connect(addWireAct, &QAction::triggered, this, &ProjectEditorWidget::createTestWire);
    m_toolBar->addAction(addWireAct);

    m_toolBar->addSeparator();
    m_toolBar->addAction("Undo", m_undoStack, &QUndoStack::undo);
    m_toolBar->addAction("Redo", m_undoStack, &QUndoStack::redo);

    // 将工具栏加入主界面（可根据需要调整位置）
    // 如果你在 MainWindow 中使用，可单独取出 m_toolBar
}

void ProjectEditorWidget::setupConnections()
{
    connect(m_scene, &QGraphicsScene::selectionChanged,
            this, &ProjectEditorWidget::onSelectionChanged);
}

void ProjectEditorWidget::addTestComponent()
{
    auto connector = new ConnectorItem();
    connector->setPos(100 + QRandomGenerator::global()->bounded(400),
                      100 + QRandomGenerator::global()->bounded(300));
    m_scene->addComponent(connector);

    m_view->centerOn(connector);
}

void ProjectEditorWidget::createTestWire()
{
    auto items = m_scene->selectedComponents();
    if (items.size() < 2) {
        QMessageBox::information(this, "提示", "请先选中两个 Connector 来测试连线");
        return;
    }

    auto c1 = qobject_cast<ConnectorItem*>(items[0]);
    auto c2 = qobject_cast<ConnectorItem*>(items[1]);

    if (c1 && c2 && !c1->ports().isEmpty() && !c2->ports().isEmpty()) {
        auto wire = m_scene->connectPorts(c1->ports().first(), c2->ports().first());
        if (wire) {
            QMessageBox::information(this, "成功", "测试线束已创建！");
        }
    }
}

void ProjectEditorWidget::onSelectionChanged()
{
    auto selected = m_scene->selectedComponents();
    if (!selected.isEmpty()) {
        m_propertyPanel->setSelectedComponent(selected.first());
    } else {
        m_propertyPanel->clear();
    }
}

BluePrintScene* ProjectEditorWidget::scene() const { return m_scene; }
BluePrintView* ProjectEditorWidget::view() const { return m_view; }