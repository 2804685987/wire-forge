#include "ProjectEditorWidget.h"

#include "Controller/ProjectController.h"
#include "Model/ProjectModel.h"
#include "Model/Components/ConnectorModel.h"
#include "View/CanvasView/ProjectCanvasWidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSet>
#include <QKeyEvent>
#include <QtGlobal>
#include <cmath>
#include <algorithm>
#include <QPainterPath>



ProjectEditorWidget::ProjectEditorWidget(QWidget *parent, ProjectController *controller)
    : QWidget(parent), m_controller(controller)
{
    if (!m_controller) {
        m_controller = new ProjectController(this);
    }

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto *toolbar = new QHBoxLayout();
    auto *btnNew = new QPushButton(tr("新建项目"), this);
    auto *btnAddNode = new QPushButton(tr("添加节点"), this);
    auto *btnAddWire = new QPushButton(tr("连接最近两个节点"), this);
    m_undoButton = new QPushButton(tr("撤销"), this);
    m_redoButton = new QPushButton(tr("重做"), this);
    m_allowInputInput = new QCheckBox(tr("允许 输入-输入"), this);
    m_allowOutputOutput = new QCheckBox(tr("允许 输出-输出"), this);
    m_allowInputToOutput = new QCheckBox(tr("允许 输入-输出"), this);
    m_allowSameNode = new QCheckBox(tr("允许 同节点"), this);
    toolbar->addWidget(btnNew);
    toolbar->addWidget(btnAddNode);
    toolbar->addWidget(btnAddWire);
    toolbar->addWidget(m_allowInputInput);
    toolbar->addWidget(m_allowOutputOutput);
    toolbar->addWidget(m_allowInputToOutput);
    toolbar->addWidget(m_allowSameNode);
    toolbar->addStretch(1);
    toolbar->addWidget(m_undoButton);
    toolbar->addWidget(m_redoButton);
    rootLayout->addLayout(toolbar);

    m_canvas = new ProjectCanvasWidget(this);
    m_canvas->setController(m_controller);
    m_canvas->setStatusCallback([this](const QString &text) { setStatus(text); });
    rootLayout->addWidget(m_canvas, 1);

    m_statusLabel = new QLabel(tr("就绪"), this);
    rootLayout->addWidget(m_statusLabel);

    connect(btnNew, &QPushButton::clicked, this, [this]() {
        if (m_controller->createProject(QString())) {
            setStatus(tr("已创建新项目"));
            setModel(m_controller->project());
        }
        updateUndoRedoButtons();
    });

    connect(btnAddNode, &QPushButton::clicked, this, [this]() {
        if (!m_controller->project()) {
            m_controller->createProject(QString());
            setModel(m_controller->project());
        }
        const QUuid id = m_controller->addNode(nextNodePosition());
        setStatus(id.isNull() ? tr("添加节点失败") : tr("已添加节点 %1").arg(id.toString(QUuid::WithoutBraces).left(8)));
        updateUndoRedoButtons();
    });

    connect(btnAddWire, &QPushButton::clicked, this, [this]() {
        auto *model = m_controller->project();
        if (!model) {
            setStatus(tr("请先创建项目"));
            return;
        }
        const auto nodes = model->nodes();
        if (nodes.size() < 2) {
            setStatus(tr("至少需要两个节点"));
            return;
        }
        NodeModel *toNode = nodes.last();
        NodeModel *fromNode = nodes.at(nodes.size() - 2);

        ConnectorModel *fromConnector = nullptr;
        ConnectorModel *toConnector = nullptr;
        for (auto *c : fromNode->connectors()) {
            if (c && c->direction() == ConnectorModel::Output) {
                fromConnector = c;
                break;
            }
        }
        for (auto *c : toNode->connectors()) {
            if (c && c->direction() == ConnectorModel::Input) {
                toConnector = c;
                break;
            }
        }
        if (!fromConnector || !toConnector) {
            setStatus(tr("节点缺少可用端口"));
            return;
        }
        const QUuid wireId = m_controller->addWire(fromConnector->id(), toConnector->id());
        setStatus(wireId.isNull() ? tr("创建连线失败") : tr("已创建连线 %1").arg(wireId.toString(QUuid::WithoutBraces).left(8)));
        updateUndoRedoButtons();
    });

    connect(m_undoButton, &QPushButton::clicked, this, [this]() {
        m_controller->undo();
        updateUndoRedoButtons();
        setStatus(tr("已撤销"));
    });

    connect(m_redoButton, &QPushButton::clicked, this, [this]() {
        m_controller->redo();
        updateUndoRedoButtons();
        setStatus(tr("已重做"));
    });

    connect(m_controller, &ProjectController::projectOpened, this, [this]() {
        setModel(m_controller->project());
        updateUndoRedoButtons();
        applyWireRules();
    });

    connect(m_allowInputInput, &QCheckBox::toggled, this, [this]() { applyWireRules(); });
    connect(m_allowOutputOutput, &QCheckBox::toggled, this, [this]() { applyWireRules(); });
    connect(m_allowInputToOutput, &QCheckBox::toggled, this, [this]() { applyWireRules(); });
    connect(m_allowSameNode, &QCheckBox::toggled, this, [this]() { applyWireRules(); });

    if (!m_controller->project()) {
        m_controller->createProject(QString());
    }
    setModel(m_controller->project());
    updateUndoRedoButtons();
    applyWireRules();
}

void ProjectEditorWidget::setModel(ProjectModel *model)
{
    if (m_canvas) {
        m_canvas->setModel(model);
    }
}

void ProjectEditorWidget::setStatus(const QString &text)
{
    if (m_statusLabel) {
        m_statusLabel->setText(text);
    }
}

QPointF ProjectEditorWidget::nextNodePosition() const
{
    if (!m_controller || !m_controller->project()) return {40.0, 40.0};
    const int index = static_cast<int>(m_controller->project()->nodes().size());
    const int cols = 4;
    const int row = index / cols;
    const int col = index % cols;
    return {40.0 + col * 180.0, 40.0 + row * 120.0};
}

void ProjectEditorWidget::updateUndoRedoButtons()
{
    if (m_undoButton) m_undoButton->setEnabled(m_controller && m_controller->canUndo());
    if (m_redoButton) m_redoButton->setEnabled(m_controller && m_controller->canRedo());
}

void ProjectEditorWidget::applyWireRules()
{
    if (!m_controller || !m_controller->project()) return;
    auto rules = m_controller->wireValidationRules();
    rules.allowInputToInput = m_allowInputInput && m_allowInputInput->isChecked();
    rules.allowOutputToOutput = m_allowOutputOutput && m_allowOutputOutput->isChecked();
    rules.allowInputToOutput = m_allowInputToOutput && m_allowInputToOutput->isChecked();
    rules.allowSameNode = m_allowSameNode && m_allowSameNode->isChecked();
    m_controller->setWireValidationRules(rules);
}
